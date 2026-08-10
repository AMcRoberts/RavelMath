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
    if (!primes.empty()) {
        auto [bound, trusted] = make_combined_padic_bound(
            primes, charpoly, precision);
        out.combined_local_bound = std::move(bound);
        out.local_bound_constructed = true;
        out.local_bound_trusted = trusted;
        out.proved = out.proved && trusted;
        if (!trusted)
            out.obstruction = "Dedekind/local-field bound is not certified";
    }
    return out;
}

}  // namespace adelic
