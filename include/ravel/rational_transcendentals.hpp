// rational_transcendentals.hpp
//
// Provably-correct rational brackets for pi, sin, cos -- the two
// transcendentals the GKW Chebyshev-node construction otherwise
// gets from plain `double` (via std::cos/std::sin and a hardcoded
// PI literal, both unverified). This closes that gap rather than
// working around it, per the explicit request not to circumvent
// transcendentals where it's tractable.
//
// Method (both are classical, provably-correct techniques, not
// heuristics):
//
//   pi: Machin's formula, pi/4 = 4*arctan(1/5) - arctan(1/239), with
//   arctan(x) computed as the alternating series
//     x - x^3/3 + x^5/5 - x^7/7 + ...
//   For a strictly alternating series with strictly decreasing terms
//   (true here, since |x|<1 for both 1/5 and 1/239), the partial sum
//   truncated after the term of order x^(2k+1) is within the NEXT
//   term's magnitude of the true value -- a fully rigorous two-sided
//   bracket, not an asymptotic estimate.
//
//   sin/cos at a rational point x: Taylor series around 0 with the
//   Lagrange remainder bound. Since every derivative of sin and cos
//   is bounded by 1 in absolute value, truncating after the x^n term
//   leaves a remainder bounded by |x|^(n+1)/(n+1)! -- again a proven
//   bound, not an estimate.
//
// Both bracket widths shrink extremely fast (factorially) in the
// number of terms taken, so reaching any requested precision is
// cheap.

#pragma once

#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"
#include <utility>

namespace ravel {

struct QInterval {
    mpq_t lo, hi;
    QInterval() { mpq_init(lo); mpq_init(hi); }
    ~QInterval() { mpq_clear(lo); mpq_clear(hi); }
    QInterval(const QInterval&) = delete;
    QInterval& operator=(const QInterval&) = delete;
};

// arctan(p/q) for a rational p/q with |p/q| < 1, bracketed to within
// the (terms+1)-th series term via the alternating-series theorem.
inline void rational_arctan(mpq_t lo_out, mpq_t hi_out,
                            long p, long q, int terms) {
    mpq_t x2, term, sum, t;
    mpq_init(x2); mpq_init(term); mpq_init(sum); mpq_init(t);

    mpq_set_si(term, p, q);            // term_0 = x
    mpq_set(sum, term);                // running partial sum
    mpq_mul(x2, term, term);           // x^2 (reused each step: term *= -x^2, /(2k+1) folded manually)

    mpq_t last_term; mpq_init(last_term);
    mpq_set(last_term, term);

    for (int k = 1; k < terms; ++k) {
        // term_k = term_{k-1} * (-x^2) * (2k-1)/(2k+1)
        mpq_mul(term, term, x2);
        mpq_neg(term, term);
        mpq_set_si(t, 2*k - 1, 2*k + 1);
        mpq_mul(term, term, t);
        mpq_add(sum, sum, term);
        mpq_set(last_term, term);
    }

    // One more term for the rigorous remainder bound.
    mpq_mul(term, term, x2);
    mpq_neg(term, term);
    mpq_set_si(t, 2*terms - 1, 2*terms + 1);
    mpq_mul(term, term, t);
    // |remainder| <= |term| (next term), and its sign alternates, so
    // sum and sum+term bracket the true value from either side.
    mpq_t a, b;
    mpq_init(a); mpq_init(b);
    mpq_set(a, sum);
    mpq_add(b, sum, term);
    if (mpq_cmp(a, b) <= 0) { mpq_set(lo_out, a); mpq_set(hi_out, b); }
    else                    { mpq_set(lo_out, b); mpq_set(hi_out, a); }

    mpq_clear(a); mpq_clear(b); mpq_clear(last_term);
    mpq_clear(x2); mpq_clear(term); mpq_clear(sum); mpq_clear(t);
}

// pi via Machin's formula, each arctan carried to `terms` series terms.
inline void rational_pi(mpq_t lo_out, mpq_t hi_out, int terms = 40) {
    mpq_t a_lo, a_hi, b_lo, b_hi, four, t;
    mpq_init(a_lo); mpq_init(a_hi); mpq_init(b_lo); mpq_init(b_hi);
    mpq_init(four); mpq_init(t);

    rational_arctan(a_lo, a_hi, 1, 5, terms);     // arctan(1/5)
    rational_arctan(b_lo, b_hi, 1, 239, terms);   // arctan(1/239)

    // pi = 16*arctan(1/5) - 4*arctan(1/239); lo/hi from the
    // (monotone-in-each-argument) combination of the two brackets.
    mpq_set_si(four, 16, 1);
    mpq_mul(a_lo, a_lo, four); mpq_mul(a_hi, a_hi, four);
    mpq_set_si(four, 4, 1);
    mpq_mul(b_lo, b_lo, four); mpq_mul(b_hi, b_hi, four);

    mpq_sub(lo_out, a_lo, b_hi);   // minimize: min(16*arctan5) - max(4*arctan239)
    mpq_sub(hi_out, a_hi, b_lo);   // maximize: max(16*arctan5) - min(4*arctan239)

    mpq_clear(a_lo); mpq_clear(a_hi); mpq_clear(b_lo); mpq_clear(b_hi);
    mpq_clear(four); mpq_clear(t);
}

// n! as an exact mpq (integer, but kept as mpq for uniform arithmetic).
inline void mpq_fact(mpq_t out, int n) {
    mpq_set_si(out, 1, 1);
    mpq_t t; mpq_init(t);
    for (int i = 2; i <= n; ++i) { mpq_set_si(t, i, 1); mpq_mul(out, out, t); }
    mpq_clear(t);
}

// sin(x) and cos(x) for rational x, bracketed via Taylor + Lagrange
// remainder: |R_n(x)| <= |x|^(n+1) / (n+1)!  (valid since every
// derivative of sin/cos has magnitude <= 1 everywhere).
inline void rational_sin(mpq_t lo_out, mpq_t hi_out, const mpq_t x, int terms) {
    mpq_t sum, term, x2, t, fact, absx, bound, xpow;
    mpq_init(sum); mpq_init(term); mpq_init(x2); mpq_init(t);
    mpq_init(fact); mpq_init(absx); mpq_init(bound); mpq_init(xpow);

    mpq_mul(x2, x, x);
    mpq_set(term, x);          // term_0 = x  (the k=0 term of sin's series)
    mpq_set(sum, term);
    for (int k = 1; k < terms; ++k) {
        mpq_mul(term, term, x2);
        mpq_neg(term, term);
        mpq_set_si(t, 1, (2*k)*(2*k+1));
        mpq_mul(term, term, t);
        mpq_add(sum, sum, term);
    }
    // Remainder bound: |x|^(2*terms+1) / (2*terms+1)!
    mpq_set(absx, x); if (mpq_sgn(absx) < 0) mpq_neg(absx, absx);
    mpq_set(xpow, absx);
    for (int i = 0; i < 2*terms; ++i) mpq_mul(xpow, xpow, absx);
    mpq_fact(fact, 2*terms + 1);
    mpq_div(bound, xpow, fact);

    mpq_sub(lo_out, sum, bound);
    mpq_add(hi_out, sum, bound);

    mpq_clear(sum); mpq_clear(term); mpq_clear(x2); mpq_clear(t);
    mpq_clear(fact); mpq_clear(absx); mpq_clear(bound); mpq_clear(xpow);
}

inline void rational_cos(mpq_t lo_out, mpq_t hi_out, const mpq_t x, int terms) {
    mpq_t sum, term, x2, t, fact, absx, bound, xpow;
    mpq_init(sum); mpq_init(term); mpq_init(x2); mpq_init(t);
    mpq_init(fact); mpq_init(absx); mpq_init(bound); mpq_init(xpow);

    mpq_mul(x2, x, x);
    mpq_set_si(term, 1, 1);    // term_0 = 1
    mpq_set(sum, term);
    for (int k = 1; k < terms; ++k) {
        mpq_mul(term, term, x2);
        mpq_neg(term, term);
        mpq_set_si(t, 1, (2*k-1)*(2*k));
        mpq_mul(term, term, t);
        mpq_add(sum, sum, term);
    }
    mpq_set(absx, x); if (mpq_sgn(absx) < 0) mpq_neg(absx, absx);
    mpq_set(xpow, absx);
    for (int i = 0; i < 2*terms - 1; ++i) mpq_mul(xpow, xpow, absx);
    mpq_fact(fact, 2*terms);
    mpq_div(bound, xpow, fact);

    mpq_sub(lo_out, sum, bound);
    mpq_add(hi_out, sum, bound);

    mpq_clear(sum); mpq_clear(term); mpq_clear(x2); mpq_clear(t);
    mpq_clear(fact); mpq_clear(absx); mpq_clear(bound); mpq_clear(xpow);
}

}  // namespace ravel
