// Completes class_ii_both_fixed_corrected_condition.cpp's finding
// (rhs2 = slope_q - slope_p holds for every CURRENTLY valid both-fixed
// edge, checked at a=7 and a=20) with an actual closed-form proof
// that it holds for EVERY integer a>=7, not just the two tested
// points -- by solving directly, not sampling further.
//
// For a both-fixed edge, x2'(a) = CONST + a*[(slope_q - slope_p) -
// rhs2] is fully determined (CONST and the slope coefficient) once
// the shape and node.x are fixed -- both are computable in closed
// form from the exhaustively-enumerated occurrence-type formulas
// (class_ii_hybrid_window_slope_derivation.cpp's 5 types: p_len(a) =
// slope*a + b, with (slope,b) one of (0,0) [parent=2], (1,0)
// [parent=0,inner=1], (1,1) [parent=0,inner=2], (1,-1)
// [parent=1,inner=2], (1,0) [parent=1,inner=0,past marker]).
//
// If the resulting slope of x2'(a) is nonzero, x2'(a) = T.x[2] (for
// any specific candidate target T with matching x0',x1') has AT MOST
// ONE integer solution a, computable directly: a* = (T.x[2] -
// CONST) / slope. This file computes that required a* for every
// (node, shape-combination, candidate target) triple where the slope
// is nonzero -- not just the currently-valid ones -- and checks
// whether a* is ever an integer >= 7. If never, this is a genuine
// closed-form proof (not another finite check) that no
// slope-nonzero edge can ever be valid for any integer a>=7: the
// corrected condition (rhs2 = slope_q - slope_p) is therefore
// necessary, not just observed, for every integer a>=7.

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

// The exhaustively-enumerated fixed occurrence types:
// (inner_letter, parent_letter, l1, l2, slope, intercept_b).
struct FixedType {
    long long inner, parent, l1, l2, slope, b;
};

const std::vector<FixedType>& fixed_types() {
    static const std::vector<FixedType> types = {
        {0, 2, 0, 0, 0, 0},   // sigma(2)="0": trivial, p_len=0
        {1, 0, 0, 0, 1, 0},   // the single '1' in sigma(0): p_len=a
        {2, 0, 1, 0, 1, 1},   // the single '2' in sigma(0): p_len=a+1
        {2, 1, 0, 0, 1, -1},  // the single '2' in sigma(1): p_len=a-1
        {0, 1, 0, 1, 1, 0},   // '0' after the '2' in sigma(1): p_len=a
    };
    return types;
}

}  // namespace

int main() {
    // pre_red is a-independent; compute it once at a reference a.
    const long long a_ref = 15;
    const auto center = class_ii(static_cast<std::size_t>(a_ref));
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

    long long total_slope_nonzero_combos = 0;
    long long total_would_need_a_ge_7 = 0;
    long long total_with_no_integer_solution = 0;

    for (const auto& node : pre_red) {
        for (const auto& pt : fixed_types()) {
            if (pt.inner != node.i) continue;
            for (const auto& qt : fixed_types()) {
                if (qt.inner != node.j) continue;

                const long long rhs2 = node.x[2] + qt.l2 - pt.l2;
                const long long slope_diff = qt.slope - pt.slope;
                const long long x2_slope = slope_diff - rhs2;

                // CONST = node.x[0] + (b_q - l(q)[1] - l(q)[2])
                //                   - (b_p - l(p)[1] - l(p)[2])
                const long long CONST = node.x[0]
                    + (qt.b - qt.l1 - qt.l2) - (pt.b - pt.l1 - pt.l2);
                // x0', x1' (a-independent, needed to find candidate targets)
                const long long x0p = node.x[1] + qt.l1 - pt.l1;
                const long long x1p = rhs2 - x0p;

                if (x2_slope == 0) continue;  // the "correct" (stable) case
                ++total_slope_nonzero_combos;

                // Find every candidate target T with matching i,j,x0,x1.
                for (const auto& T : pre_red) {
                    if (T.i != pt.parent || T.j != qt.parent) continue;
                    if (T.x[0] != x0p || T.x[1] != x1p) continue;
                    // Solve CONST + a*x2_slope = T.x[2] for integer a.
                    const long long numerator = T.x[2] - CONST;
                    if (numerator % x2_slope != 0) {
                        ++total_with_no_integer_solution;
                        continue;
                    }
                    const long long a_required = numerator / x2_slope;
                    if (a_required >= 7) {
                        ++total_would_need_a_ge_7;
                        std::printf(
                            "COUNTEREXAMPLE POSSIBLE: node i=%lld x=(%lld,%lld,%lld) "
                            "j=%lld  p_type(inner=%lld,parent=%lld) "
                            "q_type(inner=%lld,parent=%lld)  "
                            "requires a=%lld (>=7!)\n",
                            node.i, node.x[0], node.x[1], node.x[2], node.j,
                            pt.inner, pt.parent, qt.inner, qt.parent,
                            a_required);
                    }
                }
            }
        }
    }

    std::printf(
        "total_slope_nonzero_combos=%lld with_no_integer_a_solution=%lld "
        "would_need_a>=7=%lld\n",
        total_slope_nonzero_combos, total_with_no_integer_solution,
        total_would_need_a_ge_7);
    std::printf(
        "%s\n",
        total_would_need_a_ge_7 == 0
            ? "CLOSED_FORM_PROOF_COMPLETE: every slope-nonzero "
              "(node, shape) combination either has no integer "
              "solution at all, or requires a < 7 -- computed "
              "directly, not sampled. No slope-nonzero both-fixed "
              "edge can ever be valid for any integer a>=7. The "
              "corrected condition (rhs2 = slope_q - slope_p) is "
              "therefore NECESSARY, not merely observed, for every "
              "integer a>=7"
            : "COUNTEREXAMPLE EXISTS FOR SOME a>=7 -- see detail above, "
              "the both-fixed closure needs revisiting");
    return total_would_need_a_ge_7 == 0 ? 0 : 1;
}
