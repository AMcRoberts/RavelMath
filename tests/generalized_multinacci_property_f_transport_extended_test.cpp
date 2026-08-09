#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "adelic/property_f_transport_certificate.hpp"
#include "adelic/generalized_multinacci_affine_transport.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/generalized_multinacci.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/generalized_multinacci_general_m.hpp"

template <std::size_t d>
adelic::PropertyFTransportCertificate run_case(std::size_t m, long long budget) {
    const auto raw = ravel::generalized_multinacci_rule(d, m);
    std::array<std::vector<long long>, d> images;
    std::array<std::array<long long, d>, d> matrix{};
    std::vector<std::vector<long long>> dense(d, std::vector<long long>(d));
    for (std::size_t j = 0; j < d; ++j)
        for (const auto symbol : raw[j]) {
            images[j].push_back(symbol);
            ++matrix[symbol][j];
            ++dense[symbol][j];
        }
    if (!ravel::proof::check_strong_coincidence_closure<d>(
            images, matrix, 128, 1000000).holds)
        throw std::runtime_error("extended generalized multinacci coincidence failed");
    if (!ravel::proof::derive_generalized_multinacci_general_m(m).proved)
        throw std::runtime_error("extended general-m scheduler failed");

    mathlib::QBetaRing ring(ravel::generalized_multinacci_polynomial(d, m));
    const auto eigen = mathlib::left_eigenvector_via_qbeta_reduced_factor(dense, ring);
    const auto automaton = adelic::build_prefix_automaton<d>(images, eigen.v, ring);
    if (!adelic::verify_generalized_multinacci_affine_transport(automaton, m))
        throw std::runtime_error("extended affine transport law failed");
    return adelic::derive_property_f_transport_certificate<d>(
        automaton, budget, nullptr, false);
}

int main() {
    const auto d5m1 = run_case<5>(1, 1000000);
    const auto d5m2 = run_case<5>(2, 1000000);
    const auto d5m3 = run_case<5>(3, 1000000);
    const auto d6m1 = run_case<6>(1, 1000000);
    const auto d6m2 = run_case<6>(2, 500000);

    if (!d5m1.result.holds || d5m1.result.inconclusive ||
        !d5m2.result.holds || d5m2.result.inconclusive ||
        !d5m3.result.holds || d5m3.result.inconclusive ||
        !d6m1.result.holds || d6m1.result.inconclusive)
        throw std::runtime_error("an extended case failed to close");
    if (d6m2.result.holds || !d6m2.result.inconclusive ||
        d6m2.partial_nonzero_backedges != 0)
        throw std::runtime_error("d6,m2 partial obstruction signature changed");

    std::cout << "D=5 m=1 nodes=" << d5m1.result.nodes_explored
              << " depth=" << d5m1.max_depth << "\n"
              << "D=5 m=2 nodes=" << d5m2.result.nodes_explored
              << " depth=" << d5m2.max_depth << "\n"
              << "D=5 m=3 nodes=" << d5m3.result.nodes_explored
              << " depth=" << d5m3.max_depth << "\n"
              << "D=6 m=1 nodes=" << d6m1.result.nodes_explored
              << " depth=" << d6m1.max_depth << "\n"
              << "D=6 m=2 budget=" << d6m2.result.node_budget
              << " nodes=" << d6m2.result.nodes_explored
              << " depth=" << d6m2.max_depth
              << " partial_nonzero_backedges=" << d6m2.partial_nonzero_backedges
              << " (inconclusive)\n"
              << "generalized multinacci extended Property-F transport PASS\n";
}
