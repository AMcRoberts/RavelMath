#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "math/poly_z.hpp"

namespace ravel {

// Canonical simple-Parry substitution for a finite expansion
// d_beta(1) = t_1 ... t_D:
//   i -> 0^{t_{i+1}} (i+1), i < D-1
//   D-1 -> 0^{t_D}.
// The unimodular-unit slice has t_D = 1.
inline std::vector<std::vector<std::int8_t>>
simple_parry_profile_rule(const std::vector<std::size_t>& digits) {
    const std::size_t D = digits.size();
    if (D < 2)
        throw std::invalid_argument("simple_parry_profile_rule: profile length must be >= 2");
    if (digits.back() == 0)
        throw std::invalid_argument("simple_parry_profile_rule: final digit must be positive");
    std::vector<std::vector<std::int8_t>> sigma(D);
    for (std::size_t i = 0; i + 1 < D; ++i) {
        sigma[i].assign(digits[i], std::int8_t{0});
        sigma[i].push_back(static_cast<std::int8_t>(i + 1));
    }
    sigma[D - 1].assign(digits[D - 1], std::int8_t{0});
    return sigma;
}

inline mathlib::PolyZ
simple_parry_profile_polynomial(const std::vector<long long>& digits) {
    const std::size_t D = digits.size();
    if (D < 2)
        throw std::invalid_argument("simple_parry_profile_polynomial: profile length must be >= 2");
    if (digits.back() <= 0)
        throw std::invalid_argument("simple_parry_profile_polynomial: final digit must be positive");
    for (auto digit : digits)
        if (digit < 0)
            throw std::invalid_argument("simple_parry_profile_polynomial: digits must be nonnegative");
    mathlib::PolyZ polynomial;
    polynomial.ensure_size(D + 1);
    for (std::size_t j = 0; j < D; ++j)
        mathlib::set_si(polynomial.coeff(D - 1 - j), -digits[j]);
    mathlib::set_si(polynomial.coeff(D), 1);
    polynomial.trim();
    return polynomial;
}

inline double simple_parry_profile_beta(const std::vector<std::size_t>& digits) {
    std::vector<long long> signed_digits(digits.begin(), digits.end());
    const auto polynomial = simple_parry_profile_polynomial(signed_digits);
    const std::size_t D = digits.size();
    auto evaluate = [&](double x) {
        double value = 0.0;
        for (std::size_t k = D + 1; k-- > 0;)
            value = value * x + mpz_get_d(polynomial.coeff(k).get());
        return value;
    };
    double lo = 1.0;
    double hi = 2.0;
    while (evaluate(hi) <= 0.0) hi *= 2.0;
    for (int iteration = 0; iteration < 240; ++iteration) {
        const double midpoint = 0.5 * (lo + hi);
        if (evaluate(midpoint) < 0.0) lo = midpoint;
        else hi = midpoint;
    }
    return 0.5 * (lo + hi);
}

} // namespace ravel
