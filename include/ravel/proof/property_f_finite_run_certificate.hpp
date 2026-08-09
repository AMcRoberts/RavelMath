// ravel/proof/property_f_finite_run_certificate.hpp
//
// Typed reflection bridge for one closed property-(F) computation.  The
// operation records only data returned by the real checker; it does not turn
// a finite summary into an unconditional theorem about Γ.

#pragma once

#include <stdexcept>
#include <string>

#include "adelic/coincidence_and_property_f.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

inline bool stage_property_f_finite_run(const adelic::PropertyFResult& result,
                                        std::string description = {}) {
    if (result.inconclusive) return false;
    if (result.nodes_explored < 0 || result.zero_nodes < 0 || result.nonzero_nodes < 0 ||
        result.strongly_connected_components < 0 || result.nonzero_cycle_components < 0) {
        throw std::invalid_argument("property-F summary contains a negative count");
    }
    if (result.zero_nodes + result.nonzero_nodes != result.nodes_explored) {
        throw std::invalid_argument("property-F summary node classes do not partition the graph");
    }
    if (result.holds && result.nonzero_cycle_components != 0) {
        throw std::invalid_argument("property-F summary says HOLDS but has a nonzero cycle");
    }
    if (!mathlib::reflection::enabled()) return false;

    mathlib::reflection::PropertyFFiniteRunCertificate node;
    node.nodes_explored = result.nodes_explored;
    node.zero_nodes = result.zero_nodes;
    node.nonzero_nodes = result.nonzero_nodes;
    node.strongly_connected_components = result.strongly_connected_components;
    node.nonzero_cycle_components = result.nonzero_cycle_components;
    node.holds = result.holds;
    node.description = std::move(description);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication,
                                std::move(node));
    return true;
}

inline bool stage_property_f_graph(const adelic::PropertyFResult& result,
                                   const adelic::PropertyFGraph& graph,
                                   std::string description = {}) {
    if (result.inconclusive || graph.nodes.size() != static_cast<std::size_t>(result.nodes_explored)) return false;
    if (result.zero_nodes + result.nonzero_nodes != result.nodes_explored) return false;
    long long zero_count = 0;
    for (std::size_t i = 0; i < graph.nodes.size(); ++i) {
        const auto& node = graph.nodes[i];
        if (node.zero) ++zero_count;
        for (const long long successor : node.successors) {
            if (successor < 0 || successor >= result.nodes_explored) {
                throw std::invalid_argument("property-F graph has an out-of-range successor");
            }
        }
    }
    if (zero_count != result.zero_nodes) {
        throw std::invalid_argument("property-F graph zero-node count disagrees with result");
    }
    if (!mathlib::reflection::enabled()) return false;
    mathlib::reflection::PropertyFGraphCertificate node;
    node.gamma_keys.reserve(graph.nodes.size());
    node.gamma_coefficients.reserve(graph.nodes.size());
    node.letters.reserve(graph.nodes.size());
    node.zero_nodes.reserve(graph.nodes.size());
    node.successors.reserve(graph.nodes.size());
    for (const auto& source : graph.nodes) {
        node.gamma_keys.push_back(source.gamma_key);
        std::vector<mathlib::reflection::ExactRationalCoefficient> coefficients;
        coefficients.reserve(source.gamma_coefficients.size());
        for (const auto& [numerator, denominator] : source.gamma_coefficients)
            coefficients.push_back({numerator, denominator});
        node.gamma_coefficients.push_back(std::move(coefficients));
        node.letters.push_back(source.letter);
        node.zero_nodes.push_back(source.zero);
        node.successors.push_back(source.successors);
    }
    node.description = std::move(description);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication,
                                std::move(node));
    return true;
}

}  // namespace ravel::proof
