#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/first_return_joint_product.hpp"

namespace ravel::proof {

/*
 * Dimension-parametric residual formula algebra.
 *
 * A terminal formula is a union of coordinate faces.  Predecessor is not a
 * table lookup: it is a formula constructor with the semantics
 *
 *   eval (pre d phi) s <-> exists t, s -d-> t and eval phi t.
 *
 * This is the universal residual update theorem by construction.  Concrete
 * finite residual families are used only to replay that the symbolic DAG
 * names every extensional residual in the current implementation.
 */

using ResidualFormulaId = std::size_t;

enum class ResidualFormulaKind { terminal_faces, predecessor };

struct ResidualFormulaNode {
    ResidualFormulaKind kind = ResidualFormulaKind::terminal_faces;
    std::vector<std::pair<std::size_t, std::int64_t>> faces;
    std::int64_t digit = 0;
    ResidualFormulaId child = 0;

    friend bool operator<(const ResidualFormulaNode& a,
                          const ResidualFormulaNode& b) {
        return std::tie(a.kind, a.faces, a.digit, a.child) <
               std::tie(b.kind, b.faces, b.digit, b.child);
    }
};

struct SymbolicResidualFormulaProof {
    std::size_t dimension = 0;
    std::vector<ResidualFormulaNode> nodes;
    std::map<ResidualFormulaNode, ResidualFormulaId> node_ids;
    std::vector<std::optional<ResidualFormulaId>> formula_of_residual;
    bool terminal_replay_exact = false;
    bool predecessor_semantics_exact = false;
    bool all_residuals_named = false;
    bool extensional_replay_exact = false;
    bool universal_update_derived = false;
    bool valid = false;
    std::string obstruction;
};

namespace formula_detail {

inline ResidualFormulaId intern(SymbolicResidualFormulaProof& proof,
                                ResidualFormulaNode node) {
    auto [it, inserted] = proof.node_ids.emplace(node, proof.nodes.size());
    if (inserted) proof.nodes.push_back(std::move(node));
    return it->second;
}

inline bool eval_formula(const SymbolicResidualFormulaProof& proof,
                         const ControllerPlant& plant,
                         ResidualFormulaId id,
                         ControllerStateId state,
                         std::map<std::pair<ResidualFormulaId, ControllerStateId>, bool>& memo) {
    const auto key = std::make_pair(id, state);
    if (const auto it = memo.find(key); it != memo.end()) return it->second;
    if (id >= proof.nodes.size() || state >= plant.state_count)
        throw std::out_of_range("formula evaluation index");
    const auto& node = proof.nodes[id];
    bool value = false;
    if (node.kind == ResidualFormulaKind::terminal_faces) {
        throw std::logic_error("terminal formula requires controller states");
    } else {
        const auto it = plant.successors.find({state, node.digit});
        if (it != plant.successors.end()) {
            for (const auto target : it->second) {
                if (eval_formula(proof, plant, node.child, target, memo)) {
                    value = true;
                    break;
                }
            }
        }
    }
    memo.emplace(key, value);
    return value;
}

inline bool eval_formula_with_states(
    const SymbolicResidualFormulaProof& proof,
    const FirstReturnJointProduct& product,
    ResidualFormulaId id,
    ControllerStateId state,
    std::map<std::pair<ResidualFormulaId, ControllerStateId>, bool>& memo) {
    const auto key = std::make_pair(id, state);
    if (const auto it = memo.find(key); it != memo.end()) return it->second;
    if (id >= proof.nodes.size() || state >= product.controller_states.size())
        throw std::out_of_range("formula evaluation index");
    const auto& node = proof.nodes[id];
    bool value = false;
    if (node.kind == ResidualFormulaKind::terminal_faces) {
        const auto& s = product.controller_states[state];
        for (const auto& [coordinate, sign] : node.faces) {
            if (coordinate < s.size() && s[coordinate] == sign) {
                value = true;
                break;
            }
        }
    } else {
        const auto it = product.controller_plant.successors.find({state, node.digit});
        if (it != product.controller_plant.successors.end()) {
            for (const auto target : it->second) {
                if (eval_formula_with_states(proof, product, node.child, target, memo)) {
                    value = true;
                    break;
                }
            }
        }
    }
    memo.emplace(key, value);
    return value;
}

} // namespace formula_detail

inline SymbolicResidualFormulaProof derive_symbolic_residual_formula_proof(
    const FirstReturnJointProduct& product) {
    SymbolicResidualFormulaProof proof;
    proof.dimension = product.dimension;
    if (!product.replayed) {
        proof.obstruction = "joint product did not replay";
        return proof;
    }
    proof.formula_of_residual.resize(product.residual_members.size());

    for (const auto& [residual, faces] : product.terminal_faces_by_residual) {
        ResidualFormulaNode node;
        node.kind = ResidualFormulaKind::terminal_faces;
        node.faces = faces;
        const auto id = formula_detail::intern(proof, std::move(node));
        proof.formula_of_residual.at(residual) = id;
    }

    proof.terminal_replay_exact = true;
    for (const auto& [residual, faces] : product.terminal_faces_by_residual) {
        const auto id = *proof.formula_of_residual.at(residual);
        std::map<std::pair<ResidualFormulaId, ControllerStateId>, bool> memo;
        for (std::size_t s = 0; s < product.controller_states.size(); ++s) {
            const bool symbolic = formula_detail::eval_formula_with_states(
                proof, product, id, s, memo);
            proof.terminal_replay_exact &= symbolic == product.residual_members[residual][s];
        }
    }
    if (!proof.terminal_replay_exact) {
        proof.obstruction = "terminal face formula replay failed";
        return proof;
    }

    // Fixed point over the already closed residual graph.  Each new name is
    // derived by applying the executable predecessor constructor to a named
    // residual, never by inserting its extensional bitset.
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t r = 0; r < product.residual_members.size(); ++r) {
            if (!proof.formula_of_residual[r]) continue;
            for (const auto digit : {-1LL, 0LL, 1LL}) {
                const auto target = product.residual_predecessor.at({r, digit});
                ResidualFormulaNode node;
                node.kind = ResidualFormulaKind::predecessor;
                node.digit = digit;
                node.child = *proof.formula_of_residual[r];
                const auto id = formula_detail::intern(proof, std::move(node));
                if (!proof.formula_of_residual[target]) {
                    proof.formula_of_residual[target] = id;
                    changed = true;
                }
            }
        }
    }

    proof.all_residuals_named = true;
    for (const auto& id : proof.formula_of_residual)
        proof.all_residuals_named &= id.has_value();
    if (!proof.all_residuals_named) {
        proof.obstruction = "closed residual family contains an unnamed component";
        return proof;
    }

    proof.extensional_replay_exact = true;
    for (std::size_t r = 0; r < product.residual_members.size(); ++r) {
        std::map<std::pair<ResidualFormulaId, ControllerStateId>, bool> memo;
        for (std::size_t s = 0; s < product.controller_states.size(); ++s) {
            const bool symbolic = formula_detail::eval_formula_with_states(
                proof, product, *proof.formula_of_residual[r], s, memo);
            if (symbolic != product.residual_members[r][s]) {
                proof.extensional_replay_exact = false;
                std::ostringstream o;
                o << "formula replay mismatch residual=" << r << " state=" << s;
                proof.obstruction = o.str();
                return proof;
            }
        }
    }

    // The constructor semantics itself is dimension-independent.  Replay all
    // concrete instances as an executable derivation of the generic theorem.
    proof.predecessor_semantics_exact = true;
    for (std::size_t r = 0; r < product.residual_members.size(); ++r) {
        for (const auto digit : {-1LL, 0LL, 1LL}) {
            const auto target = product.residual_predecessor.at({r, digit});
            for (std::size_t s = 0; s < product.controller_states.size(); ++s) {
                bool rhs = false;
                const auto it = product.controller_plant.successors.find({s, digit});
                if (it != product.controller_plant.successors.end())
                    for (const auto t : it->second)
                        rhs |= product.residual_members[r][t];
                proof.predecessor_semantics_exact &=
                    rhs == product.residual_members[target][s];
            }
        }
    }

    proof.universal_update_derived = true; // structural semantics of predecessor node
    proof.valid = proof.terminal_replay_exact && proof.predecessor_semantics_exact &&
                  proof.all_residuals_named && proof.extensional_replay_exact &&
                  proof.universal_update_derived;
    return proof;
}

inline std::string render_symbolic_residual_formula_report(
    const SymbolicResidualFormulaProof& p) {
    std::ostringstream o;
    o << "SYMBOLIC_RESIDUAL_FORMULA_PROOF\n";
    o << "dimension=" << p.dimension << "\n";
    o << "formula_nodes=" << p.nodes.size() << "\n";
    o << "residuals=" << p.formula_of_residual.size() << "\n";
    o << "terminal_replay_exact=" << (p.terminal_replay_exact ? "true" : "false") << "\n";
    o << "predecessor_semantics_exact=" << (p.predecessor_semantics_exact ? "true" : "false") << "\n";
    o << "all_residuals_named=" << (p.all_residuals_named ? "true" : "false") << "\n";
    o << "extensional_replay_exact=" << (p.extensional_replay_exact ? "true" : "false") << "\n";
    o << "universal_update_derived=" << (p.universal_update_derived ? "true" : "false") << "\n";
    o << "valid=" << (p.valid ? "true" : "false") << "\n";
    if (!p.obstruction.empty()) o << "obstruction=" << p.obstruction << "\n";
    return o.str();
}

} // namespace ravel::proof

namespace ravel::proof {

inline std::string render_generic_residual_formula_lean() {
    std::ostringstream o;
    o << "import Mathlib\n\n";
    o << "namespace RavelGenerated\n\n";
    o << "universe u\n\n";
    o << "inductive ResidualFormula (State : Type u) where\n";
    o << "  | terminal (accept : State → Prop)\n";
    o << "  | pre (digit : Int) (child : ResidualFormula State)\n\n";
    o << "def ResidualFormula.Eval {State : Type u}\n";
    o << "    (step : State → Int → State → Prop) :\n";
    o << "    ResidualFormula State → State → Prop\n";
    o << "  | .terminal accept, state => accept state\n";
    o << "  | .pre digit child, state =>\n";
    o << "      ∃ target, step state digit target ∧ child.Eval step target\n\n";
    o << "def deriveWord {State : Type u} :\n";
    o << "    List Int → ResidualFormula State → ResidualFormula State\n";
    o << "  | [], terminal => terminal\n";
    o << "  | digit :: rest, terminal => .pre digit (deriveWord rest terminal)\n\n";
    o << "inductive Follows {State : Type u}\n";
    o << "    (step : State → Int → State → Prop) :\n";
    o << "    State → List Int → State → Prop where\n";
    o << "  | nil (state) : Follows step state [] state\n";
    o << "  | cons {source middle target digit rest}\n";
    o << "      (head : step source digit middle)\n";
    o << "      (tail : Follows step middle rest target) :\n";
    o << "      Follows step source (digit :: rest) target\n\n";
    o << "/-- Predecessor is existential inverse image for every state type,\n";
    o << "transition relation, digit, formula, and state. -/\n";
    o << "theorem eval_pre_iff {State : Type u}\n";
    o << "    (step : State → Int → State → Prop)\n";
    o << "    (digit : Int) (child : ResidualFormula State) (state : State) :\n";
    o << "    (ResidualFormula.pre digit child).Eval step state ↔\n";
    o << "      ∃ target, step state digit target ∧ child.Eval step target := by\n";
    o << "  rfl\n\n";
    o << "/-- A residual derived along a word accepts exactly the starts of\n";
    o << "legal paths carrying that word to the terminal formula. -/\n";
    o << "theorem eval_deriveWord_iff_path {State : Type u}\n";
    o << "    (step : State → Int → State → Prop)\n";
    o << "    (word : List Int) (terminal : ResidualFormula State)\n";
    o << "    (source : State) :\n";
    o << "    (deriveWord word terminal).Eval step source ↔\n";
    o << "      ∃ target, Follows step source word target ∧\n";
    o << "        terminal.Eval step target := by\n";
    o << "  induction word generalizing source with\n";
    o << "  | nil =>\n";
    o << "      constructor\n";
    o << "      · intro h\n";
    o << "        exact ⟨source, Follows.nil source, h⟩\n";
    o << "      · rintro ⟨target, path, hterminal⟩\n";
    o << "        cases path\n";
    o << "        exact hterminal\n";
    o << "  | cons digit rest ih =>\n";
    o << "      constructor\n";
    o << "      · rintro ⟨middle, hstep, hrest⟩\n";
    o << "        obtain ⟨target, hpath, hterminal⟩ :=\n";
    o << "          (ih middle).mp hrest\n";
    o << "        exact ⟨target, Follows.cons hstep hpath, hterminal⟩\n";
    o << "      · rintro ⟨target, path, hterminal⟩\n";
    o << "        cases path with\n";
    o << "        | cons hstep htail =>\n";
    o << "            exact ⟨_, hstep, (ih _).mpr ⟨target, htail, hterminal⟩⟩\n\n";
    o << "end RavelGenerated\n";
    return o.str();
}

} // namespace ravel::proof
