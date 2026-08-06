#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/cyclic_splice_completion.hpp"

namespace ravel::proof {

/** Exact evidence that a finite one-lap relation contains a directed cycle.
 * Unlike SerialLapSystem, no seriality assumption is made.  A single cyclic
 * SCC is enough to close a repeated controller splice. */
struct CyclicOneLapRelationProof {
    std::vector<std::size_t> closed_orbit;
    bool relation_replay = false;
    bool positive_period = false;
    bool valid = false;
    std::string failure;
};

inline CyclicOneLapRelationProof find_cyclic_one_lap_orbit(
    const OneLapControllerRelation& relation) {
    CyclicOneLapRelationProof proof;
    if (relation.state_count == 0 ||
        relation.successors.size() != relation.state_count) {
        proof.failure = "malformed one-lap relation";
        return proof;
    }
    for (std::size_t u = 0; u < relation.state_count; ++u) {
        for (const auto v : relation.successors[u]) {
            if (v >= relation.state_count) {
                proof.failure = "one-lap relation contains an out-of-range target";
                return proof;
            }
        }
    }

    // DFS with an explicit recursion-stack position.  The first back edge
    // reconstructs a positive directed cycle and is already a proof witness.
    std::vector<unsigned char> color(relation.state_count, 0);
    std::vector<std::size_t> stack;
    std::vector<std::size_t> stack_pos(relation.state_count,
                                       relation.state_count);
    bool found = false;
    std::function<void(std::size_t)> dfs = [&](std::size_t u) {
        if (found) return;
        color[u] = 1;
        stack_pos[u] = stack.size();
        stack.push_back(u);
        for (const auto v : relation.successors[u]) {
            if (found) break;
            if (color[v] == 0) {
                dfs(v);
            } else if (color[v] == 1) {
                const auto begin = stack_pos[v];
                proof.closed_orbit.assign(stack.begin() + begin, stack.end());
                proof.closed_orbit.push_back(v);
                found = true;
                break;
            }
        }
        stack.pop_back();
        stack_pos[u] = relation.state_count;
        color[u] = 2;
    };
    for (std::size_t u = 0; u < relation.state_count && !found; ++u)
        if (color[u] == 0) dfs(u);

    if (!found) {
        proof.failure = "one-lap relation is acyclic";
        return proof;
    }
    proof.positive_period = proof.closed_orbit.size() >= 2 &&
        proof.closed_orbit.front() == proof.closed_orbit.back();
    proof.relation_replay = proof.positive_period;
    for (std::size_t i = 0; i + 1 < proof.closed_orbit.size(); ++i) {
        const auto u = proof.closed_orbit[i];
        const auto v = proof.closed_orbit[i + 1];
        const auto& out = relation.successors[u];
        proof.relation_replay &=
            std::find(out.begin(), out.end(), v) != out.end();
    }
    proof.valid = proof.positive_period && proof.relation_replay;
    if (!proof.valid) proof.failure = "cycle reconstruction failed replay";
    return proof;
}

inline std::string render_cyclic_one_lap_relation_report(
    const CyclicOneLapRelationProof& proof) {
    return "CYCLIC_ONE_LAP_RELATION\n"
        "period_laps=" +
        std::to_string(proof.closed_orbit.empty()
            ? 0 : proof.closed_orbit.size() - 1) + "\n"
        "relation_replay=" +
        std::string(proof.relation_replay ? "true" : "false") + "\n"
        "valid=" + std::string(proof.valid ? "true" : "false") + "\n"
        "failure=" + proof.failure + "\n";
}

} // namespace ravel::proof
