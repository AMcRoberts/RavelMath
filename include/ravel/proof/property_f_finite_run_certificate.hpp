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

}  // namespace ravel::proof
