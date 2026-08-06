// ravel/proof/generator_set_from_digit_differences.hpp
//
// The general generator-production theorem, closing the question that
// motivated `first_genuine_fourth_generator.hpp`: WHICH primitive
// positive generators a canonical beta-substitution has, predicted
// directly from its digit sequence, with no boundary-graph
// construction, no role-catalogue enumeration, not even
// isolate_beta's secondary-root work -- only the digits themselves.
//
// Claim. Let (t_1,...,t_N) be a Pisot number's terminating greedy
// expansion of 1, driving the canonical substitution
// sigma(s) = 0^{t_s} (s+1) for s < N-1, sigma(N-1) = 0^{t_{N-1}}.
//
// Every nonzero letter m in {1,...,N-1} occurs as a substitution image
// symbol in EXACTLY ONE place: as the trailing symbol of state (m-1)'s
// image, at prefix length exactly t_{m-1} (0-indexed: digit[m-1]).
// Nowhere else -- only the literal symbol 0 ever recurs across
// multiple images. Call this letter m's RIGID LENGTH,
// rigid(m) = digit[m-1]. Letter 0 is the only letter with more than
// one occurrence (hence the only FLEXIBLE one): it appears in every
// image with digit[c] >= 1, at every prefix length 0..digit[c]-1.
//
// Consequence (proved by direct inspection, not searched for): a role
// (i,j) with BOTH i and j nonzero has EXACTLY ONE possible transition
// at all -- not merely one that happens to be cheapest -- namely
// defect = rigid(j) - rigid(i). No other outgoing edge exists from
// that role, of any kind, because both i's and j's parent lists are
// singletons. So whenever rigid(j)-rigid(i) is outside {-1,0,1} for
// some pair of distinct nonzero letters, that value is unconditionally
// a new primitive generator: literally the unique way to realize that
// transition, not a shortcut that merely wasn't dominated.
//
// Theorem. The complete primitive generator set is exactly
//   {0,+1,-1} union { digit[m-1] - digit[m'-1] : m != m', both in [1,N-1] }
// i.e. {0,+1,-1} union all pairwise differences of every digit EXCEPT
// the last one (digit[N-1], which is |norm(beta)| itself and never
// serves as any letter's rigid length -- it belongs to the trailing,
// letter-less padding of the final state).
//
// In particular:
//   - if the non-final digits (digit[0..N-2]) take at most one DISTINCT
//     value, no pair exists to difference, so no generator beyond
//     Q/R/S is possible, regardless of how large that one value is
//     (x^2-2x-2's (2,2): only one non-final digit value, still Q/R/S).
//   - a genuine new generator requires at least two DISTINCT non-final
//     digit values differing by >= 2 (x^3-2x^2-2's (2,0,2): values
//     {2,0}, difference 2).
//
// Checked against `canonical_substitution_generator_collapse.hpp`'s
// (expensive: builds the full role catalogue and every domination
// check) results for every case examined by this project so far --
// see tests/generator_set_from_digit_differences_test.cpp. This header
// is the cheap, closed-form predictor; that one remains the
// ground-truth verifier.
//
// SCOPE, found the hard way: this closed form is proved correct for
// TERMINATING (simple Parry) expansions specifically. It does NOT
// directly generalize to eventually-periodic ones (genuine preperiod
// > 0): `x^3-2x^2-x+1` (preperiod (2), period (0,1)) has a
// concretely-confirmed genuine fourth/fifth generator
// (`eventually_periodic_generator_witness.hpp`) despite having only
// ONE fully rigid letter -- the wraparound closing the cycle gives the
// cycle-start letter a second occurrence, so this header's core
// assumption ("only letter 0 has more than one occurrence") is false
// there. The violation still happens, through a one-step-removed
// version of the same mechanism, but this formula does not predict it.
// Use `canonical_substitution_generator_collapse.hpp` (which now
// handles both regimes) for eventually-periodic inputs; a corrected
// closed form covering both cases has not yet been derived.

#pragma once

#include <set>
#include <vector>

namespace ravel::proof {

// Pure combinatorics on the digit sequence -- no algebraic number
// machinery needed at all.
inline std::set<long long> predict_generator_set_from_digits(const std::vector<long long>& digits) {
    std::set<long long> generators = {-1, 0, 1};
    if (digits.size() < 2) return generators;  // no non-final digits to pair
    std::set<long long> non_final(digits.begin(), digits.end() - 1);
    for (long long a : non_final) for (long long b : non_final) generators.insert(a - b);
    return generators;
}

}  // namespace ravel::proof
