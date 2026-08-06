#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "math/charpoly.hpp"
#include "math/poly_z.hpp"

namespace ravel::proof {

// Source of truth for the n-bonacci characteristic matrix.
//
// The n-bonacci characteristic polynomial is
//   nbonacci_charmpoly(n) = X + X^2 + ... + X^n - 1
// as defined by the campaign's `nbonacci_charpoly` generator. Its
// companion matrix C_n is the n x n matrix
//
//   C_n[i, i + 1] = 1        for i = 0..n-2
//   C_n[n-1, j]   = -c_j    for j = 0..n-1
//   C_n[i, j]     = 0       otherwise
//
// where c_j is the coefficient of X^j in nbonacci_charmpoly(n).
// mathlib::charpoly_faddeev_leverrier is used to certify that the
// matrix actually generates the n-bonacci characteristic polynomial,
// so this object is a checked source, not a guess.
//
// The Cayley-Hamilton-verified companion's first-row cofactor
// expansion does NOT give the probe's q_matrix / r_matrix.  Those
// are independent (n-1) x (n-1) integer[X] matrices defined by
// their own entry rules.  The cofactor identity
//   X * det(q_matrix n) + (-1)^n * det(r_matrix n)
//     = nbonacci_charmpoly(n)
// is verified by direct determinant computation, not by a row
// deletion from C_n.  See q_matrix_layout / r_matrix_layout below.

struct CharacteristicMatrixSource {
    std::vector<std::vector<long long>> entries;
    mathlib::PolyZ characteristic;
};

inline long long bigint_to_ll(const mathlib::BigInt& x) {
    if (!mpz_fits_slong_p(x.get()))
        throw std::runtime_error("bigint_to_ll: coefficient out of long long range");
    return mpz_get_si(x.get());
}

inline mathlib::PolyZ nbonacci_charmpoly(std::size_t n) {
    mathlib::PolyZ result(0);
    for (std::size_t k = 1; k <= n; ++k) {
        mathlib::PolyZ term({0, 1});
        for (std::size_t i = 1; i < k; ++i) term = term * mathlib::PolyZ({0, 1});
        result = result + term;
    }
    return result - mathlib::PolyZ(1);
}

inline CharacteristicMatrixSource characteristic_matrix_for_n(std::size_t n) {
    if (n < 1) throw std::runtime_error("characteristic_matrix_for_n: n must be >= 1");
    const auto cp = nbonacci_charmpoly(n);
    if (static_cast<int>(cp.degree()) != static_cast<int>(n))
        throw std::runtime_error("characteristic_matrix_for_n: degree mismatch");
    CharacteristicMatrixSource source;
    source.characteristic = cp;
    source.entries.assign(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i + 1 < n; ++i) source.entries[i][i + 1] = 1;
    for (std::size_t j = 0; j < n; ++j)
        source.entries[n - 1][j] = -bigint_to_ll(cp.coeff(j));
    const auto verified = mathlib::charpoly_faddeev_leverrier(source.entries);
    if (!(verified == cp))
        throw std::runtime_error("characteristic_matrix_for_n: Cayley-Hamilton check failed");
    return source;
}

inline std::vector<std::vector<long long>>
cofactor_minor(const std::vector<std::vector<long long>>& A,
                std::size_t row, std::size_t column) {
    const std::size_t n = A.size();
    if (n == 0) return {};
    if (row >= n || column >= n)
        throw std::runtime_error("cofactor_minor: out of range");
    std::vector<std::vector<long long>> out;
    out.reserve(n - 1);
    for (std::size_t i = 0; i < n; ++i) {
        if (i == row) continue;
        std::vector<long long> row_out;
        row_out.reserve(n - 1);
        for (std::size_t j = 0; j < n; ++j) {
            if (j == column) continue;
            row_out.push_back(A[i][j]);
        }
        out.push_back(std::move(row_out));
    }
    return out;
}

inline long long cofactor_sign(std::size_t row, std::size_t column) {
    return (row + column) % 2 == 0 ? 1 : -1;
}

// The (n-1) x (n-1) q_matrix and r_matrix are independent integer[X]
// matrices.  Their determinants satisfy the cofactor identity
//   X * det(q_matrix n) + (-1)^n * det(r_matrix n) = nbonacci_charmpoly(n).
// These are the entry rules that produce that identity.  Reverse-
// engineered from the probe's nbonacci_charmpoly_proof_probe.cpp:
//   q_matrix n is (n-1) x (n-1) with
//     q[i, i] = X        for i = 0..n-2
//     q[i, i+1] = -1     for i = 0..n-3
//     q[n-2, j] = 1       for j = 0..n-2
//     q[n-2, n-2] = X+1   (overrides the diagonal entry in the last row)
//   r_matrix n is (n-1) x (n-1) with
//     r[i, i] = -1       for i = 0..n-2
//     r[i+1, i] = X      for i = 0..n-3
// (the explicit entry lists are exactly the probe's).  See the
// ravel::proof::PolyZ machinery in math/poly_z.hpp for the
// indeterminate `X` representation.
struct PolyMatrixLayout {
    std::size_t rows = 0;
    std::size_t cols = 0;
};

inline PolyMatrixLayout q_matrix_layout(std::size_t n) {
    if (n < 1) throw std::runtime_error("q_matrix_layout: n must be >= 1");
    return {n - 1, n - 1};
}

inline PolyMatrixLayout r_matrix_layout(std::size_t n) {
    if (n < 1) throw std::runtime_error("r_matrix_layout: n must be >= 1");
    return {n - 1, n - 1};
}

} // namespace ravel::proof
