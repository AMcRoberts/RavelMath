// ravel/proof/zero_run_forces_bounded_coincidence.hpp
//
// Generalizes Finding 17 (constant factor at position 0 forces
// depth-1 coincidence) from the "no internal zero digit" special case
// to the WHOLE canonical terminating-expansion family
// (canonical_beta_substitution_from_digits, any N, any digit sequence
// t_0..t_{N-1} with t_{N-1}>0). Found by systematically sweeping the
// family (varying N and digit patterns, not checking one-off
// examples) -- the roadmap's Stage 1 target of widening the PROVEN
// coincidence base past the trivial constant-factor case, using
// terminating_generator_theorem.hpp's already-proven, unconditional
// characterization of this family's transport generators as the
// starting point.
//
// THEOREM. Let sigma be the canonical terminating substitution for
// digits (t_0,...,t_{N-1}) (t_{N-1}>0), i.e. sigma(s) = 0^{t_s}(s+1)
// for s<N-1, sigma(N-1)=0^{t_{N-1}}. Let R = the length of the
// LONGEST RUN of consecutive zero digits among t_0,...,t_{N-2} (R=0
// if no t_s is zero -- Finding 17's case). Then every pair of letters
// resolves strong coincidence at depth <= R+1, and this bound is
// exact (achieved) whenever a run of length R actually occurs.
//
// PROOF. If t_s=0, sigma(s) = 0^0(s+1) = [s+1] exactly -- a single-
// letter "pass-through" image with no leading 0 and no branching at
// all. A maximal run of R consecutive zero digits at positions
// s,s+1,...,s+R-1 (t_s=...=t_{s+R-1}=0, and -- by maximality --
// t_{s-1}!=0 or s=0, and t_{s+R}!=0) therefore gives, for the FIRST
// letter of the run:
//   sigma(s) = [s+1], sigma^2(s) = sigma([s+1]) = [s+2], ...,
//   sigma^R(s) = [s+R].
// Every letter s+i (0<=i<R) in the run reaches EXACTLY [s+R] after
// R-i applications (by the same chaining), so ALL letters in the run
// synchronize onto the identical single-letter word [s+R] within R
// applications of the LATEST-starting one -- concretely, letter s
// itself needs exactly R applications to reach [s+R], the slowest
// case in the run. One more application (the (R+1)-th from s) gives
// sigma([s+R]) = sigma(s+R) = 0^{t_{s+R}}(s+R+1) -- and since
// t_{s+R}>0 (maximality of the run), this image genuinely BEGINS with
// letter 0. At that point, by Finding 17's own argument applied to
// depth 1 of THIS sub-step, any pair of letters whose R-step images
// both land on [s+R] (or on any OTHER letter with t>0, which begins
// producing a 0-prefix immediately) coincide via the shared leading
// zero. Letters entirely outside any zero-run already coincide at
// depth 1 by Finding 17 directly (R=0 for them). Taking the maximum
// over every run (a pair straddling the two different runs is bounded
// by the LARGER of the two, since the slower side determines when
// both first produce a shared leading letter) gives depth <= R_max+1
// for every pair, where R_max is the single longest run in the whole
// digit sequence -- matching Finding 17 exactly as the R_max=0 case.
//
// VERIFIED computationally (not just derived) against the actual
// `pair_has_coincidence`-equivalent search, worst-case-over-all-pairs
// depth, on 20 structurally diverse digit sequences spanning N=1..10
// and zero-run lengths 0..5 (including multiple separate runs of
// different lengths in the same sequence) -- exact match on every
// case, no exceptions. See the accompanying test.

#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace ravel::proof {

struct ZeroRunCoincidenceBoundCertificate {
    std::vector<long long> digits;
    long long longest_zero_run{0};       // R in the theorem (0 if no zero digit among t_0..t_{N-2})
    long long predicted_max_depth{1};    // R+1
    std::string note;
};

// Computes R (longest run of consecutive zero digits among
// t_0..t_{N-2} -- t_{N-1} itself is excluded, since it must be >0 by
// the terminating condition and never begins a run) directly from the
// digit sequence.
inline ZeroRunCoincidenceBoundCertificate derive_zero_run_coincidence_bound(
    const std::vector<long long>& digits) {
    ZeroRunCoincidenceBoundCertificate out;
    out.digits = digits;
    if (digits.empty() || digits.back() <= 0) {
        out.note = "empty or non-terminating digit sequence; premise does not apply";
        return out;
    }
    long long run = 0, best = 0;
    for (std::size_t s = 0; s + 1 < digits.size(); ++s) {  // exclude t_{N-1}
        if (digits[s] == 0) { ++run; best = std::max(best, run); }
        else { run = 0; }
    }
    out.longest_zero_run = best;
    out.predicted_max_depth = best + 1;
    out.note = best == 0
        ? "no internal zero digit -- Finding 17's case, depth 1 for every pair"
        : "longest zero-run has length " + std::to_string(best)
          + " -- every pair resolves by depth " + std::to_string(best + 1)
          + ", achieved exactly for pairs straddling that run";
    return out;
}

}  // namespace ravel::proof
