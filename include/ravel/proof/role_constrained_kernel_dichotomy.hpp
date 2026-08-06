#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <string>
#include <vector>

#include "ravel/proof/role_constrained_controller_lift.hpp"

namespace ravel::proof {

struct RoleConstrainedKernelDichotomyCertificate {
    OneLapControllerRelation relation;
    std::vector<bool> cyclic_kernel;
    std::vector<std::size_t> elimination_rank;
    std::size_t kernel_size = 0;
    bool relation_well_formed = false;
    bool kernel_replayed = false;
    bool rank_replayed = false;
    bool has_cyclic_kernel = false;
    bool valid = false;
    std::string obstruction;
};

/** Exact finite dichotomy for a role-constrained one-lap relation.
 *
 * Repeatedly remove vertices with no successor remaining in the current set.
 * The survivor is the greatest serial subrelation and therefore contains a
 * positive directed cycle exactly when it is nonempty.  If it is empty, the
 * removal time is a strict elimination rank on every relation edge.
 */
inline RoleConstrainedKernelDichotomyCertificate
 derive_role_constrained_kernel_dichotomy(
    OneLapControllerRelation relation) {
    RoleConstrainedKernelDichotomyCertificate cert;
    cert.relation = std::move(relation);
    const auto n = cert.relation.state_count;
    cert.relation_well_formed = cert.relation.successors.size() == n;
    if (!cert.relation_well_formed) {
        cert.obstruction = "one-lap relation size mismatch";
        return cert;
    }
    for (const auto& row : cert.relation.successors)
        for (const auto target : row)
            cert.relation_well_formed &= target < n;
    if (!cert.relation_well_formed) {
        cert.obstruction = "one-lap relation contains an invalid target";
        return cert;
    }

    std::vector<bool> alive(n, true);
    cert.elimination_rank.assign(n, 0);
    std::size_t round = 0;
    while (true) {
        std::vector<std::size_t> remove;
        for (std::size_t source = 0; source < n; ++source) {
            if (!alive[source]) continue;
            bool has_alive_successor = false;
            for (const auto target : cert.relation.successors[source])
                has_alive_successor |= alive[target];
            if (!has_alive_successor) remove.push_back(source);
        }
        if (remove.empty()) break;
        ++round;
        for (const auto source : remove) {
            alive[source] = false;
            cert.elimination_rank[source] = round;
        }
    }
    cert.cyclic_kernel = alive;
    cert.kernel_size = static_cast<std::size_t>(
        std::count(alive.begin(), alive.end(), true));
    cert.has_cyclic_kernel = cert.kernel_size > 0;

    cert.kernel_replayed = true;
    for (std::size_t source = 0; source < n; ++source) {
        if (!alive[source]) continue;
        bool has_kernel_successor = false;
        for (const auto target : cert.relation.successors[source])
            has_kernel_successor |= alive[target];
        cert.kernel_replayed &= has_kernel_successor;
    }

    cert.rank_replayed = true;
    if (!cert.has_cyclic_kernel) {
        for (std::size_t source = 0; source < n; ++source) {
            for (const auto target : cert.relation.successors[source]) {
                // A source can only be removed before a target that it points
                // to has disappeared.  Larger removal rank means earlier in
                // the reverse well-founded order.
                cert.rank_replayed &=
                    cert.elimination_rank[source] > cert.elimination_rank[target];
            }
        }
    }

    const auto cycle = find_cyclic_one_lap_orbit(cert.relation);
    cert.kernel_replayed &= cycle.valid == cert.has_cyclic_kernel;
    cert.valid = cert.relation_well_formed && cert.kernel_replayed &&
                 cert.rank_replayed;
    if (!cert.valid) cert.obstruction = "cyclic-kernel dichotomy replay failed";
    return cert;
}

} // namespace ravel::proof
