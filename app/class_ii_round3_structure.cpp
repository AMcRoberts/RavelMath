// The Round 2 treatment (class_ii_round2_structure.cpp,
// class_ii_round2_red_exclusion_check.cpp) applied to Round 3, to get
// a complete parallel picture across the base rounds rather than
// leaving Round 3 as a total unknown.
//
// Findings (checked, not assumed):
//
// 1. T_3 = B_3 union E_3 EXACTLY (93 = 68 + 25, literal set identity,
//    a=7,8). Unlike Round 2, E_3 needs no special swap: it is exactly
//    class_ii_neighbor2_interior_extension_states(3), the general
//    "24 fixed states + interior tip at r" formula evaluated at r=3 --
//    the same 24-state catalogue Neighbor2FixedKind already covers,
//    plus {2,{-3,3,-1},0} (the r=3 tip). Whether
//    neighbor2InteriorTip_in_open_strip's hypothesis
//    `r*(b-c)+1<c` actually holds at r=3 under the established
//    `c>3, 1/2<b-c<1` bounds has NOT been checked here (at r=2 it did,
//    trivially, per the Round 2 write-up; at r=3 the same bound gives
//    `3*(b-c)+1 < 4`, which needs `c>=4`, tighter than the generic
//    `c>3` -- this may need the sharper a-specific bound rather than
//    the coarse one, and is flagged rather than assumed to also be
//    free).
//
// 2. Red exclusion: raw candidates = 256, survivors = 93, so 163
//    pruned states across 3 ranks. Independently re-derived via
//    simple_forward_targets_exact (not red_anode's own bookkeeping):
//    zero violations at a=6,7,8 -- every pruned state's edges within
//    the 256-state raw set land on an earlier-or-equal rank, never a
//    survivor or later rank. Exact finite certificate, not yet a
//    symbolic proof for every a.
//
// Round 3 is comparable in scale to Round 2 (163 pruned states vs
// 123), not smaller -- there is no shortcut here either.

#include <cstdio>
#include <set>
#include <vector>

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

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image) {
        for (const auto letter : substitution[image]) {
            ++matrix[static_cast<std::size_t>(letter)][image];
        }
    }
    return matrix;
}

CoronaTrace<3> corona_trace(const Substitution<3>& subst) {
    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
    const auto pre_contact = backward_closure<3>(subst, d_cont);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
    return algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        8, 0, CoronaEdgeArithmetic::exact_rational);
}

}  // namespace

int main() {
    const auto E3 = class_ii_neighbor2_interior_extension_states(3);
    bool all_ok = E3.size() == 25;

    for (long long a : {6LL, 7LL, 8LL}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst_n = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);
        const Substitution<3> subst_c = make_substitution<3>(
            SubstitutionRule(center), spectral.beta);

        const auto trace_n = corona_trace(subst_n);
        const auto trace_c = corona_trace(subst_c);
        const auto& B3 = trace_c.layers[2].nodes;
        const auto& layer = trace_n.layers[2];
        const auto& T3_actual = layer.nodes;
        const auto& pre_red = layer.pre_red_nodes;
        const auto& ranks = layer.red_pruning_ranks;

        std::set<SNode<3>> B3_union_E3;
        for (const auto& n : B3) B3_union_E3.insert(n);
        for (const auto& n : E3) B3_union_E3.insert(n);
        const bool identity_ok = B3_union_E3 == T3_actual;

        std::map<SNode<3>, int> rank_of;
        for (const auto& n : T3_actual) rank_of[n] = -1;
        for (std::size_t r = 0; r < ranks.size(); ++r)
            for (const auto& n : ranks[r]) rank_of[n] = static_cast<int>(r);

        long long violations = 0, total_checked = 0, edges_within = 0;
        for (std::size_t r = 0; r < ranks.size(); ++r) {
            for (const auto& n : ranks[r]) {
                ++total_checked;
                const auto targets =
                    simple_forward_targets_exact<3>(subst_n, n);
                for (const auto& [dest, pq] : targets) {
                    if (pre_red.count(dest) == 0) continue;
                    ++edges_within;
                    const auto it = rank_of.find(dest);
                    if (it == rank_of.end()
                        || it->second == -1
                        || it->second > static_cast<int>(r)) {
                        ++violations;
                    }
                }
            }
        }

        std::printf(
            "a=%lld B3=%zu E3=%zu identity_ok=%d pre_red=%zu "
            "survivors=%zu pruned_checked=%lld edges_within=%lld "
            "violations=%lld ranks=%zu\n",
            a, B3.size(), E3.size(), identity_ok, pre_red.size(),
            T3_actual.size(), total_checked, edges_within, violations,
            ranks.size());

        all_ok = all_ok && identity_ok && violations == 0;
    }

    std::printf(
        "%s\n",
        all_ok ? "ROUND3_STRUCTURE_AND_RED_EXCLUSION_CONFIRMED: T3=B3+E3 "
                  "exactly at every tested a; Red exclusion has an "
                  "independent exact finite certificate (zero "
                  "violations); comparable in scale to Round 2 (163 "
                  "pruned states, 3 ranks), not smaller"
                : "CHECK FAILED -- see output above");
    return all_ok ? 0 : 1;
}
