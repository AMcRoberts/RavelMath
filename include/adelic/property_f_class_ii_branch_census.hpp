// Mechanical branch census for the Class-II Property-(F) rank spine.
// The high-rank tail is compared with the explicit affine spine; everything
// below the fixed collar height is retained as the finite symbolic branch
// grammar still needing an upper-bound proof.
#pragma once

#include <cstddef>
#include <set>
#include <vector>

#include "adelic/property_f_class_ii_rank_spine.hpp"

namespace adelic {

struct PropertyFClassIIBranchCensus {
    static constexpr std::size_t collar_height = 5;
    std::size_t parameter_a = 0;
    std::size_t maximum_height = 0;
    std::size_t tail_nodes = 0;
    std::size_t collar_nodes = 0;
    std::size_t tail_tail_edges = 0;
    std::size_t tail_nonchain_edges = 0;
    std::size_t tail_internal_edges = 0;
    std::size_t tail_collar_edges = 0;
    std::size_t tail_chain_edges = 0;
    std::size_t tail_non_spine_nodes = 0;
    bool rank_valid = false;
    bool spine_valid = false;
    bool tail_layers_unique = false;
    bool tail_is_spine = false;
    bool tail_chain_replayed = false;
    bool valid = false;
};

inline PropertyFClassIIBranchCensus derive_property_f_class_ii_branch_census(
        const PropertyFGraph& graph, std::size_t a) {
    PropertyFClassIIBranchCensus out;
    out.parameter_a = a;
    const auto rank = derive_property_f_escape_rank(graph);
    out.rank_valid = rank.valid;
    if (!rank.valid) return out;
    out.maximum_height = rank.maximum_height;
    const auto spine = derive_property_f_class_ii_rank_spine(graph, a);
    out.spine_valid = spine.valid;
    if (!spine.valid) return out;

    std::set<std::size_t> spine_tail;
    for (const auto node : spine.node_ids)
        if (rank.node_height[node] > PropertyFClassIIBranchCensus::collar_height)
            spine_tail.insert(node);

    std::vector<std::size_t> per_height(out.maximum_height + 1, 0);
    std::set<std::size_t> observed_tail;
    for (std::size_t node = 0; node < graph.nodes.size(); ++node) {
        const auto height = rank.node_height[node];
        if (height > PropertyFClassIIBranchCensus::collar_height) {
            ++out.tail_nodes;
            ++per_height[height];
            observed_tail.insert(node);
        } else {
            ++out.collar_nodes;
        }
    }
    out.tail_layers_unique = true;
    for (std::size_t height = PropertyFClassIIBranchCensus::collar_height + 1;
         height <= out.maximum_height; ++height)
        out.tail_layers_unique &= per_height[height] == 1;
    out.tail_is_spine = observed_tail == spine_tail;

    for (std::size_t source = 0; source < graph.nodes.size(); ++source) {
        if (rank.node_height[source] <=
            PropertyFClassIIBranchCensus::collar_height)
            continue;
        for (const auto raw_target : graph.nodes[source].successors) {
            if (raw_target < 0 ||
                static_cast<std::size_t>(raw_target) >= graph.nodes.size()) {
                out.tail_chain_replayed = false;
                continue;
            }
            const auto target = static_cast<std::size_t>(raw_target);
            if (graph.scc_labels[source] == graph.scc_labels[target]) {
                ++out.tail_internal_edges;
                continue;
            }
            if (rank.node_height[target] >
                PropertyFClassIIBranchCensus::collar_height) {
                ++out.tail_tail_edges;
                if (rank.node_height[source] == rank.node_height[target] + 1)
                    ++out.tail_chain_edges;
                else
                    ++out.tail_nonchain_edges;
            } else {
                ++out.tail_collar_edges;
            }
        }
    }
    const auto expected_chain_edges = out.maximum_height >
            PropertyFClassIIBranchCensus::collar_height
        ? out.maximum_height - PropertyFClassIIBranchCensus::collar_height - 1
        : 0;
    out.tail_chain_replayed = out.tail_chain_edges == expected_chain_edges;
    for (const auto node : observed_tail)
        if (!spine_tail.count(node)) ++out.tail_non_spine_nodes;
    out.valid = out.rank_valid && out.spine_valid &&
                out.tail_layers_unique && out.tail_is_spine &&
                out.tail_chain_replayed && out.tail_nonchain_edges == 0;
    return out;
}

}  // namespace adelic
