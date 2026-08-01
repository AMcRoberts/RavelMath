// math/poly_discriminant.hpp
//
// Arbitrary-precision polynomial discriminant, via the Sylvester
// matrix of (f, f') and fraction-free (Bareiss) Gaussian elimination
// over mathlib::BigInt.
//
// This is the arbitrary-precision upgrade of
// include/adelic/maximal_order.hpp's poly_discriminant_ll /
// integer_determinant, which are explicitly scoped (per that file's
// own header comment) to `long long` arithmetic -- "fine for the
// small coefficients this project's characteristic polynomials have,"
// not a general-purpose big-integer routine. Larger-degree number
// fields (needed for anything beyond this project's own small-
// alphabet Pisot substitutions -- e.g. the class-field-tower work
// docs/RESEARCH_STATUS.md's OpenAI/Sawin correction note flags as
// genuinely new, separate machinery) need discriminants whose
// magnitude can exceed 64 bits well before the degree gets large.
//
// Property-tested (math/tests/test_poly_discriminant.cpp) against
// poly_discriminant_ll's existing long long results for every one of
// that file's current test cases, plus a degree/coefficient regime
// where long long would overflow and BigInt must still be exact.

#pragma once

#include <cstddef>
#include <vector>

#include "math/bigint.hpp"

namespace mathlib {

// Exact BigInt determinant via fraction-free (Bareiss) Gaussian
// elimination. No precision limit beyond available memory.
inline BigInt integer_determinant_bigint(std::vector<std::vector<BigInt>> M) {
    const std::size_t n = M.size();
    if (n == 0) return BigInt(0);
    BigInt prev_pivot; set_si(prev_pivot, 1);
    int sign = 1;
    for (std::size_t k = 0; k + 1 < n; ++k) {
        if (is_zero(M[k][k])) {
            std::size_t sw = k + 1;
            while (sw < n && is_zero(M[sw][k])) ++sw;
            if (sw == n) return BigInt(0);
            std::swap(M[k], M[sw]);
            sign = -sign;
        }
        for (std::size_t i = k + 1; i < n; ++i) {
            for (std::size_t j = k + 1; j < n; ++j) {
                // M[i][j] = (M[i][j]*M[k][k] - M[i][k]*M[k][j]) / prev_pivot,
                // exact division guaranteed by the Bareiss identity.
                BigInt a, b, num;
                mul(a, M[i][j], M[k][k]);
                mul(b, M[i][k], M[k][j]);
                sub(num, a, b);
                BigInt q;
                divexact(q, num, prev_pivot);
                M[i][j] = q;
            }
            set_si(M[i][k], 0);
        }
        prev_pivot = M[k][k];
    }
    BigInt result = M[n - 1][n - 1];
    if (sign < 0) neg(result);
    return result;
}

// Discriminant of a monic integer polynomial f of degree n, via
// disc(f) = (-1)^{n(n-1)/2} * Res(f,f') for monic f, computed as the
// Sylvester-matrix determinant above. f given highest-degree-first
// (e.g. x^3 - x^2 - 2x - 8 => {1,-1,-2,-8}), same convention as
// poly_discriminant_ll.
inline BigInt poly_discriminant_bigint(const std::vector<long long>& f_high_to_low) {
    long long n = static_cast<long long>(f_high_to_low.size()) - 1;
    std::vector<BigInt> f(f_high_to_low.size());
    for (std::size_t i = 0; i < f_high_to_low.size(); ++i)
        set_si(f[i], f_high_to_low[i]);
    // f' (derivative), highest-degree-first, degree n-1.
    std::vector<BigInt> fp;
    fp.reserve(static_cast<std::size_t>(n));
    for (long long i = 0; i < n; ++i) {
        long long deg = n - i;
        BigInt term;
        mul_si(term, f[static_cast<std::size_t>(i)], deg);
        fp.push_back(term);
    }
    long long m = n - 1;  // degree of f'
    std::size_t size = static_cast<std::size_t>(n + m);
    std::vector<std::vector<BigInt>> S(
        size, std::vector<BigInt>(size, BigInt(0)));
    for (long long shift = 0; shift < m; ++shift) {
        for (std::size_t j = 0; j < f.size(); ++j) {
            S[static_cast<std::size_t>(shift)][static_cast<std::size_t>(shift) + j] = f[j];
        }
    }
    for (long long shift = 0; shift < n; ++shift) {
        for (std::size_t j = 0; j < fp.size(); ++j) {
            S[static_cast<std::size_t>(m + shift)][static_cast<std::size_t>(shift) + j] = fp[j];
        }
    }
    BigInt res = integer_determinant_bigint(std::move(S));
    long long sign_exp = (n * (n - 1) / 2) % 2;
    if (sign_exp != 0) neg(res);
    return res;
}

}  // namespace mathlib
