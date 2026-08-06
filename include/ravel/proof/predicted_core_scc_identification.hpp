#pragma once

#include "ravel/graph_divisor.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ravel::proof {

using PredictedCoreNode = nbonacci_margin::CoreState;

struct MultiSourcePathForest {
    std::vector<long long> distance;
    std::vector<std::size_t> parent;
    std::vector<bool> has_parent;
    std::vector<std::size_t> roots;
    bool reverse = false;
};

inline MultiSourcePathForest build_multi_source_path_forest(
    const WeightedDigraph& graph,
    const std::vector<std::size_t>& starts,
    bool reverse) {
    MultiSourcePathForest forest;
    forest.distance.assign(graph.n, -1);
    forest.parent.assign(graph.n, 0);
    forest.has_parent.assign(graph.n, false);
    forest.roots = starts;
    forest.reverse = reverse;

    std::queue<std::size_t> pending;
    for (const auto start : starts) {
        if (start >= graph.n)
            throw std::out_of_range("multi-source start");
        if (forest.distance[start] == 0) continue;
        forest.distance[start] = 0;
        pending.push(start);
    }

    while (!pending.empty()) {
        const auto source = pending.front();
        pending.pop();
        const auto& adjacency =
            reverse ? graph.in_adj[source] : graph.out_adj[source];
        for (const auto& [destination, ignored_weight] : adjacency) {
            (void)ignored_weight;
            if (forest.distance[destination] >= 0) continue;
            forest.distance[destination] = forest.distance[source] + 1;
            forest.parent[destination] = source;
            forest.has_parent[destination] = true;
            pending.push(destination);
        }
    }
    return forest;
}

// For reverse=false this returns root -> target.  For reverse=true the BFS
// traverses reverse edges, so reconstruction returns target -> root in the
// original graph.  Both orientations are exactly what the SCC induction uses.
inline std::vector<std::size_t> reconstruct_shadow_path(
    const MultiSourcePathForest& forest,
    std::size_t target) {
    if (target >= forest.distance.size())
        throw std::out_of_range("path target");
    if (forest.distance[target] < 0) return {};

    std::vector<std::size_t> chain{target};
    std::size_t current = target;
    while (forest.distance[current] > 0) {
        if (!forest.has_parent[current])
            throw std::runtime_error("broken BFS parent chain");
        current = forest.parent[current];
        chain.push_back(current);
    }
    if (!forest.reverse) std::reverse(chain.begin(), chain.end());
    return chain;
}

struct PredictedCoreSccCertificate {
    std::size_t dimension = 0;
    std::vector<PredictedCoreNode> nodes;
    std::vector<std::pair<std::size_t, std::size_t>> edges;
    std::vector<std::size_t> previous_shadow;
    MultiSourcePathForest from_shadow;
    MultiSourcePathForest to_shadow;
    bool node_count_exact = false;
    bool edge_count_exact = false;
    bool predecessor_table_exact = false;
    bool base_or_shadow_present = false;
    bool every_new_node_reached_from_shadow = false;
    bool every_new_node_reaches_shadow = false;
    bool distance_formulas_exact = false;
    bool direct_tarjan_one_scc = false;
    bool replay_checked = false;
};

inline PredictedCoreSccCertificate certify_predicted_core_scc(
    std::size_t n) {
    using namespace nbonacci_margin;
    if (n < 3) throw std::invalid_argument("dimension must be >= 3");

    PredictedCoreSccCertificate cert;
    cert.dimension = n;

    std::set<PredictedCoreNode> node_set;
    for (const auto pair : label_pairs(n))
        for (const auto descriptor : displacement_descriptors(n))
            if (predicted_core_member(n, pair, descriptor))
                node_set.insert({pair, descriptor});
    cert.nodes.assign(node_set.begin(), node_set.end());

    std::map<PredictedCoreNode, std::size_t> index;
    for (std::size_t k = 0; k < cert.nodes.size(); ++k)
        index.emplace(cert.nodes[k], k);

    WeightedDigraph graph(cert.nodes.size());
    for (std::size_t u = 0; u < cert.nodes.size(); ++u) {
        for (const auto& destination :
             predicted_core_successors(n, cert.nodes[u])) {
            const auto v = index.at(destination);
            graph.add_edge(u, v);
            cert.edges.emplace_back(u, v);
        }
    }

    cert.node_count_exact = cert.nodes.size() == predicted_core_size(n);
    cert.edge_count_exact = cert.edges.size() == predicted_core_edge_count(n);

    cert.predecessor_table_exact = true;
    for (std::size_t v = 0; v < cert.nodes.size(); ++v) {
        std::set<PredictedCoreNode> actual;
        for (const auto& [u, ignored] : graph.in_adj[v]) {
            (void)ignored;
            actual.insert(cert.nodes[u]);
        }
        cert.predecessor_table_exact &=
            actual == predicted_core_predecessors(n, cert.nodes[v]);
    }

    if (n == 3) {
        cert.previous_shadow.resize(cert.nodes.size());
        for (std::size_t k = 0; k < cert.nodes.size(); ++k)
            cert.previous_shadow[k] = k;
    } else {
        for (std::size_t k = 0; k < cert.nodes.size(); ++k)
            if (lies_in_previous_alphabet_shadow(n, cert.nodes[k]))
                cert.previous_shadow.push_back(k);
    }
    cert.base_or_shadow_present = !cert.previous_shadow.empty() &&
        (n == 3 || cert.previous_shadow.size() == predicted_core_size(n - 1));

    cert.from_shadow = build_multi_source_path_forest(
        graph, cert.previous_shadow, false);
    cert.to_shadow = build_multi_source_path_forest(
        graph, cert.previous_shadow, true);

    cert.every_new_node_reached_from_shadow = true;
    cert.every_new_node_reaches_shadow = true;
    cert.distance_formulas_exact = true;
    for (std::size_t k = 0; k < cert.nodes.size(); ++k) {
        if (n == 3 || lies_in_previous_alphabet_shadow(n, cert.nodes[k]))
            continue;
        cert.every_new_node_reached_from_shadow &=
            cert.from_shadow.distance[k] >= 0;
        cert.every_new_node_reaches_shadow &=
            cert.to_shadow.distance[k] >= 0;
        cert.distance_formulas_exact &=
            cert.from_shadow.distance[k] == static_cast<long long>(
                predicted_distance_from_previous_shadow(n, cert.nodes[k]));
        cert.distance_formulas_exact &=
            cert.to_shadow.distance[k] == static_cast<long long>(
                predicted_distance_to_previous_shadow(n, cert.nodes[k]));
    }

    const auto components = tarjan_scc(graph);
    cert.direct_tarjan_one_scc = components.size() == 1 &&
        components.front().size() == cert.nodes.size();

    cert.replay_checked = cert.node_count_exact && cert.edge_count_exact &&
        cert.predecessor_table_exact && cert.base_or_shadow_present &&
        cert.every_new_node_reached_from_shadow &&
        cert.every_new_node_reaches_shadow &&
        cert.distance_formulas_exact && cert.direct_tarjan_one_scc;
    return cert;
}



// Dimension-parametric, local shadow-path evidence.  Unlike
// certify_predicted_core_scc, this operation never enumerates the whole core
// graph.  It searches only the bounded predecessor/successor neighbourhood
// prescribed by the closed distance formulas (at most three steps outward and
// two steps inward), and returns concrete symbolic states whose edges replay
// through predicted_core_successors.
struct SymbolicShadowPathCertificate {
    std::size_t dimension = 0;
    PredictedCoreNode target;
    std::vector<PredictedCoreNode> from_shadow; // shadow -> target
    std::vector<PredictedCoreNode> to_shadow;   // target -> shadow
    bool target_member = false;
    bool endpoints_in_shadow = false;
    bool edges_replay = false;
    bool lengths_exact = false;
    bool derived = false;
    std::string obstruction;
};

inline bool replay_symbolic_core_path(
    std::size_t n,
    const std::vector<PredictedCoreNode>& path) {
    if (path.empty()) return false;
    for (const auto& node : path)
        if (!nbonacci_margin::predicted_core_member(n, node)) return false;
    for (std::size_t k = 0; k + 1 < path.size(); ++k)
        if (!nbonacci_margin::predicted_core_successors(n, path[k])
                 .contains(path[k + 1]))
            return false;
    return true;
}

inline std::vector<PredictedCoreNode> bounded_path_to_shadow(
    std::size_t n,
    const PredictedCoreNode& start,
    std::size_t maximum_depth,
    bool forward) {
    using Node = PredictedCoreNode;
    std::queue<Node> pending;
    std::map<Node, Node> parent;
    std::map<Node, std::size_t> depth;
    pending.push(start);
    depth.emplace(start, 0);

    std::optional<Node> found;
    while (!pending.empty()) {
        const Node current = pending.front();
        pending.pop();
        const auto d = depth.at(current);
        if (nbonacci_margin::lies_in_previous_alphabet_shadow(n, current)) {
            found = current;
            break;
        }
        if (d == maximum_depth) continue;
        const auto neighbours = forward
            ? nbonacci_margin::predicted_core_successors(n, current)
            : nbonacci_margin::predicted_core_predecessors(n, current);
        for (const auto& next : neighbours) {
            if (depth.contains(next)) continue;
            depth.emplace(next, d + 1);
            parent.emplace(next, current);
            pending.push(next);
        }
    }
    if (!found) return {};

    std::vector<Node> path{*found};
    Node current = *found;
    while (current != start) {
        current = parent.at(current);
        path.push_back(current);
    }
    // The search chain is shadow -> target for predecessor search, and
    // shadow <- target for successor search.
    if (forward) std::reverse(path.begin(), path.end());
    return path;
}

inline SymbolicShadowPathCertificate derive_symbolic_shadow_paths(
    std::size_t n,
    const PredictedCoreNode& target) {
    using namespace nbonacci_margin;
    SymbolicShadowPathCertificate cert;
    cert.dimension = n;
    cert.target = target;
    if (n < 4) {
        cert.obstruction = "symbolic shadow induction requires n >= 4";
        return cert;
    }
    cert.target_member = predicted_core_member(n, target);
    if (!cert.target_member) {
        cert.obstruction = "target is outside the predicted core";
        return cert;
    }
    if (lies_in_previous_alphabet_shadow(n, target)) {
        cert.from_shadow = {target};
        cert.to_shadow = {target};
        cert.endpoints_in_shadow = true;
        cert.edges_replay = true;
        cert.lengths_exact = true;
        cert.derived = true;
        return cert;
    }

    const auto outward = predicted_distance_from_previous_shadow(n, target);
    const auto inward = predicted_distance_to_previous_shadow(n, target);
    cert.from_shadow = bounded_path_to_shadow(n, target, outward, false);
    cert.to_shadow = bounded_path_to_shadow(n, target, inward, true);
    cert.endpoints_in_shadow =
        !cert.from_shadow.empty() && !cert.to_shadow.empty() &&
        lies_in_previous_alphabet_shadow(n, cert.from_shadow.front()) &&
        lies_in_previous_alphabet_shadow(n, cert.to_shadow.back()) &&
        cert.from_shadow.back() == target && cert.to_shadow.front() == target;
    cert.edges_replay = replay_symbolic_core_path(n, cert.from_shadow) &&
                        replay_symbolic_core_path(n, cert.to_shadow);
    cert.lengths_exact =
        cert.from_shadow.size() == outward + 1 &&
        cert.to_shadow.size() == inward + 1;
    cert.derived = cert.target_member && cert.endpoints_in_shadow &&
                   cert.edges_replay && cert.lengths_exact;
    if (!cert.derived)
        cert.obstruction = "bounded symbolic shadow path derivation failed";
    return cert;
}

inline bool replay_path_edges(
    const PredictedCoreSccCertificate& cert,
    const std::vector<std::size_t>& path) {
    if (path.empty()) return false;
    std::set<std::pair<std::size_t, std::size_t>> edges(
        cert.edges.begin(), cert.edges.end());
    for (std::size_t k = 0; k + 1 < path.size(); ++k)
        if (!edges.contains({path[k], path[k + 1]})) return false;
    return true;
}

}  // namespace ravel::proof
