// include/adelic/fp_poly.hpp
//
// F_p[x] polynomial representation and exact arithmetic (add, sub,
// mul, divmod, eval, gcd), plus the Z[x] <-> F_p[x] reduction/lift
// helpers. Extracted from dedekind_factorization.hpp so that both
// that file and fp_poly_factor.hpp (the general, any-degree factoring
// algorithm) can depend on this shared base without a circular
// include between the two of them.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"

namespace adelic {

// All operations are exact mod p.  We represent F_p[x] polynomials
// as std::vector<long long> of length degree+1, with coefficients
// in {0, 1, ..., p-1}.
struct FpPoly {
    long long p;            // The prime
    std::vector<long long> c;  // Coefficients c[0] + c[1] x + c[2] x^2 + ...
};

struct FpFactor {
    FpPoly g;       // Monic irreducible factor
    long long mult; // Multiplicity
};

// Reduce a PolyQ to F_p[x] (assume all coefficients are integers;
// clear denominators if needed, then reduce each coefficient mod p).
// Throws if any coefficient has denominator not coprime to p.
inline FpPoly reduce_to_fp(const mathlib::PolyQ& q, long long p) {
    FpPoly f;
    f.p = p;
    f.c.assign(q.coeffs_.size(), 0);
    for (std::size_t i = 0; i < q.coeffs_.size(); ++i) {
        // Get numerator and denominator as mpz_ptr (pointers to the
        // underlying __mpz_struct).  Note: mpq_numref / mpq_denref
        // are C macros in the global namespace (not in mathlib::),
        // so we use them directly without the mathlib:: prefix.
        const mpz_ptr num = mpq_numref(q.coeffs_[i].get());
        const mpz_ptr den = mpq_denref(q.coeffs_[i].get());
        // Denominator must be exactly 1 (i.e., the poly has integer
        // coefficients; PolyQ is over Q, not Z, so we need to verify).
        if (mpz_cmp_ui(den, 1) != 0) {
            throw std::invalid_argument("reduce_to_fp: non-integer coefficient");
        }
        // Reduce numerator mod p.  Use mpz_fdiv_ui to get a
        // non-negative result.
        unsigned long n = mpz_fdiv_ui(num, static_cast<unsigned long>(p));
        f.c[i] = static_cast<long long>(n);
    }
    // Trim trailing zeros
    while (f.c.size() > 1 && f.c.back() == 0) f.c.pop_back();
    return f;
}

// Add two F_p[x] polynomials (mod p).
inline FpPoly fp_add(const FpPoly& a, const FpPoly& b) {
    if (a.p != b.p) throw std::invalid_argument("fp_add: mismatched primes");
    FpPoly r;
    r.p = a.p;
    r.c.assign(std::max(a.c.size(), b.c.size()), 0);
    for (std::size_t i = 0; i < a.c.size(); ++i) {
        r.c[i] = (r.c[i] + a.c[i]) % a.p;
    }
    for (std::size_t i = 0; i < b.c.size(); ++i) {
        r.c[i] = (r.c[i] + b.c[i]) % a.p;
    }
    while (r.c.size() > 1 && r.c.back() == 0) r.c.pop_back();
    return r;
}

// Subtract.
inline FpPoly fp_sub(const FpPoly& a, const FpPoly& b) {
    if (a.p != b.p) throw std::invalid_argument("fp_sub: mismatched primes");
    FpPoly r;
    r.p = a.p;
    r.c.assign(std::max(a.c.size(), b.c.size()), 0);
    for (std::size_t i = 0; i < a.c.size(); ++i) {
        r.c[i] = (r.c[i] + a.c[i]) % a.p;
    }
    for (std::size_t i = 0; i < b.c.size(); ++i) {
        r.c[i] = (r.c[i] - b.c[i]) % a.p;
        if (r.c[i] < 0) r.c[i] += a.p;
    }
    while (r.c.size() > 1 && r.c.back() == 0) r.c.pop_back();
    return r;
}

// Multiply (convolution mod p).
inline FpPoly fp_mul(const FpPoly& a, const FpPoly& b) {
    if (a.p != b.p) throw std::invalid_argument("fp_mul: mismatched primes");
    if (a.c.size() == 0 || b.c.size() == 0) {
        FpPoly r; r.p = a.p; r.c = {0}; return r;
    }
    FpPoly r;
    r.p = a.p;
    r.c.assign(a.c.size() + b.c.size() - 1, 0);
    for (std::size_t i = 0; i < a.c.size(); ++i) {
        for (std::size_t j = 0; j < b.c.size(); ++j) {
            r.c[i + j] = (r.c[i + j] + a.c[i] * b.c[j]) % a.p;
        }
    }
    while (r.c.size() > 1 && r.c.back() == 0) r.c.pop_back();
    return r;
}

// Divide a by b in F_p[x].  Returns (q, r) with a = q*b + r and
// deg(r) < deg(b).  Throws if b is the zero polynomial.
inline std::pair<FpPoly, FpPoly> fp_divmod(const FpPoly& a, const FpPoly& b) {
    if (a.p != b.p) throw std::invalid_argument("fp_divmod: mismatched primes");
    if (b.c.size() == 1 && b.c[0] == 0) {
        throw std::invalid_argument("fp_divmod: division by zero");
    }
    long long p = a.p;
    long long db = static_cast<long long>(b.c.size()) - 1;
    long long da = static_cast<long long>(a.c.size()) - 1;
    if (da < db) return {FpPoly{p, {0}}, a};
    // Compute inverse of leading coefficient of b mod p.
    long long b_lc = b.c.back() % p;
    if (b_lc < 0) b_lc += p;
    // Find b_lc^(-1) mod p.
    long long inv = 0;
    for (long long k = 1; k < p; ++k) {
        if ((b_lc * k) % p == 1) { inv = k; break; }
    }
    FpPoly q; q.p = p; q.c.assign(static_cast<std::size_t>(da - db + 1), 0);
    FpPoly r = a;
    for (long long i = da; i >= db; --i) {
        if (r.c[static_cast<std::size_t>(i)] == 0) continue;
        long long shift = i - db;
        long long qc = (r.c[static_cast<std::size_t>(i)] * inv) % p;
        q.c[static_cast<std::size_t>(shift)] = qc;
        // Subtract qc * x^shift * b from r
        for (long long j = 0; j <= db; ++j) {
            long long rj = r.c[static_cast<std::size_t>(shift + j)]
                          - qc * b.c[static_cast<std::size_t>(j)];
            rj %= p;
            if (rj < 0) rj += p;
            r.c[static_cast<std::size_t>(shift + j)] = rj;
        }
    }
    while (r.c.size() > 1 && r.c.back() == 0) r.c.pop_back();
    while (q.c.size() > 1 && q.c.back() == 0) q.c.pop_back();
    return {q, r};
}

// Evaluate F_p[x] polynomial at a in F_p.  Horner's method.
inline long long fp_eval(const FpPoly& f, long long a) {
    if (f.c.empty()) return 0;
    long long r = f.c.back() % f.p;
    if (r < 0) r += f.p;
    for (long long i = static_cast<long long>(f.c.size()) - 2; i >= 0; --i) {
        r = (r * a + f.c[static_cast<std::size_t>(i)]) % f.p;
        if (r < 0) r += f.p;
    }
    return r;
}

// Greatest common divisor of two F_p[x] polynomials (monic).
inline FpPoly fp_gcd(const FpPoly& a, const FpPoly& b) {
    if (a.p != b.p) throw std::invalid_argument("fp_gcd: mismatched primes");
    FpPoly x = a, y = b;
    while (!(y.c.size() == 1 && y.c[0] == 0)) {
        auto dm = fp_divmod(x, y);
        x = y;
        y = dm.second;
    }
    // Make monic.
    if (x.c.empty() || x.c.back() == 0) {
        FpPoly r; r.p = a.p; r.c = {1}; return r;
    }
    long long inv = 0;
    for (long long k = 1; k < a.p; ++k) {
        if ((x.c.back() * k) % a.p == 1) { inv = k; break; }
    }
    for (auto& ci : x.c) ci = (ci * inv) % a.p;
    return x;
}

// Extended Euclidean algorithm over F_p[x]: returns (g, s, t) with
// s*a + t*b = g = gcd(a,b), g monic. Building block for multifactor
// Hensel lifting (docs/DIRECTION_AND_OPEN_THREADS.md, Item B1) --
// local_polynomial_cofactor (include/adelic/local_field.hpp) uses the
// resulting mod-p Bezout certificate to lift one selected factor
// against its coprime complement, including multiple non-simple ideals.
struct FpExtGcdResult {
    FpPoly g;  // gcd(a, b), monic
    FpPoly s;  // s*a + t*b = g
    FpPoly t;
};

inline FpExtGcdResult fp_extended_gcd(const FpPoly& a, const FpPoly& b) {
    if (a.p != b.p) throw std::invalid_argument("fp_extended_gcd: mismatched primes");
    long long p = a.p;
    FpPoly r0 = a, r1 = b;
    FpPoly s0{p, {1}}, s1{p, {0}};
    FpPoly t0{p, {0}}, t1{p, {1}};
    while (!(r1.c.size() == 1 && r1.c[0] == 0)) {
        auto dm = fp_divmod(r0, r1);
        FpPoly q = dm.first, r2 = dm.second;
        FpPoly s2 = fp_sub(s0, fp_mul(q, s1));
        FpPoly t2 = fp_sub(t0, fp_mul(q, t1));
        r0 = r1; r1 = r2;
        s0 = s1; s1 = s2;
        t0 = t1; t1 = t2;
    }
    if (r0.c.empty() || r0.c.back() == 0) {
        throw std::runtime_error("fp_extended_gcd: gcd is zero (inputs both zero?)");
    }
    long long inv = 0;
    for (long long k = 1; k < p; ++k) {
        if ((r0.c.back() * k) % p == 1) { inv = k; break; }
    }
    for (auto& ci : r0.c) ci = (ci * inv) % p;
    for (auto& ci : s0.c) ci = (ci * inv) % p;
    for (auto& ci : t0.c) ci = (ci * inv) % p;
    return {r0, s0, t0};
}

// Make a F_p[x] polynomial from a monic (x - a) factor.
inline FpPoly fp_linear(long long p, long long a) {
    // (x - a) = -a + 1·x  (in low-first coefficients, the constant
    // term is -a, the leading is 1)
    FpPoly f;
    f.p = p;
    long long ca = ((-a) % p + p) % p;
    f.c = {ca, 1};
    return f;
}

// Multiply two F_p[x] polynomials and reduce modulo `modulus` (all
// arithmetic mod p). The single canonical definition -- this used to
// exist as two separate, identical copies in maximal_order.hpp and
// fp_poly_factor.hpp, which collided once dedekind_factorization.hpp
// started including both.
inline FpPoly fp_mulmod(const FpPoly& a, const FpPoly& b, const FpPoly& modulus) {
    FpPoly prod = fp_mul(a, b);
    auto dm = fp_divmod(prod, modulus);
    return dm.second;
}

// Lift an F_p[x] irreducible factor to Z[x] (with coefficients
// in {0, 1, ..., p-1}, except leading = 1).
inline mathlib::PolyZ lift_fp_factor_to_z(const FpPoly& g) {
    mathlib::PolyZ r;
    r.ensure_size(g.c.size());
    for (std::size_t i = 0; i < g.c.size(); ++i) {
        mathlib::set_si(r.coeff(i), g.c[i]);
    }
    return r;
}

// Reduce a PolyZ (over Z) to F_p[x] directly (no PolyQ round-trip).
inline FpPoly reduce_z_to_fp(const mathlib::PolyZ& z, long long p) {
    FpPoly f;
    f.p = p;
    f.c.assign(z.coeffs_.size(), 0);
    for (std::size_t i = 0; i < z.coeffs_.size(); ++i) {
        unsigned long n = mpz_fdiv_ui(z.coeff(i).get(), static_cast<unsigned long>(p));
        f.c[i] = static_cast<long long>(n);
    }
    while (f.c.size() > 1 && f.c.back() == 0) f.c.pop_back();
    return f;
}

}  // namespace adelic
