#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ravel::proof {

struct PositiveWordGraphEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::vector<std::size_t> word;
};

struct PrimitiveExpandedEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t generator = 0;
    std::size_t macro_edge = 0;
    std::size_t position = 0;
};

struct PositiveWordGraphExpansionProof {
    std::size_t base_node_count = 0;
    std::size_t expanded_node_count = 0;
    std::size_t generator_count = 0;
    std::vector<PositiveWordGraphEdge> macro_edges;
    std::vector<PrimitiveExpandedEdge> primitive_edges;
    std::vector<std::vector<std::vector<long long>>> generator_adjacency;
    std::vector<std::vector<long long>> reconstructed_macro_adjacency;
    bool every_macro_word_nonempty = false;
    bool private_intermediates_exact = false;
    bool first_return_recovers_macro_graph = false;
    bool proved = false;
    std::string obstruction;
};

/** Expand each word-labelled macro edge into a primitive path.
 *
 * Every internal vertex is private to one macro edge. Consequently paths from
 * base vertices to base vertices that avoid base vertices internally are in
 * bijection with the input macro edges, and their generator word is exactly
 * the supplied label. This is the finite positional scheduler needed to feed
 * a macro transport graph into the free-positive-word theorem.
 */
inline PositiveWordGraphExpansionProof derive_positive_word_graph_expansion(
    std::size_t base_node_count,
    std::size_t generator_count,
    const std::vector<PositiveWordGraphEdge>& macro_edges) {
    PositiveWordGraphExpansionProof out;
    out.base_node_count = base_node_count;
    out.expanded_node_count = base_node_count;
    out.generator_count = generator_count;
    out.macro_edges = macro_edges;
    out.every_macro_word_nonempty = true;
    out.private_intermediates_exact = true;
    out.reconstructed_macro_adjacency.assign(
        base_node_count, std::vector<long long>(base_node_count, 0));

    for (std::size_t e = 0; e < macro_edges.size(); ++e) {
        const auto& macro = macro_edges[e];
        if (macro.source >= base_node_count || macro.target >= base_node_count) {
            out.obstruction = "macro edge endpoint outside base graph";
            return out;
        }
        if (macro.word.empty()) {
            out.every_macro_word_nonempty = false;
            out.obstruction = "empty macro word cannot be expanded";
            return out;
        }
        std::size_t current = macro.source;
        for (std::size_t k = 0; k < macro.word.size(); ++k) {
            if (macro.word[k] >= generator_count) {
                out.obstruction = "macro word contains unknown generator";
                return out;
            }
            const bool final = k + 1 == macro.word.size();
            const std::size_t next = final ? macro.target : out.expanded_node_count++;
            out.primitive_edges.push_back(
                {current, next, macro.word[k], e, k});
            current = next;
        }
        ++out.reconstructed_macro_adjacency[macro.source][macro.target];
    }

    out.generator_adjacency.assign(
        generator_count,
        std::vector<std::vector<long long>>(
            out.expanded_node_count,
            std::vector<long long>(out.expanded_node_count, 0)));
    for (const auto& edge : out.primitive_edges)
        ++out.generator_adjacency[edge.generator][edge.source][edge.target];

    // Every non-base node must have exactly one incoming and one outgoing edge,
    // both belonging to the same macro edge at consecutive positions.
    std::vector<std::vector<std::size_t>> incoming(out.expanded_node_count);
    std::vector<std::vector<std::size_t>> outgoing(out.expanded_node_count);
    for (std::size_t k = 0; k < out.primitive_edges.size(); ++k) {
        incoming[out.primitive_edges[k].target].push_back(k);
        outgoing[out.primitive_edges[k].source].push_back(k);
    }
    for (std::size_t node = base_node_count; node < out.expanded_node_count; ++node) {
        if (incoming[node].size() != 1 || outgoing[node].size() != 1) {
            out.private_intermediates_exact = false;
            break;
        }
        const auto& in = out.primitive_edges[incoming[node][0]];
        const auto& next = out.primitive_edges[outgoing[node][0]];
        if (in.macro_edge != next.macro_edge || in.position + 1 != next.position) {
            out.private_intermediates_exact = false;
            break;
        }
    }

    out.first_return_recovers_macro_graph = out.private_intermediates_exact;
    out.proved = out.every_macro_word_nonempty &&
                 out.private_intermediates_exact &&
                 out.first_return_recovers_macro_graph;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "positive-word graph expansion invariant failed";
    return out;
}

} // namespace ravel::proof
