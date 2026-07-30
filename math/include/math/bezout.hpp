// math/bezout.hpp
//
// Polynomial GCD and Bezout in Z[x] and Q(β).
//
// We compute the extended GCD in Z[x] using pseudo-division (so the
// algorithm always terminates, even when the leading coefficient of
// the divisor does not divide the leading coefficient of the dividend).
// The result (g, u, v) satisfies g = u·a + v·b in Z[x], with deg(g)
// <= min(deg(a), deg(b)).
//
// For Q(β) = Q[x] / charpoly, we lift a Q(β) element to Z[β] by
// clearing denominators, then compute the Bezout of the lift and the
// charpoly in Z[x].  The reduction mod charpoly of the Bezout
// coefficient gives the inverse in Q(β) (up to a unit).

#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"

namespace mathlib {

// ===================================================================
// Extended GCD in Z[x] (via pseudo-division)
// ===================================================================
//
// Returns (g, u, v) with g = u*a + v*b in Z[x].  Uses the
// subresultant-style algorithm to keep coefficients from growing
// without bound.  The "naive" version is correct but produces large
// coefficients; the subresultant version produces the primitive
// GCD.
//
// For our Q(β) application, the naive version is sufficient because
// the inputs (a polynomial and the charpoly) have small coefficients
// and the result has bounded size.

struct ExtGcdResultZ {
    PolyZ g;
    PolyZ u;
    PolyZ v;
};

inline ExtGcdResultZ ext_gcd(const PolyZ& a, const PolyZ& b) {
    if (a.is_zero() && b.is_zero()) {
        return ExtGcdResultZ{PolyZ(), PolyZ(), PolyZ()};
    }
    if (b.is_zero()) {
        // g = a, u = 1, v = 0
        PolyZ u;
        u.ensure_size(1); set_si(u.coeff(0), 1);
        return ExtGcdResultZ{a, u, PolyZ()};
    }
    if (a.is_zero()) {
        // g = b, u = 0, v = 1
        PolyZ v;
        v.ensure_size(1); set_si(v.coeff(0), 1);
        return ExtGcdResultZ{b, PolyZ(), v};
    }
    // Pseudo-division: lc(b)^k * a = q*b + r
    DivModResultZ dm = divmod(a, b);
    long long db = b.degree();
    BigInt lc_b = b.coeff(static_cast<std::size_t>(db));
    // Track the power of lc_b we multiplied by
    long long k = a.degree() - db + 1;
    // The actual recurrence: a = q*b + r/lc_b^k, i.e., r = lc_b^k * a - q * b
    // (using the pseudo-division result).
    // Recursive call: g = u' * b + v' * r
    // Substitute: r = lc_b^k * a - q * b
    // g = u' * b + v' * (lc_b^k * a - q * b)
    //   = v' * lc_b^k * a + (u' - v' * q) * b
    // So u = v' * lc_b^k, v = u' - v' * q
    //
    // Note: dm.r is the "pseudo-remainder" = lc_b^k * a - q * b (in Z[x]).
    // But wait, the pseudo-remainder of divmod should be lc_b^k * a - q*b, not
    // divided by lc_b^k.  Let me re-check.
    //
    // Pseudo-division: there exist q, r with lc_b^(da-db+1) * a = q * b + r, deg(r) < db.
    // So r = lc_b^(da-db+1) * a - q * b.
    // r IS the pseudo-remainder (no division).
    //
    // Recursive: g = u' * b + v' * r = u' * b + v' * (lc_b^k * a - q * b)
    //   = v' * lc_b^k * a + (u' - v' * q) * b
    // So u = v' * lc_b^k (multiplied by lc_b^k), v = u' - v' * q.
    ExtGcdResultZ sub = ext_gcd(b, dm.r);
    // u = v' * lc_b^k
    PolyZ u = scale_poly(sub.v, lc_b);  // *= lc_b
    for (long long i = 1; i < k; ++i) u = scale_poly(u, lc_b);
    // v = u' - v' * q
    PolyZ vq = sub.v * dm.q;
    PolyZ v = sub.u - vq;
    return ExtGcdResultZ{sub.g, u, v};
}

// ===================================================================
// Extended GCD in Q[x] (exact division, no pseudo)
// ===================================================================

struct ExtGcdResultQ {
    PolyQ g;
    PolyQ u;
    PolyQ v;
};

inline ExtGcdResultQ ext_gcd(const PolyQ& a, const PolyQ& b) {
    if (a.is_zero() && b.is_zero()) {
        return ExtGcdResultQ{PolyQ(), PolyQ(), PolyQ()};
    }
    if (b.is_zero()) {
        PolyQ u; u.ensure_size(1); set_si(u.coeff(0), 1, 1);
        return ExtGcdResultQ{a, u, PolyQ()};
    }
    if (a.is_zero()) {
        PolyQ v; v.ensure_size(1); set_si(v.coeff(0), 1, 1);
        return ExtGcdResultQ{b, PolyQ(), v};
    }
    DivModResultQ dm = divmod(a, b);
    ExtGcdResultQ sub = ext_gcd(b, dm.r);
    PolyQ vq = sub.v * dm.q;
    PolyQ v = sub.u - vq;
    return ExtGcdResultQ{sub.g, sub.v, v};
}

// ===================================================================
// Modular inverse in Q(β) (via Z[x] Bezout)
// ===================================================================

// ===================================================================
// Helper: GCD of two BigInts (for LCM computation)
// ===================================================================

inline BigInt gcd_of(const BigInt& a, const BigInt& b) {
    BigInt g;
    gcd(g, a, b);
    return g;
}

// ===================================================================
// Modular inverse in Q(β) (via Z[x] Bezout)
// ===================================================================
//
// Given a ∈ Q(β), returns a^{-1} ∈ Q(β) if a is invertible, else
// returns std::nullopt (or throws).  The algorithm:
//   1. Lift a to A ∈ Z[β] (clear denominators).
//   2. Compute Bezout of A and charpoly in Z[x] (unreduced).
//   3. Reduce the Bezout coefficient u' = u mod charpoly ∈ Z[β].
//   4. The inverse is u' / g ∈ Q(β) where g is the GCD.
//
// If g is a non-zero rational (a unit in Z[β]), the inverse is well-
// defined.  For Pisot substitutions, g is a small integer in practice.
//
// The reduction mod charpoly follows the same rule as Q(β) mul:
//   β^k = -sum_{i=0}^{d-1} charpoly_.coeff_(d-1-i) β^{k-1-i} for k >= d
// but with integer arithmetic.

inline PolyZ reduce_mod_charpoly_z(const PolyZ& p, const PolyZ& charpoly) {
    std::size_t d = static_cast<std::size_t>(charpoly.degree());
    if (d == 0) return PolyZ();
    long long dp = p.degree();
    if (dp < static_cast<long long>(d)) {
        return p;  // already reduced
    }
    PolyZ r = p;
    // For k from dp down to d, reduce: r[k-1-i] -= charpoly_.coeff_(d-1-i) * r[k]
    for (long long k = dp; k >= static_cast<long long>(d); --k) {
        if (sgn(r.coeff(static_cast<std::size_t>(k))) == 0) continue;
        for (std::size_t i = 0; i < d; ++i) {
            long long target = k - 1 - static_cast<long long>(i);
            if (target < 0) continue;
            BigInt prod;
            mul(prod, charpoly.coeff(d - 1 - i), r.coeff(static_cast<std::size_t>(k)));
            sub(r.coeff(static_cast<std::size_t>(target)), r.coeff(static_cast<std::size_t>(target)), prod);
        }
        set_ui(r.coeff(static_cast<std::size_t>(k)), 0);
    }
    r.trim();
    return r;
}

struct QBetaInverseResult {
    bool invertible;  // true if a has an inverse in Q(β)
    QElem inverse;    // the inverse, valid only if invertible
};

inline QBetaInverseResult invert_in_qbeta(const QElem& a, const QBetaRing& R) {
    const std::size_t d = R.degree();
    // 1. Lift a to Z[β]: compute lcm of denominators.
    BigInt lcm_den; set_ui(lcm_den, 1);
    for (const auto& c : a.coeffs_) {
        BigInt c_den; mpz_set(c_den.get(), mpq_denref(c.get()));
        BigInt g = gcd_of(lcm_den, c_den);
        divexact(g, lcm_den, g);
        mul(lcm_den, g, c_den);
    }
    // A = a * lcm_den in Z[β]
    PolyZ A;
    A.ensure_size(d);
    for (std::size_t i = 0; i < d; ++i) {
        BigInt num; mpz_set(num.get(), mpq_numref(a.coeffs_[i].get()));
        BigInt den; mpz_set(den.get(), mpq_denref(a.coeffs_[i].get()));
        BigInt scaled_num;
        // scaled_num = num * lcm_den / den
        mul(scaled_num, num, lcm_den);
        divexact(scaled_num, scaled_num, den);
        set(A.coeff(i), scaled_num);
    }
    A.trim();
    // 2. Bezout of A and charpoly in Z[x] (unreduced).
    ExtGcdResultZ eg = ext_gcd(A, R.charpoly());
    // 3. Reduce u mod charpoly in Z[β] (so u_red is in Z[β]).
    PolyZ u_red = reduce_mod_charpoly_z(eg.u, R.charpoly());
    // 4. Reduce g mod charpoly (g_red should be a constant for invertible a).
    PolyZ g_red = reduce_mod_charpoly_z(eg.g, R.charpoly());
    if (g_red.degree() != 0) {
        // g has β factors; a is not invertible in Q(β)
        return QBetaInverseResult{false, R.zero()};
    }
    // 5. a^{-1} = lcm_den * u_red / g_red (in Q(β))
    QElem inv_elem(d);
    for (std::size_t i = 0; i < d; ++i) {
        Rat num; set(num, u_red.coeff(i));
        mul_si(num, num, 0);  // just to make sure num is canonical
        set(num, u_red.coeff(i));
        // multiply num by lcm_den
        BigInt scaled; mul(scaled, u_red.coeff(i), lcm_den);
        set(num, scaled);
        Rat den; set(den, g_red.coeff(0));
        mathlib::div(inv_elem.coeffs_[i], num, den);
    }
    return QBetaInverseResult{true, inv_elem};
}

}  // namespace mathlib
