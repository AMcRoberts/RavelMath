// Shared data contract for the finite Property-(F) checker.
//
// This header intentionally contains no algorithms.  It is the stable
// boundary between graph construction, SCC analysis, proof staging, and
// renderers.  `coincidence_and_property_f.hpp` remains the compatibility
// façade for the historical API.
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "math/qbeta.hpp"

namespace adelic {

struct PropertyFResult {
    bool holds = false;
    bool inconclusive = false;
    long long nodes_explored = 0;
    long long zero_nodes = 0;
    long long nonzero_nodes = 0;
    long long strongly_connected_components = 0;
    long long nonzero_cycle_components = 0;
    std::vector<long long> violation_cycle_nodes;
    std::vector<std::pair<long long, long long>> violation_cycle_edges;
};

struct PropertyFGraphNode {
    std::string gamma_key;
    std::vector<std::pair<std::string, std::string>> gamma_coefficients;
    long long letter = 0;
    bool zero = false;
    std::vector<long long> successors;
    std::vector<std::vector<std::pair<std::string, std::string>>> edge_digit_coefficients;
};

struct PropertyFGraph {
    std::vector<std::string> characteristic_polynomial;
    std::vector<std::vector<std::pair<std::string, std::string>>> beta_inverse_matrix;
    std::vector<long long> scc_labels;
    std::vector<long long> scc_sizes;
    long long nonzero_cycle_components = 0;
    std::vector<PropertyFGraphNode> nodes;
};

inline std::string qelem_key(const mathlib::QElem& x) {
    std::string s;
    for (const auto& c : x.coeffs_) {
        s += mathlib::str(c);
        s += ';';
    }
    return s;
}

} // namespace adelic
