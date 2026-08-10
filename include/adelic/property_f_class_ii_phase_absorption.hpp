// Graph-level phase-strip absorption certificate for Class-II Property-(F).
//
// It measures the exact remaining theorem seam: every node outside the
// alternating affine phase strip must already lie in the fixed collar.
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>

#include "adelic/property_f_class_ii_phase_strip.hpp"
#include "adelic/property_f_escape_rank.hpp"

namespace adelic {

struct PropertyFClassIIPhaseAbsorptionCertificate {
    static constexpr std::size_t collar_height = 5;

    std::size_t parameter_a = 0;
    std::size_t node_count = 0;
    std::size_t phase_valid_nodes = 0;
    std::size_t phase_invalid_nodes = 0;
    std::size_t phase_invalid_high_nodes = 0;
    std::size_t phase_seed_nodes = 0;
    std::size_t phase_invalid_unexpected_high_nodes = 0;
    std::size_t maximum_invalid_height = 0;
    bool rank_valid = false;
    bool invalid_nodes_absorbed = false;
    bool valid = false;
};

inline PropertyFClassIIPhaseAbsorptionCertificate
derive_property_f_class_ii_phase_absorption(
        const PropertyFGraph& graph, std::size_t a) {
    PropertyFClassIIPhaseAbsorptionCertificate out;
    out.parameter_a = a;
    out.node_count = graph.nodes.size();
    const auto rank = derive_property_f_escape_rank(graph);
    out.rank_valid = rank.valid;
    if (!rank.valid) return out;
    const auto aa = static_cast<long long>(a);
    for (std::size_t node = 0; node < graph.nodes.size(); ++node) {
        bool phase_valid = false;
        if (graph.nodes[node].gamma_coefficients.size() >= 3 &&
            graph.nodes[node].gamma_coefficients[0].second == "1" &&
            graph.nodes[node].gamma_coefficients[1].second == "1" &&
            graph.nodes[node].gamma_coefficients[2].second == "1") {
            const std::array<long long, 3> coefficients{
                std::stoll(graph.nodes[node].gamma_coefficients[0].first),
                std::stoll(graph.nodes[node].gamma_coefficients[1].first),
                std::stoll(graph.nodes[node].gamma_coefficients[2].first)};
            phase_valid = property_f_class_ii_phase_pair_valid(coefficients, aa);
        }
        if (phase_valid) {
            ++out.phase_valid_nodes;
        } else {
            ++out.phase_invalid_nodes;
            out.maximum_invalid_height = std::max(
                out.maximum_invalid_height, rank.node_height[node]);
            if (rank.node_height[node] >
                PropertyFClassIIPhaseAbsorptionCertificate::collar_height) {
                ++out.phase_invalid_high_nodes;
                const bool distinguished_seed = graph.nodes[node].zero &&
                    graph.nodes[node].letter == 0 &&
                    rank.node_height[node] == rank.maximum_height;
                if (distinguished_seed)
                    ++out.phase_seed_nodes;
                else
                    ++out.phase_invalid_unexpected_high_nodes;
            }
        }
    }
    out.invalid_nodes_absorbed =
        out.phase_invalid_unexpected_high_nodes == 0;
    out.valid = out.rank_valid && out.invalid_nodes_absorbed;
    return out;
}

}  // namespace adelic
