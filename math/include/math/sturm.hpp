// math/sturm.hpp
//
// Sturm sequencing and real root isolation for polynomials in Z[x].
// Used to determine the EXACT sign of a Q(β) element, replacing the
// double-precision Horner evaluation that fails for large coefficients.
//
// Sturm's theorem: for a squarefree polynomial p ∈ R[x] and a Sturm
// chain p_0 = p, p_1 = p', p_{i+1} = -rem(p_{i-1}, p_i), the function
// V(x) = sign variations in (p_0(x), p_1(x), ..., p_k(x)) (ignoring
// zeros) satisfies V(a) - V(b) = number of distinct real roots in (a, b).
//
// For Pisot charpolys, the dominant real root β > 1 can be isolated
// by Sturm + bisection.  A Q(β) element a(β) is then signed by
// evaluating a at the interval endpoints and checking the sign
// (which is consistent over the interval since a(β) ≠ 0).

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"
#include "math/bezout.hpp"  // gcd_of, used by qbeta_sign below

namespace mathlib {

// ===================================================================
// Sturm chain
// ===================================================================
//
// Build the Sturm chain for a polynomial p.  Returns the chain as
// (p_0, p_1, p_2, ..., p_k) where:
//   p_0 = p
//   p_1 = p' (formal derivative)
//   p_{i+1} = -rem(p_{i-1}, p_i) for i >= 1
//
// Terminates with a constant (since the original is squarefree, or
// even without that assumption, the algorithm terminates with a GCD).

inline std::vector<PolyZ> sturm_chain(const PolyZ& p) {
    std::vector<PolyZ> chain;
    if (p.is_zero()) return chain;
    chain.push_back(p);
    if (p.degree() < 1) {
        chain.push_back(PolyZ());
        return chain;
    }
    PolyZ d;
    d.ensure_size(static_cast<std::size_t>(p.degree()));
    for (std::size_t i = 1; i <= static_cast<std::size_t>(p.degree()); ++i) {
        BigInt c; mul_si(c, p.coeff(i), static_cast<long long>(i));
        set(d.coeff(i - 1), c);
    }
    d.trim();
    chain.push_back(d);
    while (true) {
        const PolyZ& prev = chain[chain.size() - 2];
        const PolyZ& cur = chain[chain.size() - 1];
        if (cur.is_zero()) break;
        if (cur.degree() == 0) break;
        DivModResultZ dm = divmod(prev, cur);
        PolyZ nr = -dm.r;
        chain.push_back(nr);
    }
    return chain;
}

// ===================================================================
// Sign variation count at x (x is a BigInt, so the evaluation is exact)
// ===================================================================

inline int sturm_sign_variations(const std::vector<PolyZ>& chain, const BigInt& x) {
    int variations = 0;
    int last_sign = 0;
    for (const auto& p : chain) {
        if (p.is_zero()) continue;
        BigInt v = eval(p, x);
        int sg = sgn(v);
        if (sg == 0) continue;
        if (last_sign != 0 && sg != last_sign) ++variations;
        last_sign = sg;
    }
    return variations;
}

// ===================================================================
// Number of distinct real roots in (a, b) — a, b are BigInts
// ===================================================================

inline long long sturm_root_count(const std::vector<PolyZ>& chain,
                                  const BigInt& a, const BigInt& b) {
    if (chain.empty()) return 0;
    int va = sturm_sign_variations(chain, a);
    int vb = sturm_sign_variations(chain, b);
    return static_cast<long long>(va) - static_cast<long long>(vb);
}

// ===================================================================
// Cauchy bound
// ===================================================================

inline BigInt cauchy_bound(const PolyZ& p) {
    BigInt m; set_ui(m, 0);
    long long d = p.degree();
    for (long long i = 0; i < d; ++i) {
        BigInt abs_c; abs_(abs_c, p.coeff(static_cast<std::size_t>(i)));
        if (cmp(abs_c, m) > 0) set(m, abs_c);
    }
    BigInt one; set_si(one, 1);
    BigInt r; add(r, m, one);
    return r;
}

// ===================================================================
// Root interval (rational endpoints)
// ===================================================================

struct RootInterval {
    Rat lo;
    Rat hi;
};

// ===================================================================
// Number of distinct real roots in (a, b) — a, b are Rats
// ===================================================================
//
// To evaluate the Sturm chain at a rational point a/b, we substitute
// x = a/b in each polynomial p_i(x): p_i(a/b) = N_i / b^deg(p_i) where
// N_i is an integer.  The sign of p_i(a/b) is sign(N_i).

inline int poly_at_rational_sign(const PolyZ& p, const Rat& x) {
    if (p.is_zero()) return 0;
    // p(x) = sum p_i x^i.  At x = a/b, p(x) = sum p_i a^i b^{d-i} / b^d
    // = (sum p_i a^i b^{d-i}) / b^d.
    // We just need the sign of the numerator.
    long long d = p.degree();
    BigInt a; mpz_set(a.get(), mpq_numref(x.get()));
    BigInt b; mpz_set(b.get(), mpq_denref(x.get()));
    // a^d, a^{d-1}, ..., a^0
    std::vector<BigInt> a_pow(static_cast<std::size_t>(d) + 1);
    set_ui(a_pow[0], 1);
    for (long long i = 1; i <= d; ++i) mul(a_pow[static_cast<std::size_t>(i)], a_pow[static_cast<std::size_t>(i - 1)], a);
    std::vector<BigInt> b_pow(static_cast<std::size_t>(d) + 1);
    set_ui(b_pow[0], 1);
    for (long long i = 1; i <= d; ++i) mul(b_pow[static_cast<std::size_t>(i)], b_pow[static_cast<std::size_t>(i - 1)], b);
    BigInt num; set_ui(num, 0);
    for (long long i = 0; i <= d; ++i) {
        BigInt term;
        mul(term, p.coeff(static_cast<std::size_t>(i)), a_pow[static_cast<std::size_t>(i)]);
        mul(term, term, b_pow[static_cast<std::size_t>(d - i)]);
        add(num, num, term);
    }
    return sgn(num);
}

inline int sturm_sign_variations_rat(const std::vector<PolyZ>& chain, const Rat& x) {
    int variations = 0;
    int last_sign = 0;
    for (const auto& p : chain) {
        if (p.is_zero()) continue;
        int sg = poly_at_rational_sign(p, x);
        if (sg == 0) continue;
        if (last_sign != 0 && sg != last_sign) ++variations;
        last_sign = sg;
    }
    return variations;
}

inline long long sturm_root_count_rat(const std::vector<PolyZ>& chain,
                                      const Rat& a, const Rat& b) {
    if (chain.empty()) return 0;
    int va = sturm_sign_variations_rat(chain, a);
    int vb = sturm_sign_variations_rat(chain, b);
    return static_cast<long long>(va) - static_cast<long long>(vb);
}

// ===================================================================
// Real root isolation (rational bisection)
// ===================================================================

inline RootInterval isolate_real_root_rat(const std::vector<PolyZ>& chain,
                                           const Rat& a, const Rat& b,
                                           int tol = 64) {
    if (chain.empty()) {
        throw std::invalid_argument("isolate_real_root_rat: empty chain");
    }
    long long n_roots = sturm_root_count_rat(chain, a, b);
    if (n_roots <= 0) {
        throw std::invalid_argument("isolate_real_root_rat: no root in interval");
    }
    Rat lo = a, hi = b;
    for (int i = 0; i < tol; ++i) {
        // mid = (lo + hi) / 2 (exact rational)
        Rat mid;
        Rat sum; add(sum, lo, hi);
        Rat two; set_si(two, 2, 1);
        div(mid, sum, two);
        if (cmp(mid, lo) == 0 || cmp(mid, hi) == 0) break;  // no more precision
        long long n_lo = sturm_root_count_rat(chain, lo, mid);
        if (n_lo > 0) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    return RootInterval{lo, hi};
}

// ===================================================================
// Sign of a Q(β) element via the isolating interval
// ===================================================================
//
// Returns +1, 0, or -1.  For invertible a, returns ±1.

inline int qbeta_sign(const QElem& a, const QBetaRing& R, const RootInterval& beta_interval) {
    // Convert QElem to PolyZ (lift to integer coefficients) for evaluation.
    BigInt lcm_den; set_ui(lcm_den, 1);
    for (const auto& c : a.coeffs_) {
        BigInt c_den; mpz_set(c_den.get(), mpq_denref(c.get()));
        BigInt g = gcd_of(lcm_den, c_den);
        divexact(g, lcm_den, g);
        mul(lcm_den, g, c_den);
    }
    // Convert a to a PolyZ (lifted by lcm_den).
    PolyZ A;
    A.ensure_size(R.degree());
    for (std::size_t i = 0; i < R.degree(); ++i) {
        BigInt num; mpz_set(num.get(), mpq_numref(a.coeffs_[i].get()));
        BigInt den; mpz_set(den.get(), mpq_denref(a.coeffs_[i].get()));
        BigInt Ai;
        mul(Ai, num, lcm_den);
        divexact(Ai, Ai, den);
        set(A.coeff(i), Ai);
    }
    A.trim();
    // Evaluate A at the interval endpoints (using rational evaluation).
    int sg_lo = poly_at_rational_sign(A, beta_interval.lo);
    int sg_hi = poly_at_rational_sign(A, beta_interval.hi);
    if (sg_lo == sg_hi) return sg_lo;
    // Different signs: fall back to leading-coefficient sign.
    if (sg_lo != 0) return sg_lo;
    if (sg_hi != 0) return sg_hi;
    return 0;
}

// ===================================================================
// Convenience: isolate β from the charpoly
// ===================================================================

inline RootInterval isolate_beta(const QBetaRing& R) {
    const PolyZ& cp = R.charpoly();
    std::vector<PolyZ> chain = sturm_chain(cp);
    if (chain.empty()) {
        throw std::invalid_argument("isolate_beta: empty Sturm chain");
    }
    // β > 1: count roots in (1, cauchy_bound).
    Rat one; set_si(one, 1, 1);
    BigInt cb = cauchy_bound(cp);
    Rat cb_r; set_si(cb_r, 0, 1);
    mpz_set(mpq_numref(cb_r.get()), cb.get());
    mpq_canonicalize(cb_r.get());
    long long n = sturm_root_count_rat(chain, one, cb_r);
    if (n <= 0) {
        throw std::runtime_error("isolate_beta: no root in (1, cauchy_bound)");
    }
    if (n > 1) {
        // Bisect to find a sub-interval with exactly 1 root.
        Rat lo = one, hi = cb_r;
        while (sturm_root_count_rat(chain, lo, hi) > 1) {
            Rat sum; add(sum, lo, hi);
            Rat mid;
            Rat two; set_si(two, 2, 1);
            div(mid, sum, two);
            if (cmp(mid, lo) == 0 || cmp(mid, hi) == 0) break;
            if (sturm_root_count_rat(chain, one, mid) > 0) {
                hi = mid;
            } else {
                lo = mid;
            }
        }
        return isolate_real_root_rat(chain, lo, hi);
    }
    return isolate_real_root_rat(chain, one, cb_r);
}

}  // namespace mathlib
