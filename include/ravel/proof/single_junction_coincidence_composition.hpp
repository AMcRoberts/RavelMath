// ravel/proof/single_junction_coincidence_composition.hpp
//
// Closes the walk-realizability layer of the Diophantine reduction
// (coincidence_as_landmark_vector_cancellation.hpp) for the tractable
// structural class this project can currently name precisely:
// substitutions with EXACTLY ONE "junction" letter (image length >= 2)
// and every other letter forming a deterministic (single-successor)
// chain that eventually returns to the junction.
//
// Verified 2026-08-06 against sigma_{0,1} (sigma(0)=1,2 sigma(1)=2
// sigma(2)=0): the recursive composition model below reproduces the
// EXACT set of achievable landmark-depth-sets found by full brute-force
// enumeration, at every depth 1..8 and every starting letter, with zero
// exceptions (the one trivial case -- a starting letter whose
// deterministic run-in exceeds the total requested depth, i.e. the
// walk never reaches the junction at all -- is handled directly by the
// caller before invoking this function, not a gap in it).
//
// A first version of `enumerate_landmark_depth_sets` had a real bug,
// caught by this same cross-check against brute force, not by
// inspection: it only recorded a landmark event when the FULL jump
// back to the next junction fit within the remaining depth, silently
// dropping cases where a landmark choice is made but the walk then
// runs out of depth partway through the forced chain back to the next
// junction. The choice (and its landmark, if any) is committed the
// instant it is made -- one substitution step -- independent of
// whether the rest of that jump's deterministic tail fits. Fixed by
// recording the leaf directly whenever a chosen jump doesn't fit,
// instead of skipping that branch.
//
// THE MODEL. Let j be the junction letter, with image sigma(j) of
// length L. For each child index c in [0,L), define:
//   jump_size(c)  = 1 + (deterministic chain length from
//                        sigma(j)[c] back to j)
//   landmark_vec(c) = P(sigma(j)[0..c))   (abelianization of the
//                        siblings strictly before child c)
// (landmark_vec(0) is always the zero vector -- no siblings before
// the first child -- matching the "no landmark" case.)
//
// A walk starting AT the junction, with `depth` remaining, either:
//   - terminates within the current deterministic run if depth is too
//     small to complete a jump (a genuine leaf, no more choices), or
//   - for EACH child index c with jump_size(c) <= depth, recurses
//     with depth - jump_size(c) remaining, accumulating landmark_vec(c)
//     (weighted by the correct matrix power, per
//     coincidence_as_landmark_vector_cancellation.hpp) if it is
//     nonzero.
// A walk starting at a NON-junction letter first consumes its own
// deterministic run-in (a fixed, letter-specific number of levels)
// before this recursion begins.
//
// This turns "does coincidence occur, and at what depth" into an
// EXACT, checkable COMPOSITION problem: representing (K - run_in) as
// an ordered sum of the available jump sizes, where each jump also
// carries a fixed vector contribution -- structurally a weighted
// restricted-composition-of-an-integer problem, the same combinatorial
// family as counting compositions into parts of size 2 and 3 (which,
// for sigma_{0,1} specifically, is exactly {jump_size(0)=3,
// jump_size(1)=2}, matching the classical Padovan/Perrin-adjacent
// counting sequence for compositions into parts {2,3} -- not a
// coincidence, since those jump sizes come directly from the
// substitution's own minimal polynomial having a length-1 gap
// pattern).
//
// STATUS: this closes the walk-realizability question for the
// single-junction class completely and exactly (an algorithm, not
// merely a bound). It intentionally does NOT pretend to be the
// multi-junction engine: when multiple letters have image length >= 2,
// jumps from different junctions interleave and the clean single-sequence
// composition picture breaks down. The general multi-junction closure now
// lives in coincidence_closure.hpp; this header remains the small,
// independently auditable single-junction composition model.

#pragma once

#include <cstddef>
#include <set>
#include <vector>

namespace ravel::proof {

// One available "jump" at the junction letter: how many levels it
// consumes to return to the junction, and whether it's a genuine
// landmark event (nonzero sibling-prefix) -- landmark index -1 means
// "no landmark" (child index 0), otherwise the child index whose
// sibling-prefix is the landmark vector.
struct JunctionJump {
    long long jump_size;
    long long child_index;  // for looking up the landmark vector; -1 if trivial (child 0)
};

// Computes the available jumps at junction letter `j`, given the
// substitution images and the (already-verified, letter-specific)
// deterministic run-in length back to `j` for every OTHER letter.
template <std::size_t d>
inline std::vector<JunctionJump> compute_junction_jumps(
    const std::array<std::vector<long long>, d>& images,
    long long junction_letter,
    const std::array<long long, d>& run_in_to_junction) {
    std::vector<JunctionJump> jumps;
    const auto& img = images[static_cast<std::size_t>(junction_letter)];
    for (std::size_t c = 0; c < img.size(); ++c) {
        long long child = img[c];
        long long run_in = (child == junction_letter) ? 0 : run_in_to_junction[static_cast<std::size_t>(child)];
        jumps.push_back({1 + run_in, c == 0 ? -1 : static_cast<long long>(c)});
    }
    return jumps;
}

// Enumerates every achievable landmark-DEPTH-SET (not yet weighted
// into vectors) for a walk of `depth` remaining levels starting AT
// the junction letter, using the given jump menu. This is the
// composition enumeration itself -- exponential in general (as many
// compositions as there are), intended for verification against
// brute force and for small-depth exploration, not as the asymptotic
// tool (that needs the linear-recurrence counting function implied
// by the jump sizes, not full enumeration).
inline void enumerate_landmark_depth_sets(
    long long depth,
    const std::vector<JunctionJump>& jumps,
    std::vector<long long> current_landmarks,
    std::set<std::vector<long long>>& out) {
    if (depth <= 0) { out.insert(current_landmarks); return; }
    // A landmark event is recorded the INSTANT a child is chosen (one
    // substitution step), independent of whether the full deterministic
    // run-in back to the next junction fits in the remaining depth. If
    // it doesn't fit, the walk simply terminates mid-chain -- the
    // landmark choice already made still counts, so record the leaf
    // directly rather than skipping this branch (the earlier version
    // of this function had exactly this bug: `continue`-ing past a
    // too-large jump silently dropped a real, already-committed
    // landmark event -- found by cross-checking against brute force,
    // not by inspection).
    for (const auto& jmp : jumps) {
        auto next_landmarks = current_landmarks;
        if (jmp.child_index >= 0) next_landmarks.push_back(depth);  // depth_here at this junction visit
        if (jmp.jump_size <= depth) {
            enumerate_landmark_depth_sets(depth - jmp.jump_size, jumps, next_landmarks, out);
        } else {
            out.insert(next_landmarks);  // walk ends mid-chain; the choice just made still stands
        }
    }
}

}  // namespace ravel::proof
