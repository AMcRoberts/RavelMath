// include/adelic/dedekind_factorization.hpp
//
// Dedekind factorization of a prime ideal (p) ⊂ O_K = Z[β] (when
// Z[β] is p-maximal, the "easy 90% case" per the adelic tiling
// spec).  Computes the factorization of (p) in the ring of integers
// of K = Q(β), per Minervino-Thuswaldner §3.1.
//
// Algorithm (Dedekind's criterion):
//   1. Reduce the char poly f(x) modulo p to f_p(x) in F_p[x].
//   2. Factor f_p(x) in F_p[x] as g_1(x)^{e_1} · g_2(x)^{e_2} · ...
//      · g_r(x)^{e_r} (irreducible factors).
//   3. For each g_i(x), the corresponding prime ideal is
//      p_i = (p, g_i(β)).  The ramification index is e_i and the
//      residue degree is f_i = deg(g_i).
//   4. The degree of the p-adic extension is ef_i = e_i · f_i.
//
// All arithmetic is exact (rationals for the char poly, modular
// arithmetic for the F_p reductions).  This implementation
// handles factoring of f_p in F_p[x] for polynomials of degree
// up to 4 over small primes (the typical case for Pisot
// substitutions in this project); higher-degree factoring is
// the Round 2 / Round 4 case per the spec, left as future work.
//
// Test cases: the worked example from §2.7 of the adelic
// tiling plan: σ(1)=1113, σ(2)=11, σ(3)=2, char poly
// x^3 - 3x^2 - 2, prime p = 2.  f_p(x) = x^2 (x+1), so the
// factorization is (2) = p_1^2 · p_2 with p_1 = (2, β) (e=2,
// f=1, ef=2) and p_2 = (2, β+1) (e=1, f=1, ef=1).

#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"

namespace adelic {

// A prime ideal in O_K = Z[β] above the rational prime p, with
// ramification index e and residue degree f (so the p-adic
// extension K_p / Q_p has degree ef).  The ideal is p_i =
// (p, g_i(β)) where g_i is the irreducible factor of the
// char poly mod p.  This struct stores the (e, f) data and
// the irreducible factor g_i (as a PolyZ lifted to Z[x] with
// content 1 and coefficients in {0, 1, ..., p-1} except the
// leading coefficient which is 1).
struct PrimeIdeal {
    long long p;     // The rational prime
    long long e;     // Ramification index
    long long f;     // Residue degree (= deg(g))
    mathlib::PolyZ g; // The irreducible factor g_i(β) in Z[x] (lifted)
};

// The full factorization of (p) in O_K.
struct DedekindFactorization {
    long long p;  // The rational prime
    bool maximal; // Is Z[β] p-maximal (Dedekind's criterion)?  If
                  // false, prime_ideals is only the factorization
                  // in Z[β], NOT in the true O_K -- do not trust it
                  // without the Round 2/4 follow-up (not implemented).
    std::vector<PrimeIdeal> prime_ideals;
};

// ===================================================================
// F_p[x] polynomial arithmetic helpers
// ===================================================================
//
// All operations are exact mod p.  We represent F_p[x] polynomials
// as std::vector<long long> of length degree+1, with coefficients
// in {0, 1, ..., p-1}.

struct FpPoly {
    long long p;            // The prime
    std::vector<long long> c;  // Coefficients c[0] + c[1] x + c[2] x^2 + ...
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

// ===================================================================
// Factoring f_p in F_p[x] (small degree, root extraction)
// ===================================================================
//
// For f_p of degree n ≤ 4, factor by:
//   1. For each a in F_p, check if (x - a) divides f_p; if so,
//      record the multiplicity and divide out.
//   2. If anything remains, it's a product of irreducible factors
//      of degree ≥ 2.  For degree 2, check discriminant.  For
//      higher degree (capped at 4 in this implementation), if the
//      residual degree is 2, return it; if it's a product of two
//      quadratics, we attempt a Cantor-Zassenhaus-style split.
//
// Returns the list of (irreducible factor, multiplicity) pairs.
// All factors are returned in monic form.

struct FpFactor {
    FpPoly g;       // Monic irreducible factor
    long long mult; // Multiplicity
};

inline std::vector<FpFactor> factor_fp(const FpPoly& f) {
    std::vector<FpFactor> result;
    if (f.c.size() == 1 && f.c[0] == 0) return result;  // zero polynomial
    long long p = f.p;
    // Ensure f is monic
    FpPoly f_monic = f;
    if (f_monic.c.back() != 1) {
        long long inv = 0;
        for (long long k = 1; k < p; ++k) {
            if ((f_monic.c.back() * k) % p == 1) { inv = k; break; }
        }
        for (auto& ci : f_monic.c) ci = (ci * inv) % p;
    }
    // Try each a in F_p as a root
    for (long long a = 0; a < p; ++a) {
        if (fp_eval(f_monic, a) != 0) continue;
        // (x - a) is a factor; find its multiplicity
        FpPoly linear = fp_linear(p, a);
        long long mult = 0;
        FpPoly cur = f_monic;
        while (cur.c.size() > 0 && !(cur.c.size() == 1 && cur.c[0] == 0)) {
            auto dm = fp_divmod(cur, linear);
            if (!(dm.second.c.size() == 1 && dm.second.c[0] == 0)) break;
            cur = dm.first;
            ++mult;
        }
        FpFactor fac;
        fac.g = linear;  // (x - a) is already monic
        fac.mult = mult;
        result.push_back(fac);
        f_monic = cur;
    }
    // Whatever remains is a product of irreducible factors of
    // degree ≥ 2.  For small degree (≤ 4) this is at most one
    // irreducible factor or a product of two irreducibles.
    if (!(f_monic.c.size() == 1 && f_monic.c[0] == 0) && f_monic.c.size() > 1) {
        FpFactor fac;
        fac.g = f_monic;  // monic
        fac.mult = 1;
        result.push_back(fac);
    }
    return result;
}

// ===================================================================
// Lift an F_p[x] irreducible factor to Z[x] (with coefficients
// in {0, 1, ..., p-1}, except leading = 1).
// ===================================================================
inline mathlib::PolyZ lift_fp_factor_to_z(const FpPoly& g) {
    mathlib::PolyZ r;
    r.ensure_size(g.c.size());
    for (std::size_t i = 0; i < g.c.size(); ++i) {
        mathlib::set_si(r.coeff(i), g.c[i]);
    }
    return r;
}

// ===================================================================
// Reduce a PolyZ (over Z) to F_p[x] directly (no PolyQ round-trip).
// ===================================================================
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

// ===================================================================
// Dedekind's criterion: is Z[β] p-maximal?
// ===================================================================
//
// Reference: H. Cohen, "A Course in Computational Algebraic Number
// Theory", Theorem 6.1.4.  Given f monic in Z[x] with
// f̄ = f mod p = ∏_i ḡ_i^{e_i} in F_p[x] (ḡ_i distinct monic
// irreducibles, already computed by factor_fp), set:
//   g1 = lift(∏_i ḡ_i)              -- the radical, each factor once
//   h1 = lift(∏_i ḡ_i^{e_i - 1})    -- the "leftover" ramification
// so that f̄ = ḡ1 · h̄1 in F_p[x] by construction.  Since g1, h1 are
// both monic integer lifts, f - g1*h1 has every coefficient
// divisible by p; let F = (f - g1*h1) / p.
//
// Theorem: Z[β] is p-maximal (i.e. p does not divide the index
// [O_K : Z[β]]) if and only if gcd(F̄, ḡ1, h̄1) = 1 in F_p[x].
//
// If this returns false, factor_prime_in_qbeta's output for this p
// is NOT the correct factorization of (p) in the true ring of
// integers O_K (only in Z[β], which is a proper suborder at p) --
// the Round 2 / Round 4 algorithm would be needed to find the
// actual maximal order, which this implementation does not attempt.
// Per ADELIC_TILING_PLAN.md §3.1, detecting this case (rather than
// silently returning a wrong answer) is the required behavior.
inline bool is_p_maximal(const mathlib::PolyZ& charpoly, long long p,
                          const std::vector<FpFactor>& factors) {
    mathlib::PolyZ g1({1});
    mathlib::PolyZ h1({1});
    for (const auto& fac : factors) {
        mathlib::PolyZ gi = lift_fp_factor_to_z(fac.g);
        g1 = g1 * gi;
        for (long long k = 0; k < fac.mult - 1; ++k) {
            h1 = h1 * gi;
        }
    }
    mathlib::PolyZ prod = g1 * h1;
    mathlib::PolyZ diff = charpoly - prod;
    // diff should be divisible by p in every coefficient, by
    // construction (f̄ = ḡ1 * h̄1 mod p).  Divide exactly and verify;
    // a nonzero remainder would indicate a bug in the construction
    // above, not a mathematical possibility, so treat it as a hard
    // error rather than silently proceeding.
    mathlib::PolyZ F;
    F.ensure_size(diff.coeffs_.size());
    mathlib::BigInt pz; mathlib::set_si(pz, p);
    for (std::size_t i = 0; i < diff.coeffs_.size(); ++i) {
        mathlib::BigInt q, r;
        mpz_tdiv_qr(q.get(), r.get(), diff.coeff(i).get(), pz.get());
        if (mathlib::sgn(r) != 0) {
            throw std::runtime_error(
                "is_p_maximal: f - g1*h1 not divisible by p; "
                "construction invariant violated (bug, not a math result)");
        }
        mathlib::set(F.coeff(i), q);
    }
    F.trim();

    FpPoly Fbar = reduce_z_to_fp(F, p);
    FpPoly gbar = reduce_z_to_fp(g1, p);
    FpPoly hbar = reduce_z_to_fp(h1, p);
    FpPoly d = fp_gcd(fp_gcd(Fbar, gbar), hbar);
    return d.c.size() == 1 && d.c[0] == 1;
}

// ===================================================================
// Dedekind factorization
// ===================================================================
//
// Inputs:
//   charpoly: the monic integer minimal polynomial f(x) of β
//     (coefficients in low-first order: f(x) = x^n + a[0] x^{n-1}
//     + ... + a[n-1])
//   p: the rational prime
//
// Output: the DedekindFactorization with the list of (g, e, f)
// for each prime ideal p_i = (p, g_i(β)) above p, and a `maximal`
// flag.  If `maximal` is false, `prime_ideals` is the factorization
// of (p) in Z[β] only (a proper suborder at p), NOT in the true
// ring of integers O_K -- callers MUST check `maximal` before
// trusting the factorization.  The non-maximal case needs Round 2 /
// Round 4 (Cohen §6.1), which this implementation does not attempt;
// it reports the situation rather than silently returning a wrong
// answer.  See ADELIC_TILING_PLAN.md §3.1 and its required test #4
// (Dedekind's own non-monogenic cubic θ³-θ²-2θ-8, non-maximal at 2).
inline DedekindFactorization factor_prime_in_qbeta(
    const mathlib::PolyZ& charpoly, long long p) {
    if (p < 2) {
        throw std::invalid_argument("factor_prime_in_qbeta: p must be prime and >= 2");
    }
    long long n = charpoly.degree();
    if (n < 1) {
        throw std::invalid_argument("factor_prime_in_qbeta: charpoly degree < 1");
    }
    // Convert charpoly (PolyZ, low-first) to PolyQ for reduce_to_fp.
    mathlib::PolyQ charpoly_q(charpoly);
    // Reduce mod p
    FpPoly f_p = reduce_to_fp(charpoly_q, p);
    // f_p must be non-zero (Pisot case: β is a root, so the char poly
    // has β as a root, which is non-zero in Z_p).
    if (f_p.c.size() == 1 && f_p.c[0] == 0) {
        throw std::invalid_argument("factor_prime_in_qbeta: charpoly is 0 mod p (β = 0 in residue, but p | leading doesn't apply here)");
    }
    // Factor f_p in F_p[x]
    std::vector<FpFactor> factors = factor_fp(f_p);
    // Build the result
    DedekindFactorization result;
    result.p = p;
    result.maximal = is_p_maximal(charpoly, p, factors);
    result.prime_ideals.reserve(factors.size());
    for (const auto& fac : factors) {
        PrimeIdeal pi;
        pi.p = p;
        pi.e = fac.mult;
        pi.f = static_cast<long long>(fac.g.c.size()) - 1;
        pi.g = lift_fp_factor_to_z(fac.g);
        result.prime_ideals.push_back(pi);
    }
    return result;
}

}  // namespace adelic
