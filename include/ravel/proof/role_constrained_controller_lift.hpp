#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/cyclic_splice_compactness.hpp"
#include "ravel/proof/cyclic_continuation_controller.hpp"
#include "ravel/proof/first_return_joint_product.hpp"
#include "ravel/proof/stepped_face_role_junction.hpp"

namespace ravel::proof {

struct RoleConstrainedControllerLiftCertificate {
    std::size_t dimension = 0;
    std::vector<std::int64_t> lap_word;
    std::vector<SteppedFaceRole> lap_roles;
    OneLapControllerRelation repeated_lap_relation;
    CyclicOneLapRelationProof controller_cycle;
    CyclicRunCertificate controller_run;
    bool roles_well_formed = false;
    bool restricted_relation_replayed = false;
    bool junction_roles_replayed = false;
    bool digit_path_replayed = false;
    bool valid = false;
    std::string obstruction;
};

namespace detail {

inline bool controller_has_role(const SymbolicControllerState& state,
                                const SteppedFaceRole& role) {
    return role.first < state.size() && state[role.first] == role.second;
}

inline std::vector<std::int64_t> concatenate_words(
    const std::vector<FirstReturnWitness>& segments) {
    std::vector<std::int64_t> word;
    for (const auto& segment : segments)
        word.insert(word.end(), segment.digits.begin(), segment.digits.end());
    return word;
}

inline std::set<std::size_t> image_under_word_restricted(
    const ControllerPlant& plant,
    const std::set<std::size_t>& initial,
    const std::vector<std::int64_t>& word) {
    std::set<std::size_t> current = initial;
    for (const auto digit : word) {
        std::set<std::size_t> next;
        for (const auto state : current) {
            const auto it = plant.successors.find({state, digit});
            if (it != plant.successors.end())
                next.insert(it->second.begin(), it->second.end());
        }
        current = std::move(next);
        if (current.empty()) break;
    }
    return current;
}

inline bool reconstruct_word_path(
    const ControllerPlant& plant,
    const std::vector<std::int64_t>& word,
    std::size_t source,
    std::size_t target,
    std::vector<std::size_t>& path) {
    std::vector<std::set<std::size_t>> reachable(word.size() + 1);
    std::vector<std::map<std::size_t, std::size_t>> parent(word.size() + 1);
    reachable[0].insert(source);
    for (std::size_t k = 0; k < word.size(); ++k) {
        for (const auto state : reachable[k]) {
            const auto it = plant.successors.find({state, word[k]});
            if (it == plant.successors.end()) continue;
            for (const auto next : it->second) {
                if (!parent[k + 1].contains(next))
                    parent[k + 1][next] = state;
                reachable[k + 1].insert(next);
            }
        }
    }
    if (!reachable.back().contains(target)) return false;
    path.assign(word.size() + 1, 0);
    path.back() = target;
    for (std::size_t k = word.size(); k > 0; --k)
        path[k - 1] = parent[k].at(path[k]);
    return path.front() == source;
}

} // namespace detail

/**
 * Lift a positive cycle in the stepped-face role relation to the exact finite
 * controller-state relation selected by those roles.
 *
 * `lap_roles[k]` is the active signed face required at the beginning of lap k;
 * the final lap returns to `lap_roles.front()`.  The operation does not infer
 * controller closure from role closure.  It constructs the restricted
 * controller relation, searches that relation for a positive cycle, and then
 * reconstructs every digit-level controller transition.
 */
inline RoleConstrainedControllerLiftCertificate
 derive_role_constrained_controller_lift(
    const FirstReturnJointProduct& product,
    const std::vector<FirstReturnWitness>& source_lap_segments,
    std::vector<SteppedFaceRole> lap_roles) {
    RoleConstrainedControllerLiftCertificate cert;
    cert.dimension = product.dimension;
    cert.lap_roles = std::move(lap_roles);
    cert.lap_word = detail::concatenate_words(source_lap_segments);

    if (!product.replayed || product.controller_states.empty()) {
        cert.obstruction = "controller product did not replay";
        return cert;
    }
    if (source_lap_segments.empty() || cert.lap_word.empty() ||
        cert.lap_roles.empty()) {
        cert.obstruction = "lap segments, word, and roles must be nonempty";
        return cert;
    }
    cert.roles_well_formed = true;
    for (const auto& role : cert.lap_roles) {
        cert.roles_well_formed &= role.first < cert.dimension;
        cert.roles_well_formed &= role.second == -1 || role.second == 1;
    }
    if (!cert.roles_well_formed) {
        cert.obstruction = "malformed stepped-face role";
        return cert;
    }

    // Each state of the derived relation is a literal controller state at the
    // beginning of the full repeated role lap.  An edge traverses one complete
    // role-cycle of source laps while satisfying every junction role.
    cert.repeated_lap_relation.state_count = product.controller_states.size();
    cert.repeated_lap_relation.successors.assign(
        cert.repeated_lap_relation.state_count, {});
    cert.restricted_relation_replayed = true;

    for (std::size_t source = 0; source < product.controller_states.size(); ++source) {
        if (!detail::controller_has_role(product.controller_states[source],
                                         cert.lap_roles.front()))
            continue;
        std::set<std::size_t> current{source};
        for (std::size_t lap = 0; lap < cert.lap_roles.size(); ++lap) {
            current = detail::image_under_word_restricted(
                product.controller_plant, current, cert.lap_word);
            const auto& next_role = cert.lap_roles[(lap + 1) % cert.lap_roles.size()];
            for (auto it = current.begin(); it != current.end();) {
                if (!detail::controller_has_role(product.controller_states[*it], next_role))
                    it = current.erase(it);
                else
                    ++it;
            }
            if (current.empty()) break;
        }
        cert.repeated_lap_relation.successors[source].assign(
            current.begin(), current.end());
    }

    cert.controller_cycle = find_cyclic_one_lap_orbit(cert.repeated_lap_relation);
    if (!cert.controller_cycle.valid) {
        cert.obstruction = "role-constrained controller relation is acyclic: " +
                           cert.controller_cycle.failure;
        return cert;
    }

    // A relation-cycle edge represents all role laps.  Expand it to an exact
    // digit path, checking every intermediate role junction.
    cert.controller_run.certificate_id = "stepped-face.role-constrained-lift";
    cert.controller_run.initial_state = cert.controller_cycle.closed_orbit.front();
    cert.controller_run.controller_states = {cert.controller_run.initial_state};
    cert.junction_roles_replayed = true;
    cert.digit_path_replayed = true;

    std::vector<std::int64_t> repeated_word;
    const auto relation_period = cert.controller_cycle.closed_orbit.size() - 1;
    for (std::size_t edge = 0; edge < relation_period; ++edge) {
        auto current = cert.controller_cycle.closed_orbit[edge];
        const auto final_target = cert.controller_cycle.closed_orbit[edge + 1];
        for (std::size_t lap = 0; lap < cert.lap_roles.size(); ++lap) {
            // For nonfinal role laps, choose any endpoint in the next role that
            // can still complete the remaining role laps and relation edge.
            std::set<std::size_t> endpoints = detail::image_under_word_restricted(
                product.controller_plant, {current}, cert.lap_word);
            const auto& next_role = cert.lap_roles[(lap + 1) % cert.lap_roles.size()];
            for (auto it = endpoints.begin(); it != endpoints.end();) {
                if (!detail::controller_has_role(product.controller_states[*it], next_role))
                    it = endpoints.erase(it);
                else
                    ++it;
            }
            std::size_t chosen = product.controller_states.size();
            if (lap + 1 == cert.lap_roles.size()) {
                if (endpoints.contains(final_target)) chosen = final_target;
            } else {
                // Backward viability through the remaining laps.
                for (const auto candidate : endpoints) {
                    std::set<std::size_t> frontier{candidate};
                    bool viable = true;
                    for (std::size_t rest = lap + 1; rest < cert.lap_roles.size(); ++rest) {
                        frontier = detail::image_under_word_restricted(
                            product.controller_plant, frontier, cert.lap_word);
                        const auto& role = cert.lap_roles[(rest + 1) % cert.lap_roles.size()];
                        for (auto it = frontier.begin(); it != frontier.end();) {
                            if (!detail::controller_has_role(product.controller_states[*it], role))
                                it = frontier.erase(it);
                            else
                                ++it;
                        }
                        viable &= !frontier.empty();
                    }
                    if (viable && frontier.contains(final_target)) {
                        chosen = candidate;
                        break;
                    }
                }
            }
            if (chosen >= product.controller_states.size()) {
                cert.digit_path_replayed = false;
                cert.obstruction = "failed to reconstruct a role-constrained relation edge";
                return cert;
            }
            std::vector<std::size_t> path;
            if (!detail::reconstruct_word_path(product.controller_plant,
                                               cert.lap_word,
                                               current,
                                               chosen,
                                               path)) {
                cert.digit_path_replayed = false;
                cert.obstruction = "failed to reconstruct digit-level controller path";
                return cert;
            }
            cert.controller_run.controller_states.insert(
                cert.controller_run.controller_states.end(),
                path.begin() + 1, path.end());
            repeated_word.insert(repeated_word.end(),
                                 cert.lap_word.begin(), cert.lap_word.end());
            current = chosen;
            cert.junction_roles_replayed &= detail::controller_has_role(
                product.controller_states[current], next_role);
        }
    }

    cert.controller_run.base_word = std::move(repeated_word);
    cert.controller_run.transition_replay = true;
    for (std::size_t k = 0; k < cert.controller_run.base_word.size(); ++k) {
        const auto u = cert.controller_run.controller_states[k];
        const auto v = cert.controller_run.controller_states[k + 1];
        const auto it = product.controller_plant.successors.find(
            std::make_pair(u, cert.controller_run.base_word[k]));
        cert.controller_run.transition_replay &=
            it != product.controller_plant.successors.end() &&
            std::find(it->second.begin(), it->second.end(), v) != it->second.end();
    }
    cert.controller_run.closes =
        cert.controller_run.controller_states.front() ==
        cert.controller_run.controller_states.back();
    cert.controller_run.valid = cert.controller_run.transition_replay &&
                                cert.controller_run.closes;
    cert.valid = cert.restricted_relation_replayed &&
                 cert.junction_roles_replayed &&
                 cert.digit_path_replayed && cert.controller_run.valid;
    if (!cert.valid && cert.obstruction.empty())
        cert.obstruction = "role-constrained controller lift failed replay";
    return cert;
}

} // namespace ravel::proof
