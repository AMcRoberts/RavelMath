// math/poly_q.hpp
//
// Polynomial over Q (rational coefficients).  Built on mathlib's Rat
// wrappers around mini-gmp mpq_t.
//
// Convention: coeffs_[i] is the coefficient of x^i.  trim() trims
// trailing zeros the same way as poly_z.
//
// All operations are GENERAL-d.
//
// Reference: SymPy Poly with domain=QQ.

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"

namespace mathlib {

class PolyQ {
public:
    std::vector<Rat> coeffs_;

    PolyQ() : coeffs_(1) { set_si(coeffs_[0], 0, 1); }
    explicit PolyQ(long long n) : coeffs_(1) { set_si(coeffs_[0], n, 1); }
    PolyQ(long long n, long long d) : coeffs_(1) { set_si(coeffs_[0], n, d); }
    PolyQ(std::initializer_list<long long> ns) : coeffs_(ns.size()) {
        std::size_t i = 0;
        for (auto n : ns) { set_si(coeffs_[i], n, 1); ++i; }
    }
    PolyQ(std::initializer_list<std::pair<long long,long long>> rcs) : coeffs_(rcs.size()) {
        std::size_t i = 0;
        for (auto [n, d] : rcs) { set_si(coeffs_[i], n, d); ++i; }
    }
    explicit PolyQ(const PolyZ& p) : coeffs_(p.coeffs_.size()) {
        for (std::size_t i = 0; i < coeffs_.size(); ++i) {
            mpq_set_z(coeffs_[i].get(), p.coeffs_[i].get());
        }
    }

    Rat& coeff(std::size_t i) {
        if (i >= coeffs_.size()) {
            std::size_t old = coeffs_.size();
            coeffs_.resize(i + 1);
            for (std::size_t k = old; k <= i; ++k) set_si(coeffs_[k], 0, 1);
        }
        return coeffs_[i];
    }
    const Rat& coeff(std::size_t i) const { return coeffs_[i]; }
    void ensure_size(std::size_t n) {
        if (coeffs_.size() < n) {
            std::size_t old = coeffs_.size();
            coeffs_.resize(n);
            for (std::size_t k = old; k < n; ++k) set_si(coeffs_[k], 0, 1);
        }
    }

    long long degree() const {
        long long d = static_cast<long long>(coeffs_.size()) - 1;
        while (d > 0 && sgn(coeffs_[d]) == 0) --d;
        if (d == 0 && sgn(coeffs_[0]) == 0) return -1;
        return d;
    }
    bool is_zero() const { return degree() < 0; }
    void trim() {
        std::size_t n = coeffs_.size();
        while (n > 1 && sgn(coeffs_[n - 1]) == 0) --n;
        coeffs_.resize(n);
    }

    bool operator==(const PolyQ& o) const {
        long long da = degree(), db = o.degree();
        if (da != db) return false;
        for (std::size_t i = 0; i <= static_cast<std::size_t>(da); ++i) {
            if (cmp(coeffs_[i], o.coeffs_[i]) != 0) return false;
        }
        return true;
    }
    bool operator!=(const PolyQ& o) const { return !(*this == o); }
};

inline PolyQ zero_q() { return PolyQ(); }
inline PolyQ constant_q(long long n) { return PolyQ(n); }
inline PolyQ constant_q(long long n, long long d) { return PolyQ(n, d); }

inline std::string str(const PolyQ& p) {
    long long d = p.degree();
    if (d < 0) return "0";
    std::string out;
    for (long long i = d; i >= 0; --i) {
        if (i < d) out += " + ";
        if (i == 0) {
            out += str(p.coeffs_[0]);
        } else if (i == 1) {
            if (cmp(p.coeffs_[1], Rat(1)) == 0) out += "x";
            else if (cmp(p.coeffs_[1], Rat(-1)) == 0) out += "-x";
            else { out += str(p.coeffs_[1]); out += "*x"; }
        } else {
            if (cmp(p.coeffs_[i], Rat(1)) == 0) { out += "x^"; out += std::to_string(i); }
            else if (cmp(p.coeffs_[i], Rat(-1)) == 0) { out += "-x^"; out += std::to_string(i); }
            else { out += str(p.coeffs_[i]); out += "*x^"; out += std::to_string(i); }
        }
    }
    return out;
}

inline PolyQ operator-(const PolyQ& a) {
    PolyQ r = a;
    for (auto& c : r.coeffs_) neg(c);
    return r;
}

inline PolyQ operator+(const PolyQ& a, const PolyQ& b) {
    PolyQ r;
    std::size_t n = std::max(a.coeffs_.size(), b.coeffs_.size());
    r.coeffs_.resize(n);
    for (std::size_t i = 0; i < n; ++i) {
        if (i < a.coeffs_.size() && i < b.coeffs_.size()) {
            add(r.coeffs_[i], a.coeffs_[i], b.coeffs_[i]);
        } else if (i < a.coeffs_.size()) {
            r.coeffs_[i] = a.coeffs_[i];
        } else {
            r.coeffs_[i] = b.coeffs_[i];
        }
    }
    r.trim();
    return r;
}

inline PolyQ operator-(const PolyQ& a, const PolyQ& b) {
    PolyQ r;
    std::size_t n = std::max(a.coeffs_.size(), b.coeffs_.size());
    r.coeffs_.resize(n);
    for (std::size_t i = 0; i < n; ++i) {
        if (i < a.coeffs_.size() && i < b.coeffs_.size()) {
            sub(r.coeffs_[i], a.coeffs_[i], b.coeffs_[i]);
        } else if (i < a.coeffs_.size()) {
            r.coeffs_[i] = a.coeffs_[i];
        } else {
            neg(r.coeffs_[i], b.coeffs_[i]);
        }
    }
    r.trim();
    return r;
}

inline PolyQ operator*(const PolyQ& a, const PolyQ& b) {
    if (a.is_zero() || b.is_zero()) return PolyQ();
    std::size_t n = a.coeffs_.size() + b.coeffs_.size() - 1;
    PolyQ r;
    r.coeffs_.resize(n);
    for (auto& c : r.coeffs_) set_si(c, 0, 1);
    for (std::size_t i = 0; i < a.coeffs_.size(); ++i) {
        if (sgn(a.coeffs_[i]) == 0) continue;
        for (std::size_t j = 0; j < b.coeffs_.size(); ++j) {
            if (sgn(b.coeffs_[j]) == 0) continue;
            Rat tmp;
            mul(tmp, a.coeffs_[i], b.coeffs_[j]);
            add(r.coeffs_[i + j], r.coeffs_[i + j], tmp);
        }
    }
    r.trim();
    return r;
}

inline PolyQ operator*(const PolyQ& a, long long s) {
    PolyQ r = a;
    for (auto& c : r.coeffs_) {
        Rat t; set_si(t, s, 1);
        mul(c, c, t);
    }
    return r;
}
inline PolyQ operator*(long long s, const PolyQ& a) { return a * s; }

inline PolyQ operator*(const PolyQ& a, const Rat& s) {
    PolyQ r = a;
    for (auto& c : r.coeffs_) mul(c, c, s);
    return r;
}
inline PolyQ operator*(const Rat& s, const PolyQ& a) { return a * s; }

inline Rat eval(const PolyQ& p, const Rat& x) {
    Rat r; set_si(r, 0, 1);
    for (long long i = p.degree(); i >= 0; --i) {
        mul(r, r, x);
        add(r, r, p.coeff(static_cast<std::size_t>(i)));
    }
    return r;
}

// ---- Exact division in Q[x] ----
//
// In Q[x], division is exact: returns (q, r) with a = q b + r and
// deg(r) < deg(b).  The division has no pseudo-step.

struct DivModResultQ {
    PolyQ q;
    PolyQ r;
};

inline DivModResultQ divmod(const PolyQ& a, const PolyQ& b) {
    if (b.is_zero()) throw std::invalid_argument("PolyQ divmod by zero");
    long long da = a.degree(), db = b.degree();
    if (da < db) return DivModResultQ{PolyQ(), a};
    Rat lc_b = b.coeff(static_cast<std::size_t>(db));
    PolyQ r = a;
    PolyQ q;
    q.ensure_size(static_cast<std::size_t>(da - db + 1));
    for (long long i = da; i >= db; --i) {
        if (sgn(r.coeff(static_cast<std::size_t>(i))) == 0) continue;
        long long shift = i - db;
        Rat qc;
        div(qc, r.coeff(static_cast<std::size_t>(i)), lc_b);
        add(q.coeff(static_cast<std::size_t>(shift)), q.coeff(static_cast<std::size_t>(shift)), qc);
        for (long long j = 0; j <= db; ++j) {
            Rat prod;
            mul(prod, qc, b.coeff(static_cast<std::size_t>(j)));
            sub(r.coeff(static_cast<std::size_t>(shift + j)),
                r.coeff(static_cast<std::size_t>(shift + j)), prod);
        }
    }
    r.trim();
    q.trim();
    return DivModResultQ{q, r};
}

// ---- GCD in Q[x] ----
//
// In Q[x], every polynomial is "primitive" (content = 1 up to sign).
// GCD via Euclidean division in Q[x] is exact.

inline PolyQ gcd(const PolyQ& a, const PolyQ& b) {
    if (a.is_zero()) {
        if (b.is_zero()) return PolyQ();
        PolyQ r = b;
        Rat lc = r.coeff(static_cast<std::size_t>(r.degree()));
        for (auto& cc : r.coeffs_) div(cc, cc, lc);
        return r;
    }
    if (b.is_zero()) {
        PolyQ r = a;
        Rat lc = r.coeff(static_cast<std::size_t>(r.degree()));
        for (auto& cc : r.coeffs_) div(cc, cc, lc);
        return r;
    }
    PolyQ A = a, B = b;
    // Make A monic
    {
        Rat lcA = A.coeff(static_cast<std::size_t>(A.degree()));
        for (auto& cc : A.coeffs_) div(cc, cc, lcA);
    }
    while (!B.is_zero()) {
        DivModResultQ dm = divmod(A, B);
        A = B;
        B = dm.r;
        if (!B.is_zero()) {
            Rat lcB = B.coeff(static_cast<std::size_t>(B.degree()));
            for (auto& cc : B.coeffs_) div(cc, cc, lcB);
        }
    }
    return A;
}

}  // namespace mathlib
