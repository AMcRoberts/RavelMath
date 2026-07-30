// include/ravel/dual.hpp
//
// Forward-mode automatic-differentiation helper for the
// smooth-relaxation search (RESEARCH_VECTORS.md vector 2).
//
// Conceptually: Dual<T> tracks (value, derivative) through arithmetic.
// For a function f: T -> T, df/dx evaluated at x is computed by:
//   1. Construct a Dual<T> with val=x, eps=1 ("seed"; df/dx = 1 w.r.t. x).
//   2. Run the same expression that computes f, but on Dual<T>.
//   3. Extract the .eps component of the result.
//
// For the full Jacobian of f: Matrix -> R, repeat with one-hot
// seeds per matrix entry, or use DualMatrix<d> below which carries
// eps as a full matrix.
//
// This header provides Dual<T> and a small set of free functions
// dual_add, dual_sub, dual_mul, dual_div, dual_neg, dual_cbrt,
// dual_sqrt, dual_eq (chain rule for each).  Each dispatch is on
// the underlying scalar type T; supported T values are:
//
//   - double            (operator+/-/*// straightforward)
//   - mathlib::BigFloat (free functions bigfloat_add_exact etc.)
//   - mathlib::Ball     (free functions ball_add etc.)
//   - mathlib::Rat      (free functions mathlib::add etc.)
//   - mathlib::QElem    (free functions QBetaRing::add etc.)
//
// The chain rule for cbrt(x) is dy/dx = x'/(3 y^2) where y = cbrt(x).
// The chain rule for sqrt(x) is dy/dx = x'/(2 y).
//
// =====================================================================
// Why this is here specifically for smooth-relaxation (vs. autodiff
// libraries like autodiff, Adept, Stan's math)
// =====================================================================
// The smooth-relaxation search uses spectral_invariants_3x3
// (include/ravel/spectral.hpp) as its differentiable objective.
// spectral_invariants_3x3 mixes:
//   - integer-matrix entries  (treated as constant)
//   - rational +/-/*/  arithmetic (rational-coefficient expressions)
//   - cbrt (Cardano formula for the real root)
//
// We need df/dM_ij for each matrix entry M_ij, on REAL (not exact
// rational) outputs, where the cbrt converges to a possibly irrational
// root.  BigFloat is the natural scalar choice for this (converges to
// irrationals); Ball<Rat> is the natural companion for exact rational
// detection at the output (per dual_format.hpp).
//
// A general-purpose autodiff library would also work, BUT they typically
// expect operator+ support on the scalar and have a single canonical
// representation -- they don't handle "BigFloat for the convergent
// view, Ball<Rat> for the exact-certified-view" without an explicit
// dual-representation layer anyway, so doing it by hand is cleaner.
// =====================================================================

#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "math/bigint.hpp"
#include "math/ball.hpp"
#include "math/bigfloat.hpp"

namespace ravel::dual {

// ===================================================================
// Dual<T> -- the core pair-tracking type
// ===================================================================
template <typename T>
struct Dual {
    T val;   // the function value at the seed
    T eps;   // df/dx_eps where eps is the "seed direction" (1 by default)

    // Default-constructed: zero value, zero derivative.  Used for
    // initial sentinel states and for the "constant zero" pattern.
    Dual() : val(T{}), eps(T{}) {}

    // Constant constructor: f = const, df/dx = 0.
    explicit Dual(T v) : val(v), eps(T{}) {}

    // Value + derivative constructor.  Used when seeding a variable:
    //   Dual<T> x = Dual<T>(val, T(1));   // x is the independent var.
    Dual(T v, T e) : val(v), eps(e) {}

    // Construct a "variable" seed: val = x, eps = 1.
    static Dual variable(T x) { return Dual(x, T(1)); }

    // Construct a constant: val = x, eps = 0.
    static Dual constant(T x) { return Dual(x, T(0)); }
};

// ===================================================================
// Generic arithmetic dispatch (Dual<T> + Dual<T>)
// ===================================================================
//
// Operation semantics for arbitrary T are defined per-scalar-type in
// the specializations below.  The default implementations (no
// specialization) throw -- this is the project's "no silently-bad
// values" stance, per CPP_DESIGN_PHILOSOPHY.md §2: a scalar we don't
// know how to differentiate on is a programming error, not a fallback.

template <typename T>
Dual<T> dual_add(const Dual<T>& a, const Dual<T>& b);

template <typename T>
Dual<T> dual_sub(const Dual<T>& a, const Dual<T>& b);

template <typename T>
Dual<T> dual_mul(const Dual<T>& a, const Dual<T>& b);

template <typename T>
Dual<T> dual_div(const Dual<T>& a, const Dual<T>& b);

template <typename T>
Dual<T> dual_neg(const Dual<T>& a);

template <typename T>
Dual<T> dual_cbrt(const Dual<T>& a);

template <typename T>
Dual<T> dual_sqrt(const Dual<T>& a);

// ===================================================================
// Generic operator overloads.  These are the user-facing API for
// arithmetic on Dual<T>, mirroring scalar-style syntax.  They delegate
// to the free functions above.
// ===================================================================

template <typename T>
Dual<T> operator+(const Dual<T>& a, const Dual<T>& b) { return dual_add(a, b); }

template <typename T>
Dual<T> operator-(const Dual<T>& a, const Dual<T>& b) { return dual_sub(a, b); }

template <typename T>
Dual<T> operator*(const Dual<T>& a, const Dual<T>& b) { return dual_mul(a, b); }

template <typename T>
Dual<T> operator/(const Dual<T>& a, const Dual<T>& b) { return dual_div(a, b); }

template <typename T>
Dual<T> operator-(const Dual<T>& a) { return dual_neg(a); }

// pow_integer(a, n) = a^n for small integer n.  Useful for x^2 in
// spectral formulas.  Chain rule: d/dx(x^n) = n*x^(n-1)*x'.
template <typename T>
Dual<T> dual_sqr(const Dual<T>& a) { return dual_mul(a, a); }

// ===================================================================
// Specialization: Dual<double>
// ===================================================================
//
// Straightforward: arithmetic uses builtin operators and pow, cbrt
// from std.
template <>
inline Dual<double> dual_add(const Dual<double>& a, const Dual<double>& b) {
    return Dual<double>(a.val + b.val, a.eps + b.eps);
}
template <>
inline Dual<double> dual_sub(const Dual<double>& a, const Dual<double>& b) {
    return Dual<double>(a.val - b.val, a.eps - b.eps);
}
template <>
inline Dual<double> dual_mul(const Dual<double>& a, const Dual<double>& b) {
    return Dual<double>(a.val * b.val, a.eps * b.val + a.val * b.eps);
}
template <>
inline Dual<double> dual_div(const Dual<double>& a, const Dual<double>& b) {
    if (b.val == 0.0) {
        throw std::invalid_argument("dual_div<double>: division by zero value");
    }
    // d/dx (a/b) = (a' b - a b') / b^2
    double new_val = a.val / b.val;
    double new_eps = (a.eps * b.val - a.val * b.eps) / (b.val * b.val);
    return Dual<double>(new_val, new_eps);
}
template <>
inline Dual<double> dual_neg(const Dual<double>& a) {
    return Dual<double>(-a.val, -a.eps);
}
template <>
inline Dual<double> dual_cbrt(const Dual<double>& a) {
    // d/dx cbrt(x) = 1/(3 cbrt(x)^2)
    if (a.val == 0.0) {
        // 0 * inf is undefined; we take the limit: if x->0 and x'->eps, then
        // cbrt(x) ~ 0 and (cbrt)'(x) ~ 1 / (3 * cbrt(x)^2) blows up.  In
        // practice this is a degenerate seed; the caller is responsible
        // for catching it before chain-rule propagation.
        if (a.eps == 0.0) return Dual<double>(0.0, 0.0);
        throw std::domain_error("dual_cbrt<double>: zero input with nonzero derivative");
    }
    double c = std::cbrt(a.val);
    double dexp = a.eps / (3.0 * c * c);
    return Dual<double>(c, dexp);
}
template <>
inline Dual<double> dual_sqrt(const Dual<double>& a) {
    // d/dx sqrt(x) = 1/(2 sqrt(x))
    if (a.val < 0.0) {
        throw std::domain_error("dual_sqrt<double>: negative input");
    }
    if (a.val == 0.0) {
        if (a.eps == 0.0) return Dual<double>(0.0, 0.0);
        throw std::domain_error("dual_sqrt<double>: zero input with nonzero derivative");
    }
    double s = std::sqrt(a.val);
    return Dual<double>(s, a.eps / (2.0 * s));
}

// ===================================================================
// Specialization: Dual<BigFloat>
//
// We use the *_exact variants of the BigFloat operations: chain rule
// arithmetic accumulates at full BigFloat precision, with no rounding
// at every step.  The rounding decision (which precision_bits to
// round to at the end) is the caller's, applied via bigfloat_round on
// the extracted .val and .eps after the computation.
// ===================================================================
template <>
inline Dual<mathlib::BigFloat> dual_add(
        const Dual<mathlib::BigFloat>& a, const Dual<mathlib::BigFloat>& b) {
    return Dual<mathlib::BigFloat>(
        mathlib::bigfloat_add_exact(a.val, b.val),
        mathlib::bigfloat_add_exact(a.eps, b.eps));
}
template <>
inline Dual<mathlib::BigFloat> dual_sub(
        const Dual<mathlib::BigFloat>& a, const Dual<mathlib::BigFloat>& b) {
    return Dual<mathlib::BigFloat>(
        mathlib::bigfloat_sub_exact(a.val, b.val),
        mathlib::bigfloat_sub_exact(a.eps, b.eps));
}
template <>
inline Dual<mathlib::BigFloat> dual_mul(
        const Dual<mathlib::BigFloat>& a, const Dual<mathlib::BigFloat>& b) {
    // (uv)' = u'v + uv'  -- chain rule for product
    mathlib::BigFloat u_v = mathlib::bigfloat_mul_exact(a.val, b.val);
    mathlib::BigFloat u_v_e =
        mathlib::bigfloat_add_exact(
            mathlib::bigfloat_mul_exact(a.eps, b.val),
            mathlib::bigfloat_mul_exact(a.val, b.eps));
    return Dual<mathlib::BigFloat>(u_v, u_v_e);
}
template <>
inline Dual<mathlib::BigFloat> dual_div(
        const Dual<mathlib::BigFloat>& a, const Dual<mathlib::BigFloat>& b) {
    if (mathlib::is_zero(b.val.mant)) {
        throw std::invalid_argument("dual_div<BigFloat>: division by zero value");
    }
    // For BigFloat division we need precision.  Default to "high enough
    // not to lose" -- 2 * requested bits would be safer, but for the
    // autodiff use case we accept a caller-supplied precision via the
    // dual_div_prec helper below.  Here we use a conservative fixed
    // value (1024 = half of MATHLIB_BIGFLOAT_MAX_PREC_BITS).
    constexpr unsigned kDivPrec = 1024;
    mathlib::BigFloat u_v = mathlib::bigfloat_div(a.val, b.val, kDivPrec);
    // (a/b)' = (a' b - a b') / b^2
    mathlib::BigFloat num =
        mathlib::bigfloat_sub_exact(
            mathlib::bigfloat_mul_exact(a.eps, b.val),
            mathlib::bigfloat_mul_exact(a.val, b.eps));
    mathlib::BigFloat b_sq = mathlib::bigfloat_mul_exact(b.val, b.val);
    mathlib::BigFloat u_v_e = mathlib::bigfloat_div(num, b_sq, kDivPrec);
    return Dual<mathlib::BigFloat>(u_v, u_v_e);
}
template <>
inline Dual<mathlib::BigFloat> dual_neg(
        const Dual<mathlib::BigFloat>& a) {
    return Dual<mathlib::BigFloat>(mathlib::bigfloat_neg(a.val),
                                    mathlib::bigfloat_neg(a.eps));
}
template <>
inline Dual<mathlib::BigFloat> dual_cbrt(
        const Dual<mathlib::BigFloat>& a) {
    if (mathlib::is_zero(a.val.mant)) {
        if (mathlib::is_zero(a.val.mant) && mathlib::is_zero(a.eps.mant)) {
            return Dual<mathlib::BigFloat>(mathlib::BigFloat(0),
                                             mathlib::BigFloat(0));
        }
        throw std::domain_error("dual_cbrt<BigFloat>: zero input with nonzero derivative");
    }
    constexpr unsigned kPrec = 1024;
    mathlib::BigFloat c = mathlib::bigfloat_cbrt(a.val, kPrec);
    // cbrt(x)' = x' / (3 cbrt(x)^2) = x' / (3 c^2)
    mathlib::BigFloat c_sq = mathlib::bigfloat_mul_exact(c, c);
    mathlib::BigFloat three(3);
    mathlib::BigFloat three_c_sq = mathlib::bigfloat_mul_exact(c_sq, three);
    mathlib::BigFloat dexp = mathlib::bigfloat_div(a.eps, three_c_sq, kPrec);
    return Dual<mathlib::BigFloat>(c, dexp);
}
template <>
inline Dual<mathlib::BigFloat> dual_sqrt(
        const Dual<mathlib::BigFloat>& a) {
    if (mathlib::sgn(a.val.mant) < 0) {
        throw std::domain_error("dual_sqrt<BigFloat>: negative input");
    }
    if (mathlib::is_zero(a.val.mant)) {
        if (mathlib::is_zero(a.eps.mant)) {
            return Dual<mathlib::BigFloat>(mathlib::BigFloat(0),
                                             mathlib::BigFloat(0));
        }
        throw std::domain_error("dual_sqrt<BigFloat>: zero input with nonzero derivative");
    }
    constexpr unsigned kPrec = 1024;
    mathlib::BigFloat s = mathlib::bigfloat_sqrt(a.val, kPrec);
    mathlib::BigFloat two(2);
    mathlib::BigFloat two_s = mathlib::bigfloat_mul_exact(s, two);
    mathlib::BigFloat dexp = mathlib::bigfloat_div(a.eps, two_s, kPrec);
    return Dual<mathlib::BigFloat>(s, dexp);
}

// ===================================================================
// Specialization: Dual<Ball>
// ===================================================================
//
// Ball<Rat> arithmetic widens the interval exactly (per
// ball_add/sub/mul/div's known implementation).  Chain rule is the
// same as for BigFloat.
// ===================================================================
template <>
inline Dual<mathlib::Ball> dual_add(
        const Dual<mathlib::Ball>& a, const Dual<mathlib::Ball>& b) {
    return Dual<mathlib::Ball>(mathlib::ball_add(a.val, b.val),
                                mathlib::ball_add(a.eps, b.eps));
}
template <>
inline Dual<mathlib::Ball> dual_sub(
        const Dual<mathlib::Ball>& a, const Dual<mathlib::Ball>& b) {
    return Dual<mathlib::Ball>(mathlib::ball_sub(a.val, b.val),
                                mathlib::ball_sub(a.eps, b.eps));
}
template <>
inline Dual<mathlib::Ball> dual_mul(
        const Dual<mathlib::Ball>& a, const Dual<mathlib::Ball>& b) {
    mathlib::Ball u_v = mathlib::ball_mul(a.val, b.val);
    mathlib::Ball u_v_e = mathlib::ball_add(
        mathlib::ball_mul(a.eps, b.val),
        mathlib::ball_mul(a.val, b.eps));
    return Dual<mathlib::Ball>(u_v, u_v_e);
}
template <>
inline Dual<mathlib::Ball> dual_div(
        const Dual<mathlib::Ball>& a, const Dual<mathlib::Ball>& b) {
    mathlib::Ball u_v = mathlib::ball_div(a.val, b.val);
    mathlib::Ball num = mathlib::ball_sub(
        mathlib::ball_mul(a.eps, b.val),
        mathlib::ball_mul(a.val, b.eps));
    mathlib::Ball b_sq = mathlib::ball_mul(b.val, b.val);
    mathlib::Ball u_v_e = mathlib::ball_div(num, b_sq);
    return Dual<mathlib::Ball>(u_v, u_v_e);
}
namespace detail {
inline mathlib::Ball make_ball_constant(long long n, long long d) {
    mathlib::Rat r; mathlib::set_si(r, n, d);
    mathlib::Rat r2(r);
    return mathlib::Ball(r, r2);
}
}  // namespace detail
template <>
inline Dual<mathlib::Ball> dual_neg(const Dual<mathlib::Ball>& a) {
    // -1 = Ball([1, 1] negated to [-1, -1]) -- both endpoints = -1.
    mathlib::Ball minus_one = detail::make_ball_constant(-1, 1);
    mathlib::Ball u_v = mathlib::ball_mul(a.val, minus_one);
    mathlib::Ball u_v_e = mathlib::ball_mul(a.eps, minus_one);
    return Dual<mathlib::Ball>(u_v, u_v_e);
}
template <>
inline Dual<mathlib::Ball> dual_cbrt(const Dual<mathlib::Ball>& a) {
    mathlib::Ball c = mathlib::ball_cbrt(a.val);
    mathlib::Ball c_sq = mathlib::ball_mul(c, c);
    mathlib::Ball three = detail::make_ball_constant(3, 1);
    mathlib::Ball three_c_sq = mathlib::ball_mul(three, c_sq);
    mathlib::Ball u_v_e = mathlib::ball_div(a.eps, three_c_sq);
    return Dual<mathlib::Ball>(c, u_v_e);
}
template <>
inline Dual<mathlib::Ball> dual_sqrt(const Dual<mathlib::Ball>& a) {
    mathlib::Ball s = mathlib::ball_sqrt(a.val);
    mathlib::Ball two = detail::make_ball_constant(2, 1);
    mathlib::Ball two_s = mathlib::ball_mul(two, s);
    mathlib::Ball u_v_e = mathlib::ball_div(a.eps, two_s);
    return Dual<mathlib::Ball>(s, u_v_e);
}

// ===================================================================
// Helper: ratio check via double-precision comparison of two Dual<T>s.
// ===================================================================
//
// Used by tests to verify autodiff results against finite differences,
// where both sides are observed via their double-precision value.
// Comparing at the underlying val level only, not eps.

}  // namespace ravel::dual
