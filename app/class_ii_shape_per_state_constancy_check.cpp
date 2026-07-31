// Resolves the correction in class_ii_shape_state_consistency_check.cpp
// by asking the question that actually matters: not "is x2' constant
// across the different states that share a shape" (it isn't, and
// doesn't need to be), but "for a GIVEN state, is its own set of
// both-fixed-rule destinations constant across a." Checked directly
// at a=8 vs a=25, per state (not per shape): **45/45 states match
// exactly** -- every state's own both-fixed edges land on the same
// destinations at both tested a.
//
// This ties the whole "both-fixed" investigation back to the
// already-established whole-graph a-independence result
// (class_ii_round2_edge_structure_a_independence_probe.cpp and
// siblings, a=6..50): that result already implies per-state
// constancy for every edge, including these. The shape-level device
// was useful for classifying which SIDE ranges vs is fixed
// (class_ii_shape_classification_rule.cpp, a genuine shape-level
// fact) but was never the right level to ask about specific x2'
// values -- states are. Nothing new is proved here for literal every
// integer a (still an exact finite check, now at the right level of
// generality), but the both-fixed category needs no separate
// treatment beyond what the whole-graph checks already cover.

#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

Matrix incidence(const FiniteSubstitution& s) {
    Matrix m(3, std::vector<long long>(3, 0));
    for (std::size_t im = 0; im < 3; ++im)
        for (auto l : s[im]) ++m[static_cast<std::size_t>(l)][im];
    return m;
}

std::set<SNode<3>> pre_red_at(const Substitution<3>& subst) {
    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
    const auto pre_contact = backward_closure<3>(subst, d_cont);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
    const auto trace = algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        8, 0, CoronaEdgeArithmetic::exact_rational);
    return trace.layers[1].pre_red_nodes;
}

bool rule_predicts_range(long long inner_letter, long long parent_letter,
                          long long l1, long long l2) {
    if (inner_letter != 0) return false;
    if (parent_letter == 2) return false;
    if (l1 != 0 || l2 != 0) return false;
    return true;
}

}  // namespace

int main() {
    std::map<SNode<3>, std::set<SNode<3>>> dests_at_8, dests_at_25;

    for (long long a : {8LL, 25LL}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);
        const auto pre_red = pre_red_at(subst);

        for (const auto& node : pre_red) {
            auto parents_i = parent_decompositions<3>(subst.images, node.i);
            auto parents_j = parent_decompositions<3>(subst.images, node.j);
            for (const auto& pd : parents_i) {
                auto lp1 = abelianization<3>(pd.p);
                if (rule_predicts_range(node.i, pd.parent_letter, lp1[1],
                                         lp1[2]))
                    continue;
                for (const auto& qd : parents_j) {
                    auto lq1 = abelianization<3>(qd.p);
                    if (rule_predicts_range(node.j, qd.parent_letter,
                                             lq1[1], lq1[2]))
                        continue;
                    std::array<long long, 3> rhs;
                    for (int k = 0; k < 3; ++k)
                        rhs[k] = node.x[k] + lq1[k] - lp1[k];
                    auto xprime_opt = solve_Mx_eq_rhs_exact<3>(subst.M, rhs);
                    if (!xprime_opt.has_value()) continue;
                    auto xprime = *xprime_opt;
                    SNode<3> cand{pd.parent_letter, xprime, qd.parent_letter};
                    if (!is_valid_simple_node<3>(subst, cand)) continue;
                    if (!pre_red.count(cand)) continue;
                    if (a == 8) dests_at_8[node].insert(cand);
                    else dests_at_25[node].insert(cand);
                }
            }
        }
    }

    long long total = 0, match = 0;
    for (auto& [node, d8] : dests_at_8) {
        ++total;
        auto it = dests_at_25.find(node);
        if (it != dests_at_25.end() && it->second == d8) ++match;
    }
    std::printf(
        "per_state_both_fixed_destinations: total=%lld match(a=8 vs "
        "a=25)=%lld\n",
        total, match);
    std::printf(
        "%s\n",
        match == total
            ? "PER_STATE_CONSTANCY_CONFIRMED: every state's own "
              "both-fixed-rule destinations are identical at a=8 and "
              "a=25 -- ties this category back to the already-"
              "established whole-graph a-independence result; the "
              "shape-level device was the wrong level to ask about "
              "x2' values, states are the right level"
            : "MISMATCH FOUND -- see per-state detail needed");
    return match == total ? 0 : 1;
}
