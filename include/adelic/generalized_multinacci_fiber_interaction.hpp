// Classifies how the newly added terminal fiber meets inherited letters in a
// finite Property-F graph.  Graph edges use the checker's backward direction:
// target state -> predecessor state.
#pragma once

#include <array>
#include <cstddef>

#include "adelic/property_f_types.hpp"

namespace adelic {

struct GeneralizedMultinacciFiberInteractionSummary {
    std::array<std::array<long long, 3>, 3> edge_counts{};
    long long new_nodes = 0;
    long long new_nontrivial_scc_nodes = 0;
    long long new_zero_nontrivial_scc_nodes = 0;
    long long new_to_new_edges = 0;
    long long top_to_new_edges = 0;
    long long new_to_top_edges = 0;
    long long inherited_to_new_edges = 0;
    bool one_way_new_fiber = false;
};

template <std::size_t d>
GeneralizedMultinacciFiberInteractionSummary
classify_generalized_multinacci_fiber_interaction(
    const PropertyFGraph& graph) {
    static_assert(d >= 3, "fiber interaction requires a lifted dimension");
    GeneralizedMultinacciFiberInteractionSummary out;
    auto fiber_class = [](long long letter) {
        if (letter == static_cast<long long>(d - 1)) return 2; // new terminal
        if (letter == static_cast<long long>(d - 2)) return 1; // old top
        return 0;                                             // inherited
    };
    for (const auto& node : graph.nodes)
        if (fiber_class(node.letter) == 2) ++out.new_nodes;
    for (std::size_t u = 0; u < graph.nodes.size(); ++u) {
        const int source = fiber_class(graph.nodes[u].letter);
        for (const auto raw_v : graph.nodes[u].successors) {
            if (raw_v < 0 || raw_v >= static_cast<long long>(graph.nodes.size()))
                continue;
            const int target = fiber_class(
                graph.nodes[static_cast<std::size_t>(raw_v)].letter);
            ++out.edge_counts[source][target];
        }
    }
    out.inherited_to_new_edges = out.edge_counts[0][2];
    out.top_to_new_edges = out.edge_counts[1][2];
    out.new_to_top_edges = out.edge_counts[2][1];
    out.new_to_new_edges = out.edge_counts[2][2];
    for (std::size_t i = 0; i < graph.nodes.size(); ++i) {
        if (fiber_class(graph.nodes[i].letter) != 2) continue;
        const auto scc = graph.scc_labels.empty() ? -1 : graph.scc_labels[i];
        if (scc < 0 || scc >= static_cast<long long>(graph.scc_sizes.size()) ||
            graph.scc_sizes[static_cast<std::size_t>(scc)] <= 1)
            continue;
        ++out.new_nontrivial_scc_nodes;
        if (graph.nodes[i].zero) ++out.new_zero_nontrivial_scc_nodes;
    }
    out.one_way_new_fiber = out.new_to_new_edges == 0 &&
                            out.top_to_new_edges == 0;
    return out;
}

}  // namespace adelic
