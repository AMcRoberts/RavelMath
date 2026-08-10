// The canonical monotone non-unit Property-(F) route.
//
// This is the executable theorem boundary for the class for which the
// Frougny--Solomyak monotone coefficient criterion supplies beta-numeration
// Condition (F).  The canonical Parry substitution is checked against the
// same polynomial, then the non-unit local fibers are supplied by Dedekind's
// factorization and the exact finite graph is checked using Proposition 9.4
// of Minervino--Thuswaldner.  The result is intentionally not advertised as
// a theorem for arbitrary substitutions: the canonical substitution and the
// exact characteristic-polynomial identity are hypotheses of this route.
#pragma once

#include <array>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "adelic/maximal_order.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/qbeta.hpp"
#include "math/sturm.hpp"
#include "ravel/canonical_beta_substitution.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/canonical_parent_role_catalogue.hpp"
#include "ravel/proof/monotone_coefficient_cone.hpp"

namespace ravel::proof {

struct CanonicalNonunitPropertyFCertificate {
    std::vector<long long> greedy_digits;
    std::vector<long long> characteristic_polynomial;
    bool canonical_substitution = false;
    bool characteristic_polynomial_matches = false;
    bool monotone_condition_f = false;
    bool nonunit = false;
    bool local_factorization_trusted = false;
    bool parent_role_integer_scheme = false;
    bool strong_coincidence = false;
    bool finite_graph_closed = false;
    bool property_f_holds = false;
    long long graph_nodes = 0;
    long long nonzero_recurrent_components = 0;
    std::string obstruction;
};

inline std::vector<long long> canonical_prime_support(long long n) {
    std::vector<long long> out;
    n = std::llabs(n);
    for (long long p = 2; p * p <= n; ++p) {
        if (n % p != 0) continue;
        out.push_back(p);
        while (n % p == 0) n /= p;
    }
    if (n > 1) out.push_back(n);
    return out;
}

template <std::size_t d>
CanonicalNonunitPropertyFCertificate
derive_canonical_nonunit_property_f_certificate(
    const mathlib::QBetaRing& ring,
    const mathlib::RootInterval& beta_interval,
    long long node_budget = 1'000'000) {
    using namespace mathlib;
    CanonicalNonunitPropertyFCertificate out;
    const auto expansion = exact_greedy_beta_expansion_of_one(
        ring, beta_interval, 256);
    if (!expansion.terminated || expansion.digits.size() != d) {
        out.obstruction = "canonical route requires a terminating d-digit expansion";
        return out;
    }
    out.greedy_digits = expansion.digits;
    const auto catalogue = derive_canonical_parent_role_catalogue(
        ring, beta_interval);
    const auto integer_scheme = derive_parent_role_integer_scheme(
        catalogue, 24, 24);
    out.parent_role_integer_scheme = integer_scheme.proved;
    if (!out.parent_role_integer_scheme) {
        out.obstruction = "parent-role zero-kernel/integer transport scheme did not close";
        return out;
    }
    const auto cone = derive_monotone_coefficient_cone_certificate(
        expansion.digits);
    out.monotone_condition_f = cone.condition_f_applies;
    out.nonunit = cone.nonunit;
    if (!out.monotone_condition_f) {
        out.obstruction = "greedy coefficient vector is outside monotone cone";
        return out;
    }
    if (!out.nonunit) {
        out.obstruction = "canonical route is specifically non-unit";
        return out;
    }

    const auto images = canonical_beta_substitution_from_digits(expansion.digits);
    if (images.size() != d) {
        out.obstruction = "canonical substitution has unexpected alphabet size";
        return out;
    }
    std::array<std::vector<long long>, d> image_array;
    std::vector<std::vector<long long>> matrix(
        d, std::vector<long long>(d, 0));
    for (std::size_t source = 0; source < d; ++source) {
        image_array[source] = images[source];
        for (const long long target : images[source]) {
            if (target < 0 || static_cast<std::size_t>(target) >= d) {
                out.obstruction = "canonical substitution has invalid letter";
                return out;
            }
            ++matrix[static_cast<std::size_t>(target)][source];
        }
    }
    // The geometric Property-F theorem is used here only after the canonical
    // substitution's coincidence hypothesis has been discharged.  Keep this
    // as an executable obligation rather than relying on the fact that the
    // canonical examples are expected to coincide.
    std::array<std::array<long long, d>, d> incidence{};
    for (std::size_t source = 0; source < d; ++source)
        for (const long long target : image_array[source])
            ++incidence[static_cast<std::size_t>(target)][source];
    const auto coincidence = check_strong_coincidence_closure<d>(
        image_array, incidence, 32, 1'000'000);
    out.strong_coincidence = coincidence.holds && !coincidence.inconclusive;
    if (!out.strong_coincidence) {
        out.obstruction = "canonical substitution strong coincidence did not close";
        return out;
    }
    const auto characteristic = charpoly_faddeev_leverrier(matrix);
    for (long long i = 0; i <= characteristic.degree(); ++i)
        out.characteristic_polynomial.push_back(
            mpz_get_si(characteristic.coeff(static_cast<std::size_t>(i)).get()));
    out.canonical_substitution = true;
    if (ring.charpoly().degree() != characteristic.degree()) {
        out.characteristic_polynomial_matches = false;
    } else {
        out.characteristic_polynomial_matches = true;
        for (long long i = 0; i <= characteristic.degree(); ++i) {
            if (mpz_cmp(characteristic.coeff(static_cast<std::size_t>(i)).get(),
                        ring.charpoly().coeff(static_cast<std::size_t>(i)).get()) != 0)
                out.characteristic_polynomial_matches = false;
        }
    }
    if (!out.characteristic_polynomial_matches) {
        out.obstruction = "canonical incidence polynomial does not match beta polynomial";
        return out;
    }

    const long long determinant = adelic::integer_determinant(matrix);
    const auto primes = canonical_prime_support(determinant);
    if (primes.empty()) {
        out.obstruction = "non-unit certificate has empty determinant support";
        return out;
    }
    bool factorization_ok = true;
    for (const long long p : primes) {
        const auto factor = adelic::factor_prime_in_qbeta(ring.charpoly(), p);
        factorization_ok = factorization_ok && factor.maximal &&
            adelic::cross_check_dedekind_factorization(factor, ring.charpoly(), d);
    }
    out.local_factorization_trusted = factorization_ok;
    if (!out.local_factorization_trusted) {
        out.obstruction = "Dedekind local factors are not trusted";
        return out;
    }
    const auto eigen = mathlib::left_eigenvector_via_qbeta_reduced_factor(matrix, ring);
    if (!eigen.ok) {
        out.obstruction = "left Perron eigenvector construction failed";
        return out;
    }
    const auto automaton = adelic::build_prefix_automaton<d>(
        image_array, eigen.v, ring);
    auto [bound, bound_trusted] = adelic::make_combined_padic_bound(
        primes, ring.charpoly());
    if (!bound_trusted) {
        out.obstruction = "combined p-adic bound is not trusted";
        return out;
    }
    adelic::PropertyFGraph graph;
    const auto result = adelic::check_property_f<d>(
        automaton, node_budget, bound, nullptr, nullptr, nullptr,
        &graph, true);
    out.graph_nodes = result.nodes_explored;
    out.nonzero_recurrent_components = result.nonzero_cycle_components;
    out.finite_graph_closed = result.closure_reached && !result.inconclusive;
    out.property_f_holds = out.finite_graph_closed && result.holds;
    if (!out.finite_graph_closed)
        out.obstruction = "exact adelic zero-expansion graph did not close";
    else if (!out.property_f_holds)
        out.obstruction = "a nonzero recurrent component remains";
    return out;
}

}  // namespace ravel::proof
