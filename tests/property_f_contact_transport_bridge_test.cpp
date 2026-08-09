#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "adelic/property_f_contact_transport_bridge.hpp"
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
    ravel::SubstitutionRule rule({{0, 0, 1}, {2}, {0, 0}});
    ravel::ContactBoundaryLimits limits;
    limits.closure_cap = 40'000;
    limits.corona_cap = 200'000;
    limits.max_corona_rounds = 14;
    limits.retain_boundary_matrix = false;
    const auto bridge = adelic::derive_property_f_contact_transport_bridge<d>(
        rule, automaton, 2.3593040859717767, 2, limits);
    assert(bridge.boundary_complete);
    assert(bridge.contact_prefixes_in_adelic_alphabet);
    assert(bridge.exact_digit_labels_replay);
    assert(bridge.boundary_edges > 0);
    assert(bridge.distinct_contact_prefixes > 0);
    std::cout << "property_f_contact_transport_bridge: PASS boundary_edges="
              << bridge.boundary_edges << " contact_prefixes="
              << bridge.distinct_contact_prefixes << "\n";
}
