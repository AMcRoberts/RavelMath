// Automated symbolic-mechanism check for the recurrent-SCC exhaustion
// birth-round correspondence (2026-08-02), replacing per-a exact
// re-sweeps of the full pipeline with a cheap, targeted check that
// bypasses the expensive backward_closure/D_cont/Red machinery
// entirely and works directly from the already-proven closed-form
// pieces: the fixed, parameter-free 50-state seed
// (class_ii_neighbor2_signed_contact_set()), the fixed corona rule
// (c_corona/red, corona.hpp), and the closed-form shell formulas
// (class_ii_neighbor_regular_shell_states,
// class_ii_neighbor_special_shell_states).
//
// Claim under test: rank=a-1 and rank=a-2 (the two ranks nearest the
// dominant core) are both born at round 2 -- i.e. their catalogued
// states are a SUBSET of round 2's new arrivals (round-2 survivors not
// already present in the round-1 seed). This is the specific
// mechanism `docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s
// "birth-round/rank correspondence" section identified but did not
// finish turning into a checked, wide-range fact.
//
// Development note, kept because the mistakes were as informative as
// the result (per AM's "test the tool on a known case" instruction --
// this is exactly why that mattered): two real bugs were caught only
// by cross-validating a single build of this check against the
// trusted `algorithm2_trace` pipeline at the already-verified a=7
// before trusting any sweep.
//   1. First attempt seeded round 1 from
//      `class_ii_neighbor_dominant_core_states(2)` (39 states). Wrong:
//      that is the eventual recurrent dominant component of the FINAL
//      fixed point, not the round-1 corona input, which is the seed
//      itself (`A[1] = plus_minus_C`, 50 states). This produced a
//      uniform, a-independent false mismatch (identical numbers at
//      every tested a from 7 to 300) -- itself a tell that the bug was
//      structural, not a real a-dependent departure.
//   2. Second attempt used
//      `class_ii_neighbor_regular_shell_states(2, a, a - 2)` for
//      rank=a-2. Wrong: `class_ii_neighbor_recurrent_component_states`
//      (the trusted rank dispatcher) routes rank=a-2 for neighbor 2 to
//      the SPECIAL shell `class_ii_neighbor_special_shell_states(2, a,
//      1)` (2 states), not the regular-shell formula with
//      parameter=a-2. Diagnosing the exact missing/extra state
//      coordinates directly (not just counts) exposed this.
//   3. The check also initially required exact equality between round
//      2's new arrivals and the two expected shells. Too strict: round
//      2 legitimately contains other, uncatalogued transient states
//      too (22 of them at a=7), which the underlying claim never
//      excluded. Corrected to containment (every expected state is
//      present; nothing said about the rest).
//
// With all three fixes, this check's one-step reconstruction of round
// 2 reproduces the trusted `algorithm2_trace` pipeline's actual round-2
// survivor set EXACTLY at a=7 (72/72 states, extra=0, missing=0,
// cross-checked separately before trusting this file's sweep), and the
// containment claim holds with zero exceptions at every tested a from
// 7 through at least 2000 -- a much wider range than the expensive
// pipeline can reach in reasonable time, because this check never
// calls backward_closure, D_cont search, or the full Red-pruned corona
// iteration; it computes exactly one corona+Red step from a fixed,
// parameter-free seed.
//
// This is real, wide-range evidence for the mechanism the docs
// described, not yet a proof: it is still per-a exact computation
// (with real numeric beta/eigenvector data via same_letter_H), just at
// a scale and cost the old method could never reach. It does not by
// itself establish the mechanism for literally every integer a, and it
// only covers the round-1-to-round-2 step, not the full birth_round =
// a - rank induction for later rounds.

#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/corona.hpp"
#include "ravel/pisot_substitution_properties.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"

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
    for (std::size_t image = 0; image < 3; ++image)
        for (const auto letter : substitution[image])
            ++matrix[static_cast<std::size_t>(letter)][image];
    return matrix;
}

bool check_one(long long a, std::string& detail) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const auto& tau = neighbors[2].substitution;
    SubstitutionRule rule(tau);
    const auto subst = make_substitution<3>(rule, spectral.beta);

    const auto seed = class_ii_neighbor2_signed_contact_set();  // A[1]
    const auto corona_nodes = c_corona<3>(subst, seed, seed);
    std::vector<std::tuple<SNode<3>, SNode<3>, std::vector<long long>,
                           std::vector<long long>>> edges;
    for (const auto& n : corona_nodes) {
        auto fwd = simple_forward_targets<3>(subst, n);
        for (const auto& [dest, pq] : fwd)
            if (corona_nodes.count(dest) > 0)
                edges.push_back({n, dest, pq.first, pq.second});
    }
    const auto red_result = red<3>(corona_nodes, edges);
    const auto& round2 = red_result.first;  // A[2]

    std::set<SNode<3>> new_arrivals;
    for (const auto& n : round2) if (!seed.count(n)) new_arrivals.insert(n);

    std::set<SNode<3>> expected;
    try {
        auto rank_a_minus_1 =
            class_ii_neighbor_regular_shell_states(2, a, a - 1);
        auto rank_a_minus_2 = class_ii_neighbor_special_shell_states(2, a, 1);
        expected.insert(rank_a_minus_1.begin(), rank_a_minus_1.end());
        expected.insert(rank_a_minus_2.begin(), rank_a_minus_2.end());
    } catch (const std::exception& e) {
        detail = std::string("shell formula threw: ") + e.what();
        return false;
    }

    std::size_t missing = 0;
    for (const auto& n : expected) if (!new_arrivals.count(n)) ++missing;
    char buf[200];
    std::snprintf(buf, sizeof buf,
                  "expected=%zu missing=%zu new_arrivals=%zu",
                  expected.size(), missing, new_arrivals.size());
    detail = buf;
    return missing == 0;
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 7;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 2000;
    long long fails = 0, total = 0;
    for (long long a = a_min; a <= a_max; ++a) {
        std::string detail;
        if (!check_one(a, detail)) {
            std::printf("a=%lld MISMATCH %s\n", a, detail.c_str());
            ++fails;
        }
        ++total;
    }
    std::printf("Swept a=%lld..%lld (%lld values): %lld mismatches\n",
                a_min, a_max, total, fails);
    if (fails == 0)
        std::printf(
            "BOTH SHELLS (rank=a-1, rank=a-2) CONFIRMED BORN AT ROUND 2: "
            "zero exceptions.\n");
    return 0;
}
