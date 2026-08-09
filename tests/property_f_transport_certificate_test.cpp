#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "adelic/property_f_transport_certificate.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "math/qbeta.hpp"

int main() {
    constexpr std::size_t d = 3;
    const std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 1}, {2}, {0, 0}};
    std::vector<std::vector<long long>> matrix(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (const long long a : images[j]) ++matrix[a][j];
    mathlib::PolyZ minpoly;
    minpoly.ensure_size(4);
    mathlib::set_si(minpoly.coeff(0), -2);
    mathlib::set_si(minpoly.coeff(1), 0);
    mathlib::set_si(minpoly.coeff(2), -2);
    mathlib::set_si(minpoly.coeff(3), 1);
    mathlib::QBetaRing ring(minpoly);
    const auto eig = mathlib::left_eigenvector_via_qbeta_reduced_factor(matrix, ring);
    assert(eig.ok);
    const auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, ring);
    auto [bound, trusted] = adelic::make_combined_padic_bound({2}, minpoly);
    assert(trusted);
    const auto cert = adelic::derive_property_f_transport_certificate<d>(
        automaton, 100'000, bound, false);
    assert(cert.result.holds);
    assert(!cert.result.inconclusive);
    assert(cert.result.nonzero_cycle_components == 0);
    assert(cert.partial_nonzero_backedges == 0);
    assert(cert.result.nodes_explored == 4);
    assert(cert.max_depth >= 0);
    assert(cert.edges > 0);
    std::cout << "property_f_transport_certificate: PASS nodes="
              << cert.result.nodes_explored << " depth=" << cert.max_depth
              << " edges=" << cert.edges << "\n";
}
