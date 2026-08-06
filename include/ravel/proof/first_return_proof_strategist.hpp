#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/first_return_dimension_extension.hpp"

namespace ravel::proof {

enum class ProofBranchStatus { proved, refuted, blocked, applicable };

struct ProofBranchRecord {
    std::string name;
    ProofBranchStatus status = ProofBranchStatus::blocked;
    std::string result;
    std::vector<std::string> next_pivots;
};

struct QuotientRelationAnalysis {
    std::size_t classes = 0;
    std::size_t labelled_pairs = 0;
    std::size_t relation_arcs = 0;
    std::size_t nondeterministic_pairs = 0;
    std::size_t max_targets = 0;
    std::size_t terminal_classes = 0;
    std::size_t mixed_terminal_classes = 0;
    std::size_t existentially_winning_classes = 0;
    std::size_t universally_winning_classes = 0;
    bool all_classes_existentially_winning = false;
    bool all_classes_universally_winning = false;
};

struct FirstReturnProofStrategyResult {
    std::size_t dimension = 0;
    QuotientRelationAnalysis relation;
    std::vector<ProofBranchRecord> branches;
    bool fixed_dimension_relational_induction = false;
    bool universal_theorem = false;
    std::string strongest_result;
    std::string remaining_obstruction;
};

namespace detail {

inline const char* branch_status_name(ProofBranchStatus s) {
    switch (s) {
        case ProofBranchStatus::proved: return "PROVED";
        case ProofBranchStatus::refuted: return "REFUTED";
        case ProofBranchStatus::applicable: return "APPLICABLE";
        case ProofBranchStatus::blocked: return "BLOCKED";
    }
    return "BLOCKED";
}

inline QuotientRelationAnalysis analyze_relation(
    const FirstReturnJointProduct& product,
    const JointRoleQuotientProof& quotient) {
    QuotientRelationAnalysis a;
    a.classes = quotient.refined_classes;
    a.labelled_pairs = quotient.transition_relation.size();
    for (const auto& [_, targets] : quotient.transition_relation) {
        a.relation_arcs += targets.size();
        a.max_targets = std::max(a.max_targets, targets.size());
        if (targets.size() > 1) ++a.nondeterministic_pairs;
    }

    std::vector<bool> has_terminal(a.classes, false);
    std::vector<bool> has_nonterminal(a.classes, false);
    for (const auto& state : product.reachable) {
        const auto q = quotient.class_of.at(state);
        if (state.remaining == 0) has_terminal[q] = true;
        else has_nonterminal[q] = true;
    }
    for (std::size_t q = 0; q < a.classes; ++q) {
        if (has_terminal[q]) ++a.terminal_classes;
        if (has_terminal[q] && has_nonterminal[q]) ++a.mixed_terminal_classes;
    }

    // The witnessed joint graph decreases remaining, so winning regions can be
    // calculated exactly as least fixed points over the quotient relation.
    std::vector<bool> exists_win = has_terminal;
    std::vector<bool> forall_win = has_terminal;
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t q = 0; q < a.classes; ++q) {
            if (!exists_win[q]) {
                bool can = false;
                for (const auto& [key, targets] : quotient.transition_relation) {
                    if (key.first != q) continue;
                    for (const auto t : targets) if (exists_win[t]) { can = true; break; }
                    if (can) break;
                }
                if (can) { exists_win[q] = true; changed = true; }
            }
            if (!forall_win[q]) {
                bool has = false;
                bool all = true;
                for (const auto& [key, targets] : quotient.transition_relation) {
                    if (key.first != q) continue;
                    has = true;
                    for (const auto t : targets) if (!forall_win[t]) { all = false; break; }
                    if (!all) break;
                }
                if (has && all) { forall_win[q] = true; changed = true; }
            }
        }
    }
    a.existentially_winning_classes = std::count(exists_win.begin(), exists_win.end(), true);
    a.universally_winning_classes = std::count(forall_win.begin(), forall_win.end(), true);
    a.all_classes_existentially_winning = a.existentially_winning_classes == a.classes;
    a.all_classes_universally_winning = a.universally_winning_classes == a.classes;
    return a;
}

inline void export_strategy_data(const std::filesystem::path& dir,
                                 const FirstReturnJointProduct& product,
                                 const JointRoleQuotientProof& quotient) {
    std::filesystem::create_directories(dir);
    {
        std::ofstream out(dir / "joint_states.csv");
        out << "class,residual,remaining,plant,target_faces\n";
        for (const auto& s : product.reachable) {
            out << quotient.class_of.at(s) << ',' << s.residual << ',' << s.remaining << ",\"";
            for (std::size_t i = 0; i < s.plant_state.size(); ++i) {
                if (i) out << ' ';
                out << s.plant_state[i];
            }
            out << "\",\"";
            for (std::size_t i = 0; i < s.target_faces.size(); ++i) {
                if (i) out << ' ';
                out << s.target_faces[i].first << ':' << s.target_faces[i].second;
            }
            out << "\"\n";
        }
    }
    {
        std::ofstream out(dir / "quotient_relation.csv");
        out << "source_class,digit,target_class\n";
        for (const auto& [key, targets] : quotient.transition_relation)
            for (const auto target : targets)
                out << key.first << ',' << key.second << ',' << target << '\n';
    }
}

} // namespace detail

inline FirstReturnProofStrategyResult run_first_return_proof_strategist(
    const FirstReturnJointProduct& product,
    const JointRoleQuotientProof& quotient,
    const std::filesystem::path& export_dir = {}) {
    FirstReturnProofStrategyResult r;
    r.dimension = product.dimension;
    r.relation = detail::analyze_relation(product, quotient);
    if (!export_dir.empty()) detail::export_strategy_data(export_dir, product, quotient);

    ProofBranchRecord deterministic;
    deterministic.name = "deterministic symbolic congruence";
    if (quotient.deterministic) {
        deterministic.status = ProofBranchStatus::proved;
        deterministic.result = "the refined reachable quotient has a single target class per labelled pair";
    } else {
        deterministic.status = ProofBranchStatus::refuted;
        std::ostringstream o;
        o << r.relation.nondeterministic_pairs << " labelled quotient pairs have multiple targets; max branching="
          << r.relation.max_targets;
        deterministic.result = o.str();
        deterministic.next_pivots = {"relational simulation", "alternating/game invariant", "bounded-history refinement"};
    }
    r.branches.push_back(std::move(deterministic));

    ProofBranchRecord relational;
    relational.name = "relational least-fixed-point induction";
    if (r.relation.all_classes_existentially_winning) {
        relational.status = ProofBranchStatus::proved;
        relational.result = "every reachable symbolic class has a quotient-relation path to a terminal endpoint monitor";
        r.fixed_dimension_relational_induction = true;
    } else {
        relational.status = ProofBranchStatus::refuted;
        relational.result = "some reachable quotient classes have no relational path to a terminal class";
        relational.next_pivots = {"bounded-history refinement", "cycle/path decomposition", "repair endpoint monitor"};
    }
    r.branches.push_back(std::move(relational));

    ProofBranchRecord universal_game;
    universal_game.name = "universal/game closure";
    if (r.relation.all_classes_universally_winning) {
        universal_game.status = ProofBranchStatus::proved;
        universal_game.result = "every witnessed quotient continuation reaches a terminal class";
    } else {
        universal_game.status = ProofBranchStatus::blocked;
        std::ostringstream o;
        o << r.relation.universally_winning_classes << '/' << r.relation.classes
          << " classes are universally winning";
        universal_game.result = o.str();
        universal_game.next_pivots = {"separate plant choice from controller choice", "alternating predecessor operator", "history-sensitive strategy"};
    }
    r.branches.push_back(std::move(universal_game));

    ProofBranchRecord dimension;
    dimension.name = "dimension-uniform shadow induction";
    dimension.status = ProofBranchStatus::blocked;
    dimension.result = "requires adjacent-dimension role correspondence and symbolic boundary transition equations";
    dimension.next_pivots = {"n->n+1 shadow transport", "finite boundary-role schema", "strong induction with exceptional bases"};
    r.branches.push_back(std::move(dimension));

    if (r.fixed_dimension_relational_induction) {
        r.strongest_result = "fixed-dimensional relational completeness for every state represented in the realized witness product";
        r.remaining_obstruction = "prove that the witness product represents every realized first-return cycle uniformly in n, then transport the relational invariant through n->n+1";
    } else {
        r.strongest_result = "exact quotient relation and minimized proof-style dispatch";
        r.remaining_obstruction = "refine the reachable abstraction until relational endpoint reachability holds";
    }
    return r;
}

inline std::string render_first_return_strategy_report(
    const FirstReturnProofStrategyResult& r) {
    std::ostringstream o;
    o << "FIRST_RETURN_PROOF_STRATEGIST\n";
    o << "dimension=" << r.dimension << "\n";
    o << "classes=" << r.relation.classes << "\n";
    o << "labelled_pairs=" << r.relation.labelled_pairs << "\n";
    o << "relation_arcs=" << r.relation.relation_arcs << "\n";
    o << "nondeterministic_pairs=" << r.relation.nondeterministic_pairs << "\n";
    o << "max_targets=" << r.relation.max_targets << "\n";
    o << "terminal_classes=" << r.relation.terminal_classes << "\n";
    o << "mixed_terminal_classes=" << r.relation.mixed_terminal_classes << "\n";
    o << "existentially_winning_classes=" << r.relation.existentially_winning_classes << "\n";
    o << "universally_winning_classes=" << r.relation.universally_winning_classes << "\n";
    for (const auto& b : r.branches) {
        o << "BRANCH " << b.name << " " << detail::branch_status_name(b.status) << "\n";
        o << "  result=" << b.result << "\n";
        if (!b.next_pivots.empty()) {
            o << "  pivots=";
            for (std::size_t i = 0; i < b.next_pivots.size(); ++i) {
                if (i) o << " | ";
                o << b.next_pivots[i];
            }
            o << "\n";
        }
    }
    o << "strongest_result=" << r.strongest_result << "\n";
    o << "remaining_obstruction=" << r.remaining_obstruction << "\n";
    return o.str();
}

} // namespace ravel::proof
