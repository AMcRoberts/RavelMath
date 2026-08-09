#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "adelic/generalized_multinacci_affine_transport.hpp"
#include "adelic/generalized_multinacci_block_transport.hpp"
#include "adelic/property_f_transport_certificate.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/generalized_multinacci_general_m.hpp"

template <std::size_t d>
void check_case(std::size_t m) {
    const auto raw = ravel::generalized_multinacci_rule(d, m);
    std::array<std::vector<long long>, d> images;
    std::array<std::array<long long, d>, d> matrix{};
    std::vector<std::vector<long long>> dense(d, std::vector<long long>(d));
    for (std::size_t j = 0; j < d; ++j) {
        for (const auto symbol : raw[j]) {
            images[j].push_back(symbol);
            ++matrix[symbol][j];
            ++dense[symbol][j];
        }
    }

    const auto closure = ravel::proof::check_strong_coincidence_closure<d>(
        images, matrix, 64, 1000000);
    if (!closure.holds)
        throw std::runtime_error("generalized multinacci coincidence failed");

    const auto symbolic = ravel::proof::derive_generalized_multinacci_general_m(m);
    if (!symbolic.proved)
        throw std::runtime_error("general-m sofic scheduler certificate failed");
    if (!adelic::derive_generalized_multinacci_block_transport(d, m).proved)
        throw std::runtime_error("generalized multinacci block schema failed");

    mathlib::QBetaRing ring(ravel::generalized_multinacci_polynomial(d, m));
    const auto eigen = mathlib::left_eigenvector_via_qbeta_reduced_factor(dense, ring);
    const auto automaton = adelic::build_prefix_automaton<d>(images, eigen.v, ring);
    if (!adelic::verify_generalized_multinacci_affine_transport(automaton, m))
        throw std::runtime_error("generalized multinacci affine transport law failed");
    const auto transport = adelic::derive_property_f_transport_certificate<d>(
        automaton, 500000, nullptr, false);
    if (!transport.result.holds || transport.result.inconclusive ||
        transport.result.nonzero_cycle_components != 0 ||
        transport.partial_nonzero_backedges != 0) {
        throw std::runtime_error("generalized multinacci Property-F transport failed");
    }
    std::cout << "D=" << d << " m=" << m
              << " sofic_roof<=" << m
              << " PF_nodes=" << transport.result.nodes_explored
              << " PF_depth=" << transport.max_depth << '\n';
}

int main() {
    for (std::size_t m = 1; m <= 6; ++m) {
        check_case<2>(m);
        check_case<3>(m);
        check_case<4>(m);
    }
    std::cout << "generalized multinacci Property-F/sofic bridge PASS\n";
}
