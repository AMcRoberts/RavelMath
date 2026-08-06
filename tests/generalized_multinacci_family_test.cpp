#include <cmath>
#include <iostream>
#include <stdexcept>

#include "ravel/generalized_multinacci.hpp"

int main() {
    using ravel::generalized_multinacci_beta;
    using ravel::generalized_multinacci_polynomial;
    using ravel::generalized_multinacci_rule;

    const auto fib = generalized_multinacci_rule(3, 1);
    if (fib != std::vector<std::vector<std::int8_t>>{{0,1},{0,2},{0}})
        throw std::runtime_error("m=1 specialization failed");

    const auto doubled = generalized_multinacci_rule(4, 2);
    if (doubled != std::vector<std::vector<std::int8_t>>{{0,0,1},{0,0,2},{0,0,3},{0}})
        throw std::runtime_error("m=2 rule failed");

    const auto p = generalized_multinacci_polynomial(4, 2);
    if (mathlib::str(p) != "x^4 + -2*x^3 + -2*x^2 + -2*x + -1")
        throw std::runtime_error("polynomial construction failed: " + mathlib::str(p));

    const double beta = generalized_multinacci_beta(2, 2);
    if (std::abs(beta - (1.0 + std::sqrt(2.0))) > 1e-12)
        throw std::runtime_error("beta isolation failed");

    std::cout << "generalized multinacci family PASS\n";
}
