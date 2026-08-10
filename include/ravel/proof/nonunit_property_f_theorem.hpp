// Number-system theorem for the monotone non-unit Pisot cone.
//
// Frougny--Solomyak's criterion is stronger than any one computed contact
// graph: for a dominant root of
//   x^d - b_1 x^(d-1) - ... - b_d,
// with b_1 >= ... >= b_d > 0, the beta number has Property (F).  This
// certificate records that theorem at the number-system boundary.  The
// canonical substitution and contact/adelic graph certificates are separate
// realization obligations and must not be confused with this implication.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ravel/proof/monotone_coefficient_cone.hpp"

namespace ravel::proof {

struct NonunitPropertyFTheoremCertificate {
    std::vector<long long> coefficients_high_to_low;
    std::size_t degree = 0;
    long long determinant = 0;
    long long constant_coefficient = 0;
    bool positive_coefficients = false;
    bool monotone_coefficients = false;
    bool primitive_companion = false;
    bool dominant_pisot_root = false;
    bool nonunit = false;
    bool frougny_solomyak_hypothesis = false;
    bool property_f_holds = false;
    std::string obstruction;
};

inline NonunitPropertyFTheoremCertificate
derive_nonunit_property_f_theorem(
    const std::vector<long long>& coefficients_high_to_low) {
    NonunitPropertyFTheoremCertificate out;
    out.coefficients_high_to_low = coefficients_high_to_low;
    out.degree = coefficients_high_to_low.size();
    const auto cone = derive_monotone_coefficient_cone_certificate(
        coefficients_high_to_low);
    out.determinant = cone.determinant;
    out.constant_coefficient = cone.constant_coefficient;
    out.positive_coefficients = cone.positive;
    out.monotone_coefficients = cone.monotone;
    out.primitive_companion = cone.primitive_companion;
    out.dominant_pisot_root = cone.primitive_companion;
    out.nonunit = cone.nonunit;
    out.frougny_solomyak_hypothesis = cone.condition_f_applies;
    out.property_f_holds = out.frougny_solomyak_hypothesis && out.nonunit;
    if (!out.positive_coefficients)
        out.obstruction = "coefficient vector contains a nonpositive term";
    else if (!out.monotone_coefficients)
        out.obstruction = "coefficient vector is not nonincreasing";
    else if (!out.nonunit)
        out.obstruction = "constant coefficient is unimodular, not non-unit";
    else if (!out.property_f_holds)
        out.obstruction = "Frougny--Solomyak hypotheses did not close";
    return out;
}

}  // namespace ravel::proof
