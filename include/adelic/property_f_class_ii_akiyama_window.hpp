// Finite deviation window suggested by Akiyama's critical-coordinate bound.
//
// Akiyama's Lemma 4 gives |x_0+x_2| <= 2 for the exact cubic family once
// a>=7.  On the Class-II phase strip, the corresponding coordinate is eta,
// and an ordinary digit deviation d produces eta' = 1-eta+d.  If the
// published bound is transported through the eventual prefix/contact
// intertwiner, only five d-values can survive at one high step.  This header
// computes and checks that finite window exactly; it records a hypothesis
// interface, not the missing coordinate-transport theorem.
#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

#include "adelic/property_f_class_ii_carry_deviation.hpp"

namespace adelic {

struct PropertyFClassIIAkiyamaWindowCertificate {
    std::size_t parameter_a = 0;
    std::size_t checked_states = 0;
    std::size_t checked_digits = 0;
    std::size_t bounded_candidates = 0;
    std::size_t window_mismatches = 0;
    std::size_t eta0_candidates = 0;
    std::size_t eta1_candidates = 0;
    bool parameter_domain = false;
    bool five_value_window = false;
    bool candidate_classification_valid = false;
    bool valid = false;
};

inline bool property_f_class_ii_akiyama_eta_bound(long long eta) {
    return -2 <= eta && eta <= 2;
}

inline std::vector<long long>
property_f_class_ii_akiyama_allowed_deviations(long long eta) {
    std::vector<long long> out;
    // eta' = 1-eta+d and |eta'|<=2.
    for (long long deviation = -3; deviation <= 3; ++deviation) {
        const auto next_eta = 1 - eta + deviation;
        if (property_f_class_ii_akiyama_eta_bound(next_eta))
            out.push_back(deviation);
    }
    return out;
}

inline PropertyFClassIIAkiyamaWindowCertificate
derive_property_f_class_ii_akiyama_window(std::size_t a) {
    PropertyFClassIIAkiyamaWindowCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 7 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;
    const auto states = property_f_class_ii_rank_spine_coefficients(a);
    const auto aa = static_cast<long long>(a);
    const auto tail_count = 2 * a + 2 - 5;
    for (std::size_t step = 2; step < tail_count; ++step) {
        ++out.checked_states;
        const auto& state = states[step];
        const auto eta = state[0] + state[2];
        const auto allowed =
            property_f_class_ii_akiyama_allowed_deviations(eta);
        if (allowed.size() != 5) out.five_value_window = false;
        for (long long digit = 0; digit < aa; ++digit) {
            ++out.checked_digits;
            const auto deviation =
                property_f_class_ii_phase_deviation(state, aa, digit);
            const auto next_eta = 1 - eta + deviation;
            const bool bounded = property_f_class_ii_akiyama_eta_bound(next_eta);
            const bool listed = std::find(allowed.begin(), allowed.end(),
                                          deviation) != allowed.end();
            if (bounded) {
                ++out.bounded_candidates;
                if (eta == 0) ++out.eta0_candidates;
                if (eta == 1) ++out.eta1_candidates;
            }
            if (bounded != listed) ++out.window_mismatches;
        }
    }
    out.five_value_window =
        property_f_class_ii_akiyama_allowed_deviations(0).size() == 5 &&
        property_f_class_ii_akiyama_allowed_deviations(1).size() == 5;
    out.candidate_classification_valid = out.window_mismatches == 0;
    out.valid = out.parameter_domain && out.five_value_window &&
        out.candidate_classification_valid;
    return out;
}

}  // namespace adelic
