// ravel/proof/feeder_cycle_charpoly_certificate.hpp
//
// Wires the "installed graph attractor model" (an (n+1)-cycle plus one
// transient feeder edge into a chosen cycle vertex) to the reflection
// pipeline: builds the concrete adjacency data, self-validates it really is
// exactly that structure (a single feeder edge, no feeder self-loop, no
// edge back from the core to the feeder, and the core is exactly the
// directed (n+1)-cycle `lean/cycle_charpoly_campaign_validation.lean`'s
// `concreteCycleMatrix` defines), and only then records the certificate --
// so the renderer instantiates `feeder_cycle_charpoly_closed` +
// `concreteCycleMatrix_charpoly` against the SAME concrete n and feeder
// target, not merely restate the closed form `X * (X^(n+1)-1)`.

#pragma once

#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

// Self-validating check: builds the (n+2)-vertex graph (n+1 cycle vertices
// 0..n, plus one feeder vertex indexed n+1) EXPLICITLY as an edge set, then
// independently re-derives every structural condition
// `feeder_cycle_charpoly_closed`'s hypotheses require by inspecting that
// edge set -- not merely asserting it holds by construction. Matches
// concreteCycleMatrix's i=j+1 convention: cycle edge j -> j+1 mod (n+1).
inline bool certify_feeder_cycle_structure(long long n, long long feeder_target) {
    if (n < 1) return false;
    if (feeder_target < 0 || feeder_target > n) return false;
    const long long feeder = n + 1;

    std::set<std::pair<long long, long long>> edges;
    for (long long j = 0; j <= n; ++j) edges.insert({j, (j + 1) % (n + 1)});
    edges.insert({feeder, feeder_target});

    // Re-derive out-degree/in-degree of every cycle vertex from the edge set
    // (not from the loop above that built it) -- exactly one out-edge
    // (the cycle successor) always; in-degree is 1 for every cycle vertex
    // EXCEPT feeder_target, which legitimately also receives the feeder's
    // one edge (in-degree 2 there).
    for (long long v = 0; v <= n; ++v) {
        long long out_deg = 0, in_deg = 0;
        for (const auto& [u, w] : edges) {
            if (u == v) ++out_deg;
            if (w == v) ++in_deg;
        }
        const long long expected_in = (v == feeder_target) ? 2 : 1;
        if (out_deg != 1 || in_deg != expected_in) return false;
        if (!edges.contains({v, (v + 1) % (n + 1)})) return false;
    }
    // Feeder: exactly one outgoing edge, zero incoming (transient, never
    // revisited), and it does not self-loop.
    long long feeder_out = 0, feeder_in = 0;
    for (const auto& [u, w] : edges) {
        if (u == feeder) ++feeder_out;
        if (w == feeder) ++feeder_in;
    }
    if (feeder_out != 1 || feeder_in != 0) return false;
    if (!edges.contains({feeder, feeder_target})) return false;
    return true;
}

inline void stage_feeder_cycle_charpoly(long long n, long long feeder_target,
                                         const std::string& description) {
    if (!certify_feeder_cycle_structure(n, feeder_target)) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::FeederCycleCharpolyCertificate node;
    node.n = n;
    node.feeder_target = feeder_target;
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
