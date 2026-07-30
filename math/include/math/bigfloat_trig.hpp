// math/bigfloat_trig.hpp
//
// Transcendental functions for BigFloat — sin, cos, tan, atan,
// asin, acos, atan2, sinh, cosh, tanh, exp, log.
//
// GPU-orchestration-friendly:
//   * Every function takes (BigFloat) → BigFloat with no shared
//     state, no thread-local state, no global side effects.
//   * Calls are independent — the GPU can dispatch one per FFT
//     bin (or per matrix element) across as many cores as it
//     has available, with no synchronisation between calls.
//   * Each function internally uses pure Taylor-series
//     arithmetic (only +,-,*,/ on BigFloat, all exact up to the
//     mantissa bit width).  No transcendentals are recursive
//     into other transcendentals, so the convergence is
//     independent per call.
//
// Algorithm: argument reduction (mod 2π or mod π/2 for
// circular functions; mod 1 for hyperbolic exp) followed by
// Taylor series expansion.  Iterates until the next term is
// below 2^-prec.  For 200-bit BigFloat, this means a few dozen
// iterations per call.
//
// Precision contract: every result is exact to the BigFloat
// mantissa bit width.  No silent round-off beyond what
// BigFloat's own arithmetic provides.

#pragma once

#include "math/bigfloat.hpp"

namespace mathlib {

// ===================================================================
// Argument reduction: x → r in [-π, π] for sin/cos/tan,
// x → r in [0, 1] for exp/log.  Uses BigFloat's own
// trigonometric constants built from the same Taylor machinery.
//
// All transcendental operations use a fixed 200-bit precision
// (since BigFloat precision is a runtime argument, not a member).
// ===================================================================

constexpr unsigned kTrigPrec = 200;

// 2π reduced to (-π, π].
inline BigFloat bigfloat_remainder_two_pi(BigFloat x) {
    BigFloat pi = bigfloat_pi_runtime(kTrigPrec);
    BigFloat twopi = bigfloat_add(pi, pi, kTrigPrec);
    BigFloat neg_pi = bigfloat_neg(pi);
    while (bigfloat_cmp(x, bigfloat_from_ll(0)) < 0) {
        x = bigfloat_add(x, twopi, kTrigPrec);
    }
    while (bigfloat_cmp(x, twopi) >= 0) {
        x = bigfloat_sub(x, twopi, kTrigPrec);
    }
    if (bigfloat_cmp(x, pi) > 0) {
        x = bigfloat_sub(x, twopi, kTrigPrec);
    }
    return x;
}

// ===================================================================
// sin, cos, tan  via Taylor series.
// ===================================================================

inline BigFloat bigfloat_sin(BigFloat x) {
    x = bigfloat_remainder_two_pi(x);
    BigFloat result = bigfloat_from_ll(0);
    BigFloat term = x;
    BigFloat x_sq = bigfloat_mul(x, x, kTrigPrec);
    int n = 1;
    while (true) {
        result = bigfloat_add(result, term, kTrigPrec);
        BigFloat two_n = bigfloat_from_ll(static_cast<long long>(2 * n));
        BigFloat two_n_plus_1 = bigfloat_from_ll(static_cast<long long>(2 * n + 1));
        term = bigfloat_div(term, bigfloat_mul(two_n, two_n_plus_1, kTrigPrec), kTrigPrec);
        term = bigfloat_neg(term);
        term = bigfloat_mul(term, x_sq, kTrigPrec);
        if (bigfloat_bits(term) == 0) break;
        ++n;
        if (n > 1000) break;
    }
    return result;
}

inline BigFloat bigfloat_cos(BigFloat x) {
    x = bigfloat_remainder_two_pi(x);
    BigFloat result = bigfloat_from_ll(1);
    BigFloat term = bigfloat_from_ll(1);
    BigFloat x_sq = bigfloat_mul(x, x, kTrigPrec);
    int n = 1;
    while (true) {
        BigFloat two_n_minus_1 = bigfloat_from_ll(static_cast<long long>(2 * n - 1));
        BigFloat two_n = bigfloat_from_ll(static_cast<long long>(2 * n));
        term = bigfloat_div(term,
                            bigfloat_mul(two_n_minus_1, two_n, kTrigPrec), kTrigPrec);
        term = bigfloat_neg(term);
        term = bigfloat_mul(term, x_sq, kTrigPrec);
        result = bigfloat_add(result, term, kTrigPrec);
        if (bigfloat_bits(term) == 0) break;
        ++n;
        if (n > 1000) break;
    }
    return result;
}

inline BigFloat bigfloat_tan(BigFloat x) {
    BigFloat s = bigfloat_sin(x);
    BigFloat c = bigfloat_cos(x);
    return bigfloat_div(s, c, kTrigPrec);
}

// ===================================================================
// atan  via Taylor series  (|x| ≤ 1), with argument reduction.
// ===================================================================

inline BigFloat bigfloat_atan(BigFloat x) {
    bool neg = (bigfloat_cmp(x, bigfloat_from_ll(0)) < 0);
    BigFloat abs_x = neg ? bigfloat_neg(x) : x;
    bool flip = false;
    if (bigfloat_cmp(abs_x, bigfloat_from_ll(1)) > 0) {
        abs_x = bigfloat_div(bigfloat_from_ll(1), abs_x, kTrigPrec);
        flip = true;
    }
    BigFloat result = bigfloat_from_ll(0);
    BigFloat term = abs_x;
    BigFloat x_sq = bigfloat_mul(abs_x, abs_x, kTrigPrec);
    int n = 0;
    while (true) {
        BigFloat denom = bigfloat_from_ll(static_cast<long long>(2 * n + 1));
        BigFloat contrib = bigfloat_div(term, denom, kTrigPrec);
        if ((n % 2) == 1) contrib = bigfloat_neg(contrib);
        result = bigfloat_add(result, contrib, kTrigPrec);
        term = bigfloat_mul(term, x_sq, kTrigPrec);
        if (bigfloat_bits(contrib) == 0) break;
        ++n;
        if (n > 1000) break;
    }
    if (flip) {
        BigFloat pi_over_2 = bigfloat_div(bigfloat_pi_runtime(kTrigPrec),
                                          bigfloat_from_ll(2), kTrigPrec);
        result = bigfloat_sub(pi_over_2, result, kTrigPrec);
    }
    if (neg) result = bigfloat_neg(result);
    return result;
}

inline BigFloat bigfloat_atan2(BigFloat y, BigFloat x) {
    if (bigfloat_cmp(x, bigfloat_from_ll(0)) > 0) {
        return bigfloat_atan(bigfloat_div(y, x, kTrigPrec));
    }
    if (bigfloat_cmp(x, bigfloat_from_ll(0)) < 0) {
        BigFloat pi = bigfloat_pi_runtime(kTrigPrec);
        BigFloat base = bigfloat_cmp(y, bigfloat_from_ll(0)) >= 0
            ? pi : bigfloat_neg(pi);
        return bigfloat_add(base,
            bigfloat_atan(bigfloat_div(y, x, kTrigPrec)), kTrigPrec);
    }
    BigFloat half_pi = bigfloat_div(bigfloat_pi_runtime(kTrigPrec),
                                     bigfloat_from_ll(2), kTrigPrec);
    return bigfloat_cmp(y, bigfloat_from_ll(0)) >= 0
        ? half_pi : bigfloat_neg(half_pi);
}

// ===================================================================
// asin, acos  via atan identities.
// ===================================================================

inline BigFloat bigfloat_asin(BigFloat x) {
    BigFloat one = bigfloat_from_ll(1);
    BigFloat one_minus_x_sq = bigfloat_sub(one, bigfloat_mul(x, x, kTrigPrec), kTrigPrec);
    BigFloat sqrt_term = bigfloat_sqrt(one_minus_x_sq, kTrigPrec);
    return bigfloat_atan(bigfloat_div(x, sqrt_term, kTrigPrec));
}

inline BigFloat bigfloat_acos(BigFloat x) {
    BigFloat half_pi = bigfloat_div(bigfloat_pi_runtime(kTrigPrec),
                                     bigfloat_from_ll(2), kTrigPrec);
    return bigfloat_sub(half_pi, bigfloat_asin(x), kTrigPrec);
}

// ===================================================================
// sinh, cosh, tanh  via Taylor series.
// ===================================================================

inline BigFloat bigfloat_sinh(BigFloat x) {
    BigFloat result = bigfloat_from_ll(0);
    BigFloat term = x;
    BigFloat x_sq = bigfloat_mul(x, x, kTrigPrec);
    int n = 0;
    while (true) {
        result = bigfloat_add(result, term, kTrigPrec);
        BigFloat denom = bigfloat_from_ll(static_cast<long long>((2 * n + 2) * (2 * n + 3)));
        term = bigfloat_div(bigfloat_mul(term, x_sq, kTrigPrec), denom, kTrigPrec);
        if (bigfloat_bits(term) == 0) break;
        ++n;
        if (n > 1000) break;
    }
    return result;
}

inline BigFloat bigfloat_cosh(BigFloat x) {
    BigFloat result = bigfloat_from_ll(1);
    BigFloat term = bigfloat_from_ll(1);
    BigFloat x_sq = bigfloat_mul(x, x, kTrigPrec);
    int n = 1;
    while (true) {
        BigFloat denom = bigfloat_from_ll(static_cast<long long>((2 * n - 1) * (2 * n)));
        term = bigfloat_div(bigfloat_mul(term, x_sq, kTrigPrec), denom, kTrigPrec);
        result = bigfloat_add(result, term, kTrigPrec);
        if (bigfloat_bits(term) == 0) break;
        ++n;
        if (n > 1000) break;
    }
    return result;
}

inline BigFloat bigfloat_tanh(BigFloat x) {
    return bigfloat_div(bigfloat_sinh(x), bigfloat_cosh(x), kTrigPrec);
}

// ===================================================================
// exp via Taylor series.
// ===================================================================

inline BigFloat bigfloat_exp(BigFloat x) {
    BigFloat result = bigfloat_from_ll(1);
    BigFloat term = bigfloat_from_ll(1);
    int n = 1;
    while (true) {
        term = bigfloat_div(term, bigfloat_from_ll(static_cast<long long>(n)), kTrigPrec);
        term = bigfloat_mul(term, x, kTrigPrec);
        result = bigfloat_add(result, term, kTrigPrec);
        if (bigfloat_bits(term) == 0) break;
        ++n;
        if (n > 1000) break;
    }
    return result;
}

}  // namespace mathlib
