#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "adelic/generalized_multinacci_cross_dimension.hpp"
#include "adelic/property_f_transport_certificate.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/generalized_multinacci.hpp"

template <std::size_t d>
adelic::PropertyFTransportCertificate run_case(std::size_t m,
                                               long long budget) {
    const auto raw = ravel::generalized_multinacci_rule(d, m);
    std::array<std::vector<long long>, d> images;
    std::vector<std::vector<long long>> matrix(
        d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (const auto symbol : raw[j]) {
            images[j].push_back(symbol);
            ++matrix[symbol][j];
        }
    mathlib::QBetaRing ring(ravel::generalized_multinacci_polynomial(d, m));
    const auto eigen = mathlib::left_eigenvector_via_qbeta_reduced_factor(
        matrix, ring);
    const auto automaton = adelic::build_prefix_automaton<d>(
        images, eigen.v, ring);
    return adelic::derive_property_f_transport_certificate<d>(
        automaton, budget, nullptr, false);
}

template <std::size_t d>
void check_lift(std::size_t m, long long budget) {
    const auto lift = adelic::derive_generalized_multinacci_dimension_lift(d, m);
    if (!lift.proved)
        throw std::runtime_error("dimension lift certificate failed");
    const auto base = run_case<d>(m, budget);
    const auto raised = run_case<d + 1>(m, budget);
    if (!base.result.holds || base.result.inconclusive ||
        !raised.result.holds || raised.result.inconclusive ||
        base.result.nonzero_cycle_components != 0 ||
        raised.result.nonzero_cycle_components != 0 ||
        base.partial_nonzero_backedges != 0 ||
        raised.partial_nonzero_backedges != 0)
        throw std::runtime_error("exact lift recurrence signature changed");
    std::cout << "d=" << d << "->" << (d + 1) << " m=" << m
              << " base_nodes=" << base.result.nodes_explored
              << " lifted_nodes=" << raised.result.nodes_explored
              << " added_channels=" << lift.added_channels << "\n";
}

int main() {
    // These are exact closed pairs.  The largest point here is (5,3), well
    // below the resource-bound (5,5)/(6,2) experiments.
    check_lift<2>(1, 1000000);
    check_lift<2>(2, 1000000);
    check_lift<2>(3, 1000000);
    check_lift<2>(4, 1000000);
    check_lift<2>(5, 1000000);
    check_lift<2>(6, 1000000);
    check_lift<3>(1, 1000000);
    check_lift<3>(2, 1000000);
    check_lift<3>(3, 1000000);
    check_lift<3>(4, 1000000);
    check_lift<3>(5, 1000000);
    check_lift<3>(6, 1000000);
    check_lift<4>(1, 1000000);
    check_lift<4>(2, 1000000);
    check_lift<4>(3, 1000000);
    std::cout << "generalized multinacci cross-dimension Property-F hypothesis PASS\n";
}
