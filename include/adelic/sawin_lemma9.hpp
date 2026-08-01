// include/adelic/sawin_lemma9.hpp
//
// Numeric check of Sawin's Lemma 9 (arXiv 2605.20579, "An explicit
// lower bound for the unit distance problem"):
//
//   h^-(K) <= 8 * rd_{K/F}^2 * (sqrt(rd_{K/F}) * log(rd_{K/F}) * e / (4*pi))^d
//
// specialized to the base case F = Q, K an imaginary quadratic field
// (d = [F:Q] = 1, h^-(K) = h(K)/h(F) = h(K) since h(Q) = 1, and
// rd_{K/F} = rd_K = |Delta_K|^{1/2} since [K:Q] = 2).
//
// This is the piece include/adelic/golod_shafarevich.hpp's own header
// comment named as needing "class-group machinery this project doesn't
// have yet" -- now that include/adelic/quadratic_class_group.hpp exists,
// this closes that loose end for the one case (imaginary quadratic,
// d=1) this project's own machinery can actually compute h(K) for
// directly, rather than just trusting the bound abstractly.
//
// Scope: checks the inequality holds for concrete small discriminants.
// Does NOT re-derive or verify Louboutin's underlying analytic bound
// (Lemma 9's own proof, cited to a separate paper) -- only that
// Sawin's stated consequence of it is numerically consistent with this
// project's own independently-computed class numbers.

#pragma once

#include "math/bigfloat.hpp"
#include "math/bigfloat_trig.hpp"
#include "math/bigint.hpp"

#include "adelic/quadratic_class_group.hpp"

namespace adelic {

// The Lemma 9 RHS bound, specialized to F=Q, K=Q(sqrt(D)), d=1.
inline mathlib::BigFloat sawin_lemma9_bound(const mathlib::BigInt& D, unsigned prec = 200) {
    using namespace mathlib;
    BigInt abs_D;
    abs_(abs_D, D);
    BigFloat rd = bigfloat_sqrt(bigfloat_from_bigint(abs_D), prec);

    BigFloat log_rd = bigfloat_log(rd, prec);
    BigFloat sqrt_rd = bigfloat_sqrt(rd, prec);
    BigFloat e_const = bigfloat_exp(bigfloat_from_ll(1));
    BigFloat pi = bigfloat_pi_runtime(prec);
    BigFloat four_pi = bigfloat_mul(bigfloat_from_ll(4), pi, prec);

    BigFloat inner = bigfloat_div(
        bigfloat_mul(bigfloat_mul(sqrt_rd, log_rd, prec), e_const, prec), four_pi, prec);
    // d=1, so "inner^d" is just inner itself.
    BigFloat rd2 = bigfloat_mul(rd, rd, prec);
    BigFloat bound = bigfloat_mul(bigfloat_mul(bigfloat_from_ll(8), rd2, prec), inner, prec);
    return bound;
}

// True iff quadratic_class_number(D) <= sawin_lemma9_bound(D) (a real
// comparison at BigFloat precision, not a heuristic).
inline bool sawin_lemma9_holds(const mathlib::BigInt& D, unsigned prec = 200) {
    long long h = quadratic_class_number(D);
    mathlib::BigFloat bound = sawin_lemma9_bound(D, prec);
    mathlib::BigFloat h_f = mathlib::bigfloat_from_ll(h);
    return mathlib::bigfloat_cmp(h_f, bound) <= 0;
}

}  // namespace adelic
