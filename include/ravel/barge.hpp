// barge.hpp
//
// Barge-classification helpers for primitive Pisot substitutions.
// Mirrors `python/pisot_survey.py::barge_flags`:
//   inj_init = all column[0]'s are distinct  (distinct initials)
//   const_fin = all column[LAST]'s are equal  (constant finals)
//
// plus `classify(matrix) -> {beta, b2, pisot, irred}`, the
// Pisot-status test from `python/pisot_survey.py::classify` --
// spectral invariants for a primitive integer matrix, with
// irreducibility over Q checked by rational-root testing of the
// characteristic polynomial.

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/charpoly.hpp"

namespace ravel {

struct BargeFlags {
    bool distinct_initials;
    bool constant_finals;
};

template <typename Word>
inline BargeFlags barge_flags(const std::vector<Word>& sigma) {
    bool distinct_initials = true;
    bool constant_finals   = true;
    if (sigma.empty()) return { false, false };
    std::vector<std::int8_t> initials, finals;
    for (auto& w : sigma) {
        if (w.empty()) {
            distinct_initials = false;
            constant_finals   = false;
            continue;
        }
        initials.push_back(w.front());
        finals.push_back(w.back());
    }
    for (std::size_t i = 0; i < initials.size(); ++i) {
        for (std::size_t j = i + 1; j < initials.size(); ++j) {
            if (initials[i] == initials[j]) distinct_initials = false;
            if (finals[i]   != finals[j])   constant_finals   = false;
        }
    }
    return { distinct_initials, constant_finals };
}

struct PisotClassify {
    double beta;
    double beta_abs_conj;     // |second-largest eigenvalue|
    bool   pisot;             // dominant real > 1, others |< 1|
    bool   irred;             // characteristic polynomial irreducible over Q
};

// Integer characteristic polynomial det(xI - M) via the
// Faddeev-LeVerrier algorithm with alternating signs.  Output is
// the polynomial coefficients in descending order (highest
// degree first): cp[k] is the coefficient of x^(n-k).  The
// char poly = x^n + cp[1] x^(n-1) + cp[2] x^(n-2) + ... + cp[n],
// so for n=3 the matrix [[0,0,1],[1,0,1],[0,1,0]] yields
// [1, 0, -1, -1] -- i.e. x^3 - x - 1.
//
// Arithmetic delegates to mathlib's arbitrary-precision
// Faddeev--LeVerrier implementation, including its integrality and
// Cayley--Hamilton checks. This legacy wrapper reverses PolyZ's
// low-first order and accepts only final coefficients representable
// as long long; callers needing larger coefficients should use
// mathlib::charpoly_faddeev_leverrier directly.
inline std::vector<long long> charpoly_int(
    const std::vector<std::vector<long long>>& M) {
    const std::size_t n = M.size();
    if (n == 0) return {1};
    const auto exact = mathlib::charpoly_faddeev_leverrier(M);
    std::vector<long long> cp;
    cp.reserve(n + 1);
    for (std::size_t descending = 0; descending <= n; ++descending) {
        const auto& coefficient = exact.coeff(n - descending);
        char* raw = mpz_get_str(nullptr, 10, coefficient.get());
        if (raw == nullptr) throw std::bad_alloc();
        const std::string decimal(raw);
        std::free(raw);
        std::size_t consumed = 0;
        try {
            const long long value = std::stoll(decimal, &consumed, 10);
            if (consumed != decimal.size())
                throw std::overflow_error(
                    "charpoly_int coefficient conversion");
            cp.push_back(value);
        } catch (const std::out_of_range&) {
            throw std::overflow_error(
                "charpoly_int coefficient exceeds long long; "
                "use mathlib::charpoly_faddeev_leverrier");
        }
    }
    return cp;
}

inline long long polyval_int(const std::vector<long long>& cp, long long x) {
    mathlib::BigInt result(0);
    const mathlib::BigInt argument(x);
    for (auto coefficient : cp) {
        mathlib::BigInt product;
        mathlib::mul(product, result, argument);
        mathlib::BigInt term(coefficient);
        mathlib::add(result, product, term);
    }
    char* raw = mpz_get_str(nullptr, 10, result.get());
    if (raw == nullptr) throw std::bad_alloc();
    const std::string decimal(raw);
    std::free(raw);
    try {
        return std::stoll(decimal);
    } catch (const std::out_of_range&) {
        throw std::overflow_error(
            "polyval_int result exceeds long long");
    }
}

inline bool polyval_int_is_zero(
        const std::vector<long long>& cp, long long x) {
    mathlib::BigInt result(0);
    const mathlib::BigInt argument(x);
    for (auto coefficient : cp) {
        mathlib::BigInt product;
        mathlib::mul(product, result, argument);
        mathlib::BigInt term(coefficient);
        mathlib::add(result, product, term);
    }
    return mathlib::is_zero(result);
}

}  // namespace ravel
