// Exact boundary-layer rank for a finite Property-(F) graph.
//
// The Property-F graph is already SCC-labelled by its exact construction.
// Collapsing those SCCs gives an acyclic condensation graph.  The longest
// distance from a condensation vertex to a sink is therefore a canonical
// finite boundary-layer coordinate: it decreases whenever an edge leaves a
// layer, while transport inside an SCC is left for a secondary fibre rank.
#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <utility>
#include <vector>

#include "adelic/property_f_types.hpp"

namespace adelic {

struct PropertyFEscapeRankCertificate {
    std::size_t node_count = 0;
    std::size_t scc_count = 0;
    std::size_t condensation_edges = 0;
    std::size_t terminal_sccs = 0;
    std::size_t maximum_height = 0;
    std::vector<std::vector<std::size_t>> condensation;
    std::vector<std::size_t> scc_height;
    std::vector<std::size_t> node_height;
    bool labels_replayed = false;
    // Independent SCC replay prevents a coarser-than-SCC label table from
    // being mistaken for a valid boundary quotient.  Acyclicity alone would
    // not catch two distinct SCCs merged into one supplied label.
    std::size_t recomputed_scc_count = 0;
    bool scc_partition_replayed = false;
    bool condensation_acyclic = false;
    // Recheck the defining consequence of the longest-distance rank on the
    // original node edges.  This catches malformed node/label projections
    // even when the condensation calculation itself is well formed.
    std::size_t cross_scc_edges = 0;
    std::size_t cross_scc_nondecreasing_edges = 0;
    bool edge_rank_strictly_decreasing = false;
    bool valid = false;
};

inline PropertyFEscapeRankCertificate derive_property_f_escape_rank(
        const PropertyFGraph& graph) {
    PropertyFEscapeRankCertificate out;
    out.node_count = graph.nodes.size();
    if (graph.scc_labels.size() != out.node_count)
        throw std::invalid_argument("Property-F escape rank: missing SCC labels");
    for (const auto label : graph.scc_labels) {
        if (label < 0)
            throw std::invalid_argument("Property-F escape rank: negative SCC label");
        out.scc_count = std::max(out.scc_count,
                                 static_cast<std::size_t>(label) + 1);
    }
    out.condensation.assign(out.scc_count, {});
    std::vector<std::size_t> indegree(out.scc_count, 0);
    out.labels_replayed = true;
    std::vector<std::size_t> observed_sizes(out.scc_count, 0);
    for (const auto label : graph.scc_labels)
        ++observed_sizes[static_cast<std::size_t>(label)];
    for (const auto size : observed_sizes)
        out.labels_replayed &= size > 0;
    if (!graph.scc_sizes.empty()) {
        out.labels_replayed &= graph.scc_sizes.size() == out.scc_count;
        if (graph.scc_sizes.size() == out.scc_count)
            for (std::size_t scc = 0; scc < out.scc_count; ++scc)
                out.labels_replayed &=
                    graph.scc_sizes[scc] == static_cast<long long>(observed_sizes[scc]);
    }
    // Recompute SCCs independently with iterative Kosaraju.  The Property-F
    // search already stores labels, but a rank certificate must not trust
    // those labels merely because their quotient happens to be acyclic.
    std::vector<std::vector<std::size_t>> reverse(out.node_count);
    for (std::size_t source = 0; source < out.node_count; ++source) {
        for (const auto target : graph.nodes[source].successors) {
            if (target < 0 || static_cast<std::size_t>(target) >= out.node_count)
                throw std::invalid_argument(
                    "Property-F escape rank: successor out of range");
            reverse[static_cast<std::size_t>(target)].push_back(source);
        }
    }
    std::vector<bool> visited(out.node_count, false);
    std::vector<std::size_t> dfs_order;
    dfs_order.reserve(out.node_count);
    for (std::size_t root = 0; root < out.node_count; ++root) {
        if (visited[root]) continue;
        std::vector<std::pair<std::size_t, std::size_t>> stack;
        stack.push_back({root, 0});
        visited[root] = true;
        while (!stack.empty()) {
            auto& frame = stack.back();
            const auto node = frame.first;
            if (frame.second == graph.nodes[node].successors.size()) {
                dfs_order.push_back(node);
                stack.pop_back();
                continue;
            }
            const auto target = static_cast<std::size_t>(
                graph.nodes[node].successors[frame.second++]);
            if (!visited[target]) {
                visited[target] = true;
                stack.push_back({target, 0});
            }
        }
    }
    std::vector<long long> recomputed_label(out.node_count, -1);
    for (auto it = dfs_order.rbegin(); it != dfs_order.rend(); ++it) {
        const auto root = *it;
        if (recomputed_label[root] >= 0) continue;
        const auto component = static_cast<long long>(out.recomputed_scc_count++);
        std::vector<std::size_t> stack{root};
        recomputed_label[root] = component;
        while (!stack.empty()) {
            const auto node = stack.back();
            stack.pop_back();
            for (const auto predecessor : reverse[node]) {
                if (recomputed_label[predecessor] < 0) {
                    recomputed_label[predecessor] = component;
                    stack.push_back(predecessor);
                }
            }
        }
    }
    std::vector<long long> supplied_to_recomputed(out.scc_count, -1);
    std::vector<long long> recomputed_to_supplied(out.recomputed_scc_count, -1);
    out.scc_partition_replayed = true;
    for (std::size_t node = 0; node < out.node_count; ++node) {
        const auto supplied = static_cast<std::size_t>(graph.scc_labels[node]);
        const auto recomputed = static_cast<std::size_t>(recomputed_label[node]);
        if (supplied_to_recomputed[supplied] < 0)
            supplied_to_recomputed[supplied] =
                static_cast<long long>(recomputed);
        else if (supplied_to_recomputed[supplied] !=
                 static_cast<long long>(recomputed))
            out.scc_partition_replayed = false;
        if (recomputed_to_supplied[recomputed] < 0)
            recomputed_to_supplied[recomputed] =
                static_cast<long long>(supplied);
        else if (recomputed_to_supplied[recomputed] !=
                 static_cast<long long>(supplied))
            out.scc_partition_replayed = false;
    }
    out.scc_partition_replayed = out.scc_partition_replayed &&
        out.recomputed_scc_count == out.scc_count;
    for (std::size_t source = 0; source < out.node_count; ++source) {
        const auto from = static_cast<std::size_t>(graph.scc_labels[source]);
        for (const auto target : graph.nodes[source].successors) {
            const auto to = static_cast<std::size_t>(
                graph.scc_labels[static_cast<std::size_t>(target)]);
            if (from != to) out.condensation[from].push_back(to);
        }
    }
    for (auto& row : out.condensation) {
        std::sort(row.begin(), row.end());
        row.erase(std::unique(row.begin(), row.end()), row.end());
        row.erase(std::remove_if(row.begin(), row.end(),
                                 [&](const auto target) {
                                     return target >= out.scc_count;
                                 }), row.end());
    }
    // Rebuild the rows without internal edges.  Keeping this as a separate
    // pass makes malformed labels visible before the topological calculation.
    for (std::size_t source = 0; source < out.scc_count; ++source) {
        auto& row = out.condensation[source];
        row.erase(std::remove(row.begin(), row.end(), source), row.end());
        out.condensation_edges += row.size();
        for (const auto target : row) ++indegree[target];
        if (row.empty()) ++out.terminal_sccs;
    }
    std::deque<std::size_t> queue;
    for (std::size_t scc = 0; scc < out.scc_count; ++scc)
        if (indegree[scc] == 0) queue.push_back(scc);
    std::vector<std::size_t> order;
    order.reserve(out.scc_count);
    while (!queue.empty()) {
        const auto source = queue.front();
        queue.pop_front();
        order.push_back(source);
        for (const auto target : out.condensation[source])
            if (--indegree[target] == 0) queue.push_back(target);
    }
    out.condensation_acyclic = order.size() == out.scc_count;
    out.scc_height.assign(out.scc_count, 0);
    if (out.condensation_acyclic) {
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            for (const auto target : out.condensation[*it])
                out.scc_height[*it] = std::max(
                    out.scc_height[*it], out.scc_height[target] + 1);
            out.maximum_height = std::max(out.maximum_height,
                                          out.scc_height[*it]);
        }
    }
    out.node_height.assign(out.node_count, 0);
    for (std::size_t node = 0; node < out.node_count; ++node)
        out.node_height[node] = out.scc_height[
            static_cast<std::size_t>(graph.scc_labels[node])];
    if (out.condensation_acyclic) {
        for (std::size_t source = 0; source < out.node_count; ++source) {
            for (const auto raw_target : graph.nodes[source].successors) {
                const auto target = static_cast<std::size_t>(raw_target);
                if (graph.scc_labels[source] == graph.scc_labels[target])
                    continue;
                ++out.cross_scc_edges;
                if (out.node_height[source] <= out.node_height[target])
                    ++out.cross_scc_nondecreasing_edges;
            }
        }
        out.edge_rank_strictly_decreasing =
            out.cross_scc_nondecreasing_edges == 0;
    }
    out.valid = out.labels_replayed && out.scc_partition_replayed &&
                out.condensation_acyclic;
    return out;
}

}  // namespace adelic
