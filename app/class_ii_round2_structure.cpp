// Precise structural findings about Round 2 (checked against the
// trusted corona-trace pipeline, not assumed), replacing the vaguer
// "unverified lead" from the earlier session with concrete facts:
//
// 1. T_2 = B_2 union E_2 EXACTLY (72 = 47 + 25, zero overlap, zero
//    residue) -- the transport theorem's central claim, confirmed as
//    a literal set identity at a=7 (and, by class_ii_round2_recon.cpp,
//    stable in size across a=7,8).
//
// 2. E_2's 25 states are NOT a fresh 25-state catalogue: they are
//    (class_ii_neighbor2_fixed_extension_states(), the same 24-state
//    catalogue lean/class_ii_neighbor2_extensions.lean's
//    Neighbor2FixedKind already covers) MINUS {2,{1,-2,-1},0} PLUS
//    {2,{-2,2,-1},0} (the general interior tip at r=2, ALSO already
//    covered by neighbor2InteriorTip_in_open_strip) PLUS {0,{0,1,-1},0}
//    (genuinely new -- but its window validity is a one-line corollary
//    of the same c>3, 1/2<b-c<1 bounds Lemma 2 already establishes:
//    height = c-1 > 2 > 0, and c-1 < b iff b-c > -1, true since
//    b-c > 1/2). So ALL of E_2's window validity reduces to bounds
//    already kernel-checked for a different purpose -- there is no new
//    window-validity derivation needed for Round 2, contrary to what
//    the transport theorem's Lemma 2 section (written before this
//    check) implied might still be open.
//
// 3. The genuinely open part is exactly what the base-premises table
//    says: raw-corona reverse inclusion and Red exclusion. Calling the
//    real c_corona(T_1, +/-C) directly gives 195 raw (window-valid,
//    pre-Red) candidates, stable across a=6,7,8 (class_ii_round2_ccorona_stability.cpp).
//    Of those 195: exactly 25 land on E_2, 47 on B_2 (all of both,
//    confirmed, not just checked in aggregate), and the remaining 123
//    get removed by Red -- across THREE ranks (98, 15, 10), not one.
//    This is a materially bigger Red-exclusion argument than Round 1's
//    (two states, one rank): the pruned states' coordinates only go up
//    to |x_k|=3 versus the survivors' |x_k|<=2, so there is no simple
//    "unbounded in a" argument available the way Round 1 had -- these
//    are bounded, close-to-the-boundary states that need a real
//    multi-rank forward-edge argument, not a one-line bound.
//
// What this file checks, at a=6,7,8: the T_2 = B_2 + E_2 identity, and
// the exact rank sizes above, all stable across the tested range. It
// does NOT attempt the Red-exclusion argument for the 123 pruned
// states -- that remains open and is now precisely scoped rather than
// vaguely "comparable to Round 1."

#include <cstdio>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/class_ii_boundary_family.hpp"
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
    const auto E2 = class_ii_neighbor2_second_extension_states();
    const auto fixed24 = class_ii_neighbor2_fixed_extension_states();

    std::set<SNode<3>> e2_minus_fixed, fixed_minus_e2;
    std::set_difference(
        E2.begin(), E2.end(), fixed24.begin(), fixed24.end(),
        std::inserter(e2_minus_fixed, e2_minus_fixed.end()));
    std::set_difference(
        fixed24.begin(), fixed24.end(), E2.begin(), E2.end(),
        std::inserter(fixed_minus_e2, fixed_minus_e2.end()));

    const bool decomposition_ok = E2.size() == 25 && fixed24.size() == 24
        && e2_minus_fixed.size() == 2 && fixed_minus_e2.size() == 1
        && e2_minus_fixed.count(SNode<3>{0, {0, 1, -1}, 0})
        && e2_minus_fixed.count(SNode<3>{2, {-2, 2, -1}, 0})
        && fixed_minus_e2.count(SNode<3>{2, {1, -2, -1}, 0});

    bool all_ok = decomposition_ok;
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
        const auto& T1 = trace_n.layers[0].nodes;
        const auto& pmC = trace_n.signed_contact;
        const auto& B2 = trace_c.layers[1].nodes;
        const auto& T2_actual = trace_n.layers[1].nodes;
        const auto& red_ranks = trace_n.layers[1].red_pruning_ranks;

        std::set<SNode<3>> B2_union_E2;
        for (const auto& n : B2) B2_union_E2.insert(n);
        for (const auto& n : E2) B2_union_E2.insert(n);
        const bool t2_identity_ok = B2_union_E2 == T2_actual;

        const auto raw2 = c_corona<3>(subst_n, T1, pmC);
        const std::set<SNode<3>> raw2_set(raw2.begin(), raw2.end());

        std::set<SNode<3>> dest_in_E2, dest_in_B2;
        for (const auto& n : raw2_set) {
            if (E2.count(n)) dest_in_E2.insert(n);
            if (B2.count(n)) dest_in_B2.insert(n);
        }
        const bool coverage_ok =
            dest_in_E2.size() == 25 && dest_in_B2.size() == 47;

        std::vector<std::size_t> rank_sizes;
        for (const auto& rank : red_ranks) rank_sizes.push_back(rank.size());
        long long pruned_total = 0;
        for (auto sz : rank_sizes) pruned_total += static_cast<long long>(sz);
        const bool pruning_ok = rank_sizes.size() == 3
            && rank_sizes[0] == 98 && rank_sizes[1] == 15
            && rank_sizes[2] == 10 && pruned_total == 123
            && raw2_set.size() == 195 && T2_actual.size() == 72;

        std::printf(
            "a=%lld t2_identity_ok=%d coverage_ok=%d pruning_ok=%d "
            "raw2=%zu -> E2_covered=%zu B2_covered=%zu red_ranks=%zu,%zu,%zu "
            "pruned_total=%lld survivors=%zu\n",
            a, t2_identity_ok, coverage_ok, pruning_ok,
            raw2_set.size(), dest_in_E2.size(), dest_in_B2.size(),
            rank_sizes.size() > 0 ? rank_sizes[0] : 0,
            rank_sizes.size() > 1 ? rank_sizes[1] : 0,
            rank_sizes.size() > 2 ? rank_sizes[2] : 0,
            pruned_total, T2_actual.size());

        all_ok = all_ok && t2_identity_ok && coverage_ok && pruning_ok;
    }

    std::printf(
        "%s\n",
        all_ok ? "ROUND2_STRUCTURE_CONFIRMED: T2=B2+E2 exactly at every "
                  "tested a; E2's window validity reduces entirely to "
                  "already-established bounds; the open gap is "
                  "specifically a 3-rank, 123-state Red exclusion, "
                  "stable across a=6,7,8, not a 1-rank few-state one "
                  "like Round 1"
                : "STRUCTURE CHECK FAILED -- see flags above, do not "
                  "trust the counts in the header comment");
    return all_ok ? 0 : 1;
}
