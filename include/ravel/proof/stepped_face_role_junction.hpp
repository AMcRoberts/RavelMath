#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/cyclic_splice_compactness.hpp"
#include "ravel/proof/first_return_joint_product.hpp"

namespace ravel::proof {

using SteppedFaceRole = std::pair<std::size_t, std::int64_t>;

struct SteppedFaceRoleRelationCertificate {
    std::vector<SteppedFaceRole> source_roles;
    std::vector<SteppedFaceRole> target_roles;
    std::vector<std::vector<bool>> relation;
    bool every_source_role_continues = false;
    bool every_target_role_has_predecessor = false;
    bool transition_replay = false;
    bool valid = false;
    std::string obstruction;
};

inline SteppedFaceRoleRelationCertificate derive_stepped_face_role_relation(
    const FirstReturnJointProduct& product,
    const FirstReturnWitness& witness) {
    SteppedFaceRoleRelationCertificate cert;
    if (!product.replayed || witness.base_path.empty() ||
        witness.dimension != product.dimension) {
        cert.obstruction = "product or witness did not replay";
        return cert;
    }
    cert.source_roles = detail::target_faces(witness.base_path.front(), witness.bound);
    cert.target_roles = detail::target_faces(witness.target, witness.bound);
    cert.relation.assign(cert.source_roles.size(),
                         std::vector<bool>(cert.target_roles.size(), false));
    cert.transition_replay = true;

    for (std::size_t a = 0; a < cert.source_roles.size(); ++a) {
        const auto [source_coord, source_sign] = cert.source_roles[a];
        for (std::size_t b = 0; b < cert.target_roles.size(); ++b) {
            const auto [target_coord, target_sign] = cert.target_roles[b];
            bool witnessed = false;
            for (std::size_t q = 0; q < product.controller_states.size() && !witnessed; ++q) {
                if (product.controller_states[q][source_coord] != source_sign) continue;
                std::set<std::size_t> current{q};
                for (const auto digit : witness.digits) {
                    std::set<std::size_t> next;
                    for (const auto state : current) {
                        const auto it = product.controller_plant.successors.find({state, digit});
                        if (it != product.controller_plant.successors.end())
                            next.insert(it->second.begin(), it->second.end());
                    }
                    current = std::move(next);
                }
                for (const auto state : current) {
                    if (product.controller_states[state][target_coord] == target_sign) {
                        witnessed = true;
                        break;
                    }
                }
            }
            cert.relation[a][b] = witnessed;
        }
    }

    cert.every_source_role_continues = true;
    for (const auto& row : cert.relation)
        cert.every_source_role_continues &=
            std::any_of(row.begin(), row.end(), [](bool x) { return x; });

    cert.every_target_role_has_predecessor = true;
    for (std::size_t b = 0; b < cert.target_roles.size(); ++b) {
        bool found = false;
        for (std::size_t a = 0; a < cert.source_roles.size(); ++a)
            found |= cert.relation[a][b];
        cert.every_target_role_has_predecessor &= found;
    }
    cert.valid = !cert.source_roles.empty() && !cert.target_roles.empty() &&
                 cert.transition_replay && cert.every_source_role_continues &&
                 cert.every_target_role_has_predecessor;
    if (!cert.valid) cert.obstruction = "stepped-face role relation is not bi-serial";
    return cert;
}

struct SteppedFaceRoleLapCertificate {
    std::vector<SteppedFaceRoleRelationCertificate> segments;
    OneLapControllerRelation one_lap_roles;
    CyclicOneLapRelationProof role_cycle;
    std::vector<SteppedFaceRole> selected_roles;
    bool junction_roles_match = false;
    bool composition_replay = false;
    bool valid = false;
    std::string obstruction;
};

/** Compose face-role relations around a closed source lap.  Consecutive
 * source/target shell states must agree, hence their active face-role lists
 * are literally the same.  Bi-seriality gives a finite serial role relation;
 * finiteness then gives a positive cyclic role choice. */
inline SteppedFaceRoleLapCertificate derive_stepped_face_role_lap(
    std::vector<SteppedFaceRoleRelationCertificate> segments) {
    SteppedFaceRoleLapCertificate cert;
    cert.segments = std::move(segments);
    if (cert.segments.empty()) {
        cert.obstruction = "no role segments supplied";
        return cert;
    }
    for (const auto& segment : cert.segments) {
        if (!segment.valid) {
            cert.obstruction = "invalid role segment";
            return cert;
        }
    }
    cert.junction_roles_match = true;
    for (std::size_t k = 0; k < cert.segments.size(); ++k) {
        const auto& left = cert.segments[k];
        const auto& right = cert.segments[(k + 1) % cert.segments.size()];
        cert.junction_roles_match &= left.target_roles == right.source_roles;
    }
    if (!cert.junction_roles_match) {
        cert.obstruction = "consecutive stepped-face role lists do not match";
        return cert;
    }

    const auto& base_roles = cert.segments.front().source_roles;
    cert.one_lap_roles.state_count = base_roles.size();
    cert.one_lap_roles.successors.assign(base_roles.size(), {});
    cert.composition_replay = true;
    for (std::size_t source = 0; source < base_roles.size(); ++source) {
        std::set<std::size_t> current{source};
        for (const auto& segment : cert.segments) {
            std::set<std::size_t> next;
            for (const auto a : current)
                for (std::size_t b = 0; b < segment.target_roles.size(); ++b)
                    if (segment.relation[a][b]) next.insert(b);
            current = std::move(next);
            cert.composition_replay &= !current.empty();
        }
        cert.one_lap_roles.successors[source].assign(current.begin(), current.end());
    }
    cert.role_cycle = find_cyclic_one_lap_orbit(cert.one_lap_roles);
    if (cert.role_cycle.valid) {
        for (std::size_t k = 0; k + 1 < cert.role_cycle.closed_orbit.size(); ++k)
            cert.selected_roles.push_back(base_roles[cert.role_cycle.closed_orbit[k]]);
    }
    cert.valid = cert.composition_replay && cert.role_cycle.valid;
    if (!cert.valid) cert.obstruction = cert.role_cycle.failure;
    return cert;
}

} // namespace ravel::proof
