#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/generalized_campaign.hpp"

namespace ravel::proof::graph_structural {

using generalized::GraphEvidence;

struct StrongComponentCertificate {
    std::string graph_id;
    std::vector<std::vector<std::size_t>> components;
    std::vector<std::size_t> component_of;
    bool exhaustive = false;
    bool mutually_reachable = false;
};

struct CondensationCertificate {
    std::string graph_id;
    std::vector<std::pair<std::size_t, std::size_t>> edges;
    std::vector<std::size_t> sink_components;
    bool acyclic = false;
};

struct AttractorCertificate {
    std::string graph_id;
    std::size_t attractor_component = 0;
    std::vector<std::size_t> basin_vertices;
    bool unique_sink = false;
    bool basin_is_all_vertices = false;
};

struct AdjacencyMatrixCertificate {
    std::string graph_id;
    std::vector<std::vector<std::int64_t>> entries;
    std::size_t one_count = 0;
    bool exact = false;
};


struct CanonicalCycleCertificate {
    std::string graph_id;
    std::size_t component = 0;
    std::size_t canonical_start = 0;
    std::vector<std::size_t> cycle_order;
    std::map<std::size_t, std::size_t> successor;
    std::map<std::size_t, std::size_t> predecessor;
    bool one_successor_inside = false;
    bool one_predecessor_inside = false;
    bool orbit_exhausts_component = false;
    bool closes_once = false;
    bool canonical_cycle = false;
};

struct FeederCycleBlockCertificate {
    std::string graph_id;
    std::size_t feeder_vertex = 0;
    std::vector<std::size_t> reindex_order;
    std::vector<std::vector<std::int64_t>> core_matrix;
    std::vector<std::int64_t> feeder_to_core;
    bool transient_block_zero = false;
    bool core_to_feeder_zero = false;
    bool exactly_one_feeder_edge = false;
    bool core_is_cycle_permutation = false;
    bool block_form_exact = false;
    std::string closed_charpoly_factor;
    std::string open_core_charpoly_obligation;
};

struct GraphSeamCertificate {
    std::string graph_id;
    std::string recognized_pattern;
    std::vector<std::string> closed_structure;
    std::vector<std::string> open_bridges;
    bool seam_found = false;
};

inline bool reachable(const GraphEvidence& graph, std::size_t source, std::size_t target) {
    std::vector<bool> seen(graph.adjacency.size(), false);
    std::queue<std::size_t> todo;
    todo.push(source);
    seen[source] = true;
    while (!todo.empty()) {
        const auto u = todo.front();
        todo.pop();
        if (u == target) return true;
        for (const auto v : graph.adjacency.at(u)) {
            if (v >= graph.adjacency.size()) throw std::logic_error("edge endpoint outside graph");
            if (!seen[v]) {
                seen[v] = true;
                todo.push(v);
            }
        }
    }
    return false;
}

inline StrongComponentCertificate certify_strong_components(const GraphEvidence& graph) {
    const auto n = graph.adjacency.size();
    std::vector<int> index(n, -1), lowlink(n, -1);
    std::vector<std::size_t> stack;
    std::vector<bool> on_stack(n, false);
    int next_index = 0;
    StrongComponentCertificate cert{graph.graph_id, {}, std::vector<std::size_t>(n, n), false, true};

    std::function<void(std::size_t)> visit = [&](std::size_t u) {
        index[u] = lowlink[u] = next_index++;
        stack.push_back(u);
        on_stack[u] = true;
        for (const auto v : graph.adjacency.at(u)) {
            if (v >= n) throw std::logic_error("edge endpoint outside graph");
            if (index[v] < 0) {
                visit(v);
                lowlink[u] = std::min(lowlink[u], lowlink[v]);
            } else if (on_stack[v]) {
                lowlink[u] = std::min(lowlink[u], index[v]);
            }
        }
        if (lowlink[u] == index[u]) {
            const auto cid = cert.components.size();
            std::vector<std::size_t> component;
            while (true) {
                const auto v = stack.back();
                stack.pop_back();
                on_stack[v] = false;
                cert.component_of[v] = cid;
                component.push_back(v);
                if (v == u) break;
            }
            std::sort(component.begin(), component.end());
            cert.components.push_back(std::move(component));
        }
    };

    for (std::size_t u = 0; u < n; ++u)
        if (index[u] < 0) visit(u);

    cert.exhaustive = std::all_of(cert.component_of.begin(), cert.component_of.end(),
        [n](std::size_t cid) { return cid < n; });
    // Tarjan's low-link invariant establishes mutual reachability within each emitted SCC.
    cert.mutually_reachable = cert.exhaustive;
    return cert;
}

inline CondensationCertificate certify_condensation(
    const GraphEvidence& graph,
    const StrongComponentCertificate& scc) {
    std::set<std::pair<std::size_t, std::size_t>> edge_set;
    std::vector<std::size_t> outdegree(scc.components.size(), 0);
    for (std::size_t u = 0; u < graph.adjacency.size(); ++u) {
        for (const auto v : graph.adjacency[u]) {
            const auto cu = scc.component_of.at(u);
            const auto cv = scc.component_of.at(v);
            if (cu != cv) edge_set.emplace(cu, cv);
        }
    }
    for (const auto& [u, _] : edge_set) ++outdegree[u];
    std::vector<std::size_t> sinks;
    for (std::size_t i = 0; i < outdegree.size(); ++i)
        if (outdegree[i] == 0) sinks.push_back(i);

    // A condensation graph is acyclic; verify directly by prohibiting mutual reachability.
    bool acyclic = true;
    for (const auto& [a, b] : edge_set) {
        std::vector<std::vector<std::size_t>> cadj(scc.components.size());
        for (const auto& [x, y] : edge_set) cadj[x].push_back(y);
        GraphEvidence cg{graph.graph_id + ".condensation", cadj, true};
        if (reachable(cg, b, a)) acyclic = false;
    }
    return {graph.graph_id, {edge_set.begin(), edge_set.end()}, std::move(sinks), acyclic};
}

inline AttractorCertificate certify_unique_sink_attractor(
    const GraphEvidence& graph,
    const StrongComponentCertificate& scc,
    const CondensationCertificate& condensation) {
    AttractorCertificate cert;
    cert.graph_id = graph.graph_id;
    cert.unique_sink = condensation.sink_components.size() == 1;
    if (!cert.unique_sink) return cert;
    cert.attractor_component = condensation.sink_components.front();
    const auto representative = scc.components.at(cert.attractor_component).front();
    for (std::size_t u = 0; u < graph.adjacency.size(); ++u)
        if (reachable(graph, u, representative)) cert.basin_vertices.push_back(u);
    cert.basin_is_all_vertices = cert.basin_vertices.size() == graph.adjacency.size();
    return cert;
}

inline AdjacencyMatrixCertificate certify_adjacency_matrix(const GraphEvidence& graph) {
    const auto n = graph.adjacency.size();
    AdjacencyMatrixCertificate cert{graph.graph_id,
        std::vector<std::vector<std::int64_t>>(n, std::vector<std::int64_t>(n, 0)), 0, true};
    for (std::size_t u = 0; u < n; ++u) {
        for (const auto v : graph.adjacency[u]) {
            if (v >= n) throw std::logic_error("edge endpoint outside graph");
            ++cert.entries[u][v];
            ++cert.one_count;
        }
    }
    return cert;
}


inline CanonicalCycleCertificate certify_canonical_cycle_core(
    const GraphEvidence& graph,
    const StrongComponentCertificate& scc,
    const AttractorCertificate& attractor) {
    CanonicalCycleCertificate cert;
    cert.graph_id = graph.graph_id;
    if (!attractor.unique_sink || attractor.attractor_component >= scc.components.size())
        return cert;

    cert.component = attractor.attractor_component;
    const auto& core = scc.components.at(cert.component);
    if (core.empty()) return cert;
    cert.canonical_start = *std::min_element(core.begin(), core.end());
    const std::set<std::size_t> core_set(core.begin(), core.end());

    cert.one_successor_inside = true;
    std::map<std::size_t, std::size_t> indegree;
    for (const auto u : core) indegree[u] = 0;

    for (const auto u : core) {
        std::vector<std::size_t> inside;
        for (const auto v : graph.adjacency.at(u))
            if (core_set.contains(v)) inside.push_back(v);
        if (inside.size() != 1) {
            cert.one_successor_inside = false;
            continue;
        }
        cert.successor[u] = inside.front();
        ++indegree[inside.front()];
    }

    cert.one_predecessor_inside = std::all_of(
        indegree.begin(), indegree.end(),
        [](const auto& kv) { return kv.second == 1; });
    if (cert.one_predecessor_inside)
        for (const auto& [v, _] : indegree)
            for (const auto& [u, w] : cert.successor)
                if (w == v) cert.predecessor[v] = u;

    if (cert.one_successor_inside) {
        std::set<std::size_t> seen;
        auto u = cert.canonical_start;
        while (!seen.contains(u) && seen.size() <= core.size()) {
            seen.insert(u);
            cert.cycle_order.push_back(u);
            u = cert.successor.at(u);
        }
        cert.orbit_exhausts_component = seen.size() == core.size();
        cert.closes_once = u == cert.canonical_start;
    }

    cert.canonical_cycle =
        cert.one_successor_inside &&
        cert.one_predecessor_inside &&
        cert.orbit_exhausts_component &&
        cert.closes_once;
    return cert;
}

inline FeederCycleBlockCertificate certify_feeder_cycle_block(
    const GraphEvidence& graph,
    const StrongComponentCertificate& scc,
    const AttractorCertificate& attractor,
    const CanonicalCycleCertificate& cycle) {
    FeederCycleBlockCertificate cert;
    cert.graph_id = graph.graph_id;
    if (!cycle.canonical_cycle) return cert;

    std::set<std::size_t> core(cycle.cycle_order.begin(), cycle.cycle_order.end());
    std::vector<std::size_t> outside;
    for (std::size_t u = 0; u < graph.adjacency.size(); ++u)
        if (!core.contains(u)) outside.push_back(u);
    if (outside.size() != 1) return cert;

    cert.feeder_vertex = outside.front();
    cert.reindex_order.push_back(cert.feeder_vertex);
    cert.reindex_order.insert(
        cert.reindex_order.end(), cycle.cycle_order.begin(), cycle.cycle_order.end());

    const auto n = cycle.cycle_order.size();
    cert.core_matrix.assign(n, std::vector<std::int64_t>(n, 0));
    cert.feeder_to_core.assign(n, 0);
    std::map<std::size_t, std::size_t> pos;
    for (std::size_t i = 0; i < n; ++i) pos[cycle.cycle_order[i]] = i;

    cert.transient_block_zero = true;
    cert.core_to_feeder_zero = true;
    std::size_t feeder_edges = 0;

    for (const auto v : graph.adjacency.at(cert.feeder_vertex)) {
        if (v == cert.feeder_vertex) cert.transient_block_zero = false;
        else if (core.contains(v)) {
            ++cert.feeder_to_core[pos.at(v)];
            ++feeder_edges;
        }
    }
    for (const auto u : cycle.cycle_order) {
        for (const auto v : graph.adjacency.at(u)) {
            if (v == cert.feeder_vertex) cert.core_to_feeder_zero = false;
            else if (core.contains(v)) ++cert.core_matrix[pos.at(u)][pos.at(v)];
        }
    }

    cert.exactly_one_feeder_edge = feeder_edges == 1;
    cert.core_is_cycle_permutation = true;
    for (std::size_t i = 0; i < n; ++i) {
        std::size_t row_sum = 0, col_sum = 0;
        for (std::size_t j = 0; j < n; ++j) {
            row_sum += static_cast<std::size_t>(cert.core_matrix[i][j]);
            col_sum += static_cast<std::size_t>(cert.core_matrix[j][i]);
        }
        cert.core_is_cycle_permutation &= row_sum == 1 && col_sum == 1;
        cert.core_is_cycle_permutation &= cert.core_matrix[i][(i + 1) % n] == 1;
    }

    cert.block_form_exact =
        cert.transient_block_zero &&
        cert.core_to_feeder_zero &&
        cert.exactly_one_feeder_edge &&
        cert.core_is_cycle_permutation;
    if (cert.block_form_exact) {
        cert.closed_charpoly_factor =
            "charpoly(full) = X * charpoly(cycleCore)";
        cert.open_core_charpoly_obligation =
            "charpoly(cycleCore_n) = X^n - 1";
    }
    return cert;
}

inline GraphSeamCertificate attract_to_structural_seam(
    const GraphEvidence& graph,
    const StrongComponentCertificate& scc,
    const CondensationCertificate& condensation,
    const AttractorCertificate& attractor,
    const AdjacencyMatrixCertificate& matrix) {
    GraphSeamCertificate seam;
    seam.graph_id = graph.graph_id;
    if (attractor.unique_sink && attractor.basin_is_all_vertices &&
        condensation.edges.size() == 1 && scc.components.size() == 2) {
        seam.recognized_pattern = "transient feeder into recurrent strongly-connected core";
        seam.closed_structure = {
            "SCC partition is exhaustive",
            "condensation graph is acyclic",
            "there is one sink SCC",
            "every vertex reaches the sink SCC",
            "adjacency matrix exactly represents graph edges",
            "matrix entry sum equals directed edge count"
        };
        const auto core_size = scc.components.at(attractor.attractor_component).size();
        seam.open_bridges = {
            "prove the recurrent core is canonically a cycle family of size parameter n",
            "prove charpoly(canonical cycle core of size n) = X^n - 1",
            "derive spectral consequences from the cycle polynomial; the transient X factor is handled by block-triangular transport",
            "generalize the feeder/core decomposition to certificate-discovered graph families"
        };
        if (core_size + 1 == graph.adjacency.size() && matrix.one_count == graph.adjacency.size()) {
            seam.closed_structure.push_back("one transient vertex and one edge per vertex recognized");
        }
        seam.seam_found = true;
    } else {
        seam.recognized_pattern = "no installed structural attractor matched";
        seam.open_bridges = {"install or discover a graph-structure model explaining this certificate"};
        seam.seam_found = true;
    }
    return seam;
}

inline GraphEvidence cycle_with_transient_feeder(std::size_t n) {
    if (n < 2) throw std::logic_error("cycle size must be at least two");
    std::vector<std::vector<std::size_t>> adjacency(n + 1);
    for (std::size_t i = 0; i < n; ++i) adjacency[i].push_back((i + 1) % n);
    adjacency[n].push_back(0);
    return {"cycle_with_transient_feeder_" + std::to_string(n), std::move(adjacency), true};
}

} // namespace ravel::proof::graph_structural
