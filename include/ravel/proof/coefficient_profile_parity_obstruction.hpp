#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ravel/simple_parry_profile.hpp"
#include "ravel/proof/cyclotomic_obstruction.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct CoefficientProfileParityObstructionProof {
    std::size_t dimension = 0;
    std::vector<std::size_t> profile;
    long long value_at_minus_one = 0;
    bool even_dimension = false;
    bool cyclotomic_x_plus_one_factor = false;
    CyclotomicObstructionCertificate cyclotomic_certificate;
    bool proved = false;
    std::string obstruction;
};

// The nearest left-outside profile is
//   (1,...,1,0,1),
// obtained by deleting the zero prefix from the penultimate parent image.
// Its polynomial is x^D - x^{D-1} - ... - x^2 - 1.
// Direct alternating-sum evaluation gives p(-1)=0 iff D is even.
inline CoefficientProfileParityObstructionProof
derive_nearest_left_profile_parity_obstruction(std::size_t D) {
    CoefficientProfileParityObstructionProof out;
    out.dimension = D;
    if (D < 3) {
        out.obstruction = "nearest internal phase deletion requires D >= 3";
        return out;
    }
    out.profile.assign(D,1);
    out.profile[D-2] = 0;
    out.even_dimension = (D % 2 == 0);

    long long value = ((D % 2) == 0) ? 1 : -1; // (-1)^D
    for (std::size_t j = 0; j < D; ++j) {
        const std::size_t power = D - 1 - j;
        const long long sign = (power % 2 == 0) ? 1 : -1;
        value -= static_cast<long long>(out.profile[j]) * sign;
    }
    out.value_at_minus_one = value;
    out.cyclotomic_certificate = derive_cyclotomic_obstruction_certificate(
        simple_parry_profile_polynomial(std::vector<long long>(out.profile.begin(), out.profile.end())), 2);
    out.cyclotomic_x_plus_one_factor = out.cyclotomic_certificate.has_order(2);
    out.proved = out.cyclotomic_certificate.proved &&
                 out.cyclotomic_x_plus_one_factor == out.even_dimension &&
                 out.cyclotomic_x_plus_one_factor == (value == 0);
    if (!out.proved)
        out.obstruction = "alternating-sum parity identity failed";
    return out;
}

// Stages a `CoefficientProfileParityObstructionReflectionCertificate` for
// one dimension D -- gates on `proof.proved`, which independently
// rechecks (via the exact alternating-sum recomputation above) that
// value_at_minus_one's parity classification matches even_dimension
// before this function is ever reached.
inline void stage_coefficient_profile_parity_obstruction(
        const CoefficientProfileParityObstructionProof& proof,
        const std::string& description) {
    if (!proof.proved) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::CoefficientProfileParityObstructionReflectionCertificate node;
    node.dimension = static_cast<long long>(proof.dimension);
    node.even_dimension = proof.even_dimension;
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

} // namespace ravel::proof
