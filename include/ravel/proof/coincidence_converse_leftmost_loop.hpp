// ravel/proof/coincidence_converse_leftmost_loop.hpp
//
// A first, PROVABLE slice of the converse left open by Finding 26 (does
// every compatible-gap pair -- gap = 0 mod g -- eventually coincide?).
// Not the full converse. A genuine sub-case of it, closed rigorously.
//
// OBSERVATION (found by driving the existing, already-verified
// CoincidenceClosure at gaps it hadn't been pointed at before, per
// project practice of using the tool to explain rather than searching
// blindly): sweeping compatible gaps on the Finding-26 multi-junction
// example (g=2) showed gap in {0,4,8,12} coincide at EXACTLY K=gap,
// while gap in {2,6,10} need K=gap+10. The K=gap cases are not a
// coincidence of search order -- they have an exact structural reason.
//
// THE LEFTMOST LOOP. child_index=0 at any junction always carries the
// zero landmark vector (it is the abelianization of siblings BEFORE the
// chosen child; child 0 has none). So the walk that always takes
// child_index=0 -- "always take the leftmost branch" -- accumulates the
// zero vector at every step, by construction, regardless of which
// junctions it passes through. This walk is DETERMINISTIC (each
// junction has a unique child_index=0), so starting from any junction J
// over a finite junction graph, it is eventually periodic. If J itself
// lies on its own eventual cycle, that cycle is a closed walk from J
// back to J using only zero-landmark edges; call its total jump-size
// length L (necessarily a multiple of g, being a sum of jump sizes).
//
// COROLLARY (proved, not just observed). For any junction J on its own
// leftmost cycle of length L, and for every non-negative integer m, the
// pair with gap = mL coincides at EXACTLY K = mL: repeating the
// leftmost cycle m times is a walk of depth mL from J back to J
// contributing the zero vector throughout, i.e. reachable(J, mL)
// contains (J, zero_vector) = the unique element of reachable(J, 0).
// That is precisely the K=gap witness condition. No search needed --
// this is a direct construction.
//
// SCOPE, honestly: this covers exactly the sub-lattice of gaps that are
// multiples of L (which itself is a multiple of g, but may be a proper
// multiple -- L=4 vs g=2 in the example above). It says nothing about
// gaps that are compatible (0 mod g) but not multiples of L (gap=2 in
// the example, which needed a real search and landed at K=12, not the
// naive K=2). The full converse -- EVERY compatible gap, not just
// multiples of the leftmost cycle -- remains open.

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

#include "ravel/proof/coincidence_closure.hpp"

namespace ravel::proof {

// Follows the deterministic "always take child_index=0" walk from
// junction `start`, over the junction graph `edges`, looking for the
// first return to `start`. Returns the closed loop's total jump-size
// length if `start` lies on its own eventual leftmost cycle within
// `max_steps` junction-hops, else nullopt (start is only in the
// transient, not on the cycle itself).
template <std::size_t d>
inline std::optional<long long> leftmost_loop_length(const std::vector<JunctionEdge<d>>& edges,
                                                        long long start, long long max_steps = 10000) {
    long long current = start;
    long long total = 0;
    for (long long step = 0; step < max_steps; ++step) {
        const JunctionEdge<d>* chosen = nullptr;
        for (const auto& e : edges) {
            if (e.from_junction == current && e.child_index == 0) { chosen = &e; break; }
        }
        if (!chosen) return std::nullopt;  // shouldn't happen for a real junction
        total += chosen->jump_size;
        current = chosen->to_junction;
        if (current == start) return total;
    }
    return std::nullopt;
}

}  // namespace ravel::proof
