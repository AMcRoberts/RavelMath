// ravel/proof/coincidence_as_landmark_vector_cancellation.hpp
//
// The Diophantine-synchronization reduction for strong coincidence,
// past the boundary-letter (constant-factor / shared-first-or-last)
// case. Derived and computationally verified 2026-08-06 against
// sigma_{0,1} (sigma(0)=1,2  sigma(1)=2  sigma(2)=0), the non-AR
// control from Finding 5/16/17.
//
// SETUP. Any position within sigma^k(b) corresponds to a unique
// Dumont-Thomas walk: a sequence of (current_letter, chosen_child_index)
// pairs from depth k down to depth 0 (the prefix automaton this project
// already builds for property (F), read top-down instead of
// bottom-up). Write the walk as levels ell = k, k-1, ..., 1 (the
// current letter at level ell has k-ell substitution applications
// still to unfold below it).
//
// THEOREM (verified computationally against the real recursive
// decomposition and against direct word materialization -- not just
// derived by hand). The abelianization of the prefix up to that
// position is EXACTLY
//   P = sum over levels ell (with a genuine choice made there) of
//       M^{ell-1} * p_ell
// where p_ell is the abelianization of the SIBLING LETTERS BEFORE the
// chosen child at that level (i.e. images[letter][0 .. chosen_idx)),
// and M is the incidence matrix (M[i][j] = count of letter i in
// sigma(j)). This is exact integer-vector arithmetic, no rounding, no
// asymptotics -- verified to reproduce the true prefix abelianization
// bit-for-bit in the accompanying test.
//
// CONSEQUENCE (the actual reduction). At any level where sigma(letter)
// has ONLY ONE occurrence structure such that the chosen child is
// always the first (e.g. any letter whose image has no room for a
// "second or later" choice -- in particular any letter with an image
// of length 1, which forces chosen_idx=0 unconditionally, hence
// p_ell = the empty abelianization, hence a ZERO term regardless of
// the matrix power), the term contributes NOTHING to the sum. Only
// levels where the current letter has a genuinely nonzero "prefix
// before the chosen child" (i.e. a letter with image length >= 2,
// AND the walk chooses something other than the very first child)
// can contribute at all.
//
// For sigma_{0,1} specifically: only letter 0 has an image of length
// >= 2 (sigma(0)=[1,2]; sigma(1)=[2] and sigma(2)=[0] both have length
// 1). So the ENTIRE abelianization of any prefix, for any walk, from
// any starting letter, reduces to a sum over a SUBSET of levels
// (exactly the levels where the walk is "at letter 0 and picks the
// second child") of the single fixed "landmark vector"
// v0 = P([1]) = (0,1,0) (the abelianization of the one-letter
// sibling-list before choosing child index 1 in sigma(0)=[1,2]),
// transformed by M^{ell-1}.
//
// So strong coincidence between two starting letters b1, b2 (past the
// trivial boundary case) reduces EXACTLY to: does there exist a
// choice of two finite subsets S1, S2 of positive integers (the
// "letter-0-picks-second-child" levels along each walk) such that
//   sum_{ell in S1} M^{ell-1} v0  =  sum_{ell in S2} M^{ell-1} v0 ?
// This is now a genuine, well-posed, exact finite-linear-algebra
// question -- not a vague analogy to Diophantine approximation, an
// ACTUAL instance of it: do two subset-sums of a single vector's orbit
// under repeated application of an integer matrix collide? This is
// the honest, precise open question this header hands forward. No
// closed-form answer is claimed here -- what is established is the
// REDUCTION itself, verified exact, replacing "why does coincidence
// eventually happen" with the much sharper "when do two subset-sums
// of {M^n v0} collide", which is a real, nameable, attackable
// question (linear recurrence sequences over Z, related in spirit to
// vanishing sums of S-units / the Skolem-Mahler-Lech theorem's
// territory, though not identical to it).

#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace ravel::proof {

template <std::size_t d>
using LandmarkVec = std::array<long long, d>;

template <std::size_t d>
inline LandmarkVec<d> landmark_add(const LandmarkVec<d>& a, const LandmarkVec<d>& b) {
    LandmarkVec<d> r{};
    for (std::size_t i = 0; i < d; ++i) r[i] = a[i] + b[i];
    return r;
}

template <std::size_t d>
inline LandmarkVec<d> landmark_matvec(const std::array<std::array<long long, d>, d>& M, const LandmarkVec<d>& v) {
    LandmarkVec<d> r{};
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = 0; j < d; ++j) r[i] += M[i][j] * v[j];
    return r;
}

// One walk-step: the letter at this level, and the index of the child
// actually chosen (indexing into that letter's image).
struct WalkStep {
    long long letter;
    long long chosen_child_index;  // -1 for the terminal (depth-0) leaf
};

// Exact reconstruction of a prefix's abelianization from its walk,
// via the matrix-power-weighted sum -- the theorem stated above, as
// executable code. `walk[0]` is the TOP level (largest remaining
// depth); `walk.back()` is the depth-0 leaf (chosen_child_index=-1,
// skipped). `images[letter]` gives that letter's substitution image.
template <std::size_t d>
inline LandmarkVec<d> abelianization_from_walk(
    const std::vector<WalkStep>& walk,
    const std::array<std::vector<long long>, d>& images,
    const std::array<std::array<long long, d>, d>& M) {
    LandmarkVec<d> total{};
    const long long depth_top = static_cast<long long>(walk.size()) - 1;  // walk.back() is the depth-0 leaf
    for (std::size_t level = 0; level < walk.size(); ++level) {
        const auto& step = walk[level];
        if (step.chosen_child_index < 0) continue;  // the leaf, no substitution power to apply
        LandmarkVec<d> sibling_prefix{};
        for (long long t = 0; t < step.chosen_child_index; ++t)
            sibling_prefix[static_cast<std::size_t>(images[static_cast<std::size_t>(step.letter)][static_cast<std::size_t>(t)])] += 1;
        const long long depth_here = depth_top - static_cast<long long>(level);
        LandmarkVec<d> term = sibling_prefix;
        for (long long p = 0; p < depth_here - 1; ++p) term = landmark_matvec<d>(M, term);
        total = landmark_add<d>(total, term);
    }
    return total;
}

}  // namespace ravel::proof
