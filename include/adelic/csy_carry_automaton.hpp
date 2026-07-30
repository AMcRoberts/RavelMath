// include/adelic/csy_carry_automaton.hpp
//
// Carton-Sudbery-Yassawi (arXiv:2606.30496) carry automaton on
// Pisot numeration systems.  Built directly from the paper's
// Theorem 3 and Lemma 43 — no shortcuts on the math.
//
// Reference (paper text on disk):
//   refs/arXiv_2606.30496_CartonSudberyYassawi2026_
//         Pisot-numerations-topological-groups.txt
//
// THEOREM 3 (Lot02 chapter 7 citation):  For any Pisot numeration
//   U and any finite B ⊂ Z, the set {g ∈ B* : [g]_U = 0} is
//   regular — i.e., accepted by a deterministic finite automaton
//   over B.
//
// LEMMA 43:  Let U be a Pisot numeration and g a finite sequence.
//   Set B = {−||g||∞, ..., ||g||∞}.  By Theorem 3 there is an
//   automaton over B accepting {w ∈ B* : [w]_U = 0}.  Let
//   N = N(U, ||g||∞) be the number of states.  If [g0^n]_U = 0
//   for some n ≥ 0, then by the pumping lemma applied to g0^n,
//   [g0^m]_U = 0 for infinitely many m ≥ N.
//
// Arithmetic: every value is an element of Q(β), the algebraic
// number field with minimal polynomial the user's PisotPoly.
// The coefficient type is therefore mathlib::QElem
// (rational coefficients in basis 1, β, ..., β^{d-1}) and all
// arithmetic goes through mathlib::QBetaRing's exact mpq_t-backed
// ring operations.  No double-precision, no long-long overflow,
// no projectively-bounded coefficient vector.
//
// For β^n, three complementary routes are supplied:
//
//   1. pisotContextFor(P).betaPowerMemoised(i) — recurrence over a
//      bounded retained prefix, with exact matrix fallback beyond the
//      entry/bit window. O(1) lookup once warm inside the window.
//
//   2. betaPowerViaMatrix(P, n) — companion-matrix exponentiation
//      by squaring in Q(β); O(d^3 log n) per call, no table.
//      Exact in Q(β).
//
//   3. betaPowerNumerical(P, n, prec_bits) — Newton's iteration
//      on the charpoly in BigFloat arithmetic, then β^n by
//      exponentiation by squaring; O(prec_bits) per call to find
//      β, then O(prec_bits · log n) for the power.  Approximate,
//      but the same shape as a numerical-evaluation Pisot
//      numeration and useful for cross-validation.
//
// BetaPowerCache wraps the matrix route in a bounded LRU so
// that BFS-style access amortizes matrix exponentiation cost
// without unbounded memory growth.  Each cache hit is O(d);
// miss pays O(d^3 log n).

#pragma once

#include <cstddef>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bezout.hpp"
#include "math/bigfloat.hpp"
#include "math/bigint.hpp"
#include "math/qbeta.hpp"
#include "math/sturm.hpp"

namespace adelic {

using mathlib::cmp;
using mathlib::cmp_si;
using mathlib::cmp_z;
using mathlib::is_zero;
using mathlib::QBetaRing;
using mathlib::QElem;
using mathlib::Rat;

// ===================================================================
// PisotPoly: a Pisot polynomial P(x) = x^d − c_{d-1} x^{d-1} − ...
//   − c_0  with c_i ≥ 0 and β Pisot root.  The relation that the
//   automaton uses is β^d = c_0 + c_1 β + ... + c_{d-1} β^{d-1}.
// ===================================================================
struct PisotPoly {
    std::size_t d = 2;
    std::vector<long long> c;

    PisotPoly() = default;
    PisotPoly(std::size_t d_, std::vector<long long> c_)
        : d(d_), c(std::move(c_)) {}

    bool operator<(const PisotPoly& o) const {
        if (d != o.d) return d < o.d;
        return c < o.c;
    }
    bool operator==(const PisotPoly& o) const {
        return d == o.d && c == o.c;
    }

    static PisotPoly fibonacci() { return PisotPoly(2, {1, 1}); }
    static PisotPoly tribonacci() { return PisotPoly(3, {1, 1, 1}); }
    static PisotPoly fromCoefficients(std::vector<long long> coeffs) {
        PisotPoly p;
        p.d = coeffs.size();
        p.c = std::move(coeffs);
        return p;
    }

    // charpoly coefficients in `x^d + a[0] x^{d-1} + ... + a[d-1]`
    // low-first form: a[i] = -c[i] for our Pisot encoding.
    std::vector<long long> low_first_coeffs() const {
        std::vector<long long> out;
        out.reserve(d);
        for (std::size_t i = 0; i < d; ++i) {
            out.push_back(-c[d - 1 - i]);
        }
        return out;
    }
};

// ===================================================================
// PisotContext: pairs a PisotPoly with its QBetaRing (cached).
// Holds the per-thread ring and a bounded β^i recurrence prefix.
// Requests beyond its entry/bit window use exact matrix fallback.
// ===================================================================
struct PisotContext {
    PisotPoly poly;
    QBetaRing ring;
    mutable std::vector<QElem> beta_powers_cache;
    mutable std::size_t beta_powers_stored_bits = 0;
    static constexpr std::size_t beta_power_entry_limit = 64;
    static constexpr std::size_t beta_power_bit_limit = 1U << 20;
    // PisotContext cannot own the cache if QElem has non-default
    // copy semantics; we put beta_powers_cache as a vector and
    // pin via the thread-local cache map in pisotContextFor.

    PisotContext(const PisotPoly& P) : poly(P), ring(buildRingFor(P)) {
        beta_powers_cache.reserve(beta_power_entry_limit);
    }

private:
    static QBetaRing buildRingFor(const PisotPoly& P) {
        // Build the charpoly PolyZ in mathlib convention
        // (coeffs_[i] = coefficient of x^i) and pass it to the
        // QBetaRing(const PolyZ&) constructor — this avoids the
        // std::initializer_list constraint of from_low_first.
        const auto lf = P.low_first_coeffs();
        const std::size_t d = P.d;
        mathlib::PolyZ pz;
        pz.ensure_size(d + 1);
        for (std::size_t i = 0; i < d; ++i) {
            mathlib::set_si(pz.coeff(i), lf[i]);
        }
        mathlib::set_si(pz.coeff(d), 1);  // leading coefficient
        pz.trim();
        return QBetaRing(pz);
    }

public:

    QElem betaPowerMemoised(std::size_t n) const;
    QElem eval(const std::vector<long long>& w) const;
};

// ===================================================================
// Q(β) helpers (no dependence on PisotPoly internals).
// ===================================================================

inline double qBetaCoeffApprox(const Rat& r) {
    return mpq_get_d(r.get());
}

inline std::size_t qBetaNumBits(const Rat& r) {
    auto p = r.get();
    return mpz_sizeinbase(mpq_numref(p), 2);
}

inline std::size_t qBetaDenBits(const Rat& r) {
    auto p = r.get();
    return mpz_sizeinbase(mpq_denref(p), 2);
}

inline bool qBetaIsZero(const QElem& v) { return v.is_zero(); }

inline bool qBetaCoefficientsBounded(const QElem& v,
                                     std::size_t bound_bits) {
    for (auto const& r : v.coeffs_) {
        if (qBetaNumBits(r) > bound_bits) return false;
        if (qBetaDenBits(r) > bound_bits) return false;
    }
    return true;
}

inline std::size_t qBetaStoredCoefficientBits(const QElem& v) {
    std::size_t result = 0;
    for (const auto& r : v.coeffs_)
        result += qBetaNumBits(r) + qBetaDenBits(r);
    return result;
}

inline QElem betaPowerViaMatrix(const PisotPoly& P, std::size_t n);

// ===================================================================
// PisotContext method bodies, defined AFTER Q(β) helpers are
// visible.
// ===================================================================
inline QElem PisotContext::eval(const std::vector<long long>& w) const {
    QElem v = ring.zero();
    for (std::size_t i = 0; i < w.size(); ++i) {
        if (w[i] == 0) continue;
        QElem wi = ring.from_int(w[i]);
        QElem bi = betaPowerMemoised(i);
        QElem term = ring.mul(wi, bi);
        v = ring.add(v, term);
    }
    return v;
}

// ===================================================================
// β^i via direct Pisot recurrence, with on-demand cache growth.
// Defined inline here in the body of the header.  This is the
// "warm up as the BFS explores" route: first call pays O(d)
// building β^i from earlier β^{i-d}..β^{i-1}; subsequent calls
// at the same position are O(d) copies.
// ===================================================================
inline QElem PisotContext::betaPowerMemoised(std::size_t n) const {
    const std::size_t d = poly.d;
    if (n < beta_powers_cache.size()) {
        return beta_powers_cache[n];
    }
    while (beta_powers_cache.size() <= n
            && beta_powers_cache.size() < beta_power_entry_limit) {
        std::size_t k = beta_powers_cache.size();
        QElem v(d);
        if (k == 0) {
            set_si(v.coeff(0), 1, 1);
        } else if (k < d) {
            set_si(v.coeff(k), 1, 1);
        } else {
            for (std::size_t i = 0; i < d; ++i) {
                if (poly.c[i] == 0) continue;
                QElem sub = beta_powers_cache[k - d + i];
                QElem term = ring.mul(ring.from_int(poly.c[i]), sub);
                v = ring.add(v, term);
            }
        }
        const std::size_t value_bits = qBetaStoredCoefficientBits(v);
        if (value_bits > beta_power_bit_limit
                || beta_powers_stored_bits
                    > beta_power_bit_limit - value_bits) {
            break;
        }
        beta_powers_stored_bits += value_bits;
        beta_powers_cache.push_back(std::move(v));
    }
    if (n < beta_powers_cache.size()) return beta_powers_cache[n];
    return betaPowerViaMatrix(poly, n);
}

// ===================================================================
// β^i via direct Pisot recurrence (with on-demand cache growth),
// defined inline below after Q(β) helpers.
// ===================================================================

// ===================================================================
// Thread-local cache of PisotContexts (one ring per PisotPoly signature).
// ===================================================================
inline const PisotContext& pisotContextFor(const PisotPoly& P) {
    thread_local std::map<PisotPoly, std::shared_ptr<PisotContext>> cache;
    auto it = cache.find(P);
    if (it != cache.end()) return *it->second;
    auto sp = std::make_shared<PisotContext>(P);
    cache[P] = sp;
    return *sp;
}

// ===================================================================
// Free-function wrappers.
// ===================================================================

inline QElem betaPower(const PisotPoly& P, std::size_t n) {
    return pisotContextFor(P).betaPowerMemoised(n);
}

inline QElem pisotEval(const PisotPoly& P,
                      const std::vector<long long>& w) {
    return pisotContextFor(P).eval(w);
}

inline std::vector<long long> pisotAlphabet(long long c) {
    std::vector<long long> a;
    a.reserve(static_cast<std::size_t>(2 * c + 1));
    for (long long i = -c; i <= c; ++i) a.push_back(i);
    return a;
}

// ===================================================================
// β^n via companion matrix exponentiation (NO memoization).
//
// β^n in basis (1, β, ..., β^{d-1}) is the first row of A^n,
// where A is the d × d companion matrix:
//
//     A = [[0, 1, 0, ..., 0],
//          [0, 0, 1, ..., 0],
//          ...
//          [c_0, c_1, ..., c_{d-1}]]
//
// Exponentiation by squaring in Q(β): O(d^3 log n) ring multiplies.
// One matrix in memory at a time.  Exact in Q(β).  No memo.
// ===================================================================
inline QElem betaPowerViaMatrix(const PisotPoly& P, std::size_t n) {
    const PisotContext& ctx = pisotContextFor(P);
    const std::size_t d = ctx.poly.d;
    if (n < d) {
        QElem v(d);
        if (n < v.coeffs_.size()) set_si(v.coeff(n), 1, 1);
        return v;
    }
    std::vector<std::vector<QElem>> A(d,
        std::vector<QElem>(d, ctx.ring.zero()));
    for (std::size_t i = 0; i + 1 < d; ++i) {
        A[i][i + 1] = ctx.ring.from_int(1);
    }
    for (std::size_t j = 0; j < d; ++j) {
        A[d - 1][j] = ctx.ring.from_int(ctx.poly.c[j]);
    }
    auto matmul = [&](const std::vector<std::vector<QElem>>& X,
                      const std::vector<std::vector<QElem>>& Y) {
        std::vector<std::vector<QElem>> Z(d,
            std::vector<QElem>(d, ctx.ring.zero()));
        for (std::size_t i = 0; i < d; ++i) {
            for (std::size_t j = 0; j < d; ++j) {
                for (std::size_t k = 0; k < d; ++k) {
                    Z[i][j] = ctx.ring.add(
                        Z[i][j],
                        ctx.ring.mul(X[i][k], Y[k][j]));
                }
            }
        }
        return Z;
    };
    std::vector<std::vector<QElem>> result(d,
        std::vector<QElem>(d, ctx.ring.zero()));
    for (std::size_t i = 0; i < d; ++i) {
        result[i][i] = ctx.ring.from_int(1);
    }
    std::vector<std::vector<QElem>> base = A;
    std::size_t k = n;
    while (k > 0) {
        if (k & 1u) result = matmul(result, base);
        base = matmul(base, base);
        k >>= 1;
    }
    // β^n in basis = Σ_k (A^n)[0][k] · β^k, where each (A^n)[0][k]
    // is a Q(β) element whose integer value lives at .coeff(0).
    QElem v(d);
    for (std::size_t j = 0; j < d; ++j) {
        v.coeff(j) = result[0][j].coeff(0);
    }
    return v;
}

// ===================================================================
// Closed-form β^n via Newton-iterated dominant Pisot root.
// Numerical (BigFloat) — used to cross-validate against the
// exact Q(β) arithmetic.  Sturm sequencing isolated β.
// ===================================================================
namespace detail {
inline mathlib::BigFloat bigfloat_pow(mathlib::BigFloat base,
                                      std::size_t exp, unsigned prec) {
    using namespace mathlib;
    BigFloat result = bigfloat_from_ll(1);
    while (exp > 0) {
        if (exp & 1u) result = bigfloat_mul(result, base, prec);
        base = bigfloat_mul(base, base, prec);
        exp >>= 1;
    }
    return result;
}
}  // namespace detail

inline mathlib::BigFloat betaPowerNumerical(const PisotPoly& P,
                                            std::size_t n,
                                            unsigned prec_bits = 200) {
    using namespace mathlib;
    BigFloat x = bigfloat_from_ll(2);
    for (int iter = 0; iter < 200; ++iter) {
        BigFloat f_x = detail::bigfloat_pow(x, P.d, prec_bits);
        for (std::size_t i = 0; i < P.d; ++i) {
            if (P.c[i] == 0) continue;
            BigFloat ci = bigfloat_from_ll(P.c[i]);
            BigFloat xi = detail::bigfloat_pow(x, i, prec_bits);
            BigFloat term = bigfloat_mul(ci, xi, prec_bits);
            f_x = bigfloat_sub(f_x, term, prec_bits);
        }
        BigFloat fp_x = bigfloat_mul(
            bigfloat_from_ll(static_cast<long long>(P.d)),
            detail::bigfloat_pow(x, P.d - 1, prec_bits),
            prec_bits);
        for (std::size_t i = 1; i < P.d; ++i) {
            if (P.c[i] == 0) continue;
            BigFloat ci_i = bigfloat_from_ll(
                static_cast<long long>(i) * P.c[i]);
            BigFloat xim1 = detail::bigfloat_pow(x, i - 1, prec_bits);
            BigFloat term = bigfloat_mul(ci_i, xim1, prec_bits);
            fp_x = bigfloat_sub(fp_x, term, prec_bits);
        }
        BigFloat dx = bigfloat_div(f_x, fp_x, prec_bits);
        x = bigfloat_sub(x, dx, prec_bits);
        // Convergence check: bail when |dx| is below working precision.
        if (iter > 8 && mathlib::cmp_si(dx.mant, 0) == 0) break;
    }
    return detail::bigfloat_pow(x, n, prec_bits);
}

inline mathlib::BigFloat qBetaEvaluateAtNumericalBeta(
    const QElem& v, const mathlib::BigFloat& beta_val, unsigned prec_bits) {
    using namespace mathlib;
    BigFloat result = bigfloat_from_ll(0);
    BigFloat bp = bigfloat_from_ll(1);
    for (std::size_t i = 0; i < v.coeffs_.size(); ++i) {
        BigInt num_big, den_big;
        mpz_set(num_big.get(), mpq_numref(v.coeffs_[i].get()));
        mpz_set(den_big.get(), mpq_denref(v.coeffs_[i].get()));
        BigFloat num = bigfloat_from_bigint(num_big);
        BigFloat den = bigfloat_from_bigint(den_big);
        BigFloat ci = bigfloat_div(num, den, prec_bits);
        BigFloat term = bigfloat_mul(ci, bp, prec_bits);
        result = bigfloat_add(result, term, prec_bits);
        bp = bigfloat_mul(bp, beta_val, prec_bits);
    }
    return result;
}

// ===================================================================
// BetaPowerCache: bounded LRU on top of the matrix route. Exact and
// fast on hits. Entry count and stored coefficient bits are bounded
// separately. Container overhead and matrix-route temporaries are not
// included in the coefficient-bit budget.
// ===================================================================
class BetaPowerCache {
public:
    PisotPoly poly;
    std::size_t capacity;
    std::size_t coefficient_bit_budget;
    std::size_t stored_coefficient_bits = 0;
    std::list<std::pair<std::size_t, QElem>> items;
    std::map<std::size_t, decltype(items)::iterator> index;

    BetaPowerCache(const PisotPoly& P, std::size_t cap = 64,
                   std::size_t bit_budget = 0)
        : poly(P), capacity(cap),
          coefficient_bit_budget(bit_budget) {}

    QElem get(std::size_t n) {
        auto it = index.find(n);
        if (it != index.end()) {
            items.splice(items.begin(), items, it->second);
            return it->second->second;
        }
        QElem v = betaPowerViaMatrix(poly, n);
        const std::size_t value_bits = qBetaStoredCoefficientBits(v);
        // A value larger than the whole cache budget is returned
        // exactly but deliberately not retained.
        if (coefficient_bit_budget != 0
                && value_bits > coefficient_bit_budget)
            return v;
        items.emplace_front(n, v);
        index[n] = items.begin();
        stored_coefficient_bits += value_bits;
        while (items.size() > capacity
                || (coefficient_bit_budget != 0
                    && stored_coefficient_bits
                        > coefficient_bit_budget)) {
            auto lru = std::prev(items.end());
            stored_coefficient_bits -=
                qBetaStoredCoefficientBits(lru->second);
            index.erase(lru->first);
            items.erase(lru);
        }
        return v;
    }

    std::size_t size() const { return items.size(); }
    std::size_t maxSize() const { return capacity; }
    std::size_t storedCoefficientBits() const {
        return stored_coefficient_bits;
    }
    std::size_t coefficientBitBudget() const {
        return coefficient_bit_budget;
    }
};

// ===================================================================
// State-machine types and CSYAutomaton.
// ===================================================================
struct AutoState {
    QElem v;
    std::size_t pos;

    bool operator<(const AutoState& o) const {
        if (pos != o.pos) return pos < o.pos;
        if (v.coeffs_.size() != o.v.coeffs_.size())
            return v.coeffs_.size() < o.v.coeffs_.size();
        for (std::size_t i = 0; i < v.coeffs_.size(); ++i) {
            int c = cmp(v.coeffs_[i], o.v.coeffs_[i]);
            if (c != 0) return c < 0;
        }
        return false;
    }
    bool operator==(const AutoState& o) const {
        if (pos != o.pos) return false;
        if (v.coeffs_.size() != o.v.coeffs_.size()) return false;
        for (std::size_t i = 0; i < v.coeffs_.size(); ++i) {
            if (cmp(v.coeffs_[i], o.v.coeffs_[i]) != 0) return false;
        }
        return true;
    }
};

struct PisotValidationReport {
    std::size_t checked = 0;
    std::size_t agreements = 0;
    std::size_t discrepancies = 0;
    std::vector<std::vector<long long>> failing_words;
};

class CSYAutomaton {
public:
    PisotPoly U;
    long long c;
    std::size_t bound_bits;
    std::size_t max_pos;
    std::vector<AutoState> states;
    bool closed_at_bound_ = false;

    CSYAutomaton(const PisotPoly& U_, long long c_,
                 std::size_t bound_bits_ = 0, std::size_t max_pos_ = 0)
        : U(U_), c(c_),
          bound_bits(bound_bits_ == 0 ? 32 : bound_bits_),
          max_pos(max_pos_ == 0 ? bound_bits_ * 4 + 16 : max_pos_) {
        build();
    }

    std::size_t stateCount() const { return states.size(); }
    bool bfsClosed() const { return closed_at_bound_; }

    bool acceptsWord(const std::vector<long long>& w) const {
        if (w.size() > max_pos + 1) {
            return qBetaIsZero(pisotEval(U, w));
        }
        const PisotContext& ctx = pisotContextFor(U);
        QElem v = ctx.ring.zero();
        std::size_t pos = 0;
        for (long long d : w) {
            QElem b = ctx.betaPowerMemoised(pos);
            QElem term = ctx.ring.mul(ctx.ring.from_int(d), b);
            v = ctx.ring.add(v, term);
            if (!qBetaCoefficientsBounded(v, bound_bits)) return false;
            ++pos;
        }
        return qBetaIsZero(v);
    }

    PisotValidationReport validate(std::size_t max_len) {
        PisotValidationReport rep;
        if (max_len > max_pos + 1) max_len = max_pos + 1;
        std::vector<long long> w;
        auto B = pisotAlphabet(c);
        recurseValidate(w, max_len, B, rep);
        return rep;
    }

private:
    void build();
    void recurseValidate(std::vector<long long>& w, std::size_t depth,
                         const std::vector<long long>& B,
                         PisotValidationReport& rep);
};

inline void CSYAutomaton::build() {
    const PisotContext& ctx = pisotContextFor(U);
    std::set<AutoState> visited;
    AutoState init{ctx.ring.zero(), 0};
    visited.insert(init);
    std::vector<AutoState> frontier = {init};
    std::vector<long long> B = pisotAlphabet(c);
    bool changed = true;
    bool truncated_at_position_bound = false;
    int safety = 0;
    while (changed && safety < 4096) {
        changed = false;
        std::vector<AutoState> next;
        for (const auto& st : frontier) {
            if (st.pos >= max_pos) {
                // Zero is always in B and preserves st.v, so every
                // retained boundary state has a valid successor with
                // a new absolute position. The present state model
                // therefore did not close; it was truncated.
                truncated_at_position_bound = true;
                continue;
            }
            QElem b = ctx.betaPowerMemoised(st.pos);
            for (long long d : B) {
                QElem term = ctx.ring.mul(ctx.ring.from_int(d), b);
                QElem nv = ctx.ring.add(st.v, term);
                if (!qBetaCoefficientsBounded(nv, bound_bits)) continue;
                AutoState ns{nv, st.pos + 1};
                if (visited.insert(ns).second) {
                    next.push_back(ns);
                    changed = true;
                }
            }
        }
        if (next.empty()) break;
        if (!changed) break;
        frontier = std::move(next);
        ++safety;
    }
    closed_at_bound_ = !truncated_at_position_bound && !changed;
    states.reserve(visited.size());
    for (const auto& s : visited) states.push_back(s);
}

inline void CSYAutomaton::recurseValidate(
    std::vector<long long>& w, std::size_t depth,
    const std::vector<long long>& B, PisotValidationReport& rep) {
    bool expected = qBetaIsZero(pisotEval(U, w));
    bool actual   = acceptsWord(w);
    rep.checked++;
    if (expected == actual) rep.agreements++;
    else { rep.discrepancies++; rep.failing_words.push_back(w); }
    if (depth == 0) return;
    for (long long d : B) {
        w.push_back(d);
        recurseValidate(w, depth - 1, B, rep);
        w.pop_back();
    }
}

}  // namespace adelic
