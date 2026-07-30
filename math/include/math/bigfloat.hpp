// bigfloat.hpp
//
// BigFloat: an arbitrary-/tunable-precision binary floating-point
// number -- value = mant * 2^exp, where `mant` is a `BigInt` (mini-gmp
// mpz_t) and `exp` is a `long`.  This is literally "a different
// extension of floating point representation": IEEE double hardwires
// a 53-bit mantissa; BigFloat lets the CALLER pick the mantissa's bit
// width per operation (the `prec` parameter below), trading memory
// (a wider mantissa is a bigger heap-allocated mpz_t) for precision.
//
// WHY THIS EXISTS (see math/TOOLS.md's "Real approximation" work list,
// "MPFR-style high-precision float -- for tight numerical bounds"):
// this project already has two Perron-eigenvalue certification paths
// with a real gap between them --
//
//   1. `double` power iteration (balanced_pair.hpp::largest_abs_
//      eigenvalue, contact_boundary.hpp's dominant_eigenvalue) -- cheap,
//      ~15-16 significant decimal digits, no certified error bound.
//   2. `Ball`'s exact-rational Collatz-Wielandt bracket (ball.hpp,
//      certify_perron_bracket_exact / compare_perron_roots_exact) --
//      bit-exact, but for an IRRATIONAL Perron root the bracket's
//      rational endpoints can only ever narrow, never collapse to a
//      single point, so it can PROVE strict inequality (bracket
//      separation) but can never PROVE equality this way -- and each
//      iteration's numerator/denominator bit-length grows without
//      bound (see app/gb_bp_involution_general_n.cpp's n=3..5 output:
//      200-plus-digit fractions after 200 iterations), which is
//      expensive and eventually impractical for many iterations.
//
// BigFloat is the deliberate middle ground: round to a FIXED,
// caller-chosen bit-width after every operation (so memory per number
// is bounded and tunable, not unbounded like Ball's growing
// fractions), while still getting far more genuine precision than
// `double` by simply choosing a wider mantissa. It does NOT replace
// Ball's proof-of-inequality role or the exact charpoly-divisibility
// proof of equality already in the codebase -- it is a THIRD option
// for "how sure are we these two numbers are equal", positioned
// between "matches to 1e-4 in double" and "bit-exact proof", letting
// a caller push the numerical agreement to (say) 50, 200, or 1000
// decimal digits cheaply, as much stronger evidence than double
// precision without paying Ball's unbounded-bit-growth cost.
//
// Rounding mode: round-to-nearest, ties away from zero (simpler than
// round-to-even; this project has no requirement for IEEE-754-style
// tie-breaking, and the mathlib style elsewhere states tradeoffs in a
// comment rather than over-engineering -- see CPP_DESIGN_PHILOSOPHY.md
// Sec 2). Every arithmetic op below takes an explicit `prec` (bits)
// parameter -- there is no ambient/global precision state, so a
// caller can freely mix precisions in one computation if that's ever
// useful, and there's never a hidden default to get wrong.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "math/bigint.hpp"
#include "math/ball.hpp"

namespace mathlib {

// Helper: stringify a macro expansion.  Standard double-indirection
// pattern -- needed so the error message below can include the actual
// numeric bound rather than the literal source text of the macro.
// Defined BEFORE the bound so it's already in scope when the error
// message references it.
#ifndef MATHLIB_STRINGIFY
#define MATHLIB_STRINGIFY_(x) #x
#define MATHLIB_STRINGIFY(x) MATHLIB_STRINGIFY_(x)
#endif

// Hard upper bound on BigFloat precision bits, set via macro so it
// can be overridden at compile time (e.g. for testing) and is
// referenced uniformly across all BigFloat APIs.  Above ~4096 the
// per-iteration mantissa allocations, the size of `bigfloat_round`'s
// shift values, and the magnitude of internal Newton / Halley
// iterates all grow without much mathematical gain -- the project's
// actual uses (Perron bracket, cbrt-of-cubic) need at most a few
// hundred bits to settle, and the explicit-instantiation cost of
// allowing 2^16-1 or so would just create paths where a typo'd or
// runaway recursion silently produces gigabyte mantissas.  4096 is
// well above any actual use in the codebase and is enforced both
// at runtime (every operation below calls validate_bigfloat_precision)
// and via a compile-time helper (`ASSERT_PRECISION_LITERAL`) for
// callers that want to catch out-of-range literals at the call site.
#define MATHLIB_BIGFLOAT_MAX_PREC_BITS 4096

// Runtime check; throws std::invalid_argument with a message that
// names the offending value and the allowed range, so a caller that
// accidentally passes a computed-too-large value gets a clear error
// rather than a silent gigabyte allocation.
inline void validate_bigfloat_precision(unsigned prec) {
    if (prec == 0) {
        throw std::invalid_argument(
            "BigFloat precision_bits == 0 is meaningless (no mantissa); "
            "pass a value in [1, " MATHLIB_STRINGIFY(MATHLIB_BIGFLOAT_MAX_PREC_BITS) "].");
    }
    if (prec > MATHLIB_BIGFLOAT_MAX_PREC_BITS) {
        throw std::invalid_argument(
            "BigFloat precision_bits > " MATHLIB_STRINGIFY(MATHLIB_BIGFLOAT_MAX_PREC_BITS) " is outside the project's bound "
            "(defined in math/include/math/bigfloat.hpp as MATHLIB_BIGFLOAT_MAX_PREC_BITS). "
            "Refusing rather than silently allocating a gigabyte mantissa.");
    }
}

// Compile-time check: returns true iff `p` is in the allowed range
// [1, MATHLIB_BIGFLOAT_MAX_PREC_BITS].
constexpr bool precision_in_bounds(unsigned p) {
    return p > 0 && p <= MATHLIB_BIGFLOAT_MAX_PREC_BITS;
}

// Companion macro: plants a `static_assert` next to a literal precision
// argument so the user gets a compile-time error, not a runtime throw,
// for out-of-range literal values.  Place BEFORE the call, e.g.
//
//     ASSERT_PRECISION_LITERAL(256);
//     BigFloat r = bigfloat_sqrt(x, 256);
//
// (For runtime-computed precisions the runtime throw above still
// applies; the macro only fires when the literal is provable at
// compile time.)
#define ASSERT_PRECISION_LITERAL(p) \
    static_assert(::mathlib::precision_in_bounds(p), \
        "BigFloat precision literal out of [1, " MATHLIB_STRINGIFY(MATHLIB_BIGFLOAT_MAX_PREC_BITS) "]")

struct BigFloat {
    BigInt mant;  // value = mant * 2^exp
    long exp = 0;

    BigFloat() : mant(0), exp(0) {}
    explicit BigFloat(long long v) : mant(v), exp(0) {}
    BigFloat(BigInt m, long e) : mant(std::move(m)), exp(e) {}
};

inline bool bigfloat_is_zero(const BigFloat& x) { return is_zero(x.mant); }

// Forward declarations for primitives defined later in the file.
// These let us define BigFloat-derived utilities (e.g. pi via
// BBP) before the inline definitions appear below.
inline BigFloat bigfloat_from_ll(long long n);
inline BigFloat bigfloat_add(const BigFloat& a, const BigFloat& b, unsigned prec);
inline BigFloat bigfloat_sub(const BigFloat& a, const BigFloat& b, unsigned prec);
inline BigFloat bigfloat_mul(const BigFloat& a, const BigFloat& b, unsigned prec);
inline BigFloat bigfloat_div(const BigFloat& a, const BigFloat& b, unsigned prec);
inline BigFloat bigfloat_neg(const BigFloat& x);

// π at the requested BigFloat precision via the BBP formula.
//   π = Σ_{k=0}^∞ (1/16^k) · (4/(8k+1) − 2/(8k+4) − 1/(8k+5) − 1/(8k+6))
// 60 iterations give ~73 digits of π (well past 200-bit).
inline BigFloat bigfloat_pi_runtime(unsigned prec = 200) {
    BigFloat pi = bigfloat_from_ll(0);
    BigFloat sixteen = bigfloat_from_ll(16);
    BigFloat eight = bigfloat_from_ll(8);
    BigFloat one = bigfloat_from_ll(1);
    BigFloat four = bigfloat_from_ll(4);
    BigFloat two = bigfloat_from_ll(2);
    BigFloat five = bigfloat_from_ll(5);
    BigFloat six_ = bigfloat_from_ll(6);
    BigFloat inv16_pow = bigfloat_from_ll(1);
    for (unsigned k = 0; k < 60; ++k) {
        BigFloat k_big = bigfloat_from_ll(static_cast<long long>(k));
        BigFloat eight_k = bigfloat_mul(eight, k_big, prec);
        BigFloat t1 = bigfloat_div(four, bigfloat_add(eight_k, one, prec), prec);
        BigFloat t2 = bigfloat_div(two, bigfloat_add(eight_k, four, prec), prec);
        BigFloat t3 = bigfloat_div(one, bigfloat_add(eight_k, five, prec), prec);
        BigFloat t4 = bigfloat_div(one, bigfloat_add(eight_k, six_, prec), prec);
        BigFloat sum = bigfloat_sub(
            bigfloat_sub(t1, t2, prec),
            bigfloat_add(t3, t4, prec),
            prec);
        BigFloat term = bigfloat_mul(sum, inv16_pow, prec);
        pi = bigfloat_add(pi, term, prec);
        inv16_pow = bigfloat_div(inv16_pow, sixteen, prec);
    }
    return pi;
}

// Bit-length of |mant| (0 for a zero value).
inline std::size_t bigfloat_bits(const BigFloat& x) {
    if (is_zero(x.mant)) return 0;
    BigInt mag;
    abs_(mag, x.mant);
    return mpz_sizeinbase(mag.get(), 2);
}

// Round x to at most `prec` significant bits of |mant|, round-to-
// nearest with ties away from zero. A no-op (returns x unchanged) if
// x already fits in `prec` bits or `prec == 0` (0 is treated as "no
// rounding", i.e. keep the value exact -- used internally by
// bigfloat_add_exact/bigfloat_mul_exact below, which are exact by
// construction and only need rounding applied by the CALLER once,
// at the end of a chain of operations, not after every intermediate
// step, if the caller wants to bound cumulative growth differently).
inline BigFloat bigfloat_round(const BigFloat& x, unsigned prec) {
    if (prec == 0 || is_zero(x.mant)) return x;
    validate_bigfloat_precision(prec);
    std::size_t bl = bigfloat_bits(x);
    if (bl <= prec) return x;
    unsigned long shift = static_cast<unsigned long>(bl - prec);

    BigInt mag;
    abs_(mag, x.mant);
    BigInt q;
    mpz_fdiv_q_2exp(q.get(), mag.get(), shift);  // floor(|mant| / 2^shift)
    BigInt back;
    mpz_mul_2exp(back.get(), q.get(), shift);
    BigInt rem;
    sub(rem, mag, back);  // remainder in [0, 2^shift)
    BigInt rem2;
    mpz_mul_2exp(rem2.get(), rem.get(), 1);
    BigInt pow2;
    set_si(pow2, 1);
    mpz_mul_2exp(pow2.get(), pow2.get(), shift);
    if (cmp(rem2, pow2) >= 0) {
        BigInt one(1);
        add(q, q, one);
    }
    if (sgn(x.mant) < 0) neg(q);
    return BigFloat(std::move(q), x.exp + static_cast<long>(shift));
}

// Exact add (no rounding) -- mantissa bit-length can grow by the
// exponent gap between a and b; callers doing many additions should
// round the RESULT to their chosen precision (bigfloat_round) if they
// want bounded memory, same pattern as the exact-then-round div below.
inline BigFloat bigfloat_add_exact(const BigFloat& a, const BigFloat& b) {
    if (is_zero(a.mant)) return b;
    if (is_zero(b.mant)) return a;
    long e = std::min(a.exp, b.exp);
    BigInt am = a.mant, bm = b.mant;
    if (a.exp > e) {
        BigInt t;
        mpz_mul_2exp(t.get(), a.mant.get(), static_cast<unsigned long>(a.exp - e));
        am = std::move(t);
    }
    if (b.exp > e) {
        BigInt t;
        mpz_mul_2exp(t.get(), b.mant.get(), static_cast<unsigned long>(b.exp - e));
        bm = std::move(t);
    }
    BigInt s;
    add(s, am, bm);
    return BigFloat(std::move(s), e);
}

inline BigFloat bigfloat_neg(const BigFloat& x) {
    BigInt m;
    neg(m, x.mant);
    return BigFloat(std::move(m), x.exp);
}

inline BigFloat bigfloat_sub_exact(const BigFloat& a, const BigFloat& b) {
    return bigfloat_add_exact(a, bigfloat_neg(b));
}

// Exact multiply (no rounding needed algebraically -- mant = a.mant *
// b.mant is always exactly the true product -- but its bit-length is
// bigfloat_bits(a) + bigfloat_bits(b), so round the result if chaining
// many multiplies).
inline BigFloat bigfloat_mul_exact(const BigFloat& a, const BigFloat& b) {
    BigInt m;
    mul(m, a.mant, b.mant);
    return BigFloat(std::move(m), a.exp + b.exp);
}

inline BigFloat bigfloat_add(const BigFloat& a, const BigFloat& b, unsigned prec) {
    validate_bigfloat_precision(prec);
    return bigfloat_round(bigfloat_add_exact(a, b), prec);
}
inline BigFloat bigfloat_sub(const BigFloat& a, const BigFloat& b, unsigned prec) {
    validate_bigfloat_precision(prec);
    return bigfloat_round(bigfloat_sub_exact(a, b), prec);
}
inline BigFloat bigfloat_mul(const BigFloat& a, const BigFloat& b, unsigned prec) {
    validate_bigfloat_precision(prec);
    return bigfloat_round(bigfloat_mul_exact(a, b), prec);
}

// Division is generally not exact even for exact inputs, so compute
// directly to `prec` (plus a couple of guard bits) significant bits,
// round-to-nearest, rather than computing "exactly" first (there is
// no exact BigFloat result to compute).
inline BigFloat bigfloat_div(const BigFloat& a, const BigFloat& b, unsigned prec) {
    validate_bigfloat_precision(prec);
    if (is_zero(b.mant)) throw std::invalid_argument("bigfloat_div: division by zero");
    if (is_zero(a.mant)) return BigFloat(BigInt(0), 0);

    int sign = sgn(a.mant) * sgn(b.mant);
    BigInt am, bm;
    abs_(am, a.mant);
    abs_(bm, b.mant);
    long abl = static_cast<long>(mpz_sizeinbase(am.get(), 2));
    long bbl = static_cast<long>(mpz_sizeinbase(bm.get(), 2));
    constexpr long kGuardBits = 4;
    long shift = static_cast<long>(prec) + bbl - abl + kGuardBits;
    if (shift < 0) shift = 0;

    BigInt num;
    mpz_mul_2exp(num.get(), am.get(), static_cast<unsigned long>(shift));
    BigInt q, r;
    mpz_fdiv_qr(q.get(), r.get(), num.get(), bm.get());  // q = floor(num/bm), r in [0,bm)
    BigInt r2;
    mpz_mul_2exp(r2.get(), r.get(), 1);
    if (cmp(r2, bm) >= 0) {
        BigInt one(1);
        add(q, q, one);
    }
    if (sign < 0) neg(q);
    BigFloat res(std::move(q), a.exp - b.exp - shift);
    return bigfloat_round(res, prec);  // trim the guard bits back down to prec
}

// Comparison: exact, regardless of precision (compares the true
// mathematical values, not rounded ones) -- align exponents (same
// technique as add) and compare mantissas.
inline int bigfloat_cmp(const BigFloat& a, const BigFloat& b) {
    long e = std::min(a.exp, b.exp);
    BigInt am = a.mant, bm = b.mant;
    if (a.exp > e) {
        BigInt t;
        mpz_mul_2exp(t.get(), a.mant.get(), static_cast<unsigned long>(a.exp - e));
        am = std::move(t);
    }
    if (b.exp > e) {
        BigInt t;
        mpz_mul_2exp(t.get(), b.mant.get(), static_cast<unsigned long>(b.exp - e));
        bm = std::move(t);
    }
    return cmp(am, bm);
}

inline BigFloat bigfloat_from_bigint(const BigInt& n) { return BigFloat(n, 0); }
inline BigFloat bigfloat_from_ll(long long n) { return BigFloat(n); }

// Best-effort double conversion, for printing/logging only -- NOT
// part of any certified result, just a human-readable summary.
// Reduces the mantissa to a double-safe bit-width FIRST (tracking the
// extra shift as additional exponent) before calling mpz_get_d, then
// applies the exponent via std::ldexp. This matters at high
// precision_bits: converting a several-hundred-bit mantissa to double
// directly (via mpz_get_d) overflows to +-inf before the (correspondingly
// tiny) 2^exp scaling is ever applied, which silently produces NaN
// (inf * 0) instead of the actual, perfectly double-representable
// value. ldexp scales in one step with no such intermediate overflow.
inline double bigfloat_to_double(const BigFloat& x) {
    if (is_zero(x.mant)) return 0.0;
    BigInt mag;
    abs_(mag, x.mant);
    std::size_t bl = mpz_sizeinbase(mag.get(), 2);
    long extra_shift = 0;
    if (bl > 64) {
        extra_shift = static_cast<long>(bl - 64);
        BigInt reduced;
        mpz_tdiv_q_2exp(reduced.get(), mag.get(), static_cast<unsigned long>(extra_shift));
        mag = std::move(reduced);
    }
    double m = mpz_get_d(mag.get());
    if (sgn(x.mant) < 0) m = -m;
    return std::ldexp(m, static_cast<int>(x.exp + extra_shift));
}

// Decimal string, for display -- NOT a certified rounding of the
// value to decimal, just something readable in logs (converts via
// Rat, which mini-gmp already renders correctly). Expensive; only
// call this for a final printed result, not in a hot loop.
inline std::string bigfloat_str(const BigFloat& x) {
    Rat r;
    set_z(r, x.mant);
    if (x.exp >= 0) {
        Rat scale;
        BigInt one(1);
        BigInt p = one;
        for (long i = 0; i < x.exp; ++i) {
            BigInt t;
            mul_si(t, p, 2);
            p = std::move(t);
        }
        set_z(scale, p);
        mul(r, r, scale);
    } else {
        BigInt one(1);
        BigInt p = one;
        for (long i = 0; i < -x.exp; ++i) {
            BigInt t;
            mul_si(t, p, 2);
            p = std::move(t);
        }
        Rat denom;
        set_z(denom, p);
        div(r, r, denom);
    }
    return str(r);
}

// ===================================================================
// bigfloat_abs: |x|.  Sign is in the mantissa only (canonical form).
// ===================================================================
inline BigFloat bigfloat_abs(const BigFloat& x) {
    if (sgn(x.mant) >= 0) return x;
    BigFloat r = x;
    BigInt m;
    abs_(m, x.mant);
    r.mant = std::move(m);
    return r;
}

// ===================================================================
// bigfloat_sqrt: Newton's method for sqrt(x), at given precision.
//
// Convergence is quadratic: each iteration approximately doubles the
// number of correct mantissa bits, so 6-8 iterations suffice for any
// reasonable precision_bits.  Stops when consecutive iterates agree to
// prec bits, returning the rounded result via bigfloat_round.  Throws
// on negative input (matches BigFloat's "no silently-bad values" style,
// per CPP_DESIGN_PHILOSOPHY.md's overall exact-arithmetic discipline).
//
// Initial guess: y_0 = 2^ceil(exp/2), a 2-adic approximation that
// ensures Newton enters the right order-of-magnitude basin from step 1.
// ===================================================================
inline BigFloat bigfloat_sqrt(const BigFloat& x, unsigned prec) {
    validate_bigfloat_precision(prec);
    if (is_zero(x.mant)) return BigFloat(BigInt(0), 0);
    if (sgn(x.mant) < 0) {
        throw std::invalid_argument("bigfloat_sqrt: negative argument");
    }
    // Initial guess: y_0 such that y_0^2 is in the right magnitude ballpark.
    long half_exp = (x.exp >= 0) ? (x.exp + 1) / 2 : -( -x.exp / 2 );
    if (x.exp <= 0 && (x.exp % 2 != 0)) half_exp = -((-x.exp + 1) / 2);
    BigFloat y(BigInt(1), half_exp);
    // Convergence threshold: 2^{-prec}  (mantissa bits small enough means
    // we've reached the requested precision; the prec is in [1, 4096]
    // per the runtime check above, so -(long long)prec is at worst -4096,
    // comfortably inside `long`).
    BigFloat threshold(BigInt(1), -(long long)prec);
    for (int it = 0; it < 100; ++it) {
        BigFloat xy = bigfloat_div(x, y, prec);
        BigFloat s = bigfloat_add(y, xy, prec);
        BigFloat y_new = bigfloat_round(BigFloat(s.mant, s.exp - 1), prec);
        BigFloat diff = bigfloat_sub(y_new, y, prec);
        y = y_new;
        if (is_zero(diff.mant)) break;
        if (bigfloat_cmp(bigfloat_abs(diff), threshold) <= 0) break;
    }
    return bigfloat_round(y, prec);
}

// ===================================================================
// bigfloat_cbrt: Newton's method for cbrt(x).
//
// Convergence is cubic: each iteration ~1.585x bits of precision, so
// iter 4-5 times suffices for prec_bits up to ~256, more for huge
// prec.  Throws on anything weird (only zero is degenerate here, since
// the real cube root of any real is well-defined; negative inputs are
// perfectly fine and return negative roots).
//
// Iteration formula:
//   y_{k+1} = (2 y_k^3 + x) / (3 y_k^2)
// (derivation: y_{k+1} = y_k - (y_k^3 - x) / (3 y_k^2)).
// ===================================================================
inline BigFloat bigfloat_cbrt(const BigFloat& x, unsigned prec) {
    validate_bigfloat_precision(prec);
    if (is_zero(x.mant)) return BigFloat(BigInt(0), 0);
    long sx = sgn(x.mant);
    // Initial guess: same sign as x, with magnitude 2^(floor(exp/3)).
    // This gets the order of magnitude right; Newton's iteration
    // corrects the remaining ~3 bits per step.
    long cbrt_exp = (x.exp >= 0) ? (x.exp / 3) : -((-x.exp + 2) / 3);
    BigInt y_mant(1);
    if (sx < 0) {
        // Negate: y_mant = -1.
        BigInt neg_one(-1);
        y_mant = std::move(neg_one);
    }
    BigFloat y(std::move(y_mant), cbrt_exp);
    BigFloat threshold(BigInt(1), -(long long)prec);
    BigFloat two(2), three(3);
    for (int it = 0; it < 100; ++it) {
        BigFloat y2 = bigfloat_mul(y, y, prec);
        BigFloat y3 = bigfloat_mul(y2, y, prec);
        BigFloat two_y3 = bigfloat_mul(y3, two, prec);
        BigFloat num = bigfloat_add(two_y3, x, prec);
        BigFloat denom = bigfloat_mul(y2, three, prec);
        BigFloat y_new = bigfloat_div(num, denom, prec);
        BigFloat diff = bigfloat_sub(y_new, y, prec);
        y = y_new;
        if (is_zero(diff.mant)) break;
        if (bigfloat_cmp(bigfloat_abs(diff), threshold) <= 0) break;
    }
    return bigfloat_round(y, prec);
}

// ===================================================================
// Tunable-precision Perron-eigenvalue estimate (the switchable
// alternative to Ball's exact-rational Collatz-Wielandt bracket).
// ===================================================================
//
// Same Collatz-Wielandt iteration as certify_perron_bracket_exact
// (ball.hpp): v <- M v starting from (1,...,1), bracket = [min_i
// (Mv)_i/v_i, max_i (Mv)_i/v_i]. The difference is entirely in HOW
// the vector and the bracket are represented: BigFloat components
// rounded to `precision_bits` after every step, instead of exact
// BigInt (unbounded growth) or double (fixed 53 bits). This makes
// the per-iteration cost and memory footprint bounded and tunable --
// higher precision_bits gives a tighter/more-trustworthy bracket at
// higher memory cost per number, exactly the caller-facing knob
// requested.
//
// IMPORTANT HONESTY NOTE: unlike Ball's certify_perron_bracket_exact,
// this is NOT a rigorous enclosure -- rounding v's components at each
// step introduces a genuine (bounded but not tracked) error, so the
// returned bracket is a very strong NUMERICAL estimate at the chosen
// precision, not a proof. Use Ball/compare_perron_roots_exact when a
// bit-exact proof of inequality is needed; use this when "agrees to
// N decimal digits, cheaply, for much larger N than double offers" is
// the actual question (e.g. distinguishing "these look equal at
// double precision" from "these still look equal after 60 more
// digits", which is meaningfully stronger evidence even without being
// a proof).
struct BigFloatBracket {
    BigFloat lo;
    BigFloat hi;
    int iterations_run = 0;
};

inline BigFloatBracket certify_perron_bracket_bigfloat(
    const std::vector<std::vector<long long>>& M,
    unsigned precision_bits,
    int max_iters = 200) {
    const std::size_t n = M.size();
    if (n == 0) throw std::invalid_argument("certify_perron_bracket_bigfloat: empty matrix");
    validate_bigfloat_precision(precision_bits);
    std::vector<BigFloat> v(n, BigFloat(1));

    BigFloatBracket out;
    const int limit = std::max(1, max_iters);
    for (int it = 0; it < limit; ++it) {
        std::vector<BigFloat> mv(n, BigFloat(0));
        for (std::size_t i = 0; i < n; ++i) {
            if (M[i].size() != n) {
                throw std::invalid_argument("certify_perron_bracket_bigfloat: matrix is not square");
            }
            BigFloat acc(0);
            for (std::size_t j = 0; j < n; ++j) {
                if (M[i][j] == 0) continue;
                if (M[i][j] < 0) {
                    throw std::invalid_argument(
                        "certify_perron_bracket_bigfloat: matrix is not nonnegative");
                }
                BigFloat term = bigfloat_mul_exact(BigFloat(M[i][j]), v[j]);
                acc = bigfloat_add_exact(acc, term);
            }
            mv[i] = bigfloat_round(acc, precision_bits);
            if (bigfloat_is_zero(mv[i]) || sgn(v[i].mant) <= 0) {
                throw std::invalid_argument(
                    "certify_perron_bracket_bigfloat: v and Mv must stay strictly positive "
                    "(matrix likely not primitive from an all-ones start, or precision_bits "
                    "too low to keep a very small component from rounding to zero)");
            }
        }
        bool first = true;
        BigFloat lo, hi;
        for (std::size_t i = 0; i < n; ++i) {
            BigFloat ratio = bigfloat_div(mv[i], v[i], precision_bits);
            if (first) {
                lo = ratio;
                hi = ratio;
                first = false;
            } else {
                if (bigfloat_cmp(ratio, lo) < 0) lo = ratio;
                if (bigfloat_cmp(ratio, hi) > 0) hi = ratio;
            }
        }
        out.lo = lo;
        out.hi = hi;
        out.iterations_run = it + 1;
        v = std::move(mv);
    }
    return out;
}

// ===================================================================
// The switch: pick a certification method at the call site.
// ===================================================================

enum class PerronCertifyMethod {
    ExactRationalBracket,  // Ball's certify_perron_bracket_exact / compare_perron_roots_exact:
                           // bit-exact, unbounded per-iteration bit growth, can PROVE
                           // inequality but never equality for an irrational shared root.
    TunedBigFloat,         // this file's certify_perron_bracket_bigfloat: bounded, tunable
                           // memory per number via `precision_bits`, gives a very strong
                           // (but not rigorous) numerical estimate at whatever precision
                           // the caller asks for.
};

struct PerronCertifyResult {
    PerronCertifyMethod method;
    // Populated for ExactRationalBracket:
    Ball exact_bracket{};
    // Populated for TunedBigFloat:
    BigFloat tuned_lo, tuned_hi;
    unsigned precision_bits_used = 0;
    int iterations_run = 0;
};

// One unified entry point exercising whichever method the caller
// selects, so call sites (and the app-level driver) can flip the
// switch without duplicating the surrounding plumbing.
inline PerronCertifyResult certify_perron(
    const std::vector<std::vector<long long>>& M,
    PerronCertifyMethod method,
    int max_iters = 200,
    unsigned precision_bits = 256) {
    if (method == PerronCertifyMethod::TunedBigFloat) {
        // Cheap-fail: if a caller passes an out-of-range precision_bits
        // through the switch without going through the bracket directly,
        // make the error happen here before allocating the result
        // struct.  (certify_perron_bracket_bigfloat below also calls
        // validate_bigfloat_precision; this is the early-out.)
        validate_bigfloat_precision(precision_bits);
    }
    PerronCertifyResult out;
    out.method = method;
    if (method == PerronCertifyMethod::ExactRationalBracket) {
        auto r = certify_perron_bracket_exact(M, max_iters, 1e-12);
        out.exact_bracket = r.bracket;
        out.iterations_run = r.iterations_run;
    } else {
        auto r = certify_perron_bracket_bigfloat(M, precision_bits, max_iters);
        out.tuned_lo = r.lo;
        out.tuned_hi = r.hi;
        out.precision_bits_used = precision_bits;
        out.iterations_run = r.iterations_run;
    }
    return out;
}

}  // namespace mathlib
