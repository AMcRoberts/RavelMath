// Extends class_ii_round2_edge_mechanism_shape_count.cpp's tractability
// check to Rounds 3 and 4, then goes one step further: not just do
// all three rounds have the same COUNT of distinct group shapes (44),
// do they share the literal same SET?
//
// Result: yes. Rounds 2, 3, and 4 (a=15, past Round 4's a=6 exception)
// share the literal identical 44-shape catalogue -- confirmed by set
// equality, not just matching cardinalities. The remaining
// uniform-bound case analysis is therefore ONE shared bounded
// catalogue of 44 shapes, not three separate 44-shape catalogues that
// happen to be the same size.



#include <cstdio>
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

std::set<Shape> shapes_at(long long a, std::size_t layer_index) {
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
    const auto& pre_red = trace.layers[layer_index].pre_red_nodes;

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
    return shapes;
}

}  // namespace

int main() {
    const auto round2 = shapes_at(15, 1);
    const auto round3 = shapes_at(15, 2);
    const auto round4 = shapes_at(15, 3);
    std::printf("Round 2 (a=15, 195 raw states): %zu distinct shapes\n", round2.size());
    std::printf("Round 3 (a=15, 256 raw states): %zu distinct shapes\n", round3.size());
    std::printf("Round 4 (a=15, 325 raw states): %zu distinct shapes\n", round4.size());
    const bool same_2_3 = round2 == round3;
    const bool same_2_4 = round2 == round4;
    std::printf("round2 == round3 (literal same shape SET): %d\n", same_2_3);
    std::printf("round2 == round4 (literal same shape SET): %d\n", same_2_4);
    std::printf(
        "%s\n",
        (same_2_3 && same_2_4)
            ? "ROUND234_SHAPE_SET_IDENTICAL: all three rounds share the "
              "literal same 44-shape catalogue -- the remaining case "
              "analysis is one shared bounded catalogue, not three "
              "separate ones"
            : "ROUND234_SHAPE_SET_DIFFERS: same size, different "
              "members -- three separate (same-size) catalogues, not "
              "one shared one");
    return 0;
}
