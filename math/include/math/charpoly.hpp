// math/charpoly.hpp
//
// Characteristic polynomial of a square integer matrix, via the
// Faddeev-LeVerrier algorithm (exact rational arithmetic internally,
// verified integral on output -- this is a real theorem, not a hope:
// for an integer input matrix, every coefficient IS an integer, and
// the implementation below checks that rather than assuming it).
//
// Lifts the Work List item flagged in math/TOOLS.md under "Linear
// algebra": "Characteristic polynomial via Faddeev-LeVerrier (in
// barge.hpp -- lift to mathlib)". This is the first GENERAL
// (non-Pisot-specific, Tier 2) charpoly tool in mathlib itself;
// ravel::barge.hpp's existing copy is untouched by this file.
//
// Reference: Faddeev, D.K. & Faddeeva, V.N., "Computational Methods
// of Linear Algebra", Freeman, 1963 (the algorithm's standard
// citation); Gantmacher, F.R., "The Theory of Matrices", Vol. 1,
// Chelsea, 1959, Chapter 4, Sec. 7 (an equivalent derivation via
// resolvent expansion). This is exactly the input `perron_frobenius.hpp`
// needs to build a `QBetaRing` from a raw integer matrix without
// requiring the caller to already know the matrix's charpoly by some
// other route (previously every mathlib caller of `QBetaRing` got its
// charpoly from Pisot-specific, hand-derived sources).
//
// Recurrence (for an n x n matrix A, producing det(xI - A) = x^n +
// c_{n-1} x^{n-1} + ... + c_0):
//   M_0 = 0 (the n x n zero matrix)
//   for k = 1..n:
//     M_k = A * (M_{k-1} + c_{n-k+1} * I)
//     c_{n-k} = -tr(M_k) / k
// with the free Cayley-Hamilton self-check A * (M_n + c_0 * I) = 0.

#pragma once

#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "math/mat_q.hpp"
#include "math/poly_z.hpp"

namespace mathlib {

// Converts a Rat known to be integral to a BigInt, throwing if it
// isn't (rather than silently truncating) -- the whole point of
// running Faddeev-LeVerrier over Q internally is to catch a violated
// integrality guarantee as an error, not paper over it.
inline BigInt rat_to_bigint_exact(const Rat& x, const char* context) {
    BigInt den;
    mpz_set(den.get(), mpq_denref(x.get()));
    if (!is_one(den)) {
        throw std::runtime_error(
            std::string(context) + ": expected an integer, got a non-integral Rat");
    }
    BigInt num;
    mpz_set(num.get(), mpq_numref(x.get()));
    return num;
}

inline PolyZ charpoly_faddeev_leverrier(const std::vector<std::vector<long long>>& A_int) {
    const std::size_t n = A_int.size();
    if (n == 0) {
        throw std::invalid_argument("charpoly_faddeev_leverrier: empty matrix");
    }
    for (const auto& row : A_int) {
        if (row.size() != n) {
            throw std::invalid_argument("charpoly_faddeev_leverrier: matrix is not square");
        }
    }

    MatQ A(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            A.at(i, j) = Rat(A_int[i][j], 1);
        }
    }

    // c[k] is the coefficient of x^k; c[n] = 1 fixes monic-ness.
    std::vector<Rat> c(n + 1);
    set_si(c[n], 1);
    MatQ M = zero_mat(n, n);  // M_0

    for (std::size_t k = 1; k <= n; ++k) {
        MatQ shifted = M + (c[n - k + 1] * identity_mat(n));
        M = A * shifted;
        Rat tr;
        set_si(tr, 0);
        for (std::size_t i = 0; i < n; ++i) {
            Rat t;
            add(t, tr, M.at(i, i));
            tr = t;
        }
        Rat neg_tr;
        neg(neg_tr, tr);
        Rat k_rat;
        set_si(k_rat, static_cast<long long>(k));
        Rat ck;
        div(ck, neg_tr, k_rat);
        c[n - k] = ck;
    }

    // Free Cayley-Hamilton self-check: A * (M_n + c_0 I) must be zero.
    {
        MatQ should_be_zero = A * (M + (c[0] * identity_mat(n)));
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                if (!is_zero(should_be_zero.at(i, j))) {
                    throw std::runtime_error(
                        "charpoly_faddeev_leverrier: Cayley-Hamilton self-check "
                        "failed (A*(M_n + c_0*I) != 0) -- internal bug, please report");
                }
            }
        }
    }

    PolyZ result;
    result.ensure_size(n + 1);
    for (std::size_t k = 0; k <= n; ++k) {
        result.coeff(k) = rat_to_bigint_exact(
            c[k], "charpoly_faddeev_leverrier: non-integral coefficient "
                  "(input matrix was not integer-valued, or an internal bug)");
    }
    return result;
}

}  // namespace mathlib
