// qbasis.hpp
//
// Bit-exact Q(β) arithmetic and linear algebra for the
// contact-boundary pipeline.  Replaces the double-precision
// power-iteration approach in core.hpp::Substitution::in_H_sigma
// with exact arithmetic in the number field Q(β), where β is
// the dominant (Perron) eigenvalue of the substitution's
// incidence matrix.
//
// This header is now a thin compatibility wrapper over the
// mathlib (math/) — the canonical Q(β) primitives
// live there with full test coverage.  The ravel::qbeta
// namespace and the public API are preserved so the existing
// test_qbasis.cpp, lua_bindings.cpp, and downstream consumers
// continue to work.
//
// All arithmetic is exact (mpq_t over the mathlib, no double
// precision for sign tests).  Real evaluation for sign tests
// uses Sturm-based rational isolation (no Horner at
// double-precision β).
//
// DEPENDENCIES: math/ + mini-gmp.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

#include "mini-gmp/mini-gmp.h"

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"
#include "math/bezout.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/sturm.hpp"
#include "math/in_h_sigma.hpp"

namespace ravel {
namespace qbeta {

// ===================================================================
// Compatibility layer: ravel::qbeta::QElem ↔ mathlib::QElem
// ===================================================================
//
// The new mathlib::QElem uses mpq_t coefficients (exact rational).
// The legacy ravel::qbeta::QElem used mpz_t coefficients (no scale).
// We provide a shim that uses mathlib's storage but exposes the
// same public API.
//
// We store a mathlib::QElem + a mathlib::QBetaRing inside.
// (The charpoly is needed for all multiplication / sign tests.)

class QElem {
public:
    // Legacy-compatible fields (used by the existing test_qbasis.cpp).
    // The actual storage is in impl_ (mathlib::QElem).  The coeffs
    // array stores mpz_t views for the legacy API; these are kept in
    // sync with impl_ via a side table.
    //
    // mpz_t is __mpz_struct[1] which is not movable, so we use raw
    // new[]/delete[] (matches the original qbasis.hpp).
    std::size_t d = 0;
    mpz_t* coeffs = nullptr;
    mathlib::QElem impl_;
    std::vector<long long> charpoly_;  // legacy charpoly (lowest-degree first)

    QElem() : impl_(), charpoly_() {}

    explicit QElem(std::size_t d_) : d(d_), coeffs(nullptr), impl_(d_), charpoly_(d_, 0) {
        coeffs = new mpz_t[d_];
        for (std::size_t i = 0; i < d_; ++i) {
            mpz_init(coeffs[i]);
            mpz_set(coeffs[i], mpq_numref(impl_.coeffs_[i].get()));
        }
        for (std::size_t i = 0; i < d_; ++i) charpoly_[i] = (i == d_ - 1) ? -1 : 0;
    }

    QElem(const QElem& o) : d(o.d), coeffs(nullptr), impl_(o.impl_), charpoly_(o.charpoly_) {
        coeffs = new mpz_t[d];
        for (std::size_t i = 0; i < d; ++i) {
            mpz_init(coeffs[i]);
            // coeffs is the legacy public API; copy from o.coeffs.
            mpz_set(coeffs[i], o.coeffs[i]);
        }
        // After the copy, ensure impl_ is in sync with coeffs.
        for (std::size_t i = 0; i < d; ++i) {
            mpq_set_z(impl_.coeffs_[i].get(), coeffs[i]);
            mpz_set_ui(mpq_denref(impl_.coeffs_[i].get()), 1);
            mpq_canonicalize(impl_.coeffs_[i].get());
        }
    }
    QElem(QElem&& o) noexcept : d(o.d), coeffs(o.coeffs), impl_(std::move(o.impl_)),
                                charpoly_(std::move(o.charpoly_)) {
        o.coeffs = nullptr;
        o.d = 0;
    }
    QElem& operator=(const QElem& o) {
        if (this != &o) {
            for (std::size_t i = 0; i < d; ++i) mpz_clear(coeffs[i]);
            delete[] coeffs;
            d = o.d;
            coeffs = new mpz_t[d];
            for (std::size_t i = 0; i < d; ++i) mpz_init(coeffs[i]);
            // coeffs is the legacy public API; copy from o.coeffs.
            for (std::size_t i = 0; i < d; ++i) {
                mpz_set(coeffs[i], o.coeffs[i]);
            }
            impl_ = o.impl_;
            charpoly_ = o.charpoly_;
            // After the copy, ensure impl_ is in sync with coeffs.
            for (std::size_t i = 0; i < d; ++i) {
                mpq_set_z(impl_.coeffs_[i].get(), coeffs[i]);
                mpz_set_ui(mpq_denref(impl_.coeffs_[i].get()), 1);
                mpq_canonicalize(impl_.coeffs_[i].get());
            }
        }
        return *this;
    }
    QElem& operator=(QElem&& o) noexcept {
        if (this != &o) {
            for (std::size_t i = 0; i < d; ++i) mpz_clear(coeffs[i]);
            delete[] coeffs;
            d = o.d;
            coeffs = o.coeffs;
            o.coeffs = nullptr;
            o.d = 0;
            impl_ = std::move(o.impl_);
            charpoly_ = std::move(o.charpoly_);
        }
        return *this;
    }
    ~QElem() {
        if (coeffs) {
            for (std::size_t i = 0; i < d; ++i) mpz_clear(coeffs[i]);
            delete[] coeffs;
        }
    }

    // Sync the coeffs array with impl_ (call after any modification of impl_).
    void sync_from_impl() {
        for (std::size_t i = 0; i < d; ++i) {
            mpz_set(coeffs[i], mpq_numref(impl_.coeffs_[i].get()));
        }
    }

    // Sync impl_ from coeffs (call after any modification of coeffs).
    // The legacy public API allows direct access to coeffs (e.g.,
    // `mpz_set_si(e.coeffs[0], 1)`); we propagate that into impl_ here.
    // The denominator is assumed to be 1 (legacy coeffs are integer-only).
    void sync_to_impl() {
        for (std::size_t i = 0; i < d; ++i) {
            mpq_set_z(impl_.coeffs_[i].get(), coeffs[i]);
            // Ensure denominator is 1 (canonical form).
            mpz_set_ui(mpq_denref(impl_.coeffs_[i].get()), 1);
            mpq_canonicalize(impl_.coeffs_[i].get());
        }
    }
};

// Helper: build a mathlib::QBetaRing from a charpoly vector
inline mathlib::QBetaRing ring_from_charpoly(const std::vector<long long>& c) {
    mathlib::QBetaRing R;
    R.charpoly_ = mathlib::PolyZ();
    R.charpoly_.ensure_size(c.size() + 1);
    for (std::size_t k = 0; k < c.size(); ++k) {
        mathlib::set_si(R.charpoly_.coeff(c.size() - 1 - k), c[k]);
    }
    mathlib::set_si(R.charpoly_.coeff(c.size()), 1);
    return R;
}

// ===================================================================
// Constructors
// ===================================================================

inline QElem qzero(std::size_t d) { return QElem(d); }

inline QElem qone(std::size_t d) {
    QElem r(d);
    mathlib::set_si(r.impl_.coeffs_[0], 1, 1);
    return r;
}

inline QElem qbeta(std::size_t d, std::size_t k) {
    QElem r(d);
    if (k < d) mathlib::set_si(r.impl_.coeffs_[k], 1, 1);
    return r;
}

// ===================================================================
// Basic arithmetic
// ===================================================================

inline QElem qadd(const QElem& a, const QElem& b) {
    QElem aa = a; aa.sync_to_impl();
    QElem bb = b; bb.sync_to_impl();
    QElem r = aa;
    mathlib::QBetaRing R = ring_from_charpoly(aa.charpoly_);
    r.impl_ = R.add(aa.impl_, bb.impl_);
    r.sync_from_impl();
    return r;
}

inline QElem qsub(const QElem& a, const QElem& b) {
    QElem aa = a; aa.sync_to_impl();
    QElem bb = b; bb.sync_to_impl();
    QElem r = aa;
    mathlib::QBetaRing R = ring_from_charpoly(aa.charpoly_);
    r.impl_ = R.sub(aa.impl_, bb.impl_);
    r.sync_from_impl();
    return r;
}

inline QElem qneg(const QElem& a) {
    QElem aa = a; aa.sync_to_impl();
    QElem r = aa;
    mathlib::QBetaRing R = ring_from_charpoly(aa.charpoly_);
    r.impl_ = R.neg(aa.impl_);
    r.sync_from_impl();
    return r;
}

inline QElem qmul(const QElem& a, const QElem& b,
                  const std::vector<long long>& charpoly) {
    QElem aa = a; aa.sync_to_impl();
    QElem bb = b; bb.sync_to_impl();
    std::size_t d = charpoly.size() == 0 ? 1 : charpoly.size();
    QElem r(d);
    r.charpoly_ = charpoly;
    mathlib::QBetaRing R = ring_from_charpoly(charpoly);
    r.impl_ = R.mul(aa.impl_, bb.impl_);
    r.sync_from_impl();
    return r;
}

inline QElem qmul_si(const QElem& x, long long s) {
    QElem xx = x; xx.sync_to_impl();
    QElem r = xx;
    for (auto& c : r.impl_.coeffs_) {
        mathlib::Rat t; mathlib::set_si(t, s, 1);
        mathlib::mul(c, c, t);
    }
    r.sync_from_impl();
    return r;
}

// Multiply by β: this is qmul(x, β) but with implicit scaling handled
// at the caller.  We follow the legacy convention: β · x has the
// coefficients (in lowest-degree-first order) of β * (1 x_0 + x_1 β + ...
// + x_{d-1} β^{d-1}) = x_0 β + x_1 β^2 + ... + x_{d-1} β^d, with β^d
// reduced via the charpoly.
inline QElem qmul_by_beta(const QElem& x,
                          const std::vector<long long>& c) {
    QElem xx = x; xx.sync_to_impl();
    std::size_t d = c.size();
    QElem r(d);
    r.charpoly_ = c;
    mathlib::QBetaRing R = ring_from_charpoly(c);
    r.impl_ = R.mul(xx.impl_, R.beta_k(1));
    r.sync_from_impl();
    return r;
}

// ===================================================================
// Closed-form β⁻¹ (legacy: returns un-normalized inv; β·inv = c[d-1])
// ===================================================================
//
// The legacy code stores β⁻¹ as the polynomial p such that
//   β · p = c[d-1]
// i.e., p is the un-normalized inverse.  This differs from
// mathlib's QBetaRing::beta_inverse which returns the normalized
// inverse (β · p = 1).
//
// For compatibility, we return the un-normalized form here.  The
// new mathlib inverse is available as mathlib::QBetaRing::beta_inverse.

inline QElem qbeta_inverse_of_beta(const std::vector<long long>& c) {
    std::size_t d = c.size();
    QElem r(d);
    r.charpoly_ = c;
    mathlib::QBetaRing R = ring_from_charpoly(c);
    mathlib::QElem inv = R.beta_inverse();
    // The new β⁻¹ satisfies β · inv = 1.  Multiply by c[d-1] to get
    // the legacy un-normalized form.
    for (auto& cc : inv.coeffs_) {
        mathlib::Rat t; mathlib::set_si(t, c[d - 1], 1);
        mathlib::mul(cc, cc, t);
    }
    r.impl_ = std::move(inv);
    // Sync the coeffs array with impl_
    for (std::size_t i = 0; i < d; ++i) {
        mpz_set(r.coeffs[i], mpq_numref(r.impl_.coeffs_[i].get()));
    }
    return r;
}

// ===================================================================
// Real evaluation (legacy: returns a double for backward compat)
// ===================================================================
//
// We use the exact Sturm-based isolation and rational evaluation.
inline double qreal(const QElem& x, double beta_R) {
    // Backward-compatible approximation only; exact decisions use the
    // Sturm sign path. Evaluate rational coefficients by Horner without
    // manufacturing a common denominator (the former implementation
    // applied extra powers of that denominator and returned values
    // scaled by roughly 10^58).
    if (!std::isfinite(beta_R) || beta_R <= 1.0) {
        mathlib::QBetaRing R = ring_from_charpoly(x.charpoly_);
        mathlib::RootInterval bi = mathlib::isolate_beta(R);
        beta_R = mpz_get_d(mpq_numref(bi.lo.get()))
               / mpz_get_d(mpq_denref(bi.lo.get()));
        beta_R += mpz_get_d(mpq_numref(bi.hi.get()))
                / mpz_get_d(mpq_denref(bi.hi.get()));
        beta_R *= 0.5;
    }
    double value = 0.0;
    for (std::size_t i = x.impl_.coeffs_.size(); i-- > 0;) {
        const auto& coefficient = x.impl_.coeffs_[i];
        double c = mpz_get_d(mpq_numref(coefficient.get()))
                 / mpz_get_d(mpq_denref(coefficient.get()));
        value = value * beta_R + c;
    }
    return value;
}

// Real root isolation (legacy: returns a double)
inline double find_real_root(const std::vector<long long>& c) {
    mathlib::QBetaRing R = ring_from_charpoly(c);
    mathlib::RootInterval bi = mathlib::isolate_beta(R);
    // Convert midpoint to double
    mathlib::Rat sum; mathlib::add(sum, bi.lo, bi.hi);
    mathlib::Rat two; mathlib::set_si(two, 2, 1);
    mathlib::Rat mid; mathlib::div(mid, sum, two);
    // Convert rational to double via num/den as doubles
    char* snum = mpz_get_str(nullptr, 10, mpq_numref(mid.get()));
    char* sden = mpz_get_str(nullptr, 10, mpq_denref(mid.get()));
    double d = std::atof(snum) / std::atof(sden);
    std::free(snum);
    std::free(sden);
    return d;
}

// ===================================================================
// Right eigenvector (legacy: power iteration in Q(β))
// ===================================================================
//
// The legacy qbeta_right_eigenvector uses power iteration in Q(β),
// which has the well-known coefficient growth problem (see technical note).
// The new approach is Cramer's rule via Tier 3 (right_eigenvector_via_qbeta).
// We provide both: the power-iteration version (legacy API) and a
// Cramer's-rule version (qbeta_via_cramer).

inline std::vector<QElem> qbeta_right_eigenvector(
    const std::vector<std::vector<long long>>& M,
    const std::vector<long long>& c) {
    // Use the new Cramer's rule (replaces the broken power iteration).
    std::size_t d = c.size();
    mathlib::QBetaRing R = ring_from_charpoly(c);
    mathlib::EigenvectorResult er = mathlib::right_eigenvector_via_qbeta(M, R);
    std::vector<QElem> v;
    v.reserve(er.v.size());
    for (const auto& e : er.v) {
        QElem qe(d);
        qe.charpoly_ = c;
        qe.impl_ = e;
        for (std::size_t i = 0; i < d; ++i) {
            mpz_set(qe.coeffs[i], mpq_numref(qe.impl_.coeffs_[i].get()));
        }
        v.push_back(std::move(qe));
    }
    return v;
}

// ===================================================================
// Left eigenvector (the one qbeta_in_h_sigma actually needs)
// ===================================================================
//
// docs/RESEARCH_STATUS.md documents that this module's own
// test (tests/test_qbasis.cpp) was, until this fix, calling
// qbeta_right_eigenvector and feeding the result straight into
// qbeta_in_h_sigma -- the same eigenvector-sidedness mistake
// core.hpp::ensure_exact_qbeta() had, in an otherwise-unrelated legacy
// module.  This wrapper mirrors qbeta_right_eigenvector's shape but
// calls the math library's dedicated left_eigenvector_via_qbeta.
inline std::vector<QElem> qbeta_left_eigenvector(
    const std::vector<std::vector<long long>>& M,
    const std::vector<long long>& c) {
    std::size_t d = c.size();
    mathlib::QBetaRing R = ring_from_charpoly(c);
    mathlib::EigenvectorResult er = mathlib::left_eigenvector_via_qbeta(M, R);
    std::vector<QElem> v;
    v.reserve(er.v.size());
    for (const auto& e : er.v) {
        QElem qe(d);
        qe.charpoly_ = c;
        qe.impl_ = e;
        for (std::size_t i = 0; i < d; ++i) {
            mpz_set(qe.coeffs[i], mpq_numref(qe.impl_.coeffs_[i].get()));
        }
        v.push_back(std::move(qe));
    }
    return v;
}

inline std::vector<QElem> qbeta_via_cramer(
    const std::vector<std::vector<long long>>& M,
    const std::vector<long long>& c) {
    return qbeta_right_eigenvector(M, c);
}

// ===================================================================
// Q(β) dot product
// ===================================================================

inline long long qbeta_dot(const std::vector<long long>& x,
                            const std::vector<QElem>& v,
                            double /*beta_R*/) {
    // Returns a "double-precision" approximation of <x, v> in Q(β).
    // For tests, this is fine.  The new in_h_sigma uses exact arithmetic.
    mathlib::QBetaRing R = ring_from_charpoly(v[0].charpoly_);
    mathlib::QElem x_dot_v = mathlib::QElem(v[0].impl_.coeffs_.size());
    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i] == 0) continue;
        mathlib::QElem term = R.from_int(x[i]);
        term = R.mul(term, v[i].impl_);
        x_dot_v = R.add(x_dot_v, term);
    }
    // Convert to double
    QElem qe;
    qe.d = v[0].impl_.coeffs_.size();
    qe.coeffs = new mpz_t[qe.d];
    for (std::size_t i = 0; i < qe.d; ++i) {
        mpz_init(qe.coeffs[i]);
        mpz_set(qe.coeffs[i], mpq_numref(x_dot_v.coeffs_[i].get()));
    }
    qe.impl_ = x_dot_v;
    qe.charpoly_ = v[0].charpoly_;
    long long result = static_cast<long long>(qreal(qe, 0));
    return result;
}

// ===================================================================
// in_H_sigma test (exact Q(β) version)
// ===================================================================
//
// Returns +1 if x in H_sigma(., j), -1 if not, 0 if uncertain.
// The new implementation is exact: it uses the mathlib's qbeta_sign
// (Sturm-based) for sign determination.

inline int qbeta_in_h_sigma(const std::vector<long long>& x, std::size_t j,
                             const std::vector<QElem>& v, double /*beta_R*/) {
    if (v.empty()) return 0;
    mathlib::QBetaRing R = ring_from_charpoly(v[0].charpoly_);
    mathlib::RootInterval bi = mathlib::isolate_beta(R);
    std::vector<mathlib::QElem> v_ml;
    v_ml.reserve(v.size());
    for (const auto& qe : v) v_ml.push_back(qe.impl_);
    bool in_h = mathlib::in_h_sigma(x, j, v_ml, R, bi);
    return in_h ? +1 : -1;
}

// ===================================================================
// Inverse in Q(β) (the broken one from the technical note, now working)
// ===================================================================

struct QBetaInverseResult {
    bool invertible;
    QElem inverse;
};

inline QBetaInverseResult invert_in_qbeta(const QElem& e,
                                            const std::vector<long long>& c) {
    QElem ee = e; ee.sync_to_impl();
    mathlib::QBetaRing R = ring_from_charpoly(c);
    mathlib::QBetaInverseResult mir = mathlib::invert_in_qbeta(ee.impl_, R);
    QBetaInverseResult r;
    r.invertible = mir.invertible;
    r.inverse.charpoly_ = c;
    r.inverse.impl_ = mir.inverse;
    r.inverse.d = mir.inverse.coeffs_.size();
    r.inverse.coeffs = new mpz_t[r.inverse.d];
    for (std::size_t i = 0; i < r.inverse.d; ++i) {
        mpz_init(r.inverse.coeffs[i]);
    }
    r.inverse.sync_from_impl();
    return r;
}

}  // namespace qbeta
}  // namespace ravel
