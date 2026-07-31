// Closes the "why is the edge structure a-independent" line of
// inquiry from class_ii_round2_edge_mechanism_generality_check.cpp /
// class_ii_round2_edge_mechanism_by_type_check.cpp /
// class_ii_round2_edge_mechanism_general_slope_check.cpp: those three
// files' refinements each narrowed but never closed the gap because
// they used the WRONG independent variable. p_len and q_len (raw
// occurrence-prefix LENGTH) are not what the M(a) inverse actually
// depends on -- it depends on the prefix's per-letter abelianization
// counts specifically.
//
// From simple_forward_targets_exact's own equation (rhs = node.x +
// l(q1) - l(p1), M(a)x' = rhs) and Round 1's back-substitution
// (rows 1,2 of M(a) don't depend on a):
//   x0' = rhs[1] = node.x[1] + l(q)[1] - l(p)[1]
//   x1' = rhs[2] - rhs[1]
//   x2' = rhs[0] - a*rhs[2]
//       = (node.x[0] + l(q)[0] - l(p)[0]) - a*(node.x[2] + l(q)[2] - l(p)[2])
// Since l(p)[0] = p_len - l(p)[1] - l(p)[2] (total length minus the
// other two letters' counts), x2' is affine in (q_len - p_len) with
// slope EXACTLY 1 -- but only once l(p)[1], l(p)[2], l(q)[1], l(q)[2]
// are held fixed, which p_len/q_len alone do not guarantee (two
// occurrences at different positions can have the same total prefix
// length while differing in how that length splits across letters --
// not actually possible within a single image here, but occurrences
// from DIFFERENT positions relative to the image's fixed 1/2 markers
// can still disagree on the split even at comparable lengths).
//
// Grouping raw candidates by the FULL 6-tuple
// (parent_letter_i, parent_letter_j, l(p)[1], l(p)[2], l(q)[1], l(q)[2])
// therefore gives, by construction, x0'/x1' exactly constant and x2'
// exactly affine in (q_len - p_len) with slope exactly 1, within every
// group -- not approximately, not "mostly": checked here across every
// one of Round 2's 195 raw states at a=6,7,8,15 and found 100% clean
// (135/135 multi-edge groups, 728-1700 total edges checked depending
// on a, zero exceptions at every tested a).
//
// This closes the WITHIN-a mechanism completely (why the destination
// SET stays small even as raw candidate count grows with a) for every
// tested a. What remains for the full symbolic proof (not closed
// here): showing the achievable range of (q_len - p_len) values that
// land inside the bounded target window stabilizes once a crosses
// each state's own threshold, and that this threshold is uniformly
// bounded across all 195/256/325 states -- see
// docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md for the precise
// remaining scope.

#include <cstdio>
#include <map>
#include <set>
#include <tuple>
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

}  // namespace

int main() {
    bool all_ok = true;
    for (long long a : {6, 7, 8, 15}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

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
        const auto& pre_red = trace.layers[1].pre_red_nodes;

        long long total_groups = 0, clean_groups = 0, total_edges = 0;

        for (const auto& node : pre_red) {
            auto parents_i = parent_decompositions<3>(subst.images, node.i);
            auto parents_j = parent_decompositions<3>(subst.images, node.j);

            using Key = std::tuple<long long, long long, long long, long long,
                                    long long, long long>;
            std::map<Key, std::vector<std::tuple<long long, long long, SNode<3>>>>
                by_group;

            for (const auto& pd : parents_i) {
                auto lp1 = abelianization<3>(pd.p);
                for (const auto& qd : parents_j) {
                    auto lq1 = abelianization<3>(qd.p);
                    std::array<long long, 3> rhs;
                    for (int k = 0; k < 3; ++k)
                        rhs[k] = node.x[k] + lq1[k] - lp1[k];
                    auto xprime_opt = solve_Mx_eq_rhs_exact<3>(subst.M, rhs);
                    if (!xprime_opt.has_value()) continue;
                    auto xprime = *xprime_opt;
                    SNode<3> cand{pd.parent_letter, xprime, qd.parent_letter};
                    if (!is_valid_simple_node<3>(subst, cand)) continue;
                    if (!pre_red.count(cand)) continue;
                    Key key{pd.parent_letter, qd.parent_letter, lp1[1], lp1[2],
                             lq1[1], lq1[2]};
                    by_group[key].push_back(
                        {static_cast<long long>(pd.p.size()),
                         static_cast<long long>(qd.p.size()), cand});
                }
            }

            for (auto& [key, edges] : by_group) {
                if (edges.size() < 2) continue;
                ++total_groups;
                total_edges += edges.size();
                const auto& [bp, bq, bdest] = edges[0];
                bool clean = true;
                for (auto& [p_len, q_len, dest] : edges) {
                    if (dest.x[0] != bdest.x[0] || dest.x[1] != bdest.x[1]) {
                        clean = false;
                        break;
                    }
                    const long long expected_x2 =
                        bdest.x[2] + (q_len - p_len) - (bq - bp);
                    if (dest.x[2] != expected_x2) {
                        clean = false;
                        break;
                    }
                }
                if (clean) ++clean_groups;
            }
        }

        const bool ok = total_groups > 0 && clean_groups == total_groups;
        all_ok = all_ok && ok;
        std::printf(
            "a=%lld total_groups=%lld clean_groups=%lld total_edges=%lld "
            "ok=%d\n",
            a, total_groups, clean_groups, total_edges, ok);
    }

    std::printf(
        "%s\n",
        all_ok ? "ROUND2_EDGE_MECHANISM_EXACT_CONFIRMED: grouping by "
                  "(parent_letter_i, parent_letter_j, l(p)[1], l(p)[2], "
                  "l(q)[1], l(q)[2]) gives x0'/x1' exactly constant and "
                  "x2' exactly affine in (q_len-p_len) with slope "
                  "exactly 1, in EVERY group, at every tested a -- 100% "
                  "clean, not a partial pattern"
                : "CHECK FAILED -- see per-a output above");
    return all_ok ? 0 : 1;
}
