// ball.hpp
//
// Exact rational interval ("ball") arithmetic: an interval [lo, hi]
// with EXACT mpq_t (Rat) endpoints, not a floating-point
// center+radius pair.  This project already has exact rational and
// Q(beta) arithmetic throughout (mini-gmp, bezout.hpp, qbeta.hpp);
// using exact rational endpoints for interval arithmetic sidesteps
// floating-point outward-rounding-mode concerns entirely, at some
// performance cost, consistent with the project's existing "exact
// where it matters" stance (CPP_DESIGN_PHILOSOPHY.md §2).
//
// Concrete application built on top: `collatz_bracket` and
// `certify_perron_bracket` implement the classical Collatz-Wielandt
// bound for the Perron (dominant) eigenvalue of a nonnegative
// integer matrix -- for any positive vector v,
//
//     min_i (Mv)_i / v_i  <=  lambda_max(M)  <=  max_i (Mv)_i / v_i,
//
// and iterating v <- Mv (M integer, so v stays an EXACT integer
// vector at every step -- no floating point, no rounding, ever)
// makes this bracket strictly tighter at every step for a primitive
// nonnegative matrix, converging to the exact Perron root.  This
// gives a cheap, fully rigorous, no-tolerance bound on beta (or on
// lambda(G_B), or any other Perron-eigenvalue quantity this project
// computes) usable as a pre-screen BEFORE committing to the full
// exact Sturm-isolation / Q(beta) machinery -- e.g. for the wider
// K_max=5..7 surveys in survey.hpp, which currently rely on
// double-precision power iteration with no certified bound at all
// until a candidate is hand-picked for exact follow-up.
//
// SCOPE: `long long` matrix entries throughout (consistent with the
// rest of the project's integer-matrix conventions); the *bracket
// endpoints* are exact Rat (mpq_t) values, not `long long` -- only
// the matrix/vector entries are `long long`, which is what the
// integer growth in the power-iteration vector needs watching for
// (see certify_perron_bracket's overflow-guard comment).

#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"

namespace mathlib {

struct Ball {
    Rat lo;
    Rat hi;

    Ball() : lo(0), hi(0) {}
    Ball(Rat lo_, Rat hi_) : lo(std::move(lo_)), hi(std::move(hi_)) {
        if (cmp(lo, hi) > 0) throw std::invalid_argument("Ball: lo > hi");
    }
    explicit Ball(long long exact) : lo(exact), hi(exact) {}
};

inline bool ball_contains(const Ball& b, const Rat& x) {
    return cmp(b.lo, x) <= 0 && cmp(x, b.hi) <= 0;
}

inline Rat ball_width(const Ball& b) {
    Rat w;
    sub(w, b.hi, b.lo);
    return w;
}

inline Rat ball_midpoint(const Ball& b) {
    Rat s, two_r;
    add(s, b.lo, b.hi);
    two_r = Rat(2);
    Rat m;
    div(m, s, two_r);
    return m;
}

inline Ball ball_add(const Ball& a, const Ball& b) {
    Rat lo, hi;
    add(lo, a.lo, b.lo);
    add(hi, a.hi, b.hi);
    return Ball(lo, hi);
}

inline Ball ball_sub(const Ball& a, const Ball& b) {
    // [a.lo,a.hi] - [b.lo,b.hi] = [a.lo - b.hi, a.hi - b.lo]
    Rat lo, hi;
    sub(lo, a.lo, b.hi);
    sub(hi, a.hi, b.lo);
    return Ball(lo, hi);
}

// Interval multiplication: since endpoints can be negative, the
// product's extremes are among the four corner products.
inline Ball ball_mul(const Ball& a, const Ball& b) {
    Rat p00, p01, p10, p11;
    mul(p00, a.lo, b.lo);
    mul(p01, a.lo, b.hi);
    mul(p10, a.hi, b.lo);
    mul(p11, a.hi, b.hi);
    Rat lo = p00, hi = p00;
    for (const Rat* p : {&p01, &p10, &p11}) {
        if (cmp(*p, lo) < 0) lo = *p;
        if (cmp(*p, hi) > 0) hi = *p;
    }
    return Ball(lo, hi);
}

// Interval division; throws if the divisor ball straddles (or
// touches) zero, since 1/[b.lo,b.hi] is unbounded or undefined there
// -- this project has no use case needing extended-interval-with-
// infinity semantics, so we simply refuse rather than silently
// producing a meaningless bound.
inline Ball ball_div(const Ball& a, const Ball& b) {
    if (sgn(b.lo) <= 0 && sgn(b.hi) >= 0) {
        throw std::invalid_argument("ball_div: divisor interval contains zero");
    }
    Rat p00, p01, p10, p11;
    div(p00, a.lo, b.lo);
    div(p01, a.lo, b.hi);
    div(p10, a.hi, b.lo);
    div(p11, a.hi, b.hi);
    Rat lo = p00, hi = p00;
    for (const Rat* p : {&p01, &p10, &p11}) {
        if (cmp(*p, lo) < 0) lo = *p;
        if (cmp(*p, hi) > 0) hi = *p;
    }
    return Ball(lo, hi);
}

// ===================================================================
// Interval sqrt: ball_sqrt
// ===================================================================
//
// For a positive interval [a, b] with a >= 0: sqrt is monotone on
// positive reals, so sqrt({x : a <= x <= b}) = [sqrt(a), sqrt(b)].
//
// We need exact-rational endpoints.  Since sqrt of an arbitrary
// rational is generally irrational, we widen by 1/denominator (= 1 ULP
// at the denominator's scale), producing an exact rational endpoint
// that may OVER-shoot the true sqrt but never UNDER-shoots -- the
// returned Ball is a certified enclosure containing all true sqrt
// values of [a, b].
//
// Construction: for an exact rational r = num/den (den > 0):
//   sqrt(r) = sqrt(num)/sqrt(den) = sqrt(num * den) / den
// Lower bound: floor(sqrt(num * den)) / den   (genuine LB on R)
// Upper bound: (floor(sqrt(num * den)) + 1) / den   (genuine UB on R)
// Both are exact Rat when the inputs are exact Rat.
//
// Special cases: for r exactly 0, sqrt is 0 (no widening).  For r a
// perfect-square rational (num * den = k^2 for integer k), floor(sqrt)
// = k exactly, and the lower endpoint IS the true sqrt.  The widened
// upper endpoint is k + 1/den, an exact-rational overshoot.
//
// Throws on intervals straddling zero (matching ball_div's policy:
// 1/[lo, hi] is meaningless on [-1, 1], and sqrt([a, b]) with a < 0
// has the same issue -- the user's contract is "give me a certified
// enclosure of sqrt on this interval", which is undefined when the
// interval includes negative reals).
// ===================================================================
inline Ball ball_sqrt(const Ball& b) {
    if (cmp_si(b.lo, 0) < 0 || cmp_si(b.hi, 0) < 0) {
        throw std::invalid_argument("ball_sqrt: interval contains a non-positive value");
    }
    // Helper: ratio √(num/den) is rat-irrational whenever num*den is
    // not a perfect square, so the rational endpoint returned here is
    // an EXACT LOWER or UPPER BOUND on the true sqrt in Q, never equal
    // to it (modulo the perfect-square case which collapses to a point
    // -- we check that below).  This keeps the Ball's width nonzero
    // in the irrational case, so rationality of the sqrt is reflected
    // in the ball's degeneracy-vs-width at output time (paired with a
    // BigFloat view per docs/PROJECT_POLICY in dual_format.hpp).
    auto sqrt_endpoint = [](const Rat& x, bool upper) -> Rat {
        if (is_zero(x)) {
            Rat r; set_si(r, 0);
            return r;
        }
        // prod = num * den (both > 0 here; 0 handled above, den by
        // canonical form).
        BigInt prod;
        mul(prod, num(x), den(x));
        BigInt is; mpz_sqrt(is.get(), prod.get());
        // Perfect-square case: is^2 == prod.  Then sqrt(x) is exactly
        // is/den -- both endpoints collapse to it regardless of
        // whether we asked for LB or UB.
        BigInt ss;
        mul(ss, is, is);
        if (cmp(prod, ss) == 0) {
            Rat r;
            mpq_set_num(r.get(), is.get());
            mpq_set_den(r.get(), den(x).get());
            mpq_canonicalize(r.get());
            return r;
        }
        if (!upper) {
            Rat r;
            mpq_set_num(r.get(), is.get());
            mpq_set_den(r.get(), den(x).get());
            mpq_canonicalize(r.get());
            return r;
        }
        // Upper: (is + 1) / den.  Loosens by exactly 1/den -- the
        // smallest rational-over-sqrt gap the integer-sqrt machinery
        // can detect at this denominator scale.
        BigInt isp1;
        add_si(isp1, is, 1);
        Rat r;
        mpq_set_num(r.get(), isp1.get());
        mpq_set_den(r.get(), den(x).get());
        mpq_canonicalize(r.get());
        return r;
    };
    Rat lo = sqrt_endpoint(b.lo, false);
    Rat hi = sqrt_endpoint(b.hi, true);
    return Ball(lo, hi);
}

// ===================================================================
// Interval cbrt: ball_cbrt
// ===================================================================
//
// cbrt is monotone on ALL of R (negative inputs have negative real
// cbrt, which is finite and well-defined), so ball_cbrt accepts
// negative intervals without throwing -- this is the natural contrast
// with ball_sqrt where sqrt on negative reals is not real.
//
// For exact rational r = num/den (den > 0):
//   cbrt(r) = cbrt(num) / cbrt(den) = cbrt(num * den^2) / den
// Lower bound (num >= 0):  floor(cbrt(num * den^2)) / den
// Upper bound (num >= 0):  (floor(cbrt(num * den^2)) + 1) / den
// Lower bound (num <  0): -(floor(cbrt(|num| * den^2)) + 1) / den
// Upper bound (num <  0): -(floor(cbrt(|num| * den^2)))      / den
//
// Floor(cbrt(k)) comes from mpz_root(.., 3), provided k >= 0
// (mpz_root requires nonnegative input -- hence we flip the sign
// for negative endpoints first).
// ===================================================================
inline Ball ball_cbrt(const Ball& b) {
    // Helper: ratio cbrt(num/den) = cbrt(num * den^2) / den.  Detects
    // perfect-cube endpoints (prod == ic^3) and tightens to exact
    // -ic / den (negative endpoint) or ic / den (positive endpoint),
    // matching ball_sqrt's perfect-square detection -- same
    // dual-representation reason applies: an exactly-rational cube
    // root collapses to a point in the Ball, so rational/irrational
    // status is observable at output time per dual_format.hpp policy.
    auto cbrt_endpoint = [](const Rat& x, bool upper) -> Rat {
        if (is_zero(x)) {
            Rat r; set_si(r, 0);
            return r;
        }
        bool neg = (mpq_sgn(x.get()) < 0);
        BigInt abs_num;
        abs_(abs_num, num(x));
        BigInt den2, prod;
        mul(den2, den(x), den(x));
        mul(prod, abs_num, den2);
        BigInt ic; mpz_root(ic.get(), prod.get(), 3);
        // Perfect-cube case: ic^3 == prod implies |cbrt(x)| = ic / den.
        BigInt ic3;
        BigInt tmp; mul(tmp, ic, ic); mul(ic3, tmp, ic);
        if (cmp(prod, ic3) == 0) {
            Rat r;
            if (!neg) {
                mpq_set_num(r.get(), ic.get());
                mpq_set_den(r.get(), den(x).get());
            } else {
                // cbrt(-x) = -cbrt(x).  ic/den (positive) -> negate.
                BigInt neg_mant; mpz_neg(neg_mant.get(), ic.get());
                mpq_set_num(r.get(), neg_mant.get());
                mpq_set_den(r.get(), den(x).get());
            }
            mpq_canonicalize(r.get());
            return r;
        }
        // Non-perfect-cube case: tightening only when lb/ub asked.
        if (neg) {
            Rat r;
            if (!upper) {
                // LB for negative x: -(ic+1)/den (strict, since cbrt(x) < -ic/den).
                BigInt icp1; add_si(icp1, ic, 1);
                mpq_set_num(r.get(), icp1.get());
                mpq_set_den(r.get(), den(x).get());
                mpq_neg(r.get(), r.get());
            } else {
                // UB for negative x: -ic/den (strict).
                BigInt neg_mant; mpz_neg(neg_mant.get(), ic.get());
                mpq_set_num(r.get(), neg_mant.get());
                mpq_set_den(r.get(), den(x).get());
            }
            mpq_canonicalize(r.get());
            return r;
        } else {
            Rat r;
            if (!upper) {
                mpq_set_num(r.get(), ic.get());
                mpq_set_den(r.get(), den(x).get());
            } else {
                BigInt icp1; add_si(icp1, ic, 1);
                mpq_set_num(r.get(), icp1.get());
                mpq_set_den(r.get(), den(x).get());
            }
            mpq_canonicalize(r.get());
            return r;
        }
    };
    Rat lo = cbrt_endpoint(b.lo, false);
    Rat hi = cbrt_endpoint(b.hi, true);
    return Ball(lo, hi);
}

// ===================================================================
// Collatz-Wielandt Perron-eigenvalue bracket.
// ===================================================================

// One Collatz-Wielandt step: given a nonnegative integer matrix M
// and a strictly positive integer vector v, returns the exact
// rational bracket [min_i (Mv)_i/v_i, max_i (Mv)_i/v_i] together with
// the new vector Mv (for the caller to iterate with).
struct CollatzStep {
    Ball bracket;
    std::vector<long long> next_v;
};

struct ExactCollatzStep {
    Ball bracket;
    std::vector<BigInt> next_v;
};

inline ExactCollatzStep collatz_step_exact(
    const std::vector<std::vector<long long>>& M,
    const std::vector<BigInt>& v) {
    const std::size_t n = M.size();
    if (n == 0 || v.size() != n) {
        throw std::invalid_argument("collatz_step_exact: dimension mismatch");
    }
    std::vector<BigInt> mv(n);
    for (std::size_t i = 0; i < n; ++i) {
        if (M[i].size() != n) {
            throw std::invalid_argument("collatz_step_exact: matrix is not square");
        }
        for (std::size_t j = 0; j < n; ++j) {
            if (M[i][j] < 0) {
                throw std::invalid_argument("collatz_step_exact: matrix is not nonnegative");
            }
            addmul_ui(mv[i], v[j], static_cast<unsigned long>(M[i][j]));
        }
    }
    bool first = true;
    Rat lo, hi;
    for (std::size_t i = 0; i < n; ++i) {
        if (cmp_si(v[i], 0) <= 0) {
            throw std::invalid_argument("collatz_step_exact: v must be strictly positive");
        }
        if (cmp_si(mv[i], 0) <= 0) {
            throw std::invalid_argument("collatz_step_exact: Mv must be strictly positive");
        }
        Rat ratio;
        set(ratio, mv[i], v[i]);
        mpq_canonicalize(ratio.get());
        if (first) {
            lo = ratio;
            hi = ratio;
            first = false;
        } else {
            if (cmp(ratio, lo) < 0) lo = ratio;
            if (cmp(ratio, hi) > 0) hi = ratio;
        }
    }
    BigInt common;
    bool have_common = false;
    for (const auto& x : mv) {
        if (!have_common) {
            set(common, x);
            have_common = true;
        } else {
            BigInt next;
            gcd(next, common, x);
            common = std::move(next);
        }
    }
    if (cmp_si(common, 1) > 0) {
        for (auto& x : mv) {
            BigInt reduced;
            divexact(reduced, x, common);
            x = std::move(reduced);
        }
    }
    ExactCollatzStep out;
    out.bracket = Ball(lo, hi);
    out.next_v = std::move(mv);
    return out;
}

inline CollatzStep collatz_step(
    const std::vector<std::vector<long long>>& M,
    const std::vector<long long>& v) {
    const std::size_t n = M.size();
    if (v.size() != n) throw std::invalid_argument("collatz_step: dimension mismatch");
    std::vector<long long> mv(n, 0);
    for (std::size_t i = 0; i < n; ++i) {
        long long s = 0;
        for (std::size_t j = 0; j < n; ++j) s += M[i][j] * v[j];
        mv[i] = s;
        if (s < 0) {
            throw std::invalid_argument("collatz_step: matrix or vector not nonnegative");
        }
    }
    bool first = true;
    Rat lo, hi;
    for (std::size_t i = 0; i < n; ++i) {
        if (v[i] <= 0) {
            throw std::invalid_argument("collatz_step: v must be strictly positive");
        }
        Rat ratio(mv[i], v[i]);
        if (first) {
            lo = ratio;
            hi = ratio;
            first = false;
        } else {
            if (cmp(ratio, lo) < 0) lo = ratio;
            if (cmp(ratio, hi) > 0) hi = ratio;
        }
    }
    CollatzStep out;
    out.bracket = Ball(lo, hi);
    out.next_v = std::move(mv);
    return out;
}

struct PerronBracketResult {
    Ball bracket;         // final (tightest found) bracket
    int iterations_run;
    bool stopped_early;   // true if a rescale was needed to avoid overflow
                           // before reaching max_iters (see below)
};

// Iterates collatz_step starting from v = (1,1,...,1), narrowing the
// bracket at each step (guaranteed non-widening for ANY nonnegative
// matrix; STRICTLY narrowing in the limit for a primitive one -- the
// case this project's Pisot incidence matrices and G_B adjacency
// matrices are both expected to satisfy).  Stops after `max_iters`
// steps or once the bracket width is below `target_width`, whichever
// comes first.
//
// Overflow guard: v's entries grow like beta^k (the vector's own
// dominant-eigenvalue growth rate).  Since this module works in
// plain `long long`, entries can overflow before max_iters is
// reached for a fast-growing matrix; this function detects that (via
// a conservative pre-multiplication magnitude check) and stops early
// rather than silently wrapping, so a caller always gets a VALID
// bracket, just possibly not as tight as `target_width` asked for.
inline PerronBracketResult certify_perron_bracket(
    const std::vector<std::vector<long long>>& M,
    int max_iters = 200,
    double target_width = 1e-6) {
    const std::size_t n = M.size();
    std::vector<long long> v(n, 1);
    PerronBracketResult out;
    out.iterations_run = 0;
    out.stopped_early = false;

    // Rough a priori bound on the growth factor per step, used only
    // for the overflow pre-check below (max row sum is a standard,
    // cheap upper bound on the Perron root).
    long long max_row_sum = 0;
    for (const auto& row : M) {
        long long s = 0;
        for (long long x : row) s += x;
        max_row_sum = std::max(max_row_sum, s);
    }
    if (max_row_sum <= 0) {
        throw std::invalid_argument("certify_perron_bracket: matrix has no positive row sum");
    }

    constexpr long long kOverflowGuard = (1LL << 60);  // headroom under int64 max
    Ball best;
    bool have_best = false;
    for (int it = 0; it < max_iters; ++it) {
        long long cur_max = *std::max_element(v.begin(), v.end());
        if (cur_max > kOverflowGuard / max_row_sum) {
            out.stopped_early = true;
            break;
        }
        auto step = collatz_step(M, v);
        best = step.bracket;
        have_best = true;
        v = std::move(step.next_v);
        out.iterations_run = it + 1;

        // Check width against target (converted to a rational
        // comparison to avoid a lossy Rat->double round-trip
        // affecting the stopping decision, though the target itself
        // is caller-supplied as a double for convenience).
        Rat w = ball_width(best);
        // width < target_width  <=>  width * 10^9 < target_width * 10^9,
        // approximated via a plain double comparison of the ratio's
        // convergents is unnecessary here -- a direct double
        // conversion of the (small, already-bounded) width is fine
        // for a STOPPING heuristic (it never affects correctness of
        // the returned bracket, only how many extra iterations run).
        double w_d = mpq_get_d(w.get());
        if (w_d < target_width) break;
    }
    if (!have_best) {
        // max_iters == 0 or immediate overflow guard trip before any
        // step; fall back to a single step's bracket so the caller
        // still gets something valid.
        auto step = collatz_step(M, v);
        best = step.bracket;
        out.iterations_run = 1;
    }
    out.bracket = best;
    return out;
}

enum class PerronOrder {
    less,
    equal,
    greater,
    inconclusive,
};

struct PerronComparisonResult {
    PerronOrder order = PerronOrder::inconclusive;
    PerronBracketResult lhs;
    PerronBracketResult rhs;
};

inline PerronBracketResult certify_perron_bracket_exact(
    const std::vector<std::vector<long long>>& M,
    int max_iters = 200,
    double target_width = 1e-12) {
    const std::size_t n = M.size();
    if (n == 0) {
        throw std::invalid_argument("certify_perron_bracket_exact: empty matrix");
    }
    std::vector<BigInt> v;
    v.reserve(n);
    for (std::size_t i = 0; i < n; ++i) v.emplace_back(1);
    PerronBracketResult out;
    out.iterations_run = 0;
    out.stopped_early = false;
    Ball best;
    bool have_best = false;
    const int limit = std::max(1, max_iters);
    for (int it = 0; it < limit; ++it) {
        auto step = collatz_step_exact(M, v);
        best = step.bracket;
        have_best = true;
        v = std::move(step.next_v);
        out.iterations_run = it + 1;
        if (target_width > 0.0 &&
            mpq_get_d(ball_width(best).get()) < target_width) {
            break;
        }
    }
    if (!have_best) {
        throw std::runtime_error("certify_perron_bracket_exact: no iteration completed");
    }
    out.bracket = best;
    return out;
}

inline PerronComparisonResult compare_perron_roots_exact(
    const std::vector<std::vector<long long>>& lhs,
    const std::vector<std::vector<long long>>& rhs,
    int max_iters = 200,
    double target_width = 1e-12) {
    PerronComparisonResult out;
    out.lhs = certify_perron_bracket_exact(lhs, max_iters, target_width);
    out.rhs = certify_perron_bracket_exact(rhs, max_iters, target_width);
    if (cmp(out.lhs.bracket.hi, out.rhs.bracket.lo) < 0) {
        out.order = PerronOrder::less;
    } else if (cmp(out.lhs.bracket.lo, out.rhs.bracket.hi) > 0) {
        out.order = PerronOrder::greater;
    } else if (cmp(out.lhs.bracket.lo, out.lhs.bracket.hi) == 0 &&
               cmp(out.rhs.bracket.lo, out.rhs.bracket.hi) == 0 &&
               cmp(out.lhs.bracket.lo, out.rhs.bracket.lo) == 0) {
        out.order = PerronOrder::equal;
    }
    return out;
}

}  // namespace mathlib
