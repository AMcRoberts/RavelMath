// Exact deviation law for the Class-II phase carrier.
//
// Akiyama's bounded-correction argument separates a preferred leading
// coordinate from a finite correction window.  Before identifying that
// window with the literal prefix/contact coordinates, we can expose the
// exact local law already present in our affine model: the forced digit is
// the zero-deviation digit, q' is always eta, and a digit deviation d changes
// the next eta by exactly d.  Consequently every nonzero deviation leaves
// the two-state phase strip in one step.
#pragma once

#include <array>
#include <cstddef>

#include "adelic/property_f_class_ii_phase_strip.hpp"

namespace adelic {

struct PropertyFClassIICarryDeviationCertificate {
    std::size_t parameter_a = 0;
    std::size_t checked_states = 0;
    std::size_t checked_digits = 0;
    std::size_t quotient_mismatches = 0;
    std::size_t eta_mismatches = 0;
    std::size_t zero_deviation_preserving = 0;
    std::size_t nonzero_deviation_exits = 0;
    std::size_t unexpected_phase_returns = 0;
    bool parameter_domain = false;
    bool quotient_law_valid = false;
    bool deviation_law_valid = false;
    bool strip_exit_valid = false;
    bool valid = false;
};

inline long long property_f_class_ii_phase_deviation(
        const std::array<long long, 3>& state,
        long long a,
        long long digit) {
    return digit - property_f_class_ii_phase_forced_digit(state, a);
}

inline long long property_f_class_ii_expected_next_eta(
        const std::array<long long, 3>& state,
        long long a,
        long long digit) {
    const auto eta = state[0] + state[2];
    return 1 - eta + property_f_class_ii_phase_deviation(state, a, digit);
}

inline PropertyFClassIICarryDeviationCertificate
derive_property_f_class_ii_carry_deviation(std::size_t a) {
    PropertyFClassIICarryDeviationCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 4 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;

    const auto states = property_f_class_ii_rank_spine_coefficients(a);
    const auto aa = static_cast<long long>(a);
    out.quotient_law_valid = true;
    out.deviation_law_valid = true;
    out.strip_exit_valid = true;
    // The first two states are the seed and letter-1 return.  The phase law
    // begins at the alternating zero-prefix tail, exactly as in the phase
    // strip certificate.
    const auto tail_count = 2 * a + 2 - 5;
    for (std::size_t step = 2; step < tail_count; ++step) {
        const auto& state = states[step];
        ++out.checked_states;
        const auto eta = state[0] + state[2];
        const auto forced = property_f_class_ii_phase_forced_digit(state, aa);
        for (long long digit = 0; digit < aa; ++digit) {
            ++out.checked_digits;
            const auto candidate = property_f_class_ii_affine_tail_step(
                state, aa, digit);
            const auto q_next = candidate[1] + (aa + 1) * candidate[2];
            if (q_next != eta) ++out.quotient_mismatches;
            const auto expected_eta =
                property_f_class_ii_expected_next_eta(state, aa, digit);
            if (candidate[0] + candidate[2] != expected_eta)
                ++out.eta_mismatches;

            const auto deviation = digit - forced;
            if (deviation == 0) {
                ++out.zero_deviation_preserving;
                if (!property_f_class_ii_phase_pair_valid(candidate, aa))
                    ++out.unexpected_phase_returns;
            } else {
                ++out.nonzero_deviation_exits;
                if (property_f_class_ii_phase_pair_valid(candidate, aa))
                    ++out.unexpected_phase_returns;
            }
        }
    }
    out.quotient_law_valid = out.quotient_mismatches == 0;
    out.deviation_law_valid = out.eta_mismatches == 0;
    out.strip_exit_valid = out.unexpected_phase_returns == 0;
    out.valid = out.parameter_domain && out.quotient_law_valid &&
                out.deviation_law_valid && out.strip_exit_valid;
    return out;
}

}  // namespace adelic
