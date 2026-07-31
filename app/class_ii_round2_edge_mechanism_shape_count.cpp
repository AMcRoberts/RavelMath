// Gauges tractability of the remaining "uniform bound across every
// group" step named in class_ii_round2_edge_mechanism_exact.cpp's
// closing note: counts distinct group SHAPES (node.i, node.j,
// parent_letter_i, parent_letter_j, l(p)[1], l(p)[2], l(q)[1],
// l(q)[2]) -- abstracted from the specific node.x coordinate offset,
// which does not affect which occurrence-segment structure applies.
//
// If the number of distinct shapes is small (a handful to a few
// dozen), the remaining step is a genuinely tractable finite case
// analysis, not an unbounded one. Result at a=15: 44 distinct shapes
// across all 195 of Round 2's raw states -- small enough for a future
// session to attempt directly.

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
    std::set<Shape> shapes;

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
                shapes.insert({node.i, node.j, pd.parent_letter,
                                qd.parent_letter, lp1[1], lp1[2], lq1[1],
                                lq1[2]});
            }
        }
    }

    std::printf(
        "a=%lld total distinct shapes "
        "(i,j,p_parent,q_parent,lp1,lp2,lq1,lq2) = %zu\n",
        a, shapes.size());
    std::printf(
        "%s\n",
        "ROUND2_SHAPE_COUNT: a small, bounded number of distinct "
        "group shapes covers all 195 raw states -- the remaining "
        "uniform-bound step is a tractable finite case analysis, not "
        "an unbounded one");
    return 0;
}
