#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/maximum_shell_maximality_contradiction.hpp"
#include "ravel/proof/role_lap_residual_tower.hpp"

namespace ravel::proof {

struct MaximumShellReturnRankCertificate {
    std::size_t state_count = 0;
    std::vector<std::vector<std::size_t>> successors;
    std::vector<std::size_t> rank;
    bool graph_replayed = false;
    bool acyclic = false;
    bool strict_on_edges = false;
    bool valid = false;
    std::string obstruction;
};

/** Derive a replayable strict rank for a finite maximum-shell first-return
 * relation.  Rank is longest-path height from a source in a topological order.
 * This operation does not assume acyclicity: a cycle is reported as an open
 * obstruction rather than assigned a fabricated rank. */
inline MaximumShellReturnRankCertificate derive_maximum_shell_return_rank(
    std::vector<std::vector<std::size_t>> successors) {
    MaximumShellReturnRankCertificate cert;
    cert.state_count = successors.size();
    cert.successors = std::move(successors);
    cert.graph_replayed = true;
    std::vector<std::size_t> indegree(cert.state_count, 0);
    for (auto& row : cert.successors) {
        std::sort(row.begin(), row.end());
        row.erase(std::unique(row.begin(), row.end()), row.end());
        for (const auto target : row) {
            cert.graph_replayed &= target < cert.state_count;
            if (target < cert.state_count) ++indegree[target];
        }
    }
    if (!cert.graph_replayed) {
        cert.obstruction = "malformed maximum-shell return relation";
        return cert;
    }
    std::deque<std::size_t> queue;
    for (std::size_t i = 0; i < cert.state_count; ++i)
        if (indegree[i] == 0) queue.push_back(i);
    cert.rank.assign(cert.state_count, 0);
    std::size_t visited = 0;
    while (!queue.empty()) {
        const auto source = queue.front();
        queue.pop_front();
        ++visited;
        for (const auto target : cert.successors[source]) {
            cert.rank[target] = std::max(cert.rank[target], cert.rank[source] + 1);
            if (--indegree[target] == 0) queue.push_back(target);
        }
    }
    cert.acyclic = visited == cert.state_count;
    if (!cert.acyclic) {
        cert.obstruction = "maximum-shell first-return relation contains a cycle";
        return cert;
    }
    cert.strict_on_edges = true;
    for (std::size_t source = 0; source < cert.state_count; ++source)
        for (const auto target : cert.successors[source])
            cert.strict_on_edges &= cert.rank[source] < cert.rank[target];
    cert.valid = cert.graph_replayed && cert.acyclic && cert.strict_on_edges;
    if (!cert.valid) cert.obstruction = "maximum-shell return rank replay failed";
    return cert;
}

enum class MaximumShellExclusionBranch {
    unsupported,
    strict_pump_contradiction,
    strict_first_return_rank,
};

struct MaximumShellExclusionCertificate {
    MaximumShellExclusionBranch branch = MaximumShellExclusionBranch::unsupported;
    RoleLapResidualTowerCertificate tower;
    MaximumShellMaximalityContradictionCertificate pump_contradiction;
    MaximumShellReturnRankCertificate return_rank;
    bool recurrent_cycle_excluded = false;
    bool valid = false;
    std::string obstruction;
};

/** Complete finite maximum-shell exclusion by the exact proof disjunction.
 *
 *  - A nonempty role-lap cyclic kernel is lifted and pumped; the supplied
 *    maximality certificate is the contradiction.
 *  - If the controller kernel is empty, a strict rank on the actual shell
 *    first-return relation excludes every recurrent source lap directly.
 *
 * The two branches are logically independent.  This is deliberately stronger
 * engineering than treating an empty controller kernel as a failed proof.
 */
inline MaximumShellExclusionCertificate certify_maximum_shell_exclusion(
    RoleLapResidualTowerCertificate tower,
    MaximumShellMaximalityContradictionCertificate pump_contradiction,
    MaximumShellReturnRankCertificate return_rank) {
    MaximumShellExclusionCertificate cert;
    cert.tower = std::move(tower);
    cert.pump_contradiction = std::move(pump_contradiction);
    cert.return_rank = std::move(return_rank);
    if (!cert.tower.valid) {
        cert.obstruction = "role-lap residual tower is invalid";
        return cert;
    }
    if (cert.tower.nonempty_periodic) {
        if (!cert.pump_contradiction.valid) {
            cert.obstruction = "cyclic controller kernel lacks maximality contradiction";
            return cert;
        }
        cert.branch = MaximumShellExclusionBranch::strict_pump_contradiction;
        cert.recurrent_cycle_excluded = true;
    } else if (cert.tower.reached_empty) {
        if (!cert.return_rank.valid) {
            cert.obstruction = "empty controller kernel lacks shell return rank";
            return cert;
        }
        cert.branch = MaximumShellExclusionBranch::strict_first_return_rank;
        cert.recurrent_cycle_excluded = true;
    } else {
        cert.obstruction = "tower dichotomy has neither branch";
        return cert;
    }
    cert.valid = cert.recurrent_cycle_excluded;
    return cert;
}

} // namespace ravel::proof
