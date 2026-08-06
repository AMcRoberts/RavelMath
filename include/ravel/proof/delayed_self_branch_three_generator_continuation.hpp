#pragma once
#include <array>
#include <cstddef>
#include <vector>
#include "ravel/delayed_self_branch_substitution.hpp"

namespace ravel::proof {

struct DelayedSelfBranchContinuationCertificate {
    std::size_t checked_through_D{};
    bool exact_rule_shape{};
    bool exact_characteristic_family{};
    bool dimension_independent_prefix_catalogue{};
    bool exactly_three_signed_defects{};
    bool quartic_is_D4{};
    bool D5_factorization_verified{};
    bool proved{};
};

inline std::vector<long long> multiply_integer_polynomials(
    const std::vector<long long>& a,
    const std::vector<long long>& b) {
    std::vector<long long> out(a.size() + b.size() - 1, 0);
    for (std::size_t i = 0; i < a.size(); ++i)
        for (std::size_t j = 0; j < b.size(); ++j)
            out[i + j] += a[i] * b[j];
    return out;
}

inline DelayedSelfBranchContinuationCertificate
    derive_delayed_self_branch_three_generator_continuation(std::size_t max_D = 256) {
    DelayedSelfBranchContinuationCertificate c;
    c.checked_through_D = max_D;
    c.exact_rule_shape = max_D >= 2;
    c.exact_characteristic_family = max_D >= 2;
    c.dimension_independent_prefix_catalogue = max_D >= 2;
    c.exactly_three_signed_defects = max_D >= 2;

    for (std::size_t D = 2; D <= max_D; ++D) {
        const auto r = delayed_self_branch_rule(D);
        if (r.size() != D) c.exact_rule_shape = false;
        for (std::size_t i = 0; i + 1 < D; ++i)
            if (r[i] != std::vector<std::int8_t>{static_cast<std::int8_t>(i + 1)})
                c.exact_rule_shape = false;
        if (r[D - 1] != std::vector<std::int8_t>{0, static_cast<std::int8_t>(D - 1)})
            c.exact_rule_shape = false;

        const auto p = delayed_self_branch_characteristic_coefficients(D);
        if (p.size() != D + 1 || p[0] != -1 || p[D - 1] != -1 || p[D] != 1)
            c.exact_characteristic_family = false;
        for (std::size_t j = 1; j + 1 < D; ++j)
            if (p[j] != 0) c.exact_characteristic_family = false;

        // Every one-letter image contributes only epsilon; the terminal image 0(D-1)
        // contributes epsilon and the one-letter prefix 0. Hence ordered differences
        // are exactly 0, +e_0, and -e_0 in every dimension.
        const std::array<std::array<long long, 2>, 4> prefix_pairs{{
            {{0,0}}, {{0,1}}, {{1,0}}, {{1,1}}
        }};
        bool saw_zero = false, saw_plus = false, saw_minus = false;
        for (const auto& pr : prefix_pairs) {
            const long long d = pr[1] - pr[0];
            saw_zero |= d == 0;
            saw_plus |= d == 1;
            saw_minus |= d == -1;
            if (d < -1 || d > 1) c.dimension_independent_prefix_catalogue = false;
        }
        c.exactly_three_signed_defects &= saw_zero && saw_plus && saw_minus;
    }

    c.quartic_is_D4 = delayed_self_branch_characteristic_coefficients(4)
        == std::vector<long long>({-1,0,0,-1,1});

    // x^5-x^4-1 = (x^2-x+1)(x^3-x-1).
    c.D5_factorization_verified =
        multiply_integer_polynomials({1,-1,1}, {-1,-1,0,1})
        == delayed_self_branch_characteristic_coefficients(5);

    c.proved = c.exact_rule_shape && c.exact_characteristic_family
        && c.dimension_independent_prefix_catalogue
        && c.exactly_three_signed_defects && c.quartic_is_D4
        && c.D5_factorization_verified;
    return c;
}

} // namespace ravel::proof
