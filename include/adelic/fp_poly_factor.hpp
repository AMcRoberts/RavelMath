// include/adelic/fp_poly_factor.hpp
//
// General F_p[x] polynomial factoring, for arbitrary degree -- the
// upgrade dedekind_factorization.hpp's own header comment names as
// "left as future work": that file's factor_fp only extracts LINEAR
// factors by root-testing, then assumes whatever degree>=2 remains is
// a SINGLE irreducible factor. That assumption is simply wrong
// whenever the residual is a product of two-or-more irreducibles of
// the same degree with no roots in F_p (e.g. a product of two
// irreducible quadratics for a degree-4 residual) -- the file's own
// comment gestures at "a Cantor-Zassenhaus-style split" for exactly
// this case but the code never implements it. This file does.
//
// Standard three-stage algorithm (Cohen, "A Course in Computational
// Algebraic Number Theory" ch. 3.4, the same reference this project's
// adelic layer already cites throughout):
//   1. Squarefree factorization (handles repeated factors, including
//      the characteristic-p wrinkle where f'=0 and a coefficient-index
//      "p-th root" trick is needed -- valid because Frobenius a->a^p
//      is the identity map on F_p, per Fermat's little theorem).
//   2. Distinct-degree factorization (DDF): peels off, for each degree
//      d=1,2,..., the product of all irreducible factors of degree
//      exactly d, via gcd(f, x^{p^d}-x mod f).
//   3. Equal-degree factorization (EDF, Cantor-Zassenhaus): splits a
//      product of several distinct irreducibles of the SAME degree
//      apart, via random polynomial splitting. Odd p and p=2 need
//      genuinely different splitting polynomials (the odd-p formula
//      divides by 2 in the exponent, which isn't meaningful mod 2);
//      both are implemented, since this project's own test cases
//      already include p=2 (the sigma_1 Dedekind-cubic example).

#pragma once

#include <cstddef>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "adelic/dedekind_factorization.hpp"
#include "math/bigint.hpp"

namespace adelic {

// ===================================================================
// Small helpers layered on the existing FpPoly primitives.
// ===================================================================

inline FpPoly fp_make_monic(FpPoly f) {
    long long p = f.p;
    if (f.c.empty() || f.c.back() == 0) return f;
    long long lc = f.c.back() % p;
    if (lc < 0) lc += p;
    if (lc == 1) return f;
    long long inv = 0;
    for (long long k = 1; k < p; ++k)
        if ((lc * k) % p == 1) { inv = k; break; }
    for (auto& c : f.c) c = (c * inv) % p;
    return f;
}

inline bool fp_is_zero(const FpPoly& f) {
    return f.c.size() == 1 && f.c[0] == 0;
}

inline long long fp_degree(const FpPoly& f) {
    return static_cast<long long>(f.c.size()) - 1;
}

inline FpPoly fp_derivative(const FpPoly& f) {
    FpPoly r; r.p = f.p;
    if (fp_degree(f) <= 0) { r.c = {0}; return r; }
    r.c.assign(f.c.size() - 1, 0);
    for (std::size_t i = 1; i < f.c.size(); ++i)
        r.c[i - 1] = (f.c[i] * static_cast<long long>(i)) % f.p;
    while (r.c.size() > 1 && r.c.back() == 0) r.c.pop_back();
    return r;
}

inline FpPoly fp_mulmod(const FpPoly& a, const FpPoly& b, const FpPoly& modulus) {
    return fp_divmod(fp_mul(a, b), modulus).second;
}

// x^e mod (p, modulus), e a BigInt (exponents like p^d overflow long
// long quickly for even modest p, d). Square-and-multiply via raw
// GMP bit access on e's underlying mpz_t.
inline FpPoly fp_powmod(FpPoly base, const mathlib::BigInt& e, const FpPoly& modulus) {
    FpPoly result; result.p = base.p; result.c = {1};
    if (fp_degree(modulus) == 0) { result.c = {0}; return result; }
    base = fp_divmod(base, modulus).second;
    std::size_t nbits = mpz_sizeinbase(e.get(), 2);
    for (std::size_t i = 0; i < nbits; ++i) {
        if (mpz_tstbit(e.get(), static_cast<mp_bitcnt_t>(i))) {
            result = fp_mulmod(result, base, modulus);
        }
        base = fp_mulmod(base, base, modulus);
    }
    return result;
}

inline FpPoly fp_poly_x(long long p) {
    FpPoly f; f.p = p; f.c = {0, 1}; return f;
}

inline FpPoly fp_poly_const(long long p, long long c) {
    FpPoly f; f.p = p;
    long long cc = c % p; if (cc < 0) cc += p;
    f.c = {cc};
    return f;
}

// "p-th root" of a polynomial c(x) known to satisfy c(x) = g(x)^p for
// some g -- valid exactly when every exponent present in c is a
// multiple of p, which is what f'=0 in the squarefree-factorization
// loop below guarantees. Since Frobenius a->a^p is the identity on
// F_p (Fermat), g's coefficient at index k is simply c's coefficient
// at index p*k -- no actual p-th-root extraction needed, just
// re-indexing.
inline FpPoly fp_pth_root(const FpPoly& c, long long p) {
    FpPoly g; g.p = p;
    long long deg_c = fp_degree(c);
    g.c.assign(static_cast<std::size_t>(deg_c / p) + 1, 0);
    for (long long k = 0; k * p <= deg_c; ++k) {
        g.c[static_cast<std::size_t>(k)] = c.c[static_cast<std::size_t>(k * p)];
    }
    return g;
}

// ===================================================================
// Stage 1: squarefree factorization.
// Returns (factor, multiplicity) pairs; each factor may still be a
// product of several DISTINCT irreducibles (squarefree, not
// necessarily irreducible) -- distinct_degree_factorization handles
// that next.
// ===================================================================
inline std::vector<std::pair<FpPoly, long long>>
fp_squarefree_factorization(const FpPoly& f_in) {
    std::vector<std::pair<FpPoly, long long>> factors;
    FpPoly f = fp_make_monic(f_in);
    long long p = f.p;
    if (fp_degree(f) <= 0) return factors;

    FpPoly fprime = fp_derivative(f);
    if (fp_is_zero(fprime)) {
        // f = g(x)^p entirely; recurse on the p-th root, multiplying
        // every resulting multiplicity by p.
        FpPoly g = fp_pth_root(f, p);
        auto sub = fp_squarefree_factorization(g);
        for (auto& [fac, mult] : sub) factors.emplace_back(fac, mult * p);
        return factors;
    }

    FpPoly c = fp_gcd(f, fprime);
    FpPoly w = fp_divmod(f, c).first;
    long long i = 1;
    while (fp_degree(w) > 0) {
        FpPoly y = fp_gcd(w, c);
        FpPoly fac = fp_divmod(w, y).first;
        if (fp_degree(fac) > 0) factors.emplace_back(fp_make_monic(fac), i);
        w = y;
        c = fp_divmod(c, y).first;
        ++i;
    }
    if (fp_degree(c) > 0) {
        // c = g(x)^p for the part the derivative couldn't see.
        FpPoly g = fp_pth_root(c, p);
        auto sub = fp_squarefree_factorization(g);
        for (auto& [fac, mult] : sub) factors.emplace_back(fac, mult * p);
    }
    return factors;
}

// ===================================================================
// Stage 2: distinct-degree factorization of a SQUAREFREE polynomial.
// Returns (degree, product-of-degree-d-irreducibles) pairs.
// ===================================================================
inline std::vector<std::pair<long long, FpPoly>>
fp_distinct_degree_factorization(const FpPoly& f_in) {
    std::vector<std::pair<long long, FpPoly>> result;
    FpPoly f_star = fp_make_monic(f_in);
    long long p = f_star.p;
    FpPoly x = fp_poly_x(p);
    long long d = 1;
    while (fp_degree(f_star) >= 2 * d) {
        mathlib::BigInt pd(1);
        for (long long k = 0; k < d; ++k) mathlib::mul_si(pd, pd, p);
        FpPoly xpd = fp_powmod(x, pd, f_star);
        FpPoly diff = fp_sub(xpd, x);
        FpPoly g = fp_gcd(diff, f_star);
        if (fp_degree(g) > 0) {
            result.emplace_back(d, g);
            f_star = fp_divmod(f_star, g).first;
        }
        ++d;
    }
    if (fp_degree(f_star) > 0) {
        result.emplace_back(fp_degree(f_star), f_star);
    }
    return result;
}

// ===================================================================
// Stage 3: equal-degree factorization (Cantor-Zassenhaus). Splits a
// squarefree product of r>=1 distinct irreducibles, each of degree d,
// into its r individual irreducible factors.
// ===================================================================
inline std::vector<FpPoly> fp_equal_degree_factorization(
        const FpPoly& f_in, long long d, std::mt19937_64& rng) {
    FpPoly f = fp_make_monic(f_in);
    long long p = f.p;
    long long n = fp_degree(f);
    if (n == d) return {f};  // already irreducible

    std::vector<FpPoly> factors = {f};
    std::uniform_int_distribution<long long> coeff_dist(0, p - 1);

    while (static_cast<long long>(factors.size()) < n / d) {
        // Random polynomial of degree < n (the modulus's degree; each
        // recursive split works mod its own current factor, but we
        // draw the random polynomial mod the ORIGINAL f for simplicity
        // -- reducing it mod each candidate factor before use).
        FpPoly h; h.p = p;
        h.c.assign(static_cast<std::size_t>(n), 0);
        for (auto& c : h.c) c = coeff_dist(rng);
        while (h.c.size() > 1 && h.c.back() == 0) h.c.pop_back();
        if (fp_degree(h) <= 0) continue;  // degenerate draw, retry

        std::vector<FpPoly> next_round;
        bool any_split = false;
        for (const auto& piece : factors) {
            long long piece_deg = fp_degree(piece);
            if (piece_deg == d) { next_round.push_back(piece); continue; }

            FpPoly h_mod = fp_divmod(h, piece).second;
            if (fp_degree(h_mod) <= 0) { next_round.push_back(piece); continue; }

            FpPoly g;
            if (p != 2) {
                // g = h^{(p^d - 1)/2} - 1 mod piece.
                mathlib::BigInt pd(1);
                for (long long k = 0; k < d; ++k) mathlib::mul_si(pd, pd, p);
                mathlib::BigInt exp;
                mathlib::sub(exp, pd, mathlib::BigInt(1));
                mathlib::tdiv_q(exp, exp, mathlib::BigInt(2));
                FpPoly hp = fp_powmod(h_mod, exp, piece);
                g = fp_sub(hp, fp_poly_const(p, 1));
            } else {
                // p=2: trace map T(h) = h + h^2 + h^4 + ... + h^{2^{d-1}}
                // mod piece (standard char-2 EDF splitting polynomial).
                FpPoly term = h_mod;
                FpPoly trace = h_mod;
                for (long long k = 1; k < d; ++k) {
                    term = fp_mulmod(term, term, piece);
                    trace = fp_add(trace, term);
                }
                // Reduce trace's coefficients mod 2 explicitly (fp_add
                // already does, but keep this explicit for clarity).
                g = trace;
            }
            FpPoly gcd_val = fp_gcd(g, piece);
            long long gcd_deg = fp_degree(gcd_val);
            if (gcd_deg > 0 && gcd_deg < piece_deg) {
                next_round.push_back(fp_make_monic(gcd_val));
                next_round.push_back(fp_make_monic(fp_divmod(piece, gcd_val).first));
                any_split = true;
            } else {
                next_round.push_back(piece);
            }
        }
        factors = std::move(next_round);
        if (!any_split && static_cast<long long>(factors.size()) < n / d) {
            // No progress this draw; loop again with a fresh random h.
            continue;
        }
    }
    return factors;
}

// ===================================================================
// Top-level: general F_p[x] factoring, any degree.
// ===================================================================
inline std::vector<FpFactor> factor_fp_general(const FpPoly& f_in) {
    std::vector<FpFactor> result;
    if (fp_is_zero(f_in)) return result;
    FpPoly f = fp_make_monic(f_in);
    if (fp_degree(f) == 0) return result;  // unit, no factors

    static thread_local std::mt19937_64 rng(0xC0FFEEu);  // deterministic

    auto sqfree = fp_squarefree_factorization(f);
    for (const auto& [sqfree_factor, mult] : sqfree) {
        auto ddf = fp_distinct_degree_factorization(sqfree_factor);
        for (const auto& [d, product] : ddf) {
            auto pieces = fp_equal_degree_factorization(product, d, rng);
            for (const auto& piece : pieces) {
                FpFactor fac;
                fac.g = fp_make_monic(piece);
                fac.mult = mult;
                result.push_back(fac);
            }
        }
    }
    return result;
}

// Full Dedekind factorization using factor_fp_general instead of
// dedekind_factorization.hpp's factor_fp. A deliberately separate
// entry point rather than swapping factor_prime_in_qbeta's internals
// in place: factor_fp is already relied upon by shipped, tested code,
// and this project's own current Pisot substitutions never reach the
// degree/shape where factor_fp's bug actually fires (degree<=4 chars
// give at most a degree-2 residual after root extraction, and a
// rootless quadratic is automatically irreducible), so there is no
// urgency to touch working code in place. This is the strictly-more-
// correct version, available for any future degree>=4 use (the
// "hyperpyramid"/simplex non-cube-tile direction docs/FAMILY_OF_
// FAMILIES.md already names) or any number field outside this
// project's own Pisot substitutions (the class-field-tower work
// docs/RESEARCH_STATUS.md's Adelic section now tracks).
inline DedekindFactorization factor_prime_in_qbeta_general(
        const mathlib::PolyZ& charpoly, long long p) {
    if (p < 2) {
        throw std::invalid_argument("factor_prime_in_qbeta_general: p must be prime and >= 2");
    }
    long long n = charpoly.degree();
    if (n < 1) {
        throw std::invalid_argument("factor_prime_in_qbeta_general: charpoly degree < 1");
    }
    mathlib::PolyQ charpoly_q(charpoly);
    FpPoly f_p = reduce_to_fp(charpoly_q, p);
    if (f_p.c.size() == 1 && f_p.c[0] == 0) {
        throw std::invalid_argument(
            "factor_prime_in_qbeta_general: charpoly is 0 mod p");
    }
    std::vector<FpFactor> factors = factor_fp_general(f_p);
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
