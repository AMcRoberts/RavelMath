#pragma once

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/cyclic_splice_completion.hpp"

namespace ravel::proof {

struct SegmentControllerRelation {
    std::size_t state_count = 0;
    std::vector<std::vector<std::size_t>> successors;
    bool replay_checked = false;
};

struct RecurrentLapSpliceCertificate {
    std::size_t state_count = 0;
    std::size_t segment_count = 0;
    std::vector<std::size_t> invariant_fiber;
    OneLapControllerRelation one_lap;
    PeriodicControllerOrbitProof periodic_orbit;
    bool segments_well_formed = false;
    bool fiber_preserved = false;
    bool every_segment_serial_on_fiber = false;
    bool composition_replay = false;
    bool one_lap_serial_on_fiber = false;
    bool valid = false;
    std::string obstruction;
};

inline std::set<std::size_t> relation_image(
    const SegmentControllerRelation& relation,
    const std::set<std::size_t>& sources) {
    std::set<std::size_t> out;
    for (const auto source : sources) {
        if (source >= relation.state_count) continue;
        out.insert(relation.successors[source].begin(),
                   relation.successors[source].end());
    }
    return out;
}

/** Compose exact controller relations for the consecutive first-return
 * segments of one recurrent source lap.  Seriality on a common finite
 * invariant fiber is preserved by relational composition, and the resulting
 * one-lap serial relation has a positive periodic orbit by finite recurrence.
 */
inline RecurrentLapSpliceCertificate derive_recurrent_lap_splice(
    std::vector<SegmentControllerRelation> segments,
    std::vector<std::size_t> invariant_fiber,
    std::size_t initial_state = 0) {
    RecurrentLapSpliceCertificate cert;
    cert.segment_count = segments.size();
    cert.invariant_fiber = std::move(invariant_fiber);
    if (segments.empty()) {
        cert.obstruction = "no segment relations supplied";
        return cert;
    }
    cert.state_count = segments.front().state_count;
    cert.segments_well_formed = cert.state_count > 0;
    for (const auto& segment : segments) {
        cert.segments_well_formed &= segment.replay_checked;
        cert.segments_well_formed &= segment.state_count == cert.state_count;
        cert.segments_well_formed &=
            segment.successors.size() == cert.state_count;
        if (segment.successors.size() == cert.state_count) {
            for (const auto& out : segment.successors)
                for (const auto target : out)
                    cert.segments_well_formed &= target < cert.state_count;
        }
    }
    if (!cert.segments_well_formed) {
        cert.obstruction = "malformed or unreplayed segment relation";
        return cert;
    }

    std::sort(cert.invariant_fiber.begin(), cert.invariant_fiber.end());
    cert.invariant_fiber.erase(
        std::unique(cert.invariant_fiber.begin(), cert.invariant_fiber.end()),
        cert.invariant_fiber.end());
    if (cert.invariant_fiber.empty() ||
        cert.invariant_fiber.back() >= cert.state_count) {
        cert.obstruction = "invalid invariant fiber";
        return cert;
    }
    const std::set<std::size_t> fiber_set(
        cert.invariant_fiber.begin(), cert.invariant_fiber.end());

    cert.fiber_preserved = true;
    cert.every_segment_serial_on_fiber = true;
    for (const auto& segment : segments) {
        for (const auto source : cert.invariant_fiber) {
            bool has_fiber_successor = false;
            for (const auto target : segment.successors[source]) {
                cert.fiber_preserved &= fiber_set.contains(target);
                has_fiber_successor |= fiber_set.contains(target);
            }
            cert.every_segment_serial_on_fiber &= has_fiber_successor;
        }
    }
    if (!cert.fiber_preserved || !cert.every_segment_serial_on_fiber) {
        cert.obstruction = !cert.fiber_preserved
            ? "segment relation exits the invariant fiber"
            : "segment relation is not serial on the invariant fiber";
        return cert;
    }

    cert.one_lap.state_count = cert.state_count;
    cert.one_lap.successors.assign(cert.state_count, {});
    cert.composition_replay = true;
    for (const auto source : cert.invariant_fiber) {
        std::set<std::size_t> current{source};
        for (const auto& segment : segments) {
            current = relation_image(segment, current);
            std::set<std::size_t> restricted;
            for (const auto target : current)
                if (fiber_set.contains(target)) restricted.insert(target);
            current = std::move(restricted);
            cert.composition_replay &= !current.empty();
        }
        cert.one_lap.successors[source].assign(current.begin(), current.end());
    }

    cert.one_lap_serial_on_fiber = cert.composition_replay;
    for (const auto source : cert.invariant_fiber)
        cert.one_lap_serial_on_fiber &=
            !cert.one_lap.successors[source].empty();
    if (!cert.one_lap_serial_on_fiber) {
        cert.obstruction = "composed one-lap relation is not serial";
        return cert;
    }

    // Restrict and renumber the fiber for the generic finite periodic-orbit
    // theorem; this avoids imposing seriality on irrelevant controller states.
    OneLapControllerRelation restricted;
    restricted.state_count = cert.invariant_fiber.size();
    restricted.successors.assign(restricted.state_count, {});
    for (std::size_t i = 0; i < cert.invariant_fiber.size(); ++i) {
        const auto source = cert.invariant_fiber[i];
        for (const auto target : cert.one_lap.successors[source]) {
            const auto it = std::lower_bound(cert.invariant_fiber.begin(),
                                             cert.invariant_fiber.end(), target);
            if (it != cert.invariant_fiber.end() && *it == target)
                restricted.successors[i].push_back(
                    static_cast<std::size_t>(it - cert.invariant_fiber.begin()));
        }
    }
    const auto initial_it = std::lower_bound(cert.invariant_fiber.begin(),
                                             cert.invariant_fiber.end(), initial_state);
    const auto restricted_initial =
        initial_it != cert.invariant_fiber.end() && *initial_it == initial_state
            ? static_cast<std::size_t>(initial_it - cert.invariant_fiber.begin())
            : std::size_t{0};
    cert.periodic_orbit = derive_periodic_controller_orbit(
        restricted, restricted_initial);
    cert.valid = cert.periodic_orbit.valid;
    if (!cert.valid) cert.obstruction = cert.periodic_orbit.failure;
    return cert;
}

} // namespace ravel::proof
