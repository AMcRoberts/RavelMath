#include <cassert>
#include <iostream>
#include <initializer_list>

#include "math/qbeta.hpp"
#include "ravel/proof/canonical_nonunit_property_f.hpp"
#include "ravel/proof/canonical_parent_role_catalogue.hpp"

void run_case(std::initializer_list<long long> coefficients) {
    const auto ring = mathlib::QBetaRing::from_low_first(coefficients);
    const auto interval = mathlib::isolate_beta(ring);
    const auto catalogue = ravel::proof::derive_canonical_parent_role_catalogue(
        ring, interval);
    const auto integer_scheme = ravel::proof::derive_parent_role_integer_scheme(
        catalogue, 24, 24);
    assert(integer_scheme.proved);
    const auto certificate =
        ravel::proof::derive_canonical_nonunit_property_f_certificate<2>(
            ring, interval, 100'000);
    assert(certificate.canonical_substitution);
    assert(certificate.characteristic_polynomial_matches);
    assert(certificate.monotone_condition_f);
    assert(certificate.nonunit);
    assert(certificate.strong_coincidence);
    assert(certificate.local_factorization_trusted);
    assert(certificate.finite_graph_closed);
    assert(certificate.property_f_holds);
    assert(certificate.nonzero_recurrent_components == 0);
    std::cout << "canonical non-unit Property-F theorem route: PASS nodes="
              << certificate.graph_nodes << "\n";
}

int main() {
    run_case({-2, -2});
    run_case({-3, -3});
}
