// include/adelic/ideal_arithmetic.hpp
//
// §3.1b of docs/ADELIC_TILING_PLAN.md: an independent, from-scratch
// cross-checker for dedekind_factorization.hpp's claimed
// factorization of (p), built via a genuinely different algorithm
// (direct ideal-lattice arithmetic in Hermite Normal Form) rather
// than a second copy of Dedekind's criterion.  Agreement between
// the two independently-built pieces of code is the actual evidence
// a factorization is right -- no external CAS, no unverifiable
// "I'm pretty sure this is correct."
//
// Method (Cohen, "A Course in Computational Algebraic Number
// Theory", §2.4.5 for HNF; the ideal-arithmetic layer on top is
// standard):
//
//   Represent Z[β] as Z^n via the power basis (1, β, ..., β^{n-1})
//   (the same representation dedekind_factorization.hpp's
//   mathlib::PolyZ / ModPolyRing convention already uses).  An
//   ideal of Z[β] is then a rank-n sublattice of Z^n, represented by
//   an n×n integer matrix in Hermite Normal Form (HNF) -- the
//   lattice-arithmetic analogue of row-reduced echelon form, unique
//   for a given lattice.  Uniqueness is exactly what makes HNF
//   usable as an exact equality test.
//
// Pieces:
//   1. HNF reduction of a spanning set of Z^n vectors down to the
//      canonical n-vector basis.
//   2. Ideal from generators: {β^j · γ_i : 0<=j<n} for each
//      generator γ_i, HNF-reduced.
//   3. Ideal multiplication: all n² pairwise products of basis
//      vectors (via ring multiplication mod the charpoly), HNF-reduced.
//   4. Ideal power: repeated multiplication.
//   5. Ideal equality: HNF bases identical, entry for entry.
//
// The cross-check: take Dedekind's claimed (p) = ∏ p_i^{e_i} with
// each p_i = (p, g_i(β)); build each p_i's HNF basis, raise to e_i,
// multiply all together, and check the result equals the HNF basis
// of (p) = p·Z[β] (trivially p times the identity matrix).

#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"

namespace adelic {

using IntVec = std::vector<mathlib::BigInt>;
using IntCols = std::vector<IntVec>;  // list of columns, each length n

// An ideal of Z[β], represented by its canonical HNF basis: n
// columns, each a length-n integer vector (coordinates in the power
// basis 1, β, ..., β^{n-1}).
struct IdealHNF {
    long long n;
    IntCols basis;  // basis[j] is column j; basis[j][i] = 0 for i > j
                     // (upper-triangular-by-column convention), and
                     // 0 <= basis[j][i] < basis[i][i] for i < j
                     // (canonical reduced form).
};

// ===================================================================
// Small vector helpers.
// ===================================================================
inline IntVec vec_zero(long long n) {
    IntVec v(static_cast<std::size_t>(n));
    for (auto& e : v) mathlib::set_si(e, 0);
    return v;
}
inline IntVec vec_add(const IntVec& a, const IntVec& b) {
    IntVec r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) mathlib::add(r[i], a[i], b[i]);
    return r;
}
inline IntVec vec_sub(const IntVec& a, const IntVec& b) {
    IntVec r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) mathlib::sub(r[i], a[i], b[i]);
    return r;
}
inline IntVec vec_scale(const IntVec& a, const mathlib::BigInt& c) {
    IntVec r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) mathlib::mul(r[i], c, a[i]);
    return r;
}

// ===================================================================
// HNF reduction (Cohen §2.4.5): reduce a spanning set of >= n
// full-rank columns (each length n) to the canonical n-column HNF
// basis.  Throws if the input does not have full rank n.
// ===================================================================
inline IntCols hnf_reduce(IntCols cols, long long n) {
    std::size_t un = static_cast<std::size_t>(n);
    IntCols pivots(un);
    IntCols working = std::move(cols);

    for (std::size_t r = 0; r < un; ++r) {
        // Combine columns pairwise (via extended gcd on their row-r
        // entry) until at most one working column has a nonzero
        // entry at row r.  Every combination step is a unimodular
        // (invertible-over-Z) transform, so it never changes the
        // lattice spanned.
        for (;;) {
            long long idx1 = -1, idx2 = -1;
            for (std::size_t k = 0; k < working.size(); ++k) {
                if (mathlib::sgn(working[k][r]) != 0) {
                    if (idx1 < 0) idx1 = static_cast<long long>(k);
                    else { idx2 = static_cast<long long>(k); break; }
                }
            }
            if (idx2 < 0) break;
            IntVec& c1 = working[static_cast<std::size_t>(idx1)];
            IntVec& c2 = working[static_cast<std::size_t>(idx2)];
            const mathlib::BigInt& a = c1[r];
            const mathlib::BigInt& b = c2[r];
            mathlib::BigInt g, x, y;
            mpz_gcdext(g.get(), x.get(), y.get(), a.get(), b.get());
            // new_c1 = x*c1 + y*c2 (entry g at row r)
            IntVec new_c1 = vec_add(vec_scale(c1, x), vec_scale(c2, y));
            // new_c2 = (a/g)*c2 - (b/g)*c1 (entry 0 at row r; standard
            // unimodular completion so the pair spans the same
            // sublattice as before)
            mathlib::BigInt ag, bg;
            mathlib::divexact(ag, a, g);
            mathlib::divexact(bg, b, g);
            IntVec new_c2 = vec_sub(vec_scale(c2, ag), vec_scale(c1, bg));
            c1 = new_c1;
            c2 = new_c2;
        }
        long long pividx = -1;
        for (std::size_t k = 0; k < working.size(); ++k) {
            if (mathlib::sgn(working[k][r]) != 0) { pividx = static_cast<long long>(k); break; }
        }
        if (pividx < 0) {
            throw std::runtime_error("hnf_reduce: input spanning set is not full rank");
        }
        IntVec pivot = working[static_cast<std::size_t>(pividx)];
        if (mathlib::sgn(pivot[r]) < 0) {
            for (auto& e : pivot) mathlib::neg(e);
        }
        pivots[r] = pivot;
        working.erase(working.begin() + pividx);
    }

    // Canonical reduction: by construction, pivots[j][i] = 0 for all
    // i < j already (established incrementally during the main loop
    // above -- each pivot is drawn from a working set that has
    // already been zeroed at every earlier row).  The entries that
    // are NOT yet canonical are pivots[j][i] for i > j (rows handled
    // in LATER iterations, which never touch pivots[j] again once
    // it's fixed) -- reduce those modulo the corresponding later
    // pivot's diagonal entry, per Cohen's HNF convention.
    for (std::size_t j = 0; j + 1 < un; ++j) {
        for (std::size_t i = j + 1; i < un; ++i) {
            if (mathlib::sgn(pivots[i][i]) == 0) continue;
            mathlib::BigInt q;
            mpz_fdiv_q(q.get(), pivots[j][i].get(), pivots[i][i].get());
            pivots[j] = vec_sub(pivots[j], vec_scale(pivots[i], q));
        }
    }
    return pivots;
}

// ===================================================================
// The whole ring Z[β] (the "unit ideal"): standard power basis,
// i.e. the n×n identity matrix.
// ===================================================================
inline IdealHNF whole_ring(long long n) {
    IdealHNF r;
    r.n = n;
    r.basis.assign(static_cast<std::size_t>(n), vec_zero(n));
    for (long long i = 0; i < n; ++i) {
        mathlib::set_si(r.basis[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)], 1);
    }
    return r;
}

// (p) = p * Z[β]: p times the identity.
inline IdealHNF principal_int_ideal(long long p, long long n) {
    IdealHNF r = whole_ring(n);
    for (auto& col : r.basis) {
        for (auto& e : col) mathlib::mul_si(e, e, p);
    }
    return r;
}

// ===================================================================
// Ring multiplication mod charpoly (charpoly monic, so ordinary
// PolyZ divmod gives an exact reduction -- no denominators appear).
// ===================================================================
inline mathlib::PolyZ ring_mul(const mathlib::PolyZ& a, const mathlib::PolyZ& b,
                                const mathlib::PolyZ& charpoly) {
    mathlib::PolyZ prod = a * b;
    if (prod.degree() < charpoly.degree()) return prod;
    return mathlib::divmod(prod, charpoly).r;
}

inline mathlib::PolyZ vec_to_polyz(const IntVec& v) {
    mathlib::PolyZ p;
    p.ensure_size(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) mathlib::set(p.coeff(i), v[i]);
    return p;
}
inline IntVec polyz_to_vec(const mathlib::PolyZ& p, long long n) {
    IntVec v = vec_zero(n);
    for (std::size_t i = 0; i < v.size() && i < p.coeffs_.size(); ++i) {
        mathlib::set(v[i], p.coeff(i));
    }
    return v;
}

// ===================================================================
// Ideal from generators: {β^j · γ_i : 0 <= j < n, for each generator
// γ_i}, HNF-reduced.
// ===================================================================
inline IdealHNF ideal_from_generators(const std::vector<mathlib::PolyZ>& generators,
                                       const mathlib::PolyZ& charpoly, long long n) {
    IntCols cols;
    cols.reserve(generators.size() * static_cast<std::size_t>(n));
    for (const auto& gamma : generators) {
        mathlib::PolyZ beta_j;
        beta_j.ensure_size(1);
        mathlib::set_si(beta_j.coeff(0), 1);  // beta^0 = 1
        for (long long j = 0; j < n; ++j) {
            mathlib::PolyZ prod = ring_mul(beta_j, gamma, charpoly);
            cols.push_back(polyz_to_vec(prod, n));
            // beta_j *= beta  (i.e. multiply by x, then reduce)
            mathlib::PolyZ x;
            x.ensure_size(2);
            mathlib::set_si(x.coeff(1), 1);
            beta_j = ring_mul(beta_j, x, charpoly);
        }
    }
    IdealHNF result;
    result.n = n;
    result.basis = hnf_reduce(std::move(cols), n);
    return result;
}

// Convenience: the prime ideal (p, g(β)) from a Dedekind PrimeIdeal.
inline IdealHNF ideal_from_prime(const PrimeIdeal& pi, long long n,
                                  const mathlib::PolyZ& charpoly) {
    mathlib::PolyZ p_const;
    p_const.ensure_size(1);
    mathlib::set_si(p_const.coeff(0), pi.p);
    std::vector<mathlib::PolyZ> gens = {p_const, pi.g};
    return ideal_from_generators(gens, charpoly, n);
}

// ===================================================================
// Ideal multiplication: all n^2 pairwise products of basis vectors,
// HNF-reduced.
// ===================================================================
inline IdealHNF ideal_mul(const IdealHNF& I, const IdealHNF& J,
                           const mathlib::PolyZ& charpoly) {
    long long n = I.n;
    IntCols cols;
    cols.reserve(static_cast<std::size_t>(n * n));
    for (const auto& bi : I.basis) {
        mathlib::PolyZ pi = vec_to_polyz(bi);
        for (const auto& bj : J.basis) {
            mathlib::PolyZ pj = vec_to_polyz(bj);
            mathlib::PolyZ prod = ring_mul(pi, pj, charpoly);
            cols.push_back(polyz_to_vec(prod, n));
        }
    }
    IdealHNF result;
    result.n = n;
    result.basis = hnf_reduce(std::move(cols), n);
    return result;
}

// Ideal power via repeated multiplication.  e = 0 gives the whole ring.
inline IdealHNF ideal_pow(const IdealHNF& I, long long e, const mathlib::PolyZ& charpoly) {
    IdealHNF result = whole_ring(I.n);
    for (long long k = 0; k < e; ++k) {
        result = ideal_mul(result, I, charpoly);
    }
    return result;
}

// ===================================================================
// Ideal equality: HNF bases identical, entry for entry.  Exact,
// unambiguous -- no floating point or heuristic anywhere.
// ===================================================================
inline bool ideal_equal(const IdealHNF& I, const IdealHNF& J) {
    if (I.n != J.n) return false;
    for (std::size_t j = 0; j < I.basis.size(); ++j) {
        for (std::size_t i = 0; i < I.basis[j].size(); ++i) {
            if (mathlib::cmp(I.basis[j][i], J.basis[j][i]) != 0) return false;
        }
    }
    return true;
}

// ===================================================================
// The actual cross-check: does the Dedekind factorization's claimed
// (p) = prod p_i^{e_i} equal (p) = p*Z[beta], verified by direct,
// independent ideal-lattice multiplication?
//
// NOTE: this checks the ALGEBRAIC IDENTITY prod p_i^{e_i} = (p),
// which is necessary but, by itself, does not re-derive p-maximality
// -- see the important caveat in cross_check_dedekind_factorization's
// doc comment below for why a passing cross-check on a NON-maximal
// order is still possible in principle (though for the specific
// non-monogenic test case in ideal_arithmetic_test.cpp, it does in
// fact also fail, which is a useful second confirmation, not a
// logical guarantee).
// ===================================================================
inline bool cross_check_dedekind_factorization(const DedekindFactorization& fac,
                                                 const mathlib::PolyZ& charpoly,
                                                 long long n) {
    IdealHNF product = whole_ring(n);
    for (const auto& pi : fac.prime_ideals) {
        IdealHNF pideal = ideal_from_prime(pi, n, charpoly);
        IdealHNF ppow = ideal_pow(pideal, pi.e, charpoly);
        product = ideal_mul(product, ppow, charpoly);
    }
    IdealHNF expected = principal_int_ideal(fac.p, n);
    return ideal_equal(product, expected);
}

}  // namespace adelic
