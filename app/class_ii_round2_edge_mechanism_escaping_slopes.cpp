// Tests the "escaping" label given to the 20 both-fixed shapes in
// class_ii_round2_edge_mechanism_shape_range_check.cpp's docs
// follow-up: that label was an analogy to Round 1's two D_cont seeds
// (whose forward images are genuinely affine in a with nonzero slope,
// -a, a, a+1, 1-a), assumed rather than checked. Checked directly by
// computing each shape's single x2' value at two widely separated a
// (a=7 and a=40) and comparing: **every one of the 20 has slope
// exactly zero between these two points**. x2' looks CONSTANT for
// these shapes, not escaping to infinity -- the Round-1 analogy was
// wrong for this category.
//
// NOT claimed: that two sample points prove constancy for literal
// every integer a. The algebra (x2' = rhs[0] - a*rhs[2], slope =
// -(node.x[2]+l(q)[2]-l(p)[2])) shows zero slope holds iff
// node.x[2] = l(p)[2]-l(q)[2] for every state with this shape --
// plausible, not yet derived as necessary. This corrects the framing
// (escaping vs. constant), not the proof status (still open).

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

using Shape = std::tuple<long long, long long, long long, long long,
                          long long, long long, long long, long long>;

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

}  // namespace

int main() {
    std::map<Shape, long long> x2_at_7, x2_at_40;

    for (long long a : {7LL, 40LL}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);
        const auto pre_red = pre_red_at(subst);

        std::map<Shape, std::set<long long>> plens, qlens;
        std::map<Shape, long long> x2val;
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
                    Shape sh{node.i, node.j, pd.parent_letter,
                              qd.parent_letter, lp1[1], lp1[2], lq1[1],
                              lq1[2]};
                    plens[sh].insert(static_cast<long long>(pd.p.size()));
                    qlens[sh].insert(static_cast<long long>(qd.p.size()));
                    x2val[sh] = cand.x[2];
                }
            }
        }
        for (auto& [sh, ps] : plens) {
            if (ps.size() == 1 && qlens[sh].size() == 1) {
                if (a == 7) x2_at_7[sh] = x2val[sh];
                else x2_at_40[sh] = x2val[sh];
            }
        }
    }

    int total = 0, nonzero_slope = 0;
    for (auto& [sh, x7] : x2_at_7) {
        ++total;
        if (x2_at_40.count(sh) && x2_at_40.at(sh) != x7) ++nonzero_slope;
    }
    std::printf("total_both_fixed_shapes=%d nonzero_slope_count=%d\n",
                total, nonzero_slope);
    std::printf(
        "%s\n",
        nonzero_slope == 0
            ? "ROUND2_ESCAPING_SHAPES_APPEAR_CONSTANT: all both-fixed "
              "shapes have x2' identical at a=7 and a=40 -- zero "
              "slope between these two points, not escaping. Does "
              "NOT by itself prove constancy for every integer a; "
              "see the file header for the exact algebraic condition "
              "(node.x[2] = l(p)[2]-l(q)[2]) still needing derivation"
            : "SOME SHAPES DO HAVE NONZERO SLOPE -- see per-shape "
              "detail needed");
    return nonzero_slope == 0 ? 0 : 1;
}
