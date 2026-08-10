// Exact algebraic reductions for Akiyama's finite correction cases.
//
// Section 4 of Akiyama's cubic proof reduces the bounded (kappa_1,kappa_2)
// window to a small collection of identities involving U=a+beta^{-1} and
// beta^{-2}.  These are checked in the same power-basis coefficient lattice
// as the Class-II spine.  The identities are algebraic reductions only: the
// beta-expansion admissibility of the reduced fundamental terms is a separate
// obligation and is deliberately not inferred here.
#pragma once

#include <cstddef>

#include "adelic/property_f_class_ii_akiyama_spine.hpp"

namespace adelic {

struct PropertyFClassIIAkiyamaCorrectionCaseCertificate {
    std::size_t parameter_a = 0;
    std::size_t identities_checked = 0;
    std::size_t mismatches = 0;
    bool parameter_domain = false;
    bool identities_valid = false;
    bool valid = false;
};

inline std::array<long long, 3>
property_f_class_ii_akiyama_beta_inverse_squared(
        const std::array<long long, 3>& value, long long a) {
    return property_f_class_ii_beta_inverse(
        property_f_class_ii_beta_inverse(value, a), a);
}

inline PropertyFClassIIAkiyamaCorrectionCaseCertificate
derive_property_f_class_ii_akiyama_correction_cases(std::size_t a) {
    PropertyFClassIIAkiyamaCorrectionCaseCertificate out;
    out.parameter_a = a;
    // The identities are used at Akiyama's sharp final threshold a>=15.
    out.parameter_domain = a >= 15 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;

    const auto aa = static_cast<long long>(a);
    const auto one = std::array<long long, 3>{1, 0, 0};
    const auto beta = std::array<long long, 3>{0, 1, 0};
    const auto beta_squared = std::array<long long, 3>{0, 0, 1};
    const auto unit = property_f_class_ii_akiyama_unit(aa, 1);
    const auto add = [](const auto& x, const auto& y) {
        return property_f_class_ii_coeff_add(x, y);
    };
    const auto scale = [](const auto& x, long long k) {
        return property_f_class_ii_coeff_scale(x, k);
    };
    const auto inv2 = [&](const auto& x) {
        return property_f_class_ii_akiyama_beta_inverse_squared(x, aa);
    };
    const auto check = [&](const auto& lhs, const auto& rhs,
                           auto& result) {
        ++result.identities_checked;
        if (lhs != rhs) ++result.mismatches;
    };

    // aU + 2 beta - beta^2 = 1 + beta^{-2}((a-2)U + beta).
    check(add(add(scale(unit, aa), scale(beta, 2)), scale(beta_squared, -1)),
          add(one, inv2(add(scale(unit, aa - 2), beta))), out);
    // (a+1)U + beta - beta^2 = beta^{-2}((a-1)U + beta).
    check(add(add(scale(unit, aa + 1), beta), scale(beta_squared, -1)),
          inv2(add(scale(unit, aa - 1), beta)), out);
    // The next boundary case is the preceding identity with one beta added.
    check(add(add(scale(unit, aa + 1), scale(beta, 2)),
              scale(beta_squared, -1)),
          add(beta, inv2(add(scale(unit, aa - 1), beta))), out);
    // beta - U = a beta^{-1} + beta^{-2}.
    check(add(beta, scale(unit, -1)),
          add(scale(property_f_class_ii_beta_inverse(one, aa), aa),
              property_f_class_ii_akiyama_beta_inverse_squared(one, aa)),
          out);
    // 2 beta^2 - aU - 2 beta
    //   = a beta + (a-1) + beta^{-2}(beta^2-(a-2)U).
    check(add(add(scale(beta_squared, 2), scale(unit, -aa)),
              scale(beta, -2)),
          add(add(scale(beta, aa), scale(one, aa - 1)),
              inv2(add(beta_squared, scale(unit, -(aa - 2))))), out);
    // 2 beta^2 - (a+1)U - beta
    //   = a beta + a + beta^{-2}(beta^2-(a-1)U).
    check(add(add(scale(beta_squared, 2), scale(unit, -(aa + 1))),
              scale(beta, -1)),
          add(add(scale(beta, aa), scale(one, aa)),
              inv2(add(beta_squared, scale(unit, -(aa - 1))))), out);

    out.identities_valid = out.mismatches == 0;
    out.valid = out.parameter_domain && out.identities_valid;
    return out;
}

}  // namespace adelic
