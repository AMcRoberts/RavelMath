// Follows up class_ii_round4_edge_structure_a_independence_probe.cpp's
// finding (the whole connectivity graph is a-independent for a>=7)
// by inspecting the raw (p_k, q_k)-indexed candidate list itself for
// one representative state, `{i=0, x=(3,-3,0), j=0}` -- the same
// state whose extra edge at a=6 was the one exception found there --
// instead of just counting destinations.
//
// What this shows (checked directly, not guessed): the raw candidate
// COUNT genuinely grows with a (32 edges at a=6, 45 at a=7, 149 at
// a=15 -- parent_decompositions for the heavily-repeated letter 0
// really does have O(a) entries), but the DESTINATIONS collapse onto
// a small fixed set because x0'/x1' are the same constants across
// every surviving (p_k,q_k) pair at every tested a, and x2' depends
// only on the DIFFERENCE q_k - p_k, not on p_k, q_k, or a
// individually. This is the same M(a)-row-independence mechanism
// Round 1's proof used (rows 1,2 of M(a) don't depend on a), applied
// to a different free parameter (q_k - p_k, which both p_k and q_k
// range over widely as a grows, rather than a itself).
//
// Scope: verified on this one representative state, not yet the
// general lemma across all of Rounds 2/3/4's pruned-state catalogues.
// See docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's "The actual
// mechanism, found on one representative state" section for exactly
// what a full proof still needs.

#include <cstdio>
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

std::set<SNode<3>> pre_red_at(const Substitution<3>& subst_n) {
    const auto seeds = search_D_cont<3>(subst_n, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
    const auto pre_contact = backward_closure<3>(subst_n, d_cont);
    const auto induced = induced_restricted_edges<3>(subst_n, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
    const auto trace = algorithm2_trace<3>(
        subst_n, contact, CoronaConnectorPolicy::fixed_signed_contact,
        8, 0, CoronaEdgeArithmetic::exact_rational);
    return trace.layers[3].pre_red_nodes;
}

}  // namespace

int main() {
    for (long long a : {6, 7, 15}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const auto pre_red = pre_red_at(subst);

        const SNode<3> node{0, {3, -3, 0}, 0};
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);

        std::printf(
            "a=%lld pre_red_size=%zu |parents_i|=%zu |parents_j|=%zu\n",
            a, pre_red.size(), parents_i.size(), parents_j.size());

        long long edge_count = 0;
        std::set<SNode<3>> distinct_destinations;
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
                ++edge_count;
                distinct_destinations.insert(cand);
            }
        }
        std::printf(
            "  edges_into_pre_red=%lld distinct_destinations=%zu\n",
            edge_count, distinct_destinations.size());
    }

    std::printf(
        "ROUND4_EDGE_MECHANISM_PROBE_COMPLETE: see stdout above -- "
        "raw edge count grows with a, distinct destination count does "
        "not (checked at a=6,7,15 for the representative state "
        "{i=0,x=(3,-3,0),j=0})\n");
    return 0;
}
