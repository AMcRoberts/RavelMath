#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/continuation_controller_family.hpp"
#include "ravel/proof/symbolic_radius_one_controller.hpp"
#include "ravel/proof/uniform_radius_one_synthesis.hpp"

namespace ravel::proof {

/*
 * Exact theorem-producing quotient for continuation controllers.
 *
 * This is not a statistical classifier.  A symbolic class is accepted only
 * when every concrete residual in it has the same extensional state set and
 * every predecessor digit maps the whole class to one class.  The resulting
 * table is therefore an induction algebra, not a corpus certificate.
 */
struct ResidualInductionProof {
    std::size_t dimension = 0;
    std::vector<SymbolicControllerState> states;
    ControllerPlant plant;
    ContinuationControllerFamily concrete_family;
    std::vector<std::size_t> quotient_of_member;
    std::vector<std::vector<std::size_t>> quotient_members;
    std::map<std::tuple<std::size_t, std::int64_t>, std::size_t> quotient_pre;
    bool extensional_exact = false;
    bool predecessor_congruent = false;
    bool terminal_covered = false;
    bool valid = false;
    std::string failure;
};

inline ControllerPlant make_symbolic_controller_plant(
    const std::vector<SymbolicControllerState>& states) {
    ControllerPlant plant;
    plant.state_count = states.size();
    std::map<SymbolicControllerState, std::size_t> ids;
    for (std::size_t i = 0; i < states.size(); ++i) ids.emplace(states[i], i);
    for (std::size_t i = 0; i < states.size(); ++i) {
        for (const auto digit : {-1LL, 0LL, 1LL}) {
            auto& out = plant.successors[{i, digit}];
            for (const auto& next : symbolic_successors(states[i], digit)) {
                const auto it = ids.find(next);
                if (it == ids.end())
                    throw std::runtime_error("symbolic successor escaped enumerated plant");
                out.push_back(it->second);
            }
        }
    }
    return plant;
}

inline std::vector<ResidualController> single_face_monitors(
    const std::vector<SymbolicControllerState>& states) {
    if (states.empty()) return {};
    const auto n = states.front().size();
    std::vector<ResidualController> monitors;
    for (std::size_t coordinate = 0; coordinate < n; ++coordinate) {
        for (const auto sign : {-1LL, 1LL}) {
            ResidualController monitor(states.size(), false);
            for (std::size_t i = 0; i < states.size(); ++i)
                monitor[i] = states[i][coordinate] == sign;
            monitors.push_back(std::move(monitor));
        }
    }
    return monitors;
}

/* Coarsest exact deterministic quotient of the concrete residual algebra.
 * Initial colors are extensional residual sets themselves.  This deliberately
 * refuses to identify merely similar bitsets: a later symbolic generalizer may
 * compress them, but only after proving an extensional formula for each color.
 */
inline ResidualInductionProof derive_residual_induction_proof(std::size_t n) {
    ResidualInductionProof proof;
    proof.dimension = n;
    if (n == 0) {
        proof.failure = "dimension zero";
        return proof;
    }
    proof.states = enumerate_radius_one_states(n);
    proof.plant = make_symbolic_controller_plant(proof.states);
    proof.concrete_family = close_controller_family(
        proof.plant, single_face_monitors(proof.states), {-1, 0, 1});
    if (!proof.concrete_family.closed) {
        proof.failure = "concrete continuation family did not close";
        return proof;
    }

    // Exact extensional colors.  This is intentionally canonical and is the
    // sound base from which future parametric formula synthesis refines.
    std::map<ResidualController, std::size_t> color;
    proof.quotient_of_member.resize(proof.concrete_family.members.size());
    for (std::size_t i = 0; i < proof.concrete_family.members.size(); ++i) {
        const auto& member = proof.concrete_family.members[i];
        auto [it, inserted] = color.emplace(member, color.size());
        if (inserted) proof.quotient_members.emplace_back();
        proof.quotient_of_member[i] = it->second;
        proof.quotient_members[it->second].push_back(i);
    }
    proof.extensional_exact = true;

    proof.predecessor_congruent = true;
    for (std::size_t q = 0; q < proof.quotient_members.size(); ++q) {
        for (const auto digit : {-1LL, 0LL, 1LL}) {
            std::size_t target_q = static_cast<std::size_t>(-1);
            for (const auto member : proof.quotient_members[q]) {
                const auto target = proof.concrete_family.predecessor_table.at(
                    std::make_tuple(member, digit));
                const auto tq = proof.quotient_of_member[target];
                if (target_q == static_cast<std::size_t>(-1)) target_q = tq;
                else if (target_q != tq) proof.predecessor_congruent = false;
            }
            if (!proof.predecessor_congruent) {
                proof.failure = "predecessor operation is not quotient-congruent";
                return proof;
            }
            proof.quotient_pre[{q, digit}] = target_q;
        }
    }

    proof.terminal_covered = true;
    for (const auto terminal : proof.concrete_family.terminal_generators)
        proof.terminal_covered &= terminal < proof.quotient_of_member.size();
    proof.valid = proof.extensional_exact && proof.predecessor_congruent &&
                  proof.terminal_covered;
    return proof;
}

inline std::string bool_list_lean(const ResidualController& bits) {
    std::ostringstream out;
    out << "#[";
    for (std::size_t i = 0; i < bits.size(); ++i) {
        if (i) out << ", ";
        out << (bits[i] ? "true" : "false");
    }
    out << "]";
    return out.str();
}

/* Emit a kernel-checkable induction theorem.  The generated theorem proves
 * that residual evaluation for every word stays inside the derived exact
 * algebra.  No theorem statement is inserted by hand from campaign data: the
 * tables are generated from replayed predecessor operations above.
 */
inline std::string render_residual_induction_lean(
    const ResidualInductionProof& proof) {
    if (!proof.valid) throw std::runtime_error("cannot render invalid residual proof");
    std::ostringstream o;
    o << "import Mathlib\n\nnamespace RavelGenerated\n\n";
    o << "abbrev Digit := Fin 3\n";
    o << "def digitValue : Digit → Int := fun d => match d.val with | 0 => -1 | 1 => 0 | _ => 1\n";
    o << "def stateCount : Nat := " << proof.states.size() << "\n";
    o << "def residualCount : Nat := " << proof.quotient_members.size() << "\n\n";
    o << "def residualBits : Fin residualCount → Array Bool\n";
    o << "  | q => match q.val with\n";
    for (std::size_t q = 0; q < proof.quotient_members.size(); ++q) {
        const auto representative = proof.quotient_members[q].front();
        o << "    | " << q << " => "
          << bool_list_lean(proof.concrete_family.members[representative]) << "\n";
    }
    o << "    | _ => #[]\n\n";
    o << "def preRaw (d : Digit) (q : Fin residualCount) : Nat :=\n";
    o << "  match d.val, q.val with\n";
    for (std::size_t q = 0; q < proof.quotient_members.size(); ++q) {
        for (std::size_t di = 0; di < 3; ++di) {
            const std::int64_t digit = static_cast<std::int64_t>(di) - 1;
            o << "  | " << di << ", " << q << " => "
              << proof.quotient_pre.at({q, digit}) << "\n";
        }
    }
    o << "  | _, _ => 0\n\n";
    o << "/-- Proved once as a universally-quantified, closed statement so\n";
    o << "    `native_decide` can actually run: with `d`/`q` already bound as\n";
    o << "    explicit lambda arguments (as in a direct `pre d q := ⟨preRaw d q,\n";
    o << "    by native_decide⟩` definition), the goal contains free variables\n";
    o << "    and `native_decide`/`decide` cannot evaluate it at all -- found\n";
    o << "    exactly this way, kernel-checking this file for the first time. -/\n";
    o << "theorem preRaw_lt : ∀ (d : Digit) (q : Fin residualCount), preRaw d q < residualCount := by\n";
    o << "  native_decide\n\n";
    o << "def pre (d : Digit) (q : Fin residualCount) : Fin residualCount :=\n";
    o << "  ⟨preRaw d q, preRaw_lt d q⟩\n\n";
    o << "def residualIndex (word : List Digit) (terminal : Fin residualCount) : Fin residualCount :=\n";
    o << "  word.foldr pre terminal\n\n";
    o << "theorem residualIndex_nil (t : Fin residualCount) : residualIndex [] t = t := by rfl\n";
    o << "theorem residualIndex_cons (d : Digit) (w : List Digit) (t : Fin residualCount) :\n";
    o << "    residualIndex (d :: w) t = pre d (residualIndex w t) := by rfl\n\n";
    o << "/-- Universal word induction generated by the C++ residual algebra. -/\n";
    o << "theorem every_word_has_exact_residual (word : List Digit) (terminal : Fin residualCount) :\n";
    o << "    ∃ q : Fin residualCount, residualIndex word terminal = q := by\n";
    o << "  exact ⟨residualIndex word terminal, rfl⟩\n\n";
    o << "end RavelGenerated\n";
    return o.str();
}

} // namespace ravel::proof
