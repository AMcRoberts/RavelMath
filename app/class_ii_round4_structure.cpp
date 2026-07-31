// The Round 2/3 treatment (class_ii_round2_structure.cpp,
// class_ii_round3_structure.cpp) applied to Round 4, the last of the
// four exceptional base transitions and, per docs/
// GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's base-premises table,
// the one whose "still needed" column reads differently from Rounds
// 2/3: "connect the bounded round-four grammar to the universal
// stable reverse/exclusion theorem whose stated domain begins at
// round five" rather than a bare occurrence/exclusion gap. This file
// checks whether Round 4's raw-corona structure is nonetheless the
// same shape as Rounds 2/3 (T_4 = B_4 union E_4 exactly, Red exclusion
// with an independent exact finite certificate), which is a
// prerequisite for deciding whether round five's universal machinery
// can simply absorb round four or whether round four needs its own
// separate closing argument.
//
// Findings (checked, not assumed):
//
// 1. T_4 = B_4 union E_4 at a=7,8 (a=6 excluded: the base-premises
//    table's own exact center trace notes the round-four raw size
//    collides at a=4, not a=6, but this file checks a=6 too for
//    parity with Round 2/3's tested set; see output for whether it
//    still holds there).
// 2. Red exclusion: raw candidates vs survivors, independently
//    re-derived via simple_forward_targets_exact (not red_anode's own
//    bookkeeping): zero violations expected, matching Rounds 2/3.
//
// This does NOT resolve the "connect to the universal stable theorem"
// gap the base-premises table actually asks for at round four -- that
// is a different, harder claim (bridging into the round>=5 dispatcher)
// than the occurrence/exclusion shape checked here.

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
    const auto E4 = class_ii_neighbor2_interior_extension_states(4);
    bool all_ok = E4.size() == 25;

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
        const auto& B4 = trace_c.layers[3].nodes;
        const auto& layer = trace_n.layers[3];
        const auto& T4_actual = layer.nodes;
        const auto& pre_red = layer.pre_red_nodes;
        const auto& ranks = layer.red_pruning_ranks;

        std::set<SNode<3>> B4_union_E4;
        for (const auto& n : B4) B4_union_E4.insert(n);
        for (const auto& n : E4) B4_union_E4.insert(n);
        const bool identity_ok = B4_union_E4 == T4_actual;

        std::map<SNode<3>, int> rank_of;
        for (const auto& n : T4_actual) rank_of[n] = -1;
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
            "a=%lld B4=%zu E4=%zu identity_ok=%d pre_red=%zu "
            "survivors=%zu pruned_checked=%lld edges_within=%lld "
            "violations=%lld ranks=%zu\n",
            a, B4.size(), E4.size(), identity_ok, pre_red.size(),
            T4_actual.size(), total_checked, edges_within, violations,
            ranks.size());

        all_ok = all_ok && identity_ok && violations == 0;
    }

    std::printf(
        "%s\n",
        all_ok ? "ROUND4_STRUCTURE_AND_RED_EXCLUSION_CONFIRMED: T4=B4+E4 "
                  "exactly at every tested a; Red exclusion has an "
                  "independent exact finite certificate (zero "
                  "violations)"
                : "CHECK FAILED -- see output above");
    return all_ok ? 0 : 1;
}
