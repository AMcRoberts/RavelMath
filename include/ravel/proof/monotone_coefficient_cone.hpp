// Condition-F certificate for the monotone coefficient cone.
//
// P_a(x) = x^d - a[d-1]x^(d-1) - ... - a[0],
// with a[d-1] >= ... >= a[0] > 0.  The cone contains the generalized
// multinacci family (m,...,m,1) and exposes the determinant/valuation split
// through its constant coefficient a[0].
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace ravel::proof {

struct MonotoneCoefficientConeCertificate {
    std::vector<long long> coefficients_high_to_low;
    bool positive = false;
    bool monotone = false;
    bool primitive_companion = false;
    bool condition_f_applies = false;
    bool unimodular = false;
    bool nonunit = false;
    long long determinant = 0;
    long long constant_coefficient = 0;
    std::string obstruction;
};

inline MonotoneCoefficientConeCertificate
derive_monotone_coefficient_cone_certificate(
    const std::vector<long long>& coefficients_high_to_low) {
    MonotoneCoefficientConeCertificate out;
    out.coefficients_high_to_low = coefficients_high_to_low;
    if (coefficients_high_to_low.empty()) {
        out.obstruction = "coefficient cone requires positive degree";
        return out;
    }
    out.positive = true;
    for (const long long a : coefficients_high_to_low)
        if (a <= 0) out.positive = false;
    out.monotone = true;
    for (std::size_t i = 1; i < coefficients_high_to_low.size(); ++i)
        if (coefficients_high_to_low[i - 1] < coefficients_high_to_low[i])
            out.monotone = false;
    out.constant_coefficient = coefficients_high_to_low.back();
    out.determinant = out.constant_coefficient;
    out.unimodular = out.constant_coefficient == 1;
    out.nonunit = out.constant_coefficient > 1;
    // Strictly positive companion coefficients give a strongly connected
    // companion graph with aperiodicity from the constant term, hence a
    // unique Perron root. Frougny-Solomyak then supplies Condition F on the
    // monotone cone.
    out.primitive_companion = out.positive;
    out.condition_f_applies = out.positive && out.monotone &&
                              out.primitive_companion;
    if (!out.condition_f_applies)
        out.obstruction = "coefficient vector is outside the monotone cone";
    return out;
}

}  // namespace ravel::proof
