// Candidate-digit census on the realized Class-II affine tail.
//
// This records every outgoing transport edge from a spine state and separates
// the expected affine successor, collar exits, and any unexpected high-tail
// successor.  Internal SCC recurrence is retained as its own category.
#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>

#include "adelic/property_f_class_ii_rank_spine.hpp"
#include "adelic/property_f_class_ii_prefix_role_grammar.hpp"

namespace adelic {

struct PropertyFClassIITailCandidateCensus {
    static constexpr std::size_t collar_height = 5;

    std::size_t parameter_a = 0;
    std::size_t tail_sources = 0;
    std::size_t expected_edges = 0;
    std::size_t alternate_collar_edges = 0;
    std::size_t alternate_high_edges = 0;
    std::size_t internal_edges = 0;
    std::size_t malformed_edges = 0;
    std::map<std::size_t, std::set<std::string>> collar_labels_by_step;
    std::map<std::size_t, std::set<std::string>> high_labels_by_step;
    bool spine_valid = false;
    bool expected_successor_unique = false;
    bool no_alternate_high = false;
    bool digit_support_valid = false;
    bool role_grammar_valid = false;
    bool valid = false;
};

inline PropertyFClassIITailCandidateCensus
derive_property_f_class_ii_tail_candidate_census(
        const PropertyFGraph& graph, std::size_t a) {
    PropertyFClassIITailCandidateCensus out;
    out.parameter_a = a;
    out.role_grammar_valid =
        derive_property_f_class_ii_prefix_role_grammar(a).valid;
    const auto rank = derive_property_f_escape_rank(graph);
    if (!rank.valid) return out;
    const auto spine = derive_property_f_class_ii_rank_spine(graph, a);
    out.spine_valid = spine.valid;
    if (!spine.valid) return out;

    std::map<std::size_t, std::size_t> step_by_node;
    for (std::size_t step = 0; step < spine.node_ids.size(); ++step)
        step_by_node[spine.node_ids[step]] = step;

    out.expected_successor_unique = true;
    for (const auto [source, step] : step_by_node) {
        if (rank.node_height[source] <=
            PropertyFClassIITailCandidateCensus::collar_height)
            continue;
        ++out.tail_sources;
        const auto expected_target = step + 1 < spine.node_ids.size()
            ? spine.node_ids[step + 1] : graph.nodes.size();
        std::size_t expected_count = 0;
        for (std::size_t edge = 0;
             edge < graph.nodes[source].successors.size(); ++edge) {
            const auto raw_target = graph.nodes[source].successors[edge];
            if (raw_target < 0 ||
                static_cast<std::size_t>(raw_target) >= graph.nodes.size()) {
                ++out.malformed_edges;
                continue;
            }
            const auto target = static_cast<std::size_t>(raw_target);
            const auto label = edge < graph.nodes[source].edge_digit_coefficients.size()
                ? property_f_birth_round_digit_key(
                      graph.nodes[source].edge_digit_coefficients[edge])
                : std::string{};
            if (graph.scc_labels[source] == graph.scc_labels[target]) {
                ++out.internal_edges;
                continue;
            }
            if (target == expected_target) {
                ++expected_count;
                ++out.expected_edges;
                continue;
            }
            if (rank.node_height[target] >
                PropertyFClassIITailCandidateCensus::collar_height) {
                ++out.alternate_high_edges;
                out.high_labels_by_step[step].insert(label);
            } else {
                ++out.alternate_collar_edges;
                out.collar_labels_by_step[step].insert(label);
            }
        }
        // The last tail state has a collar successor; every other tail state
        // has exactly one expected spine successor.  A missing or duplicated
        // expected edge is a branch-grammar failure.
        if (expected_count != 1) out.expected_successor_unique = false;
    }
    out.no_alternate_high = out.alternate_high_edges == 0;
    // The Class-II prefix automaton gives every letter-0 tail state the
    // complete zero-prefix digit family j=0..a-1.  Letter 1 has only its
    // forced return edge.  Keep this inexpensive role check bounded so the
    // certificate never turns a large parameter into an unbounded loop.
    if (a <= 10'000) {
        out.digit_support_valid = true;
        for (const auto [source, step] : step_by_node) {
            if (rank.node_height[source] <=
                PropertyFClassIITailCandidateCensus::collar_height)
                continue;
            const auto& labels = out.collar_labels_by_step[step];
            if (graph.nodes[source].letter == 0) {
                for (std::size_t digit = 0; digit < a; ++digit)
                    if (!labels.count(property_f_class_ii_spine_digit_key(
                            static_cast<long long>(digit))))
                        out.digit_support_valid = false;
            } else if (graph.nodes[source].letter == 1) {
                if (!labels.empty()) out.digit_support_valid = false;
            } else {
                out.digit_support_valid = false;
            }
        }
    }
    out.valid = out.spine_valid && out.malformed_edges == 0 &&
                out.expected_successor_unique && out.no_alternate_high &&
                out.digit_support_valid && out.role_grammar_valid;
    return out;
}

}  // namespace adelic
