// Refines class_ii_round2_edge_mechanism_generality_check.cpp's
// negative result: that file pooled ALL surviving (p_k,q_k) pairs for
// a state together when testing "x2' is a function of q_k-p_k alone,"
// which mixes pairs from genuinely different (parent_letter_i,
// parent_letter_j) TYPES -- different types can have different
// abelianization vectors even at the same occurrence LENGTH (e.g. the
// unique letter-2 occurrence in sigma(0)="0^a 1 2" has prefix "0^a 1"
// while the unique letter-2 occurrence in sigma(1)="0^(a-1) 2 0" has
// prefix "0^(a-1)" -- same rough length, different letter-1 content),
// so pooling them together breaks any single-slope relationship even
// when each TYPE individually has one.
//
// This regroups by (parent_letter_i, parent_letter_j) TYPE first --
// matching the granularity Round 1's proof actually used ("A type1",
// "A type2", "B type1", "B type2") -- and tests the stricter claim
// "x0'/x1' constant and x2' = (q_len - p_len) + a fixed offset" within
// each type.
//
// Result at a=8: 94/139 multi-edge types are clean under this test.
// Better than the pooled test (which effectively tested 0 types
// correctly), but still not universal -- the untested next step is
// whether the "dirty" types have SOME OTHER fixed slope (not
// necessarily 1) relating x2' to q_len-p_len, which this file does
// not check.

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
    const long long a = 8;
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

    long long total_types = 0, clean_types = 0;

    for (const auto& node : pre_red) {
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);

        std::map<std::pair<long long, long long>,
                  std::vector<std::tuple<long long, long long, SNode<3>>>>
            by_type;

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
                by_type[{pd.parent_letter, qd.parent_letter}].push_back(
                    {static_cast<long long>(pd.p.size()),
                     static_cast<long long>(qd.p.size()), cand});
            }
        }

        for (auto& [type, edges] : by_type) {
            if (edges.size() < 2) continue;
            ++total_types;
            const auto& [p0, q0, dest0] = edges[0];
            const long long baseline_offset = dest0.x[2] - (q0 - p0);
            const long long bx0 = dest0.x[0], bx1 = dest0.x[1];
            bool clean = true;
            for (auto& [p_len, q_len, dest] : edges) {
                const long long offset = dest.x[2] - (q_len - p_len);
                if (offset != baseline_offset || dest.x[0] != bx0
                    || dest.x[1] != bx1) {
                    clean = false;
                    break;
                }
            }
            if (clean) ++clean_types;
        }
    }

    std::printf(
        "a=%lld total_multi_edge_types=%lld "
        "clean_types(x0x1_const_and_x2=diff+offset)=%lld\n",
        a, total_types, clean_types);
    std::printf(
        "%s\n",
        "ROUND2_TYPE_GROUPED_CHECK_COMPLETE: type-level grouping "
        "(matching Round 1's A-type1/type2, B-type1/type2 granularity) "
        "is a real improvement over the pooled test but still not "
        "universal -- untested next step: whether dirty types have "
        "some other fixed slope, not necessarily 1, relating x2' to "
        "q_len-p_len");
    return 0;
}
