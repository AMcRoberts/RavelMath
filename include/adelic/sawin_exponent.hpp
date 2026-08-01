// include/adelic/sawin_exponent.hpp
//
// Numeric verification of Sawin's Proposition 10 exponent formula
// (arXiv 2605.20579, "An explicit lower bound for the unit distance
// problem"), specialized to formula (11) in the proof of Theorem 1
// (p. 12): the delta such that #{unit-distance pairs} >= (#U)^{1+delta}
// / (8 * lambda^2), for the specific T, S_Q the paper publishes.
//
//   delta =
//     [ log(1 - 1/R) + log(2*pi/e)/2
//       + sum_{p in S_Q} log(k(p)+1) / (4*e(p))
//       - log(4*prod(T)) / 8
//       - log(log(sqrt(4*prod(T)))) / 2 ]
//     / log( 2*R * prod_{p in S_Q} p^{k(p)/(2*e(p))} + 1 )
//
// e(p) = 2 if p in T or p == 2, else 1 (the ramification index from
// Lemma 12); k(p), e(p), R = 72 are the paper's own published values.
// The paper states the result is delta = 0.014114... (giving the
// exponent 1.014114 in Theorem 1).
//
// This is the piece include/adelic/golod_shafarevich.hpp's own header
// comment named as NOT done: "Proposition 10's own logarithm-heavy
// computation remains a separate, unimplemented piece." It needed
// math/bigfloat_trig.hpp's bigfloat_log, which didn't exist until this
// session (and which surfaced and fixed a real range-reduction bug in
// bigfloat_exp along the way -- see that file's own history).
//
// Scope: this verifies Proposition 10's OWN formula reproduces the
// paper's stated delta for the paper's own published parameters. It
// does not re-derive or independently check Lemma 9's class-number
// bound (Proposition 10's proof uses it, but the formula itself, once
// F/K/S_Q/k/e/f/lambda are given, is pure real arithmetic) or Lemma
// 12's construction of F and K (also assumed, not reverified here --
// that's a Galois-theoretic existence argument, not a numeric one).

#pragma once

#include <stdexcept>
#include <vector>

#include "math/bigfloat.hpp"
#include "math/bigfloat_trig.hpp"
#include "math/bigint.hpp"

namespace adelic {

struct SawinExponentTerm {
    long long p;
    long long k_p;
    long long e_p;
};

struct SawinExponentResult {
    mathlib::BigFloat delta;
    mathlib::BigFloat numerator;
    mathlib::BigFloat denominator;
};

// prec: BigFloat precision in bits for every intermediate computation
// (the paper's own numbers are given to ~6 decimal digits, so prec=200
// is comfortably more than enough headroom).
inline SawinExponentResult compute_sawin_delta(
    const mathlib::BigInt& prod_T,     // prod_{q in T} q
    long long R,
    const std::vector<SawinExponentTerm>& S_Q_terms,
    unsigned prec = 200) {
    using namespace mathlib;

    if (R <= 1) {
        throw std::invalid_argument("compute_sawin_delta: R must be > 1");
    }
    for (const auto& t : S_Q_terms) {
        if (t.k_p <= 0 || t.e_p <= 0) {
            throw std::invalid_argument("compute_sawin_delta: k(p) and e(p) must be positive");
        }
    }

    BigFloat one = bigfloat_from_ll(1);
    BigFloat two = bigfloat_from_ll(2);
    BigFloat four = bigfloat_from_ll(4);
    BigFloat eight = bigfloat_from_ll(8);

    // log(1 - 1/R)
    BigFloat R_f = bigfloat_from_ll(R);
    BigFloat one_minus_inv_R = bigfloat_sub(one, bigfloat_div(one, R_f, prec), prec);
    BigFloat term_R = bigfloat_log(one_minus_inv_R, prec);

    // log(2*pi/e) / 2
    BigFloat pi = bigfloat_pi_runtime(prec);
    BigFloat e_const = bigfloat_exp(one);
    BigFloat two_pi_over_e = bigfloat_div(bigfloat_mul(two, pi, prec), e_const, prec);
    BigFloat term_pi_e = bigfloat_div(bigfloat_log(two_pi_over_e, prec), two, prec);

    // sum_p log(k(p)+1) / (4*e(p))
    BigFloat sum_p = bigfloat_from_ll(0);
    for (const auto& t : S_Q_terms) {
        BigFloat log_kp1 = bigfloat_log(bigfloat_from_ll(t.k_p + 1), prec);
        BigFloat denom = bigfloat_mul(four, bigfloat_from_ll(t.e_p), prec);
        sum_p = bigfloat_add(sum_p, bigfloat_div(log_kp1, denom, prec), prec);
    }

    // lambda = sqrt(4 * prod(T)); log(4*prod(T))/8 and log(log(lambda))/2
    BigFloat four_N = bigfloat_mul(four, bigfloat_from_bigint(prod_T), prec);
    BigFloat log_4N = bigfloat_log(four_N, prec);
    BigFloat term_logN = bigfloat_div(log_4N, eight, prec);

    BigFloat lambda = bigfloat_sqrt(four_N, prec);
    BigFloat log_lambda = bigfloat_log(lambda, prec);
    BigFloat term_loglogN = bigfloat_div(bigfloat_log(log_lambda, prec), two, prec);

    BigFloat numerator = bigfloat_sub(
        bigfloat_sub(
            bigfloat_add(bigfloat_add(term_R, term_pi_e, prec), sum_p, prec),
            term_logN, prec),
        term_loglogN, prec);

    // denominator: log(2*R * prod_p p^{k(p)/(2*e(p))} + 1)
    BigFloat prod_powers = one;
    for (const auto& t : S_Q_terms) {
        BigFloat log_p = bigfloat_log(bigfloat_from_ll(t.p), prec);
        BigFloat exponent = bigfloat_div(bigfloat_from_ll(t.k_p),
                                          bigfloat_mul(two, bigfloat_from_ll(t.e_p), prec), prec);
        BigFloat p_pow = bigfloat_exp(bigfloat_mul(log_p, exponent, prec));
        prod_powers = bigfloat_mul(prod_powers, p_pow, prec);
    }
    BigFloat inside = bigfloat_add(bigfloat_mul(bigfloat_mul(two, R_f, prec), prod_powers, prec), one, prec);
    BigFloat denominator = bigfloat_log(inside, prec);

    SawinExponentResult result;
    result.numerator = numerator;
    result.denominator = denominator;
    result.delta = bigfloat_div(numerator, denominator, prec);
    return result;
}

}  // namespace adelic
