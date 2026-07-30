// math/qbeta.hpp
//
// Q(β) arithmetic for arbitrary degree d.
//
// Q(β) = Q[x] / charpoly(β), where charpoly is a monic polynomial in
// Z[x] of degree d.  Elements are polynomials in β with rational
// coefficients; we represent them as the d-tuple of coefficients in
// the basis (1, β, β^2, ..., β^{d-1}).
//
// All arithmetic is EXACT (mpq_t everywhere, no double precision).
// Real evaluation for sign tests is in eval.hpp / sturm.hpp.
//
// API: a QBetaRing wraps a charpoly; operations are member functions
// of the ring.  This decouples storage (QElem) from operations (ring)
// so we can swap implementations (e.g., faster CRT-based arithmetic
// later) without breaking callers.
//
// Reference: SymPy QQ.algebraic_field(minimal_polynomial) for the
// equivalent algebraic number field, with minpoly matching our
// charpoly(β).

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"

namespace mathlib {

// ===================================================================
// QElem: an element of Q(β)
// ===================================================================
//
// Coefficients are in basis (1, β, β^2, ..., β^{d-1}).  coeffs_[i] is
// the coefficient of β^i.  The vector always has size d (no
// trailing-zero trimming; elements are always represented in their
// canonical reduced form).

class QElem {
public:
    std::vector<Rat> coeffs_;

    QElem() : coeffs_() {}
    explicit QElem(std::size_t d) : coeffs_(d) {
        for (auto& c : coeffs_) set_si(c, 0, 1);
    }
    QElem(std::initializer_list<long long> cs) : coeffs_(cs.size()) {
        std::size_t i = 0;
        for (auto c : cs) { set_si(coeffs_[i], c, 1); ++i; }
    }
    QElem(std::initializer_list<std::pair<long long,long long>> rcs) : coeffs_(rcs.size()) {
        std::size_t i = 0;
        for (auto [n, d] : rcs) { set_si(coeffs_[i], n, d); ++i; }
    }
    QElem(const QElem& o) = default;
    QElem(QElem&& o) noexcept = default;
    QElem& operator=(const QElem& o) = default;
    QElem& operator=(QElem&& o) noexcept = default;

    std::size_t degree_limit() const { return coeffs_.size(); }
    void ensure_size(std::size_t n) {
        if (coeffs_.size() < n) {
            std::size_t old = coeffs_.size();
            coeffs_.resize(n);
            for (std::size_t k = old; k < n; ++k) set_si(coeffs_[k], 0, 1);
        }
    }
    Rat& coeff(std::size_t i) {
        ensure_size(i + 1);
        return coeffs_[i];
    }
    const Rat& coeff(std::size_t i) const { return coeffs_[i]; }

    long long degree() const {
        long long d = static_cast<long long>(coeffs_.size()) - 1;
        while (d > 0 && sgn(coeffs_[d]) == 0) --d;
        if (d == 0 && sgn(coeffs_[0]) == 0) return -1;
        return d;
    }
    bool is_zero() const { return degree() < 0; }

    bool operator==(const QElem& o) const {
        if (coeffs_.size() != o.coeffs_.size()) return false;
        for (std::size_t i = 0; i < coeffs_.size(); ++i) {
            if (cmp(coeffs_[i], o.coeffs_[i]) != 0) return false;
        }
        return true;
    }
    bool operator!=(const QElem& o) const { return !(*this == o); }
};

inline std::string str(const QElem& x) {
    std::string out;
    for (long long i = x.degree(); i >= 0; --i) {
        if (i < x.degree()) out += " + ";
        if (i == 0) {
            out += str(x.coeffs_[0]);
        } else if (i == 1) {
            if (cmp(x.coeffs_[1], Rat(1)) == 0) out += "β";
            else if (cmp(x.coeffs_[1], Rat(-1)) == 0) out += "-β";
            else { out += str(x.coeffs_[1]); out += "*β"; }
        } else {
            if (cmp(x.coeffs_[i], Rat(1)) == 0) { out += "β^"; out += std::to_string(i); }
            else if (cmp(x.coeffs_[i], Rat(-1)) == 0) { out += "-β^"; out += std::to_string(i); }
            else { out += str(x.coeffs_[i]); out += "*β^"; out += std::to_string(i); }
        }
    }
    return out.empty() ? "0" : out;
}

// ===================================================================
// QBetaRing: the field Q(β) = Q[x] / charpoly(β)
// ===================================================================
//
// Construction: QBetaRing(PolyZ c) where c is the charpoly in
// lowest-degree-first order (so c[i] is the coefficient of x^i and
// c is monic of degree d, meaning c[d] = 1 in highest-degree-first
// indexing, equivalently c has d+1 coefficients with coeffs[d]=0 in
// lowest-degree-first... wait, no).
//
// Wait, let me be careful.  In the mathlib::PolyZ convention,
// coeffs_[i] is the coefficient of x^i.  The charpoly
//   x^d + c[0] x^{d-1} + c[1] x^{d-2} + ... + c[d-1]
// in lowest-degree-first is:
//   c[d-1] + c[d-2] x + ... + c[0] x^{d-1} + x^d
// So coeffs_[i] for i in [0, d-1] are c[d-1-i], and coeffs_[d] = 1.
//
// The QBetaRing takes a PolyZ charpoly of degree d (so d+1
// coefficients, with coeffs_[d] = 1).  Use QBetaRing::from_low_first
// if you have the charpoly in the c[0] + c[1] x + ... format (where
// c[i] is the coefficient of x^{d-i} in the charpoly).
//
// Actually, the convention is confusing.  Let me use a clear naming:
//   "charpoly": the monic polynomial x^d + a[0] x^{d-1} + ... + a[d-1]
//   "low_first" coeffs: [a[0], a[1], ..., a[d-1]] (length d)
//   "polyz" format: PolyZ({a[d-1], a[d-2], ..., a[0], 1}) (length d+1)

class QBetaRing {
public:
    // Charpoly in highest-degree-first [a[d-1], a[d-2], ..., a[0], 1]
    // (PolyZ::coeffs_[i] = coefficient of x^i, so the leading
    // coefficient is coeffs_[d]).
    PolyZ charpoly_;

    // Default-construct a Fibonacci-like ring (charpoly = x^2 - x - 1).
    QBetaRing() : charpoly_({-1, -1, 1}) {}

    // Construct from a PolyZ charpoly (degree d, monic).
    explicit QBetaRing(const PolyZ& c) : charpoly_(c) {
        if (c.degree() < 1) throw std::invalid_argument("QBetaRing: charpoly degree < 1");
        if (!is_one(c.coeff(static_cast<std::size_t>(c.degree())))) {
            throw std::invalid_argument("QBetaRing: charpoly not monic");
        }
    }

    // Construct from low-first coeffs: x^d + a[0] x^{d-1} + ... + a[d-1]
    // So the PolyZ is {a[d-1], a[d-2], ..., a[0], 1}.
    static QBetaRing from_low_first(std::initializer_list<long long> low_first) {
        std::vector<long long> polyz_coeffs(low_first.size() + 1);
        std::size_t n = low_first.size();  // d
        std::size_t i = 0;
        for (auto v : low_first) { polyz_coeffs[n - 1 - i] = v; ++i; }
        polyz_coeffs[n] = 1;
        PolyZ p;
        p.ensure_size(n + 1);
        for (std::size_t k = 0; k <= n; ++k) {
            set_si(p.coeff(k), polyz_coeffs[k]);
        }
        return QBetaRing(p);
    }

    std::size_t degree() const { return static_cast<std::size_t>(charpoly_.degree()); }
    const PolyZ& charpoly() const { return charpoly_; }

    // ----- Element constructors -----

    QElem zero() const { return QElem(degree()); }
    QElem one() const {
        QElem r(degree());
        set_si(r.coeffs_[0], 1, 1);
        return r;
    }
    QElem beta_k(std::size_t k) const {
        QElem r(degree());
        if (k < r.coeffs_.size()) set_si(r.coeffs_[k], 1, 1);
        return r;
    }
    QElem from_int(long long n) const {
        QElem r(degree());
        set_si(r.coeffs_[0], n, 1);
        return r;
    }

    // ----- Addition / subtraction / negation -----

    QElem add(const QElem& a, const QElem& b) const {
        std::size_t n = std::max(a.coeffs_.size(), b.coeffs_.size());
        QElem r(degree());
        r.ensure_size(n);
        for (std::size_t i = 0; i < n; ++i) {
            if (i < a.coeffs_.size() && i < b.coeffs_.size()) {
                mathlib::add(r.coeffs_[i], a.coeffs_[i], b.coeffs_[i]);
            } else if (i < a.coeffs_.size()) {
                r.coeffs_[i] = a.coeffs_[i];
            } else {
                r.coeffs_[i] = b.coeffs_[i];
            }
        }
        return r;
    }

    QElem sub(const QElem& a, const QElem& b) const {
        std::size_t n = std::max(a.coeffs_.size(), b.coeffs_.size());
        QElem r(degree());
        r.ensure_size(n);
        for (std::size_t i = 0; i < n; ++i) {
            if (i < a.coeffs_.size() && i < b.coeffs_.size()) {
                mathlib::sub(r.coeffs_[i], a.coeffs_[i], b.coeffs_[i]);
            } else if (i < a.coeffs_.size()) {
                r.coeffs_[i] = a.coeffs_[i];
            } else {
                mathlib::neg(r.coeffs_[i], b.coeffs_[i]);
            }
        }
        return r;
    }

    QElem neg(const QElem& a) const {
        QElem r = a;
        for (auto& c : r.coeffs_) mathlib::neg(c);
        return r;
    }

    // ----- Multiplication -----
    //
    // Multiply a and b in Z[β] (no reduction), producing a
    // 2d-1 coefficient polynomial, then reduce mod charpoly.
    //
    // Reduction: β^d = -charpoly_.coeff_[d-1] - charpoly_.coeff_[d-2] β - ... - charpoly_.coeff_[0] β^{d-1}.
    // For a degree-d+ k term β^{d+k} (k = 0..d-2), we reduce:
    //   β^{d+k} = β^k * β^d = -sum_{i=0}^{d-1} charpoly_.coeff_[i] β^{d-1-i+k}

    QElem mul(const QElem& a, const QElem& b) const {
        const std::size_t d = degree();
        // Full product (length 2d-1)
        std::vector<Rat> full(2 * d - 1);
        for (auto& c : full) set_si(c, 0, 1);
        for (std::size_t i = 0; i < a.coeffs_.size(); ++i) {
            if (is_zero(a.coeffs_[i])) continue;
            for (std::size_t j = 0; j < b.coeffs_.size(); ++j) {
                if (is_zero(b.coeffs_[j])) continue;
                Rat tmp;
                mathlib::mul(tmp, a.coeffs_[i], b.coeffs_[j]);
                mathlib::add(full[i + j], full[i + j], tmp);
            }
        }
        // Reduce mod charpoly.
        //
        // For k >= d, β^k = β^{k-d} * β^d.  And β^d = -c[0] β^{d-1} - c[1] β^{d-2} - ... - c[d-1].
        // So β^k = -c[0] β^{k-1} - c[1] β^{k-2} - ... - c[d-1] β^{k-d}.
        // Coefficient of β^{k-1-i} in β^k is -c[i], for i in [0, d-1].
        //
        // In PolyZ convention: charpoly_.coeff_(j) = coefficient of x^j in charpoly
        // = c[d-1-j] for j < d, charpoly_.coeff_(d) = 1.  So c[i] = charpoly_.coeff_(d-1-i).
        //
        // Process highest first; each reduction only contributes to indices < k, so we don't
        // need a temporary.  Indices < d never need reduction.
        for (long long k = 2 * static_cast<long long>(d) - 2; k >= static_cast<long long>(d); --k) {
            if (is_zero(full[static_cast<std::size_t>(k)])) continue;
            for (std::size_t i = 0; i < d; ++i) {
                long long target = k - 1 - static_cast<long long>(i);
                if (target < 0) continue;
                // full[target] -= c[i] * full[k] = charpoly_.coeff_(d-1-i) * full[k]
                Rat prod;
                mathlib::mul(prod, charpoly_.coeff(d - 1 - i), full[static_cast<std::size_t>(k)]);
                mathlib::sub(full[static_cast<std::size_t>(target)], full[static_cast<std::size_t>(target)], prod);
            }
            set_si(full[static_cast<std::size_t>(k)], 0, 1);
        }
        // Copy the first d coefficients to result
        QElem r(degree());
        for (std::size_t i = 0; i < d; ++i) {
            r.coeffs_[i] = full[i];
        }
        return r;
    }

    // β^{-1} in Q(β) via the closed-form.
    //
    // From β^d + c[0] β^{d-1} + c[1] β^{d-2} + ... + c[d-1] = 0 we get
    //   β^{d-1} = -c[0] β^{d-2} - c[1] β^{d-3} - ... - c[d-1] β^{-1}
    //   c[d-1] β^{-1} = -c[0] β^{d-2} - c[1] β^{d-3} - ... - c[d-2] - β^{d-1}
    //   β^{-1} = -1/c[d-1] · (c[0] β^{d-2} + c[1] β^{d-3} + ... + c[d-2] + β^{d-1})
    //
    // In our PolyZ convention charpoly_.coeff_(k) = c[d-1-k] for k in
    // [0, d-1] and charpoly_.coeff_(d) = 1.  So c[i] = charpoly_.coeff_(d-1-i).
    //
    // Coefficient of β^i in β^{-1} for i in [0, d-2]:
    //   -c[d-2-i] / c[d-1] = -charpoly_.coeff_(1+i) / charpoly_.coeff_(0)
    // Coefficient of β^{d-1} in β^{-1}:
    //   -1 / c[d-1] = -1 / charpoly_.coeff_(0)
    QElem beta_inverse() const {
        const std::size_t d = degree();
        if (is_zero(charpoly_.coeff(0))) {
            throw std::invalid_argument("beta_inverse: c[d-1] = 0 (β not invertible)");
        }
        QElem r(d);
        for (std::size_t i = 0; i + 1 < d; ++i) {
            // coefficient of β^i in β^{-1}: -charpoly_.coeff_(1+i) / charpoly_.coeff_(0)
            Rat num; set(num, charpoly_.coeff(1 + i));
            mathlib::neg(num);
            Rat den; set(den, charpoly_.coeff(0));
            mathlib::div(r.coeffs_[i], num, den);
        }
        // coefficient of β^{d-1}: -1 / charpoly_.coeff_(0)
        Rat num; set_si(num, -1, 1);
        Rat den; set(den, charpoly_.coeff(0));
        mathlib::div(r.coeffs_[d - 1], num, den);
        return r;
    }

    // ----- Test helpers -----

    bool charpoly_matches(const PolyZ& p) const {
        return charpoly_ == p;
    }
};

}  // namespace mathlib
