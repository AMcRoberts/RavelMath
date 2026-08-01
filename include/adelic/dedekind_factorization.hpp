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
// arithmetic for the F_p reductions). factor_fp below (any degree,
// any prime) is fp_poly_factor.hpp's factor_fp_general under the
// name every existing caller already uses -- squarefree factorization
// + distinct-degree factorization + Cantor-Zassenhaus equal-degree
// factorization (Cohen ch. 3.4). An earlier version of this file
// implemented only linear-factor (root) extraction and assumed any
// degree>=2 residual was a single irreducible, which is wrong
// whenever that residual is a product of two-or-more irreducibles of
// the same degree with no roots in F_p (demonstrated concretely:
// x^4+1 mod 5 = (x^2+2)(x^2+3), which the old code reported as one
// wrong quartic factor, and which propagated to factor_prime_in_qbeta
// wrongly reporting one prime above 5 with residue degree 4 instead
// of two primes with residue degree 2 each -- see
// tests/fp_poly_factor_test.cpp for the demonstration and reconstruc-
// tion-based verification). This project's own degree<=4 Pisot
// substitutions never reach the shape where that bug fired (their
// residuals are at most degree 2, and a rootless quadratic is
// automatically irreducible), so nothing that shipped was ever wrong
// in practice -- but factor_fp itself is now the corrected algorithm,
// not a parallel one, so every caller (direct or via
// factor_prime_in_qbeta) gets the fix with no call-site changes.
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

#include "adelic/fp_poly.hpp"
#include "adelic/fp_poly_factor.hpp"
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

// Factor f in F_p[x], any degree, any prime -- see this file's own
// header comment above for what "any degree" fixes relative to an
// earlier, degree<=4-only version of this function. Implemented in
// fp_poly_factor.hpp (squarefree + distinct-degree + Cantor-
// Zassenhaus equal-degree factorization); re-exposed under this name
// here so every existing caller (this file's factor_prime_in_qbeta,
// and the direct callers in local_field.hpp, maximal_order.hpp,
// coincidence_and_property_f.hpp) gets the corrected algorithm with
// no call-site changes.
inline std::vector<FpFactor> factor_fp(const FpPoly& f) {
    return factor_fp_general(f);
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
