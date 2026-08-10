// Exact coefficient catalogue for the Class-II prefix/contact corrections.
//
// Every ordinary zero-prefix role contributes an integer constant after the
// beta-inverse step.  The sole exceptional role (the 0^a 1 prefix before the
// final 2 in sigma_{a,1}(0)) has reduced contribution [a+1,a+1,-1].  This
// header identifies that exceptional contribution as a + beta^(-1)U, where
// U=a+beta^(-1) is Akiyama's fundamental unit.  It is the finite symbolic
// interface between the literal prefix grammar and the bounded-correction
// coordinate system; it does not yet prove the global collar bound.
#pragma once

#include <array>
#include <cstddef>

#include "adelic/property_f_class_ii_akiyama_spine.hpp"
#include "adelic/property_f_class_ii_symbolic_tail_grammar.hpp"

namespace adelic {

struct PropertyFClassIIPrefixCorrectionCertificate {
    std::size_t parameter_a = 0;
    std::size_t ordinary_prefixes_checked = 0;
    std::size_t ordinary_mismatches = 0;
    std::size_t exceptional_delta_mismatches = 0;
    std::size_t exceptional_contribution_mismatches = 0;
    std::size_t unit_decomposition_mismatches = 0;
    std::size_t high_tail_steps_checked = 0;
    std::size_t exceptional_high_tail_hits = 0;
    bool parameter_domain = false;
    bool ordinary_roles_valid = false;
    bool exceptional_role_valid = false;
    bool unit_decomposition_valid = false;
    bool valid = false;
};

inline std::array<long long, 3>
property_f_class_ii_exceptional_prefix_delta() {
    // delta(0^a 1) = a*beta + U = beta^2 - 1.
    return {-1, 0, 1};
}

inline std::array<long long, 3>
property_f_class_ii_exceptional_prefix_contribution(std::size_t a) {
    return {static_cast<long long>(a + 1),
            static_cast<long long>(a + 1), -1};
}

inline PropertyFClassIIPrefixCorrectionCertificate
derive_property_f_class_ii_prefix_correction(std::size_t a) {
    PropertyFClassIIPrefixCorrectionCertificate out;
    out.parameter_a = a;
    out.parameter_domain = a >= 4 &&
        a <= property_f_class_ii_spine_max_safe_a;
    if (!out.parameter_domain) return out;
    const auto aa = static_cast<long long>(a);
    const auto roles = derive_property_f_class_ii_prefix_role_grammar(a);
    if (!roles.valid) return out;

    out.ordinary_roles_valid = true;
    for (std::size_t position = 0; position < a; ++position) {
        ++out.ordinary_prefixes_checked;
        const auto contribution = property_f_class_ii_role_contribution(
            a, 0, 0, position);
        if (contribution != std::array<long long, 3>{
                static_cast<long long>(position), 0, 0})
            ++out.ordinary_mismatches;
    }
    out.ordinary_roles_valid = out.ordinary_mismatches == 0;

    const auto unit = property_f_class_ii_akiyama_unit(aa, 1);
    const auto beta = std::array<long long, 3>{0, 1, 0};
    const auto delta = property_f_class_ii_exceptional_prefix_delta();
    const auto expected_delta = property_f_class_ii_coeff_add(
        unit, property_f_class_ii_coeff_scale(beta, aa));
    if (delta != expected_delta) ++out.exceptional_delta_mismatches;

    const auto contribution = property_f_class_ii_role_contribution(
        a, 2, 0, a + 1);
    const auto expected_contribution =
        property_f_class_ii_exceptional_prefix_contribution(a);
    if (contribution != expected_contribution)
        ++out.exceptional_contribution_mismatches;

    const auto inverse_unit = property_f_class_ii_beta_inverse(unit, aa);
    const auto reconstructed_contribution = property_f_class_ii_coeff_add(
        std::array<long long, 3>{aa, 0, 0}, inverse_unit);
    if (reconstructed_contribution != contribution)
        ++out.unit_decomposition_mismatches;

    // The exceptional role targets letter 2.  Every state on the
    // parameterized high tail after the seed/return has current letter 0,
    // so the high-tail induction sees only ordinary integer digits.
    const auto tail_count = 2 * a + 2 - 5;
    for (std::size_t step = 2; step < tail_count; ++step) {
        ++out.high_tail_steps_checked;
        const auto current_letter = static_cast<long long>(
            property_f_class_ii_spine_letter(step));
        if (current_letter == 2) ++out.exceptional_high_tail_hits;
        const auto roles_for_current =
            property_f_class_ii_prefix_roles_for_target(a, current_letter);
        const auto exceptional_parent = roles_for_current.find(0);
        if (exceptional_parent != roles_for_current.end())
            for (const auto position : exceptional_parent->second)
                if (current_letter == 2 && position == a + 1)
                    ++out.exceptional_high_tail_hits;
    }

    out.exceptional_role_valid = out.exceptional_delta_mismatches == 0 &&
        out.exceptional_contribution_mismatches == 0;
    out.unit_decomposition_valid = out.unit_decomposition_mismatches == 0;
    out.valid = out.parameter_domain && out.ordinary_roles_valid &&
        out.exceptional_role_valid && out.unit_decomposition_valid &&
        out.exceptional_high_tail_hits == 0;
    return out;
}

}  // namespace adelic
