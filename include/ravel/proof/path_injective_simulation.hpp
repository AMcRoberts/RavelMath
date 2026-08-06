#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace ravel::proof {

using MultiAdjacency = std::vector<std::vector<std::size_t>>;

struct PathInjectiveSimulationCertificate {
    std::size_t source_vertices = 0;
    std::size_t target_vertices = 0;
    std::size_t initial_pairs = 0;
    std::size_t surviving_pairs = 0;
    std::size_t covered_source_vertices = 0;
    std::vector<std::vector<bool>> relation;
    bool local_edge_injections = false;
    bool total = false;
    bool path_injection_all_lengths = false;
    bool spectral_radius_nonstrict = false;
    std::string obstruction;
};

inline bool bipartite_injection_exists(
    const std::vector<std::vector<std::size_t>>& candidates,
    std::size_t target_edge_count) {
    std::vector<int> owner(target_edge_count, -1);
    std::function<bool(std::size_t,std::vector<bool>&)> augment =
        [&](std::size_t left, std::vector<bool>& seen) {
            for (const auto right : candidates[left]) {
                if (right >= target_edge_count || seen[right]) continue;
                seen[right] = true;
                if (owner[right] < 0 || augment(static_cast<std::size_t>(owner[right]), seen)) {
                    owner[right] = static_cast<int>(left);
                    return true;
                }
            }
            return false;
        };
    for (std::size_t left = 0; left < candidates.size(); ++left) {
        std::vector<bool> seen(target_edge_count, false);
        if (!augment(left, seen)) return false;
    }
    return true;
}

/** Greatest multiplicity-preserving forward simulation.
 *
 * A live pair (s,t) must admit an injection from every outgoing source edge
 * s->s' to a distinct outgoing target edge t->t' with live(s',t').  Parallel
 * edges are represented by repeated target IDs in the adjacency rows and are
 * therefore matched with their full multiplicity.
 */
inline PathInjectiveSimulationCertificate derive_path_injective_simulation(
    const MultiAdjacency& source,
    const MultiAdjacency& target,
    std::function<bool(std::size_t,std::size_t)> initially_allowed = {}) {
    PathInjectiveSimulationCertificate c;
    c.source_vertices = source.size();
    c.target_vertices = target.size();
    if (source.empty() || target.empty()) {
        c.obstruction = "path-injective simulation requires nonempty graphs";
        return c;
    }
    c.relation.assign(source.size(), std::vector<bool>(target.size(), false));
    for (std::size_t s = 0; s < source.size(); ++s)
        for (std::size_t t = 0; t < target.size(); ++t)
            if (!initially_allowed || initially_allowed(s,t)) {
                c.relation[s][t] = true;
                ++c.initial_pairs;
            }

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t s = 0; s < source.size(); ++s) {
            for (std::size_t t = 0; t < target.size(); ++t) {
                if (!c.relation[s][t]) continue;
                std::vector<std::vector<std::size_t>> candidates(source[s].size());
                for (std::size_t se = 0; se < source[s].size(); ++se) {
                    const auto sp = source[s][se];
                    if (sp >= source.size()) {
                        c.obstruction = "source edge endpoint outside graph";
                        return c;
                    }
                    for (std::size_t te = 0; te < target[t].size(); ++te) {
                        const auto tp = target[t][te];
                        if (tp >= target.size()) {
                            c.obstruction = "target edge endpoint outside graph";
                            return c;
                        }
                        if (c.relation[sp][tp]) candidates[se].push_back(te);
                    }
                }
                if (!bipartite_injection_exists(candidates, target[t].size())) {
                    c.relation[s][t] = false;
                    changed = true;
                }
            }
        }
    }

    c.local_edge_injections = true;
    for (std::size_t s = 0; s < source.size(); ++s) {
        bool covered = false;
        for (std::size_t t = 0; t < target.size(); ++t)
            if (c.relation[s][t]) {
                ++c.surviving_pairs;
                covered = true;
            }
        c.covered_source_vertices += covered;
    }
    c.total = c.covered_source_vertices == source.size();
    // Local edge injections compose recursively to path injections at every
    // length.  Hence every source row path count is bounded by one target row
    // path count and rho(source)<=rho(target).
    c.path_injection_all_lengths = c.total && c.local_edge_injections;
    c.spectral_radius_nonstrict = c.path_injection_all_lengths;
    if (!c.total) c.obstruction = "no total multiplicity-preserving simulation";
    return c;
}

struct GradedPathInjectionCertificate {
    std::size_t maximum_grade = 0;
    std::vector<PathInjectiveSimulationCertificate> descents;
    bool every_grade_descends = false;
    bool spectral_radius_bounded_by_grade_one = false;
    std::string obstruction;
};

inline GradedPathInjectionCertificate compose_graded_path_injections(
    std::vector<PathInjectiveSimulationCertificate> descents) {
    GradedPathInjectionCertificate c;
    c.maximum_grade = descents.size() + 1;
    c.descents = std::move(descents);
    c.every_grade_descends = std::all_of(
        c.descents.begin(), c.descents.end(),
        [](const auto& d){ return d.spectral_radius_nonstrict; });
    c.spectral_radius_bounded_by_grade_one = c.every_grade_descends;
    if (!c.every_grade_descends)
        c.obstruction = "at least one grade lacks a path-injective descent";
    return c;
}

} // namespace ravel::proof
