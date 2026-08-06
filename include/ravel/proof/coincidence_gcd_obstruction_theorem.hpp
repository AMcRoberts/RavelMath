// ravel/proof/coincidence_gcd_obstruction_theorem.hpp
//
// Closes both open questions left by Finding 25 in one proof: the
// gcd-obstruction is rigorous (not just observed to depth 40), and it
// extends to multi-junction substitutions unchanged.
//
// SETUP. For any letter x in the alphabet, define dist(x) = the number
// of forced (deterministic, single-successor) steps from x to the
// nearest junction letter (0 if x is itself a junction). This is
// well-defined for every letter: junction letters trivially have
// dist=0; every other letter has a UNIQUE forward path (image length
// exactly 1) that must eventually reach some junction (guaranteed by
// irreducibility -- see build_junction_graph's own assertion).
//
// Let g = gcd of EVERY jump size in the WHOLE junction graph -- across
// ALL junctions, not just one.
//
// THEOREM. For any state (terminal, vector) in reachable(junction, d)
// (coincidence_closure.hpp's own closure), d + dist(terminal) = 0 (mod g).
//
// PROOF (induction on d; this is what was missing from Finding 25 --
// the mid-chain-cutoff case, worked through explicitly, not skipped).
//
//   Base case (d = 0): terminal = junction itself (the closure's own
//   base case). dist(junction) = 0 by definition. 0 + 0 = 0 (mod g). OK.
//
//   Clean edge (jump_size s <= d, s = 0 mod g since s is one of the
//   jump sizes whose gcd is g): recurses into reachable(target, d-s).
//   By the inductive hypothesis, for any (term, vec) from that
//   recursive call, (d-s) + dist(term) = 0 (mod g). Since s = 0 (mod g),
//   d + dist(term) = (d-s) + dist(term) + s = 0 + 0 = 0 (mod g). OK.
//   (This step never used whether the edge's target junction equals
//   the source junction -- it holds identically whether the walk
//   stays at one junction or crosses to a different one, which is
//   exactly why the theorem needs no extra argument for the
//   multi-junction case.)
//
//   Mid-chain cutoff (jump_size s > d): the walk commits to this edge's
//   choice but runs out of depth partway through the forced chain
//   after it. The terminal is chain[d-1] (0-indexed; valid since
//   d-1 < s-1 = chain.size()-1 when s > d). By construction of the
//   chain, dist(chain[i]) = (s-1) - i for each i (chain[s-1] is the
//   junction itself, dist 0; chain[0] is the chosen child, dist s-1).
//   So dist(terminal) = dist(chain[d-1]) = (s-1) - (d-1) = s - d.
//   Then d + dist(terminal) = d + (s-d) = s = 0 (mod g), directly,
//   since s is by definition a multiple of g. OK.
//
// All three cases close the induction. QED.
//
// CONSEQUENCE (the gcd-obstruction, now proven not observed). If
// reachable(0, K-r_a) and reachable(0, K-r_b) share a common state,
// applying the theorem to that SAME state at both depths gives
// (K-r_a)+dist(term) = 0 (mod g) and (K-r_b)+dist(term) = 0 (mod g).
// Subtracting: r_a - r_b = 0 (mod g). So a pair whose run-in gap is
// NOT divisible by g can NEVER share a state, for ANY K, full stop --
// not "not found within a search bound".
//
// SCOPE, stated honestly: this proves incompatible-gap pairs can NEVER
// coincide. It does NOT prove that compatible-gap pairs (gap = 0 mod g)
// always DO eventually coincide -- that direction remains empirical
// (strongly supported by every case checked, not derived here). The
// theorem is a genuine impossibility result on one side; the other
// side is still open.
//
// VERIFIED computationally (not just derived): the invariant itself
// (not merely its consequence) checked directly against real closures,
// zero violations, on both a single-junction case (gcd=1, vacuous but
// consistent) and a genuine multi-junction case (two junctions, edges
// crossing between them, gcd=2) -- see the accompanying test. Run
// under an explicit memory/time cap; the check only needs a few
// hundred states to be conclusive, since this is verifying an exact
// algebraic identity, not searching for a witness.

#pragma once

#include <array>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace ravel::proof {

// dist(letter): forced deterministic steps to the nearest junction
// (0 if `letter` is itself a junction). Throws if a non-junction
// letter's image length isn't exactly 1, or if no junction is ever
// reached (matching build_junction_graph's own irreducibility check).
template <std::size_t d>
inline long long letter_distance_to_junction(const std::array<std::vector<long long>, d>& images, long long letter) {
    auto is_junction = [&](long long l) { return images[static_cast<std::size_t>(l)].size() >= 2; };
    long long steps = 0;
    while (!is_junction(letter)) {
        if (images[static_cast<std::size_t>(letter)].size() != 1) {
            throw std::runtime_error("letter_distance_to_junction: non-junction letter must have image length 1");
        }
        letter = images[static_cast<std::size_t>(letter)][0];
        ++steps;
        if (steps > 100000) throw std::runtime_error("letter_distance_to_junction: no junction reached (not irreducible?)");
    }
    return steps;
}

}  // namespace ravel::proof
