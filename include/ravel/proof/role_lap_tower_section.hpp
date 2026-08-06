#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "ravel/proof/role_lap_residual_tower.hpp"

namespace ravel::proof {

struct RoleLapTowerSectionCertificate {
    OneLapControllerRelation relation;
    std::vector<std::size_t> section;
    std::size_t repeated_first = 0;
    std::size_t repeated_second = 0;
    std::vector<std::size_t> closed_cycle;
    bool length_sufficient = false;
    bool path_replayed = false;
    bool repetition_derived = false;
    bool cycle_replayed = false;
    bool tower_nonempty = false;
    bool valid = false;
    std::string obstruction;
};

/** A finite section of N complete role laps on N controller states forces a
 * repeated controller state, hence a positive cycle.  This is the executable
 * finite-tower form of the compactness argument: no infinite controller orbit
 * is assumed or searched for. */
inline RoleLapTowerSectionCertificate certify_role_lap_tower_section(
    OneLapControllerRelation relation,
    std::vector<std::size_t> section) {
    RoleLapTowerSectionCertificate cert;
    cert.relation = std::move(relation);
    cert.section = std::move(section);
    const auto n = cert.relation.state_count;
    cert.length_sufficient = cert.section.size() >= n + 1;
    if (!cert.length_sufficient || cert.relation.successors.size() != n) {
        cert.obstruction = "tower section is shorter than state-count horizon";
        return cert;
    }
    cert.path_replayed = true;
    for (std::size_t k = 0; k + 1 < cert.section.size(); ++k) {
        const auto source = cert.section[k];
        const auto target = cert.section[k + 1];
        cert.path_replayed &= source < n && target < n;
        if (source < n) {
            const auto& row = cert.relation.successors[source];
            cert.path_replayed &=
                std::find(row.begin(), row.end(), target) != row.end();
        }
    }
    if (!cert.path_replayed) {
        cert.obstruction = "tower section contains a non-edge";
        return cert;
    }

    std::vector<std::size_t> first(n, static_cast<std::size_t>(-1));
    for (std::size_t k = 0; k < cert.section.size(); ++k) {
        const auto state = cert.section[k];
        if (first[state] != static_cast<std::size_t>(-1)) {
            cert.repeated_first = first[state];
            cert.repeated_second = k;
            cert.repetition_derived = cert.repeated_second > cert.repeated_first;
            break;
        }
        first[state] = k;
    }
    if (!cert.repetition_derived) {
        cert.obstruction = "pigeonhole repetition was not found";
        return cert;
    }
    cert.closed_cycle.assign(
        cert.section.begin() + static_cast<std::ptrdiff_t>(cert.repeated_first),
        cert.section.begin() + static_cast<std::ptrdiff_t>(cert.repeated_second + 1));
    cert.cycle_replayed = cert.closed_cycle.size() >= 2 &&
                          cert.closed_cycle.front() == cert.closed_cycle.back();
    for (std::size_t k = 0; k + 1 < cert.closed_cycle.size(); ++k) {
        const auto& row = cert.relation.successors[cert.closed_cycle[k]];
        cert.cycle_replayed &= std::find(row.begin(), row.end(),
                                        cert.closed_cycle[k + 1]) != row.end();
    }

    const auto tower = derive_role_lap_residual_tower(cert.relation);
    cert.tower_nonempty = tower.valid && tower.nonempty_periodic;
    cert.valid = cert.length_sufficient && cert.path_replayed &&
                 cert.repetition_derived && cert.cycle_replayed &&
                 cert.tower_nonempty;
    if (!cert.valid) cert.obstruction = "bounded tower section did not force a cycle";
    return cert;
}

} // namespace ravel::proof
