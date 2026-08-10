// Exact finite birth-round/transport grammar for a closed Property-(F) graph.
//
// This is the reusable finite derivation boundary for the adelic bridge.  It
// derives the birth round from the independently replayed SCC condensation,
// then records every edge as an internal transport step or a strictly
// descending shell transition.  It deliberately does not infer a
// parameterized family formula from one graph; a caller must still supply the
// ordered-prefix grammar when making a uniform theorem.
#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "adelic/property_f_escape_rank.hpp"

namespace adelic {

struct PropertyFBirthRoundTransition {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t source_round = 0;
    std::size_t target_round = 0;
    std::size_t drop = 0;
    bool internal_scc = false;
    std::string transport_label;
};

struct PropertyFBirthRoundGrammar {
    std::size_t node_count = 0;
    std::size_t edge_count = 0;
    std::size_t layer_count = 0;
    std::size_t terminal_layer_nodes = 0;
    std::size_t internal_edges = 0;
    std::size_t cross_scc_edges = 0;
    std::size_t nondecreasing_cross_scc_edges = 0;
    std::map<std::size_t, std::size_t> layer_sizes;
    std::map<std::size_t, std::size_t> drop_histogram;
    std::vector<std::size_t> node_round;
    std::vector<PropertyFBirthRoundTransition> transitions;
    bool rank_valid = false;
    bool terminal_layer_zero = false;
    bool edge_rank_strictly_decreasing = false;
    bool valid = false;
};

inline std::string property_f_birth_round_digit_key(
        const std::vector<std::pair<std::string, std::string>>& coefficients) {
    std::string key;
    for (const auto& [numerator, denominator] : coefficients) {
        key += numerator;
        key += '/';
        key += denominator;
        key += ';';
    }
    return key;
}

inline PropertyFBirthRoundGrammar derive_property_f_birth_round_grammar(
        const PropertyFGraph& graph) {
    PropertyFBirthRoundGrammar out;
    out.node_count = graph.nodes.size();
    const auto rank = derive_property_f_escape_rank(graph);
    out.rank_valid = rank.valid;
    if (!rank.valid) return out;

    out.node_round = rank.node_height;
    for (const auto round : out.node_round) {
        ++out.layer_sizes[round];
        out.layer_count = std::max(out.layer_count, round + 1);
    }
    for (const auto& node : graph.nodes)
        if (node.successors.empty()) ++out.terminal_layer_nodes;
    out.terminal_layer_zero = true;
    for (std::size_t node = 0; node < graph.nodes.size(); ++node)
        if (graph.nodes[node].successors.empty() && out.node_round[node] != 0)
            out.terminal_layer_zero = false;

    for (std::size_t source = 0; source < graph.nodes.size(); ++source) {
        out.edge_count += graph.nodes[source].successors.size();
        for (std::size_t edge = 0;
             edge < graph.nodes[source].successors.size(); ++edge) {
            const auto raw_target = graph.nodes[source].successors[edge];
            const auto target = static_cast<std::size_t>(raw_target);
            const auto label = edge < graph.nodes[source].edge_digit_coefficients.size()
                ? property_f_birth_round_digit_key(
                      graph.nodes[source].edge_digit_coefficients[edge])
                : std::string{};
            const bool internal = graph.scc_labels[source] ==
                                  graph.scc_labels[target];
            if (internal) {
                ++out.internal_edges;
                out.transitions.push_back({source, target,
                    out.node_round[source], out.node_round[target], 0, true,
                    label});
                continue;
            }
            ++out.cross_scc_edges;
            if (out.node_round[source] <= out.node_round[target]) {
                ++out.nondecreasing_cross_scc_edges;
                out.transitions.push_back({source, target,
                    out.node_round[source], out.node_round[target], 0, false,
                    label});
                continue;
            }
            const auto drop = out.node_round[source] - out.node_round[target];
            ++out.drop_histogram[drop];
            out.transitions.push_back({source, target,
                out.node_round[source], out.node_round[target], drop, false,
                label});
        }
    }
    out.edge_rank_strictly_decreasing =
        out.nondecreasing_cross_scc_edges == 0;
    out.valid = out.rank_valid && out.terminal_layer_zero &&
                out.edge_rank_strictly_decreasing;
    return out;
}

}  // namespace adelic
