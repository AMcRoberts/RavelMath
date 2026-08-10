// Exact coordinate adapter for Akiyama's cubic finite-expansion recurrence.
//
// For p_a(x)=x^3-a*x^2-(a+1)*x-1, write U=a+beta^(-1).  In the power basis
// (1,beta,beta^2), kU is [-k,-a*k,k].  Removing the leading (k-1)beta term
// gives [-k,-(k*(a+1)-1),k], exactly the negative Class-II spine state.
// The published recursion for kU then becomes the alternating affine steps
// with digits k and a-k.  This header proves that coordinate adapter and
// recurrence; it deliberately does not claim the beta-expansion inequalities
// or the literal prefix/contact intertwiner.
#pragma once

#include <array>
#include <cstddef>

#include "adelic/property_f_class_ii_affine_tail.hpp"

namespace adelic {

struct PropertyFClassIIAkiyamaSpineCertificate {
    std::size_t parameter_a = 0;
    std::size_t fundamental_identities_checked = 0;
    std::size_t residual_matches = 0;
    std::size_t forward_steps_checked = 0;
    std::size_t backward_steps_checked = 0;
    std::size_t identity_mismatches = 0;
    std::size_t residual_mismatches = 0;
    std::size_t spine_coordinate_mismatches = 0;
    std::size_t forward_mismatches = 0;
    std::size_t backward_mismatches = 0;
    bool parameter_domain = false;
    bool fundamental_recurrence_valid = false;
    bool spine_adapter_valid = false;
    bool alternating_recurrence_valid = false;
    bool valid = false;
};

inline std::array<long long, 3> property_f_class_ii_coeff_add(
        const std::array<long long, 3>& left,
        const std::array<long long, 3>& right) {
    return {left[0] + right[0], left[1] + right[1], left[2] + right[2]};
}

inline std::array<long long, 3> property_f_class_ii_coeff_scale(
        const std::array<long long, 3>& value, long long factor) {
    return {factor * value[0], factor * value[1], factor * value[2]};
}

// U = a + beta^(-1) = beta^2 - a*beta - 1.
inline std::array<long long, 3> property_f_class_ii_akiyama_unit(
        long long a, long long k) {
    return {-k, -a * k, k};
}

// kU - (k-1) beta: the residual after the leading digit in Lemma 7.
inline std::array<long long, 3> property_f_class_ii_akiyama_residual(
        long long a, long long k) {
    return {-k, -(k * (a + 1) - 1), k};
}

inline std::array<long long, 3> property_f_class_ii_beta_inverse(
        const std::array<long long, 3>& state, long long a) {
    return property_f_class_ii_affine_tail_step(state, a, 0);
}

inline PropertyFClassIIAkiyamaSpineCertificate
derive_property_f_class_ii_akiyama_spine(std::size_t a) {
    PropertyFClassIIAkiyamaSpineCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 4 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;

    const auto aa = static_cast<long long>(a);
    const auto unit = property_f_class_ii_akiyama_unit(aa, 1);
    const auto beta = std::array<long long, 3>{0, 1, 0};
    const auto spine = property_f_class_ii_rank_spine_coefficients(a);
    const auto beta_inverse = property_f_class_ii_beta_inverse(
        std::array<long long, 3>{1, 0, 0}, aa);
    // Akiyama's Lemma 7 ranges through the integer part [beta]=a+1.  The
    // spine adapter uses only k<=a-1, but the fundamental recurrence itself
    // is cheap to check over the full published range.
    for (long long k = 1; k <= aa + 1; ++k) {
        ++out.fundamental_identities_checked;
        const auto ku = property_f_class_ii_akiyama_unit(aa, k);
        const auto rhs = property_f_class_ii_coeff_add(
            property_f_class_ii_coeff_add(
                property_f_class_ii_coeff_scale(beta, k - 1),
                std::array<long long, 3>{aa - k + 1, 0, 0}),
            property_f_class_ii_coeff_add(
                beta_inverse,
                property_f_class_ii_coeff_scale(
                    property_f_class_ii_beta_inverse(
                        property_f_class_ii_beta_inverse(unit, aa), aa),
                    k - 1)));
        if (rhs != ku) ++out.identity_mismatches;

        if (k <= aa - 1) {
            const auto residual = property_f_class_ii_akiyama_residual(aa, k);
            const auto expected = property_f_class_ii_coeff_add(
                ku, property_f_class_ii_coeff_scale(beta, -(k - 1)));
            if (residual != expected) ++out.residual_mismatches;
            else ++out.residual_matches;
            const auto spine_index = static_cast<std::size_t>(2 * k);
            if (spine_index >= spine.size() || spine[spine_index] != residual)
                ++out.spine_coordinate_mismatches;
        }
    }
    out.fundamental_recurrence_valid = out.identity_mismatches == 0;
    out.spine_adapter_valid = out.residual_mismatches == 0 &&
        out.spine_coordinate_mismatches == 0;

    for (long long k = 1; k <= aa - 2; ++k) {
        const auto negative = property_f_class_ii_akiyama_residual(aa, k);
        const auto positive = std::array<long long, 3>{
            k + 1, k * (aa + 1), -k};
        const auto next_negative =
            property_f_class_ii_akiyama_residual(aa, k + 1);
        const auto positive_index = static_cast<std::size_t>(2 * k + 1);
        if (positive_index >= spine.size() || spine[positive_index] != positive)
            ++out.spine_coordinate_mismatches;
        ++out.forward_steps_checked;
        if (property_f_class_ii_affine_tail_step(negative, aa, k) != positive)
            ++out.forward_mismatches;
        ++out.backward_steps_checked;
        if (property_f_class_ii_affine_tail_step(positive, aa, aa - k) !=
            next_negative)
            ++out.backward_mismatches;
    }
    out.alternating_recurrence_valid = out.forward_mismatches == 0 &&
        out.backward_mismatches == 0;
    out.valid = out.parameter_domain && out.fundamental_recurrence_valid &&
        out.spine_adapter_valid && out.alternating_recurrence_valid;
    return out;
}

}  // namespace adelic
