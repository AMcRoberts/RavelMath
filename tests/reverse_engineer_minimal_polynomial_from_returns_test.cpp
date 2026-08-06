// Verifies AM's proposed reverse-engineering technique on two
// structurally different substitutions with known minimal polynomials,
// confirming the recovered recurrence matches exactly in both cases.

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/reverse_engineer_minimal_polynomial_from_returns.hpp"

using namespace ravel::proof;

int main() {
    // sigma_{0,1}: known minimal polynomial x^3 - x - 1 (the plastic
    // number), predicting g(K) = g(K-2) + g(K-3).
    {
        std::array<std::vector<long long>, 3> images = {
            std::vector<long long>{1, 2}, std::vector<long long>{2}, std::vector<long long>{0}};
        auto g = count_clean_returns<3>(images, 0, 24);
        auto fit = fit_minimal_linear_recurrence(g);
        std::cout << "sigma_{0,1}: found=" << fit.found << " order=" << fit.order << " coeffs=[";
        for (auto c : fit.coefficients) std::cout << c << " ";
        std::cout << "]\n";
        assert(fit.found);
        assert(fit.order == 3);
        assert((fit.coefficients == std::vector<long long>{0, 1, 1}));  // g(K)=0*g(K-1)+1*g(K-2)+1*g(K-3)
    }

    // x^3-2x^2-x+1's substitution (sigma(0)=0,0,1 sigma(1)=2
    // sigma(2)=0,1): known minimal polynomial x^3-2x^2-x+1, predicting
    // g(K) = 2 g(K-1) + g(K-2) - g(K-3).
    {
        std::array<std::vector<long long>, 3> images = {
            std::vector<long long>{0, 0, 1}, std::vector<long long>{2}, std::vector<long long>{0, 1}};
        auto g = count_clean_returns<3>(images, 0, 24);
        auto fit = fit_minimal_linear_recurrence(g);
        std::cout << "x^3-2x^2-x+1 substitution: found=" << fit.found << " order=" << fit.order << " coeffs=[";
        for (auto c : fit.coefficients) std::cout << c << " ";
        std::cout << "]\n";
        assert(fit.found);
        assert(fit.order == 3);
        assert((fit.coefficients == std::vector<long long>{2, 1, -1}));  // g(K)=2g(K-1)+1g(K-2)-1g(K-3)
    }

    // Sanity control: a sequence with NO small linear recurrence should
    // correctly report not-found, not a spurious match. (Squares was
    // tried first and rejected as a control: a degree-d polynomial
    // sequence DOES satisfy a genuine order-(d+1) linear recurrence
    // with characteristic polynomial (x-1)^(d+1) -- i^2 satisfies
    // a(n)=3a(n-1)-3a(n-2)+a(n-3) exactly, so it is not a valid
    // negative control. Primes have no such structure.)
    {
        std::vector<long long> primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73};
        auto fit = fit_minimal_linear_recurrence(primes, 4);
        std::cout << "control (primes, order<=4): found=" << fit.found << "\n";
        assert(!fit.found);
    }

    std::cout << "reverse_engineer_minimal_polynomial_from_returns: both substitutions' minimal "
                 "polynomials recovered exactly from pure counting data; control correctly finds nothing.\n";
    return 0;
}
