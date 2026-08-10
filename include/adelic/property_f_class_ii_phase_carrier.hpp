// Uniform scalar carrier behind the Class-II affine phase strip.
// On a phase state, the affine recurrence reduces to an eta/c0 odometer:
// eta=0 uses digit -c0 and eta=1 uses digit a+1-c0; two such steps move c0
// by one. This isolates the growing tail length from the finite boundary
// correction grammar.
#pragma once

#include <cstddef>

#include "adelic/property_f_class_ii_phase_strip.hpp"

namespace adelic {

struct PropertyFClassIIPhaseCarrierCertificate {
    std::size_t parameter_a = 0;
    std::size_t checked_eta0_states = 0;
    std::size_t checked_eta1_states = 0;
    std::size_t max_zero_channel_steps = 0;
    std::size_t expected_spine_height = 0;
    bool interval_law_valid = false;
    bool forced_digit_law_valid = false;
    bool two_step_law_valid = false;
    bool seed_return_valid = false;
    bool length_bound_valid = false;
    bool valid = false;
};

inline long long property_f_class_ii_phase_carrier_forced_digit(
        long long eta, long long c0, long long a) {
    return (a + 1) * eta - c0;
}

inline bool property_f_class_ii_phase_carrier_digit_admissible(
        long long eta, long long c0, long long a) {
    const auto digit = property_f_class_ii_phase_carrier_forced_digit(
        eta, c0, a);
    return 0 <= digit && digit < a;
}

inline PropertyFClassIIPhaseCarrierCertificate
derive_property_f_class_ii_phase_carrier(std::size_t a) {
    PropertyFClassIIPhaseCarrierCertificate out;
    out.parameter_a = a;
    if (a < 4 || a > property_f_class_ii_spine_max_safe_a) return out;
    const auto aa = static_cast<long long>(a);
    out.interval_law_valid = true;
    out.forced_digit_law_valid = true;
    out.two_step_law_valid = true;
    for (long long c0 = -aa - 1; c0 <= 1; ++c0) {
        ++out.checked_eta0_states;
        const bool expected = -(aa - 1) <= c0 && c0 <= 0;
        if (property_f_class_ii_phase_carrier_digit_admissible(0, c0, aa) !=
            expected)
            out.interval_law_valid = false;
        if (property_f_class_ii_phase_carrier_forced_digit(0, c0, aa) != -c0)
            out.forced_digit_law_valid = false;
    }
    for (long long c0 = 0; c0 <= aa + 2; ++c0) {
        ++out.checked_eta1_states;
        const bool expected = 2 <= c0 && c0 <= aa + 1;
        if (property_f_class_ii_phase_carrier_digit_admissible(1, c0, aa) !=
            expected)
            out.interval_law_valid = false;
        if (property_f_class_ii_phase_carrier_forced_digit(1, c0, aa) !=
            aa + 1 - c0)
            out.forced_digit_law_valid = false;
    }
    for (long long x = -(aa - 1); x <= 0; ++x) {
        const auto first = 1 - x;
        const auto second = -first;
        if (second != x - 1) out.two_step_law_valid = false;
    }
    for (long long x = 2; x <= aa + 1; ++x) {
        const auto first = -x;
        const auto second = 1 - first;
        if (second != x + 1) out.two_step_law_valid = false;
    }
    out.seed_return_valid =
        property_f_class_ii_phase_carrier_forced_digit(1, 1, aa) == aa &&
        property_f_class_ii_phase_carrier_forced_digit(0, -1, aa) == 1;
    out.max_zero_channel_steps = 2 * (a - 1);
    out.expected_spine_height = 2 * a + 2;
    out.length_bound_valid = out.max_zero_channel_steps <= 2 * (a - 1) &&
                             out.expected_spine_height == 2 * a + 2;
    out.valid = out.interval_law_valid && out.forced_digit_law_valid &&
                out.two_step_law_valid && out.seed_return_valid &&
                out.length_bound_valid;
    return out;
}

}  // namespace adelic
