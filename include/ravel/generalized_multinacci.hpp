#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "math/poly_z.hpp"

namespace ravel {

// Unimodular confluent-Parry / generalized multinacci unit family:
//   sigma(i) = 0^m (i+1), i < D-1; sigma(D-1) = 0
//   p_{D,m}(x) = x^D - m(x + ... + x^{D-1}) - 1.
inline std::vector<std::vector<std::int8_t>>
generalized_multinacci_rule(std::size_t dimension, std::size_t multiplicity) {
    if (dimension < 2)
        throw std::invalid_argument("generalized_multinacci_rule: dimension must be >= 2");
    if (multiplicity < 1)
        throw std::invalid_argument("generalized_multinacci_rule: multiplicity must be positive");
    std::vector<std::vector<std::int8_t>> sigma(dimension);
    for (std::size_t i = 0; i + 1 < dimension; ++i) {
        sigma[i].assign(multiplicity, std::int8_t{0});
        sigma[i].push_back(static_cast<std::int8_t>(i + 1));
    }
    sigma[dimension - 1] = {0};
    return sigma;
}

inline mathlib::PolyZ generalized_multinacci_polynomial(
    std::size_t dimension, long long multiplicity) {
    if (dimension < 2)
        throw std::invalid_argument("generalized_multinacci_polynomial: dimension must be >= 2");
    if (multiplicity < 1)
        throw std::invalid_argument("generalized_multinacci_polynomial: multiplicity must be positive");
    mathlib::PolyZ polynomial;
    polynomial.ensure_size(dimension + 1);
    mathlib::set_si(polynomial.coeff(0), -1);
    for (std::size_t k = 1; k < dimension; ++k)
        mathlib::set_si(polynomial.coeff(k), -multiplicity);
    mathlib::set_si(polynomial.coeff(dimension), 1);
    polynomial.trim();
    return polynomial;
}

inline double generalized_multinacci_beta(
    std::size_t dimension, std::size_t multiplicity) {
    const auto polynomial = generalized_multinacci_polynomial(
        dimension, static_cast<long long>(multiplicity));
    auto evaluate = [&](double x) {
        double value = 0.0;
        for (std::size_t k = dimension + 1; k-- > 0;)
            value = value * x + mpz_get_d(polynomial.coeff(k).get());
        return value;
    };
    double lo = 1.0;
    double hi = static_cast<double>(multiplicity + 2);
    while (evaluate(hi) <= 0.0) hi *= 2.0;
    for (int iteration = 0; iteration < 240; ++iteration) {
        const double midpoint = 0.5 * (lo + hi);
        if (evaluate(midpoint) < 0.0) lo = midpoint;
        else hi = midpoint;
    }
    return 0.5 * (lo + hi);
}

}  // namespace ravel
