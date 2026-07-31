// Corrects an overclaim from a first pass at classifying the 44
// shapes (class_ii_round34_edge_mechanism_shape_count.cpp's docs
// follow-up): grouping shapes by whether their abelianization
// components were all zero does NOT track whether p_len/q_len are
// genuinely ranges or fixed single values -- a shape can have a
// nonzero component on one side while the other side still ranges
// freely. This checks the real thing directly: for each shape, how
// many distinct p_len (resp. q_len) values does it actually admit at
// a=15 among edges landing in pre_red.
//
// Result: 20 shapes have both sides fixed (true Round-1-style
// escaping candidates), 20 have exactly one side ranging (a hybrid
// coverage question with an a-shifting anchor), and 4 have both sides
// ranging (pure coverage). Not the originally claimed 22/22 split --
// three categories, not two, with different membership than first
// reported. See docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's
// correction note for the full account.

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

    int both_fixed = 0, one_ranges = 0, both_range = 0;
    for (auto& [sh, ps] : plens) {
        const bool p_range = ps.size() > 1;
        const bool q_range = qlens[sh].size() > 1;
        if (!p_range && !q_range) ++both_fixed;
        else if (p_range != q_range) ++one_ranges;
        else ++both_range;
    }
    std::printf(
        "both_fixed(escaping)=%d one_side_ranges(hybrid)=%d "
        "both_range(pure_coverage)=%d total=%zu\n",
        both_fixed, one_ranges, both_range, plens.size());
    return 0;
}
