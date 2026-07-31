// Answers AM's question directly: is there a pattern in the 44-shape
// classification (class_ii_round2_edge_mechanism_shape_range_check.cpp's
// 4 both-range / 20 one-side-ranges / 20 both-fixed split)? Yes, and
// it follows directly from tau_a's fixed word forms
// (sigma(0)=0^a 1 2, sigma(1)=0^(a-1) 2 0, sigma(2)=0):
//
// A side (p or q) RANGES over a growing interval as `a` increases
// if and only if BOTH:
//   1. its inner search letter (node.i for the p-side, node.j for
//      the q-side) is 0, AND
//   2. its occurrence sits in one of the two LEADING zero-runs
//      (parent_letter=0 with l(p)[1]=l(p)[2]=0 -- sigma(0)'s
//      length-a leading run; or parent_letter=1 with
//      l(p)[1]=l(p)[2]=0 -- sigma(1)'s length-(a-1) leading run).
//
// Every other combination gives a FIXED single occurrence, not a
// range:
//   - inner letter is 1 (occurs exactly once, total, across all
//     images -- sigma(0) only) or 2 (occurs exactly twice, total,
//     but only once per specific image), regardless of which parent
//     or position;
//   - inner letter is 0 but parent_letter=2 (sigma(2)="0" is a
//     single letter, trivially one occurrence);
//   - inner letter is 0, parent_letter in {0,1}, but the occurrence
//     is AFTER a marker (l(p)[1]!=0 or l(p)[2]!=0) -- there is at
//     most one further occurrence past a marker in either image.
//
// This is not a guess: this file computes the rule's prediction for
// every one of the 44 shapes and checks it against the actual
// observed range/fixed behavior (same computation
// class_ii_round2_edge_mechanism_shape_range_check.cpp already
// trusted). 100% agreement would mean the classification is fully
// explained, not just empirically observed.

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

// The classification rule: does (inner_letter, parent_letter, l1, l2)
// correspond to a ranging (growing-with-a) occurrence?
bool rule_predicts_range(long long inner_letter, long long parent_letter,
                          long long l1, long long l2) {
    if (inner_letter != 0) return false;      // letters 1,2: always fixed
    if (parent_letter == 2) return false;      // sigma(2)="0": trivial, fixed
    if (l1 != 0 || l2 != 0) return false;      // past a marker: fixed
    return true;  // parent in {0,1}, pure leading-run prefix: ranges
}

}  // namespace

int main() {
    const long long a = 15;
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

    using Shape = std::tuple<long long, long long, long long, long long,
                              long long, long long, long long, long long>;
    std::map<Shape, std::set<long long>> plens, qlens;

    for (const auto& node : pre_red) {
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);
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
                Shape sh{node.i, node.j, pd.parent_letter, qd.parent_letter,
                          lp1[1], lp1[2], lq1[1], lq1[2]};
                plens[sh].insert(static_cast<long long>(pd.p.size()));
                qlens[sh].insert(static_cast<long long>(qd.p.size()));
            }
        }
    }

    long long total = 0, agree = 0;
    long long rule_both_range = 0, rule_one_ranges = 0, rule_both_fixed = 0;
    for (auto& [sh, ps] : plens) {
        const auto& [i, j, pp, qp, lp1, lp2, lq1, lq2] = sh;
        const bool p_actual_range = ps.size() > 1;
        const bool q_actual_range = qlens[sh].size() > 1;
        const bool p_rule = rule_predicts_range(i, pp, lp1, lp2);
        const bool q_rule = rule_predicts_range(j, qp, lq1, lq2);
        ++total;
        if (p_rule == p_actual_range && q_rule == q_actual_range) ++agree;
        else
            std::printf(
                "DISAGREE: i=%lld j=%lld pp=%lld qp=%lld "
                "lp=(%lld,%lld) lq=(%lld,%lld) "
                "rule=(%d,%d) actual=(%d,%d)\n",
                i, j, pp, qp, lp1, lp2, lq1, lq2, p_rule, q_rule,
                p_actual_range, q_actual_range);
        if (p_rule && q_rule) ++rule_both_range;
        else if (p_rule != q_rule) ++rule_one_ranges;
        else ++rule_both_fixed;
    }

    std::printf(
        "total=%lld agree=%lld  rule_predicts: both_range=%lld "
        "one_ranges=%lld both_fixed=%lld\n",
        total, agree, rule_both_range, rule_one_ranges, rule_both_fixed);
    std::printf(
        "%s\n",
        agree == total
            ? "CLASSIFICATION_RULE_CONFIRMED: a side ranges iff its "
              "inner letter is 0 and its occurrence is a pure "
              "leading-run prefix (parent in {0,1}, no marker "
              "crossed) -- this rule alone, derived directly from "
              "tau_a's word structure, exactly reproduces the "
              "4/20/20 split for every one of the 44 shapes"
            : "RULE DOES NOT FULLY EXPLAIN THE SPLIT -- see "
              "disagreements above");
    return agree == total ? 0 : 1;
}
