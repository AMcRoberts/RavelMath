// Symbolic phase-strip forcing for the Class-II affine tail.
//
// The high spine alternates between two affine phase values.  Once a state is
// in that strip, the zero-prefix digit family has exactly one digit that can
// remain in it; every other digit exits the strip immediately.
#pragma once

#include <array>
#include <cstddef>

#include "adelic/property_f_class_ii_affine_tail.hpp"

namespace adelic {

struct PropertyFClassIIPhaseStripCertificate {
    std::size_t parameter_a = 0;
    std::size_t phase_states = 0;
    std::size_t phase_mismatches = 0;
    std::size_t forced_steps = 0;
    std::size_t forced_mismatches = 0;
    std::size_t phase_preserving_steps = 0;
    std::size_t phase_preserving_digit_mismatches = 0;
    bool parameter_domain = false;
    bool phase_strip_valid = false;
    bool forced_seed_valid = false;
    bool unique_digit_valid = false;
    bool valid = false;
};

inline bool property_f_class_ii_phase_pair_valid(
        const std::array<long long, 3>& state, long long a) {
    const auto eta = state[0] + state[2];
    const auto q = state[1] + (a + 1) * state[2];
    return (eta == 0 && q == 1) || (eta == 1 && q == 0);
}

inline PropertyFClassIIPhaseStripCertificate
derive_property_f_class_ii_phase_strip_certificate(std::size_t a) {
    PropertyFClassIIPhaseStripCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 4 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;
    const auto states = property_f_class_ii_rank_spine_coefficients(a);
    const auto digits = property_f_class_ii_rank_spine_digits(a);
    const auto aa = static_cast<long long>(a);
    const auto tail_count = 2 * a + 2 - 5;
    out.phase_strip_valid = true;
    for (std::size_t step = 1; step < tail_count; ++step) {
        ++out.phase_states;
        if (!property_f_class_ii_phase_pair_valid(states[step], aa)) {
            ++out.phase_mismatches;
            out.phase_strip_valid = false;
        }
    }

    // The zero seed uses digit 1 to enter the letter-1 state; that state then
    // has its unique prefix length a before the alternating zero tail begins.
    out.forced_steps = 2;
    const auto seed_next = property_f_class_ii_affine_tail_step(
        states[0], aa, 1);
    const auto one_next = property_f_class_ii_affine_tail_step(
        states[1], aa, aa);
    out.forced_seed_valid = seed_next == states[1] &&
        one_next == states[2];
    if (!out.forced_seed_valid) ++out.forced_mismatches;

    out.unique_digit_valid = true;
    for (std::size_t step = 2; step < tail_count; ++step) {
        ++out.phase_preserving_steps;
        std::size_t preserving = 0;
        std::size_t preserving_digit = 0;
        for (std::size_t digit = 0; digit < a; ++digit) {
            const auto candidate = property_f_class_ii_affine_tail_step(
                states[step], aa, static_cast<long long>(digit));
            if (property_f_class_ii_phase_pair_valid(candidate, aa)) {
                ++preserving;
                preserving_digit = digit;
            }
        }
        if (preserving != 1 || preserving_digit !=
            static_cast<std::size_t>(digits[step])) {
            ++out.phase_preserving_digit_mismatches;
            out.unique_digit_valid = false;
        }
    }
    out.valid = out.parameter_domain && out.phase_strip_valid &&
                out.forced_seed_valid && out.unique_digit_valid;
    return out;
}

}  // namespace adelic
