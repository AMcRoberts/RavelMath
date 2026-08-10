// Realized-graph census for the finite Akiyama deviation window.
//
// This joins the symbolic five-value hypothesis to actual Class-II graph
// edges.  It decodes ordinary integer prefix labels on high-tail sources,
// computes their deviations from the forced digit, and records which of the
// candidates satisfy the transported |eta|<=2 proxy.  A nonzero bounded
// edge returning to the high tail is an explicit falsifier; bounded edges
// landing in the collar are the finite cases still needing a symbolic proof.
#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "adelic/property_f_class_ii_akiyama_window.hpp"
#include "adelic/property_f_class_ii_rank_spine.hpp"
#include "adelic/property_f_escape_rank.hpp"

namespace adelic {

struct PropertyFClassIIAkiyamaWindowCensus {
    static constexpr std::size_t collar_height = 5;

    std::size_t parameter_a = 0;
    std::size_t tail_sources = 0;
    std::size_t outgoing_edges = 0;
    std::size_t ordinary_edges = 0;
    std::size_t nonordinary_edges = 0;
    std::size_t bounded_candidates = 0;
    std::size_t bounded_collar_edges = 0;
    std::size_t bounded_high_edges = 0;
    std::size_t bounded_internal_high_edges = 0;
    std::size_t bounded_high_nonzero_deviations = 0;
    std::size_t malformed_edges = 0;
    std::map<long long, std::size_t> bounded_deviation_histogram;
    std::map<long long, std::set<std::string>> bounded_labels_by_deviation;
    bool parameter_domain = false;
    bool rank_valid = false;
    bool spine_valid = false;
    bool no_bounded_high_deviation = false;
    bool valid = false;
};

inline bool property_f_class_ii_decode_integer_digit(
        const std::vector<std::pair<std::string, std::string>>& coefficients,
        long long& digit) {
    if (coefficients.size() != 3) return false;
    if (coefficients[0].first != "0" || coefficients[0].second != "1" ||
        coefficients[2].first != "0" || coefficients[2].second != "1" ||
        coefficients[1].second != "1")
        return false;
    try {
        digit = std::stoll(coefficients[1].first);
    } catch (...) {
        return false;
    }
    return true;
}

inline bool property_f_class_ii_node_integer_state(
        const PropertyFGraphNode& node,
        std::array<long long, 3>& state) {
    if (node.gamma_coefficients.size() != 3) return false;
    try {
        for (std::size_t i = 0; i < 3; ++i) {
            if (node.gamma_coefficients[i].second != "1") return false;
            state[i] = std::stoll(node.gamma_coefficients[i].first);
        }
    } catch (...) {
        return false;
    }
    return true;
}

inline PropertyFClassIIAkiyamaWindowCensus
derive_property_f_class_ii_akiyama_window_census(
        const PropertyFGraph& graph, std::size_t a) {
    PropertyFClassIIAkiyamaWindowCensus out;
    out.parameter_a = a;
    out.parameter_domain = a >= 7 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;
    const auto rank = derive_property_f_escape_rank(graph);
    out.rank_valid = rank.valid;
    if (!rank.valid) return out;
    const auto spine = derive_property_f_class_ii_rank_spine(graph, a);
    out.spine_valid = spine.valid;
    if (!spine.valid) return out;
    const auto aa = static_cast<long long>(a);
    for (const auto source : spine.node_ids) {
        if (rank.node_height[source] <=
            PropertyFClassIIAkiyamaWindowCensus::collar_height)
            continue;
        ++out.tail_sources;
        std::array<long long, 3> source_state{};
        if (!property_f_class_ii_node_integer_state(
                graph.nodes[source], source_state)) {
            ++out.malformed_edges;
            continue;
        }
        const auto forced = property_f_class_ii_phase_forced_digit(
            source_state, aa);
        for (std::size_t edge = 0;
             edge < graph.nodes[source].successors.size(); ++edge) {
            ++out.outgoing_edges;
            const auto raw_target = graph.nodes[source].successors[edge];
            if (raw_target < 0 || static_cast<std::size_t>(raw_target) >=
                graph.nodes.size()) {
                ++out.malformed_edges;
                continue;
            }
            const auto target = static_cast<std::size_t>(raw_target);
            if (edge >= graph.nodes[source].edge_digit_coefficients.size()) {
                ++out.malformed_edges;
                continue;
            }
            long long digit = 0;
            if (!property_f_class_ii_decode_integer_digit(
                    graph.nodes[source].edge_digit_coefficients[edge], digit)) {
                ++out.nonordinary_edges;
                continue;
            }
            ++out.ordinary_edges;
            std::array<long long, 3> target_state{};
            if (!property_f_class_ii_node_integer_state(
                    graph.nodes[target], target_state)) {
                ++out.malformed_edges;
                continue;
            }
            const auto target_eta = target_state[0] + target_state[2];
            if (!property_f_class_ii_akiyama_eta_bound(target_eta)) continue;
            ++out.bounded_candidates;
            const auto deviation = digit - forced;
            ++out.bounded_deviation_histogram[deviation];
            out.bounded_labels_by_deviation[deviation].insert(
                property_f_birth_round_digit_key(
                    graph.nodes[source].edge_digit_coefficients[edge]));
            if (rank.node_height[target] <=
                PropertyFClassIIAkiyamaWindowCensus::collar_height) {
                ++out.bounded_collar_edges;
            } else {
                ++out.bounded_high_edges;
                const bool internal = graph.scc_labels[source] ==
                    graph.scc_labels[target];
                if (internal)
                    ++out.bounded_internal_high_edges;
                if (deviation != 0 && !internal)
                    ++out.bounded_high_nonzero_deviations;
            }
        }
    }
    out.no_bounded_high_deviation =
        out.bounded_high_nonzero_deviations == 0;
    out.valid = out.parameter_domain && out.rank_valid && out.spine_valid &&
        out.malformed_edges == 0 && out.no_bounded_high_deviation;
    return out;
}

}  // namespace adelic
