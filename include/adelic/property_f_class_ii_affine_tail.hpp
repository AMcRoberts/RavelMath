// Parameterized affine recurrence underlying the Class-II Property-(F)
// rank spine.  This is independent of graph enumeration: it checks that the
// explicit coefficient formulas and digit schedule are closed under the
// beta^{-1} carry map for every checked representable a>=4.
#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "adelic/property_f_class_ii_rank_spine.hpp"

namespace adelic {

struct PropertyFClassIIAffineTailCertificate {
    std::size_t parameter_a = 0;
    std::size_t state_count = 0;
    std::size_t digit_count = 0;
    std::size_t recurrence_steps = 0;
    std::size_t recurrence_mismatches = 0;
    bool parameter_domain = false;
    bool dimensions_match = false;
    bool valid = false;
    std::vector<std::array<long long, 3>> states;
    std::vector<long long> digits;
};

inline std::array<long long, 3> property_f_class_ii_affine_tail_step(
        const std::array<long long, 3>& state,
        long long a, long long digit) {
    // For p_a(x)=x^3-a*x^2-(a+1)*x-1, multiplication by beta^{-1}
    // sends (c0,c1,c2) to (c1-(a+1)c0,c2-a*c0,c0); [0,digit,0]
    // contributes digit to the first coordinate.
    return {state[1] - (a + 1) * state[0] + digit,
            state[2] - a * state[0], state[0]};
}

inline PropertyFClassIIAffineTailCertificate
derive_property_f_class_ii_affine_tail_certificate(std::size_t a) {
    PropertyFClassIIAffineTailCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 4 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;
    out.states = property_f_class_ii_rank_spine_coefficients(a);
    out.digits = property_f_class_ii_rank_spine_digits(a);
    out.state_count = out.states.size();
    out.digit_count = out.digits.size();
    out.dimensions_match = out.state_count == out.digit_count + 1;
    if (!out.dimensions_match) return out;
    const auto aa = static_cast<long long>(a);
    for (std::size_t step = 0; step < out.digit_count; ++step) {
        const auto next = property_f_class_ii_affine_tail_step(
            out.states[step], aa, out.digits[step]);
        ++out.recurrence_steps;
        if (next != out.states[step + 1]) ++out.recurrence_mismatches;
    }
    out.valid = out.parameter_domain && out.dimensions_match &&
                out.recurrence_mismatches == 0;
    return out;
}

}  // namespace adelic
