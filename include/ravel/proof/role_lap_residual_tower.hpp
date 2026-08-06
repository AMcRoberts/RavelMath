#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "ravel/proof/continuation_controller_family.hpp"
#include "ravel/proof/role_constrained_kernel_dichotomy.hpp"

namespace ravel::proof {

struct RoleLapResidualTowerCertificate {
    OneLapControllerRelation relation;
    ControllerPlant macro_plant;
    ContinuationControllerFamily tower;
    std::vector<std::size_t> orbit;
    std::size_t first_repeat = 0;
    std::size_t repeat_of = 0;
    std::size_t extinction_height = 0;
    bool relation_replayed = false;
    bool tower_closed = false;
    bool orbit_replayed = false;
    bool reached_empty = false;
    bool nonempty_periodic = false;
    bool kernel_equivalence_replayed = false;
    bool valid = false;
    std::string obstruction;
};

/**
 * Exact predecessor tower for a role-constrained one-lap relation.
 *
 * The existing continuation-controller closure is reused verbatim by viewing
 * one complete role lap as a single macro digit.  The terminal residual is the
 * full controller universe.  Its k-th predecessor is exactly the set of
 * controller states from which a role-constrained path of k complete laps
 * exists.
 *
 * In a finite graph this tower has an exact dichotomy:
 *   - it reaches the empty residual, yielding a bounded extinction height; or
 *   - it repeats a nonempty residual, yielding arbitrarily long lap paths and
 *     hence a positive controller cycle.
 */
inline RoleLapResidualTowerCertificate derive_role_lap_residual_tower(
    OneLapControllerRelation relation) {
    RoleLapResidualTowerCertificate cert;
    cert.relation = std::move(relation);
    const auto n = cert.relation.state_count;
    cert.relation_replayed = cert.relation.successors.size() == n;
    cert.macro_plant.state_count = n;
    for (std::size_t source = 0; source < n; ++source) {
        auto row = cert.relation.successors[source];
        std::sort(row.begin(), row.end());
        row.erase(std::unique(row.begin(), row.end()), row.end());
        for (const auto target : row) cert.relation_replayed &= target < n;
        cert.macro_plant.successors[{source, 0}] = std::move(row);
    }
    if (!cert.relation_replayed) {
        cert.obstruction = "malformed one-lap relation";
        return cert;
    }

    ResidualController universe(n, true);
    cert.tower = close_controller_family(cert.macro_plant, {universe}, {0});
    cert.tower_closed = cert.tower.closed;
    if (!cert.tower_closed || cert.tower.members.empty()) {
        cert.obstruction = "macro predecessor tower did not close";
        return cert;
    }

    const ResidualController empty(n, false);
    std::vector<std::size_t> first_seen(cert.tower.members.size(),
                                        static_cast<std::size_t>(-1));
    std::size_t current = cert.tower.member_ids.at(universe);
    cert.orbit_replayed = true;
    for (std::size_t step = 0;; ++step) {
        if (first_seen[current] != static_cast<std::size_t>(-1)) {
            cert.first_repeat = step;
            cert.repeat_of = first_seen[current];
            cert.nonempty_periodic = cert.tower.members[current] != empty;
            break;
        }
        first_seen[current] = step;
        cert.orbit.push_back(current);
        if (cert.tower.members[current] == empty) {
            cert.reached_empty = true;
            cert.extinction_height = step;
            cert.first_repeat = step + 1;
            cert.repeat_of = step;
            break;
        }
        const auto key = std::make_tuple(current, std::int64_t{0});
        const auto next = cert.tower.predecessor_table.at(key);
        cert.orbit_replayed &=
            cert.tower.members[next] == predecessor_controller(
                cert.macro_plant, cert.tower.members[current], 0);
        current = next;
    }

    const auto dichotomy = derive_role_constrained_kernel_dichotomy(cert.relation);
    cert.kernel_equivalence_replayed = dichotomy.valid &&
        (dichotomy.has_cyclic_kernel == cert.nonempty_periodic) &&
        (!dichotomy.has_cyclic_kernel == cert.reached_empty);
    if (cert.reached_empty)
        cert.kernel_equivalence_replayed &= cert.extinction_height <= n;

    cert.valid = cert.relation_replayed && cert.tower_closed &&
                 cert.orbit_replayed && cert.kernel_equivalence_replayed &&
                 (cert.reached_empty != cert.nonempty_periodic);
    if (!cert.valid) cert.obstruction = "role-lap residual tower replay failed";
    return cert;
}

} // namespace ravel::proof
