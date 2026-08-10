// Finite symbolic collar grammar for the Class-II Property-(F) family.
//
// The rank-spine certificate isolates the high-rank affine orbit.  This
// companion extractor retains the low-height transition language as an
// explicit finite object, so a future family proof can bound the collar
// without treating the full expanding graph as one undifferentiated search.
#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <set>
#include <utility>
#include <vector>

#include "adelic/property_f_birth_round_grammar.hpp"

namespace adelic {

struct PropertyFClassIICollarGrammar {
    static constexpr std::size_t collar_height = 5;

    std::size_t parameter_a = 0;
    std::size_t node_count = 0;
    std::size_t collar_nodes = 0;
    std::size_t tail_nodes = 0;
    std::size_t collar_edges = 0;
    std::size_t collar_internal_edges = 0;
    std::size_t collar_cross_scc_edges = 0;
    std::size_t collar_to_tail_edges = 0;
    std::size_t branching_nodes = 0;
    std::size_t maximum_target_height = 0;
    std::map<std::size_t, std::size_t> layer_sizes;
    std::map<std::pair<std::size_t, std::size_t>, std::size_t>
        height_transition_counts;
    std::vector<PropertyFBirthRoundTransition> transitions;
    bool rank_valid = false;
    bool no_tail_reentry = false;
    bool no_collar_internal_transport = false;
    bool height_support_valid = false;
    bool finite = false;
    bool valid = false;
};

inline const std::set<std::pair<std::size_t, std::size_t>>&
property_f_class_ii_collar_height_support() {
    static const std::set<std::pair<std::size_t, std::size_t>> support{
        {1, 0},
        {2, 0}, {2, 1},
        {3, 0}, {3, 1}, {3, 2},
        {4, 1}, {4, 2}, {4, 3},
        {5, 2}, {5, 3}, {5, 4}};
    return support;
}

inline PropertyFClassIICollarGrammar derive_property_f_class_ii_collar_grammar(
        const PropertyFGraph& graph, std::size_t a) {
    PropertyFClassIICollarGrammar out;
    out.parameter_a = a;
    out.node_count = graph.nodes.size();
    const auto rank = derive_property_f_escape_rank(graph);
    out.rank_valid = rank.valid;
    if (!rank.valid) return out;

    for (const auto height : rank.node_height) {
        ++out.layer_sizes[height];
        if (height <= PropertyFClassIICollarGrammar::collar_height)
            ++out.collar_nodes;
        else
            ++out.tail_nodes;
    }
    out.no_tail_reentry = true;
    out.no_collar_internal_transport = true;
    out.finite = true;
    out.transitions.reserve(out.collar_nodes);
    for (std::size_t source = 0; source < graph.nodes.size(); ++source) {
        const auto source_height = rank.node_height[source];
        if (source_height > PropertyFClassIICollarGrammar::collar_height)
            continue;
        if (graph.nodes[source].successors.size() > 1)
            ++out.branching_nodes;
        for (std::size_t edge = 0;
             edge < graph.nodes[source].successors.size(); ++edge) {
            const auto raw_target = graph.nodes[source].successors[edge];
            if (raw_target < 0 ||
                static_cast<std::size_t>(raw_target) >= graph.nodes.size()) {
                out.finite = false;
                continue;
            }
            const auto target = static_cast<std::size_t>(raw_target);
            const auto target_height = rank.node_height[target];
            ++out.collar_edges;
            ++out.height_transition_counts[{source_height, target_height}];
            out.maximum_target_height =
                std::max(out.maximum_target_height, target_height);
            const bool internal = graph.scc_labels[source] ==
                                  graph.scc_labels[target];
            if (internal) {
                ++out.collar_internal_edges;
                out.no_collar_internal_transport = false;
            } else {
                ++out.collar_cross_scc_edges;
            }
            if (target_height > PropertyFClassIICollarGrammar::collar_height) {
                ++out.collar_to_tail_edges;
                out.no_tail_reentry = false;
            }
            const auto label = edge < graph.nodes[source].edge_digit_coefficients.size()
                ? property_f_birth_round_digit_key(
                      graph.nodes[source].edge_digit_coefficients[edge])
                : std::string{};
            const auto drop = source_height > target_height
                ? source_height - target_height : 0;
            out.transitions.push_back({source, target, source_height,
                target_height, drop, internal, label});
        }
    }
    out.height_support_valid = true;
    for (const auto& [heights, count] : out.height_transition_counts) {
        (void)count;
        if (!property_f_class_ii_collar_height_support().count(heights))
            out.height_support_valid = false;
    }
    if (a >= 4)
        for (const auto& heights : property_f_class_ii_collar_height_support())
            if (out.height_transition_counts[heights] == 0)
                out.height_support_valid = false;
    out.valid = out.rank_valid && out.finite && out.no_tail_reentry;
    if (a >= 4) out.valid = out.valid && out.height_support_valid;
    if (a >= 4) out.valid = out.valid && out.no_collar_internal_transport;
    return out;
}

}  // namespace adelic
