#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

/** A finite relation representing one complete traversal of a recurrent base
 * cycle.  An arc u -> v means that the controller can traverse one lap from
 * controller state u to controller state v while preserving the required
 * face-alignment invariant. */
struct OneLapControllerRelation {
    std::size_t state_count = 0;
    std::vector<std::vector<std::size_t>> successors;
};

struct PeriodicControllerOrbitProof {
    std::size_t initial_state = 0;
    std::size_t transient_laps = 0;
    std::size_t period_laps = 0;
    std::vector<std::size_t> orbit;
    bool relation_replay = false;
    bool closes = false;
    bool valid = false;
    std::string failure;
};

/**
 * Derive a periodic controller orbit from a finite serial relation.
 *
 * This is the exact graph-theoretic splice operation needed by the shell-pump
 * proof.  It does not require a one-lap fixed point.  Starting from any state
 * in the invariant fiber, choose one legal successor per lap.  Finiteness
 * forces a repeated controller state; the segment between repetitions is a
 * closed controller run over a positive number of repeated base-cycle laps.
 */
inline PeriodicControllerOrbitProof derive_periodic_controller_orbit(
    const OneLapControllerRelation& relation,
    std::size_t initial_state = 0) {
    PeriodicControllerOrbitProof proof;
    proof.initial_state = initial_state;
    if (relation.state_count == 0 ||
        relation.successors.size() != relation.state_count) {
        proof.failure = "malformed finite one-lap relation";
        return proof;
    }
    if (initial_state >= relation.state_count) {
        proof.failure = "initial controller state is out of range";
        return proof;
    }
    for (std::size_t s = 0; s < relation.state_count; ++s) {
        if (relation.successors[s].empty()) {
            proof.failure = "one-lap relation is not serial on the invariant fiber";
            return proof;
        }
        for (const auto t : relation.successors[s]) {
            if (t >= relation.state_count) {
                proof.failure = "one-lap relation contains an out-of-range target";
                return proof;
            }
        }
    }

    std::map<std::size_t, std::size_t> first_seen;
    std::size_t current = initial_state;
    while (!first_seen.contains(current)) {
        first_seen.emplace(current, proof.orbit.size());
        proof.orbit.push_back(current);
        // Canonical deterministic choice.  The proof only uses membership in
        // the original relation, so no mathematical claim depends on ordering.
        current = relation.successors[current].front();
    }
    const auto cycle_begin = first_seen.at(current);
    proof.orbit.push_back(current);
    proof.transient_laps = cycle_begin;
    proof.period_laps = proof.orbit.size() - 1 - cycle_begin;
    proof.relation_replay = true;
    for (std::size_t i = 0; i + 1 < proof.orbit.size(); ++i) {
        const auto& out = relation.successors[proof.orbit[i]];
        proof.relation_replay &=
            std::find(out.begin(), out.end(), proof.orbit[i + 1]) != out.end();
    }
    proof.closes = proof.period_laps > 0 &&
        proof.orbit[cycle_begin] == proof.orbit.back();
    proof.valid = proof.relation_replay && proof.closes;
    if (!proof.valid) proof.failure = "failed to derive a positive periodic orbit";
    return proof;
}

inline std::string render_periodic_controller_orbit_report(
    const PeriodicControllerOrbitProof& proof) {
    return "PERIODIC_CONTROLLER_ORBIT\n"
        "initial_state=" + std::to_string(proof.initial_state) + "\n"
        "transient_laps=" + std::to_string(proof.transient_laps) + "\n"
        "period_laps=" + std::to_string(proof.period_laps) + "\n"
        "relation_replay=" + std::string(proof.relation_replay ? "true" : "false") + "\n"
        "closes=" + std::string(proof.closes ? "true" : "false") + "\n"
        "valid=" + std::string(proof.valid ? "true" : "false") + "\n"
        "failure=" + proof.failure + "\n";
}

// Stages a `CyclicSpliceCompletionReflectionCertificate` -- gates on
// `proof.valid` AND an independent recomputation of the deterministic
// "always pick the first successor" iteration from `relation` (not
// trusting the search that originally found the repeat), checking the
// same repeat C++ found genuinely holds under that recomputed function
// before staging.
inline void stage_cyclic_splice_completion(
        const OneLapControllerRelation& relation,
        const PeriodicControllerOrbitProof& proof,
        const std::string& description) {
    if (!proof.valid) return;
    std::vector<long long> next;
    for (const auto& s : relation.successors) {
        if (s.empty()) return;
        next.push_back(static_cast<long long>(s.front()));
    }
    std::size_t cur = proof.initial_state;
    for (std::size_t i = 0; i < proof.transient_laps; ++i)
        cur = static_cast<std::size_t>(next[cur]);
    const std::size_t at_transient = cur;
    for (std::size_t i = 0; i < proof.period_laps; ++i)
        cur = static_cast<std::size_t>(next[cur]);
    if (cur != at_transient) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::CyclicSpliceCompletionReflectionCertificate node;
    node.state_count = static_cast<long long>(relation.state_count);
    node.deterministic_next = next;
    node.initial_state = static_cast<long long>(proof.initial_state);
    node.transient_laps = static_cast<long long>(proof.transient_laps);
    node.period_laps = static_cast<long long>(proof.period_laps);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
