// nbonacci_margin_core_graph_test.cpp
//
// Integer-only construction of the dimension-parametric candidate cyclic
// core.  Unlike the corona-backed certificate and the exact Q(beta) window
// synthesizer, this test uses only the compressed face/displacement grammar.
// It is therefore cheap enough to push well beyond the discovery range and
// is the substrate for mining constructive all-n connectivity witnesses.

#include "ravel/graph_divisor.hpp"
#include "ravel/nbonacci_margin_invariant.hpp"

#include <cstdio>
#include <queue>
#include <map>
#include <set>
#include <vector>

using namespace ravel;
using namespace ravel::nbonacci_margin;

namespace {

using CoreNode = CoreState;

int checks = 0;
int failures = 0;

void require(bool condition, const char* label, std::size_t n) {
    ++checks;
    if (condition) return;
    ++failures;
    std::printf("FAIL n=%zu: %s\n", n, label);
}

std::set<CoreNode> core_nodes(std::size_t n) {
    std::set<CoreNode> result;
    for (const LabelPair pair : label_pairs(n))
        for (const auto descriptor : displacement_descriptors(n))
            if (predicted_core_member(n, pair, descriptor))
                result.insert({pair, descriptor});
    return result;
}

std::set<std::pair<CoreNode, CoreNode>> core_edges(
    std::size_t n, const std::set<CoreNode>& nodes) {
    std::set<std::pair<CoreNode, CoreNode>> result;
    for (const CoreNode& source : nodes)
        for (const CoreNode& destination :
             predicted_core_successors(n, source))
            result.insert({source, destination});
    return result;
}

std::vector<long long> distances_from(
    const WeightedDigraph& graph, std::size_t start, bool reverse) {
    std::vector<long long> distance(graph.n, -1);
    std::queue<std::size_t> pending;
    distance[start] = 0;
    pending.push(start);
    while (!pending.empty()) {
        const auto source = pending.front();
        pending.pop();
        const auto& adjacency =
            reverse ? graph.in_adj[source] : graph.out_adj[source];
        for (const auto& [destination, ignored_weight] : adjacency) {
            if (distance[destination] >= 0) continue;
            distance[destination] = distance[source] + 1;
            pending.push(destination);
            (void)ignored_weight;
        }
    }
    return distance;
}

std::vector<long long> distances_from_set(
    const WeightedDigraph& graph, const std::vector<std::size_t>& starts,
    bool reverse) {
    std::vector<long long> distance(graph.n, -1);
    std::queue<std::size_t> pending;
    for (const auto start : starts) {
        distance[start] = 0;
        pending.push(start);
    }
    while (!pending.empty()) {
        const auto source = pending.front();
        pending.pop();
        const auto& adjacency =
            reverse ? graph.in_adj[source] : graph.out_adj[source];
        for (const auto& [destination, ignored_weight] : adjacency) {
            if (distance[destination] >= 0) continue;
            distance[destination] = distance[source] + 1;
            pending.push(destination);
            (void)ignored_weight;
        }
    }
    return distance;
}

}  // namespace

int main() {
    for (std::size_t n = 3; n <= 32; ++n) {
        const auto nodes = core_nodes(n);
        const auto edges = core_edges(n, nodes);
        require(nodes.size() == predicted_core_size(n),
                "direct node count", n);
        require(edges.size() == predicted_core_edge_count(n),
                "direct edge count", n);

        std::map<CoreNode, std::size_t> index;
        std::vector<CoreNode> order(nodes.begin(), nodes.end());
        for (std::size_t k = 0; k < order.size(); ++k) index[order[k]] = k;
        WeightedDigraph graph(order.size());
        std::set<LabelTransition> transitions;
        std::vector<std::size_t> outdegree(order.size(), 0);
        std::vector<std::size_t> indegree(order.size(), 0);
        for (const auto& [source, destination] : edges) {
            const auto u = index.at(source), v = index.at(destination);
            graph.add_edge(u, v);
            ++outdegree[u];
            ++indegree[v];
            transitions.insert({
                source.pair, destination.pair,
                (source.pair.j > 0 ? 1LL : 0LL) -
                    (source.pair.i > 0 ? 1LL : 0LL)});
        }
        bool inverse_table_exact = true;
        for (std::size_t k = 0; k < order.size(); ++k) {
            std::set<CoreNode> actual_predecessors;
            for (const auto& [u, ignored_weight] : graph.in_adj[k]) {
                actual_predecessors.insert(order[u]);
                (void)ignored_weight;
            }
            inverse_table_exact = inverse_table_exact &&
                actual_predecessors == predicted_core_predecessors(n, order[k]);
        }
        require(inverse_table_exact,
                "finite predecessor table inverts every edge", n);
        const auto components = tarjan_scc(graph);
        require(components.size() == 1 &&
                    components.front().size() == nodes.size(),
                "direct graph is strongly connected", n);
        require(transitions == label_transitions(n),
                "direct graph realizes every label transition", n);
        bool no_sources_or_sinks = true;
        for (std::size_t k = 0; k < order.size(); ++k)
            no_sources_or_sinks = no_sources_or_sinks &&
                                  outdegree[k] > 0 && indegree[k] > 0;
        require(no_sources_or_sinks, "every node has in/out edge", n);

        const CoreNode hub{
            {0, 0}, {DisplacementKind::Root, 1, n - 2, n - 1, 0}};
        const auto hub_it = index.find(hub);
        require(hub_it != index.end(), "canonical hub occurs", n);
        long long max_from_hub = -1, max_to_hub = -1;
        std::size_t farthest_from = 0, farthest_to = 0;
        if (hub_it != index.end()) {
            const auto from_hub = distances_from(graph, hub_it->second, false);
            const auto to_hub = distances_from(graph, hub_it->second, true);
            for (std::size_t k = 0; k < order.size(); ++k) {
                if (from_hub[k] > max_from_hub) {
                    max_from_hub = from_hub[k];
                    farthest_from = k;
                }
                if (to_hub[k] > max_to_hub) {
                    max_to_hub = to_hub[k];
                    farthest_to = k;
                }
            }
        }
        require(max_from_hub == static_cast<long long>(3 * n - 1) &&
                    max_to_hub == static_cast<long long>(3 * n - 1),
                "canonical hub radii are 3n-1", n);

        long long layer_out_radius = 0, layer_in_radius = 0;
        if (n >= 4) {
            std::vector<std::size_t> old_indices;
            for (std::size_t k = 0; k < order.size(); ++k)
                if (lies_in_previous_alphabet_shadow(n, order[k]))
                    old_indices.push_back(k);
            require(old_indices.size() == predicted_core_size(n - 1),
                    "previous-alphabet shadow has exact node count", n);
            const auto out_from_old =
                distances_from_set(graph, old_indices, false);
            const auto in_to_old =
                distances_from_set(graph, old_indices, true);
            bool layer_distance_formulas_exact = true;
            for (std::size_t k = 0; k < order.size(); ++k) {
                if (lies_in_previous_alphabet_shadow(n, order[k])) continue;
                layer_out_radius = std::max(layer_out_radius, out_from_old[k]);
                layer_in_radius = std::max(layer_in_radius, in_to_old[k]);
                layer_distance_formulas_exact =
                    layer_distance_formulas_exact &&
                    out_from_old[k] == static_cast<long long>(
                        predicted_distance_from_previous_shadow(n, order[k])) &&
                    in_to_old[k] == static_cast<long long>(
                        predicted_distance_to_previous_shadow(n, order[k]));
            }
            require(layer_distance_formulas_exact,
                    "bounded layer-distance formulas are exact", n);
            require(layer_out_radius == 3,
                    "new layer is reached from old layer within three", n);
            require(layer_in_radius == 2,
                    "new layer returns to old layer within two", n);
        }

        bool mirror_ok = true;
        for (const CoreNode& node : nodes) {
            auto mirrored_displacement = node.displacement;
            mirrored_displacement.sign = -mirrored_displacement.sign;
            const CoreNode mirror{
                {node.pair.j, node.pair.i}, mirrored_displacement};
            mirror_ok = mirror_ok && mirror != node && nodes.count(mirror) == 1;
        }
        for (const auto& [source, destination] : edges) {
            auto source_x = source.displacement;
            auto destination_x = destination.displacement;
            source_x.sign = -source_x.sign;
            destination_x.sign = -destination_x.sign;
            mirror_ok = mirror_ok && edges.count({
                {{source.pair.j, source.pair.i}, source_x},
                {{destination.pair.j, destination.pair.i}, destination_x}}) == 1;
        }
        require(mirror_ok, "free graph involution", n);

        bool endpoints_present = true;
        for (const LabelPair pair : label_pairs(n)) {
            const auto endpoints = endpoint_witnesses(n, pair);
            const auto lower = describe_displacement(endpoints.lower);
            const auto upper = describe_displacement(endpoints.upper);
            endpoints_present = endpoints_present && lower && upper &&
                nodes.count({pair, *lower}) == 1 &&
                nodes.count({pair, *upper}) == 1;
        }
        require(endpoints_present, "endpoint witnesses occur directly", n);

        std::printf("n=%zu direct core: %zu nodes, %zu edges, one SCC; "
                    "hub radii out=%lld in=%lld; far out=(%zu,%zu,%c%lld:%zu,%zu,%zu) "
                    "in=(%zu,%zu,%c%lld:%zu,%zu,%zu); layer out=%lld in=%lld\n",
                    n, nodes.size(), edges.size(), max_from_hub, max_to_hub,
                    order[farthest_from].pair.i, order[farthest_from].pair.j,
                    order[farthest_from].displacement.kind ==
                            DisplacementKind::Root ? 'R' : 'T',
                    order[farthest_from].displacement.sign,
                    order[farthest_from].displacement.a,
                    order[farthest_from].displacement.b,
                    order[farthest_from].displacement.c,
                    order[farthest_to].pair.i, order[farthest_to].pair.j,
                    order[farthest_to].displacement.kind ==
                            DisplacementKind::Root ? 'R' : 'T',
                    order[farthest_to].displacement.sign,
                    order[farthest_to].displacement.a,
                    order[farthest_to].displacement.b,
                    order[farthest_to].displacement.c,
                    layer_out_radius, layer_in_radius);
    }
    std::printf("n-bonacci direct core graph: %d checks, %d failures\n",
                checks, failures);
    return failures == 0 ? 0 : 1;
}
