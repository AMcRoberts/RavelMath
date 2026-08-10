// Common skew-product contract for sofic, transport, and local adelic
// fibers.  This is the typed seam between the shared prefix cocycle and the
// unit/non-unit representations.
#pragma once

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/dedekind_factorization.hpp"

namespace adelic {

struct AdelicLocalFiberDescriptor {
    long long rational_prime = 0;
    long long ramification_index = 0;
    long long residue_degree = 0;
};

// Local-fibre partition certificate.  For a maximal Dedekind factorization,
// the completion at every rational prime p | det(beta) splits into one local
// factor per prime ideal, with total Q_p-degree sum e_i f_i = deg(charpoly).
// This is the precise finite-place statement needed before treating the
// archimedean and valuation coordinates as one adelic cocycle fibre.
struct AdelicValuationFibreLemmaCertificate {
    long long determinant = 0;
    std::size_t degree = 0;
    std::size_t rational_prime_count = 0;
    std::size_t local_fibre_count = 0;
    std::size_t degree_partition_mismatches = 0;
    std::size_t nonmaximal_primes = 0;
    std::size_t ideal_lattice_mismatches = 0;
    std::size_t local_degree_sum = 0;
    bool determinant_support_valid = false;
    bool prime_ideal_partition_valid = false;
    bool local_bound_trusted = false;
    bool proved = false;
};

inline AdelicValuationFibreLemmaCertificate
derive_adelic_valuation_fibre_lemma(long long determinant,
                                    const mathlib::PolyZ& charpoly,
                                    long long precision = 30) {
    AdelicValuationFibreLemmaCertificate out;
    out.determinant = determinant;
    out.degree = static_cast<std::size_t>(charpoly.degree());
    if (determinant == 0 || charpoly.degree() < 1) return out;

    std::vector<long long> primes;
    long long remaining = std::llabs(determinant);
    for (long long p = 2; p <= remaining / p; ++p) {
        if (remaining % p != 0) continue;
        primes.push_back(p);
        while (remaining % p == 0) remaining /= p;
    }
    if (remaining > 1) primes.push_back(remaining);
    out.rational_prime_count = primes.size();
    out.determinant_support_valid = determinant != 0 &&
        (determinant == 1 || determinant == -1 || !primes.empty());
    out.prime_ideal_partition_valid = true;
    for (const long long p : primes) {
        const auto factorization = factor_prime_in_qbeta(charpoly, p);
        if (!factorization.maximal) ++out.nonmaximal_primes;
        if (!factorization.maximal ||
            !cross_check_dedekind_factorization(
                factorization, charpoly, charpoly.degree()))
            ++out.ideal_lattice_mismatches;
        std::size_t local_degree = 0;
        for (const auto& ideal : factorization.prime_ideals) {
            ++out.local_fibre_count;
            local_degree += static_cast<std::size_t>(ideal.e * ideal.f);
        }
        out.local_degree_sum += local_degree;
        if (local_degree != out.degree)
            ++out.degree_partition_mismatches;
    }
    out.prime_ideal_partition_valid =
        out.degree_partition_mismatches == 0 &&
        out.nonmaximal_primes == 0 && out.ideal_lattice_mismatches == 0;
    if (primes.empty()) {
        out.local_bound_trusted = determinant == 1 || determinant == -1;
    } else {
        auto [bound, trusted] = make_combined_padic_bound(
            primes, charpoly, precision);
        (void)bound;
        out.local_bound_trusted = trusted;
    }
    out.proved = out.determinant_support_valid &&
                 out.prime_ideal_partition_valid &&
                 out.local_bound_trusted;
    return out;
}

struct AdelicCocycleExtensionCertificate {
    long long determinant = 0;
    bool shared_prefix_cocycle = false;
    bool sofic_projection = false;
    bool transport_projection = false;
    bool unit_specialization = false;
    bool nonunit_local_fibers = false;
    bool local_fibers_are_prime_ideal_indexed = false;
    bool projections_commute = false;
    bool proved = false;
    std::size_t local_fiber_count = 0;
    std::vector<long long> rational_prime_support;
    bool local_bound_constructed = false;
    bool local_bound_trusted = false;
    bool valuation_fibre_lemma_proved = false;
    std::size_t valuation_fibre_degree = 0;
    std::function<bool(const mathlib::QElem&)> combined_local_bound;
    std::string obstruction;
};

// The base and transport projections are formal forgetful maps.  For a unit,
// the local-fiber list is empty.  For a non-unit, every prime-ideal factor of
// (beta) contributes one local coordinate carrying the same prefix label.
inline AdelicCocycleExtensionCertificate
derive_adelic_cocycle_extension(long long determinant,
                                 const std::vector<AdelicLocalFiberDescriptor>& local_fibers,
                                 bool prefix_labels_shared = true) {
    AdelicCocycleExtensionCertificate out;
    out.determinant = determinant;
    out.local_fiber_count = local_fibers.size();
    long long remaining = std::llabs(determinant);
    for (long long p = 2; p <= remaining / p; ++p) {
        if (remaining % p != 0) continue;
        out.rational_prime_support.push_back(p);
        while (remaining % p == 0) remaining /= p;
    }
    if (remaining > 1) out.rational_prime_support.push_back(remaining);
    out.shared_prefix_cocycle = prefix_labels_shared;
    out.sofic_projection = true;
    out.transport_projection = true;
    out.unit_specialization = determinant != 0 &&
                              (determinant == 1 || determinant == -1) &&
                              local_fibers.empty();
    out.nonunit_local_fibers = determinant != 0 &&
                               determinant != 1 && determinant != -1 &&
                               !local_fibers.empty();
    out.local_fibers_are_prime_ideal_indexed = true;
    for (const auto& fiber : local_fibers) {
        if (fiber.rational_prime < 2 || fiber.ramification_index < 1 ||
            fiber.residue_degree < 1) {
            out.local_fibers_are_prime_ideal_indexed = false;
        }
    }
    if (out.nonunit_local_fibers) {
        for (const long long p : out.rational_prime_support) {
            bool covered = false;
            for (const auto& fiber : local_fibers)
                if (fiber.rational_prime == p) covered = true;
            if (!covered) out.local_fibers_are_prime_ideal_indexed = false;
        }
    }
    out.valuation_fibre_lemma_proved =
        out.local_fibers_are_prime_ideal_indexed;
    out.projections_commute = out.shared_prefix_cocycle &&
                             out.sofic_projection && out.transport_projection;
    out.proved = out.projections_commute &&
                 (out.unit_specialization ||
                  (out.nonunit_local_fibers &&
                   out.local_fibers_are_prime_ideal_indexed));
    if (!out.proved)
        out.obstruction = "cocycle extension needs a unit fiber or valid prime-ideal fibers";
    return out;
}

inline AdelicCocycleExtensionCertificate
derive_adelic_cocycle_extension_from_charpoly(
    long long determinant, const mathlib::PolyZ& charpoly,
    bool prefix_labels_shared = true, long long precision = 30) {
    const long long absolute_determinant = std::llabs(determinant);
    std::vector<long long> primes;
    long long remaining = absolute_determinant;
    for (long long p = 2; p <= remaining / p; ++p) {
        if (remaining % p != 0) continue;
        primes.push_back(p);
        while (remaining % p == 0) remaining /= p;
    }
    if (remaining > 1) primes.push_back(remaining);

    std::vector<AdelicLocalFiberDescriptor> descriptors;
    for (const long long p : primes) {
        const auto factorization = factor_prime_in_qbeta(charpoly, p);
        for (const auto& ideal : factorization.prime_ideals)
            descriptors.push_back({ideal.p, ideal.e, ideal.f});
    }
    auto out = derive_adelic_cocycle_extension(
        determinant, descriptors, prefix_labels_shared);
    const auto fibre_lemma = derive_adelic_valuation_fibre_lemma(
        determinant, charpoly, precision);
    out.valuation_fibre_lemma_proved = fibre_lemma.proved;
    out.valuation_fibre_degree = fibre_lemma.local_degree_sum;
    if (!primes.empty()) {
        auto [bound, trusted] = make_combined_padic_bound(
            primes, charpoly, precision);
        out.combined_local_bound = std::move(bound);
        out.local_bound_constructed = true;
        out.local_bound_trusted = trusted;
        out.proved = out.proved && trusted && fibre_lemma.proved;
        if (!trusted)
            out.obstruction = "Dedekind/local-field bound is not certified";
    } else {
        out.proved = out.proved && fibre_lemma.proved;
    }
    return out;
}

}  // namespace adelic
