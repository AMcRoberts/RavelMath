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

inline mathlib::Rat property_f_parse_rat(const std::string& numerator,
                                          const std::string& denominator) {
    mathlib::Rat out;
    const std::string encoded = numerator + "/" + denominator;
    if (mpq_set_str(out.get(), encoded.c_str(), 10) != 0)
        throw std::invalid_argument("property-F graph contains an invalid rational coefficient");
    mpq_canonicalize(out.get());
    return out;
}

inline mathlib::QElem property_f_parse_qelem(
    const std::vector<std::pair<std::string, std::string>>& coefficients,
    const mathlib::QBetaRing& ring) {
    if (coefficients.size() != ring.degree())
        throw std::invalid_argument("property-F graph coefficient vector has the wrong degree");
    mathlib::QElem out(ring.degree());
    for (std::size_t i = 0; i < coefficients.size(); ++i)
        out.coeff(i) = property_f_parse_rat(coefficients[i].first, coefficients[i].second);
    return out;
}

inline bool stage_property_f_finite_run(const adelic::PropertyFResult& result,
                                        std::string description = {}) {
    if (result.inconclusive) return false;
    if (!result.closure_reached || !result.archimedean_bound_applied || result.node_budget <= 0) {
        throw std::invalid_argument("property-F summary lacks a closed bounded search contract");
    }
    if (result.boundary_edges < 0) {
        throw std::invalid_argument("property-F summary contains a negative boundary-edge count");
    }
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
    node.closure_reached = result.closure_reached;
    node.archimedean_bound_applied = result.archimedean_bound_applied;
    node.extra_bound_applied = result.extra_bound_applied;
    node.node_budget = result.node_budget;
    node.boundary_edges = result.boundary_edges;
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
                                   const mathlib::QBetaRing& ring,
                                   std::string description = {}) {
    // This renderer proves the no-obstruction branch only.  A definitive
    // failure needs a separate typed witness path; never serialize it as if
    // the finite graph satisfied the property.
    if (result.inconclusive || !result.holds || result.nonzero_cycle_components != 0 ||
        graph.nonzero_cycle_components != 0 ||
        graph.nodes.size() != static_cast<std::size_t>(result.nodes_explored)) return false;
    if (result.zero_nodes + result.nonzero_nodes != result.nodes_explored) return false;
    long long zero_count = 0;
    for (std::size_t i = 0; i < graph.nodes.size(); ++i) {
        const auto& node = graph.nodes[i];
        if (node.letter < 0 || node.letter >= static_cast<long long>(ring.degree()))
            throw std::invalid_argument("property-F graph node letter is out of range");
        if (node.zero) ++zero_count;
        for (const long long successor : node.successors) {
            if (successor < 0 || successor >= result.nodes_explored) {
                throw std::invalid_argument("property-F graph has an out-of-range successor");
            }
        }
        if (node.edge_digit_coefficients.size() != node.successors.size()) {
            throw std::invalid_argument("property-F graph edge digit data is misaligned");
        }
    }
    if (zero_count != result.zero_nodes) {
        throw std::invalid_argument("property-F graph zero-node count disagrees with result");
    }
    if (graph.characteristic_polynomial.size() !=
        static_cast<std::size_t>(ring.charpoly().degree() + 1)) {
        throw std::invalid_argument("property-F graph characteristic polynomial has the wrong degree");
    }
    for (std::size_t i = 0; i < graph.characteristic_polynomial.size(); ++i) {
        if (graph.characteristic_polynomial[i] != mathlib::str(ring.charpoly().coeff(i)))
            throw std::invalid_argument("property-F graph characteristic polynomial disagrees with ring");
    }
    mathlib::QElem beta = ring.from_int(0);
    beta.coeff(1) = mathlib::Rat(1, 1);
    const auto inverse = mathlib::invert_in_qbeta(beta, ring);
    if (!inverse.invertible) throw std::invalid_argument("property-F beta is not invertible");
    if (graph.scc_labels.size() != graph.nodes.size())
        throw std::invalid_argument("property-F graph SCC labels have the wrong length");
    if (graph.scc_sizes.empty() && !graph.nodes.empty())
        throw std::invalid_argument("property-F graph has no SCC size data");
    long long size_sum = 0;
    for (const long long size : graph.scc_sizes) {
        if (size < 0) throw std::invalid_argument("property-F graph has a negative SCC size");
        size_sum += size;
    }
    for (const long long label : graph.scc_labels) {
        if (label < 0 || label >= static_cast<long long>(graph.scc_sizes.size()))
            throw std::invalid_argument("property-F graph SCC label is out of range");
    }
    std::vector<long long> label_counts(graph.scc_sizes.size(), 0);
    for (const long long label : graph.scc_labels)
        ++label_counts[static_cast<std::size_t>(label)];
    if (label_counts != graph.scc_sizes)
        throw std::invalid_argument("property-F graph SCC labels do not match component sizes");
    if (size_sum != static_cast<long long>(graph.nodes.size()))
        throw std::invalid_argument("property-F graph SCC sizes do not partition the nodes");
    if (graph.nonzero_cycle_components < 0)
        throw std::invalid_argument("property-F graph has a negative nonzero-cycle count");
    if (graph.beta_inverse_matrix.size() != ring.degree())
        throw std::invalid_argument("property-F beta-inverse matrix has the wrong row count");
    for (std::size_t row = 0; row < ring.degree(); ++row) {
        if (graph.beta_inverse_matrix[row].size() != ring.degree())
            throw std::invalid_argument("property-F beta-inverse matrix has the wrong column count");
        for (std::size_t column = 0; column < ring.degree(); ++column) {
            mathlib::QElem basis = ring.from_int(0);
            basis.coeff(column) = mathlib::Rat(1, 1);
            const auto image = ring.mul(inverse.inverse, basis);
            const auto observed = property_f_parse_rat(
                graph.beta_inverse_matrix[row][column].first,
                graph.beta_inverse_matrix[row][column].second);
            if (mathlib::cmp(observed, image.coeff(row)) != 0)
                throw std::invalid_argument("property-F beta-inverse matrix disagrees with Q(beta)");
        }
    }
    for (std::size_t source = 0; source < graph.nodes.size(); ++source) {
        const auto gamma = property_f_parse_qelem(graph.nodes[source].gamma_coefficients, ring);
        for (std::size_t edge = 0; edge < graph.nodes[source].successors.size(); ++edge) {
            const auto target = static_cast<std::size_t>(graph.nodes[source].successors[edge]);
            const auto digit = property_f_parse_qelem(
                graph.nodes[source].edge_digit_coefficients[edge], ring);
            const auto target_gamma = property_f_parse_qelem(graph.nodes[target].gamma_coefficients, ring);
            const auto expected = ring.mul(inverse.inverse, ring.add(gamma, digit));
            if (expected != target_gamma)
                throw std::invalid_argument("property-F graph contains an invalid beta-inverse edge");
        }
    }
    if (!mathlib::reflection::enabled()) return false;
    mathlib::reflection::PropertyFGraphCertificate node;
    node.characteristic_polynomial = graph.characteristic_polynomial;
    node.scc_labels = graph.scc_labels;
    node.scc_sizes = graph.scc_sizes;
    node.nonzero_cycle_components = graph.nonzero_cycle_components;
    node.beta_inverse_matrix.reserve(graph.beta_inverse_matrix.size());
    for (const auto& row : graph.beta_inverse_matrix) {
        std::vector<mathlib::reflection::ExactRationalCoefficient> converted;
        converted.reserve(row.size());
        for (const auto& [numerator, denominator] : row)
            converted.push_back({numerator, denominator});
        node.beta_inverse_matrix.push_back(std::move(converted));
    }
    node.gamma_keys.reserve(graph.nodes.size());
    node.gamma_coefficients.reserve(graph.nodes.size());
    node.letters.reserve(graph.nodes.size());
    node.zero_nodes.reserve(graph.nodes.size());
    node.successors.reserve(graph.nodes.size());
    node.edge_digit_coefficients.reserve(graph.nodes.size());
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
        std::vector<std::vector<mathlib::reflection::ExactRationalCoefficient>> edge_digits;
        edge_digits.reserve(source.edge_digit_coefficients.size());
        for (const auto& digit : source.edge_digit_coefficients) {
            std::vector<mathlib::reflection::ExactRationalCoefficient> coefficients;
            coefficients.reserve(digit.size());
        for (const auto& [numerator, denominator] : digit)
                coefficients.push_back({numerator, denominator});
            edge_digits.push_back(std::move(coefficients));
        }
        node.edge_digit_coefficients.push_back(std::move(edge_digits));
    }
    node.description = std::move(description);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication,
                                std::move(node));
    return true;
}

inline bool stage_property_f_violation(const adelic::PropertyFResult& result,
                                       const adelic::PropertyFGraph& graph,
                                       std::string description = {}) {
    if (result.inconclusive || result.holds || result.violation_cycle_nodes.size() < 2 ||
        result.violation_cycle_edges.size() + 1 != result.violation_cycle_nodes.size()) return false;
    if (result.violation_cycle_nodes.front() != result.violation_cycle_nodes.back())
        throw std::invalid_argument("property-F violation witness is not closed");
    bool contains_nonzero = false;
    for (std::size_t i = 0; i + 1 < result.violation_cycle_nodes.size(); ++i) {
        const long long source = result.violation_cycle_nodes[i];
        const long long target = result.violation_cycle_nodes[i + 1];
        if (source < 0 || target < 0 || source >= static_cast<long long>(graph.nodes.size()) ||
            target >= static_cast<long long>(graph.nodes.size()))
            throw std::invalid_argument("property-F violation witness has an out-of-range node");
        if (!graph.nodes[static_cast<std::size_t>(source)].zero) contains_nonzero = true;
        if (result.violation_cycle_edges[i] != std::make_pair(source, target))
            throw std::invalid_argument("property-F violation witness edge list is misaligned");
        const auto& successors = graph.nodes[static_cast<std::size_t>(source)].successors;
        if (std::find(successors.begin(), successors.end(), target) == successors.end())
            throw std::invalid_argument("property-F violation witness uses a missing graph edge");
    }
    if (!contains_nonzero) throw std::invalid_argument("property-F violation witness is entirely zero");
    if (!mathlib::reflection::enabled()) return false;
    mathlib::reflection::PropertyFViolationCertificate node;
    node.cycle_nodes = result.violation_cycle_nodes;
    node.cycle_edges = result.violation_cycle_edges;
    node.description = std::move(description);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication,
                                std::move(node));
    return true;
}

}  // namespace ravel::proof
