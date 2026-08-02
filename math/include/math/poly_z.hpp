// math/poly_z.hpp
//
// Polynomial over Z (integer coefficients).  Built on mathlib's BigInt
// wrappers around mini-gmp mpz_t.
//
// Convention: coeffs_[i] is the coefficient of x^i.  trim() trims
// trailing zeros and ensures at least coeffs_[0] is present (the zero
// polynomial has degree -inf, but we always store at least one
// coefficient).
//
// All operations are GENERAL-d (no upper bound on degree).
//
// Reference: SymPy Poly with domain=ZZ.

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"

namespace mathlib {

class PolyZ {
public:
    std::vector<BigInt> coeffs_;

    PolyZ() : coeffs_(1) { set_ui(coeffs_[0], 0); }
    explicit PolyZ(long long c) : coeffs_(1) { set_si(coeffs_[0], c); }
    PolyZ(std::initializer_list<long long> cs) : coeffs_(cs.size()) {
        std::size_t i = 0;
        for (auto c : cs) { set_si(coeffs_[i], c); ++i; }
    }

    // Check coefficient accessor (clamped; auto-extends with zeros).
    BigInt& coeff(std::size_t i) {
        if (i >= coeffs_.size()) {
            coeffs_.resize(i + 1);
            for (std::size_t k = coeffs_.size(); k <= i; ++k) set_ui(coeffs_[k], 0);
            coeffs_.resize(i + 1);
        }
        return coeffs_[i];
    }
    const BigInt& coeff(std::size_t i) const {
        return coeffs_[i];
    }
    void ensure_size(std::size_t n) {
        if (coeffs_.size() < n) {
            std::size_t old = coeffs_.size();
            coeffs_.resize(n);
            for (std::size_t k = old; k < n; ++k) set_ui(coeffs_[k], 0);
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

    bool operator==(const PolyZ& o) const {
        long long da = degree(), db = o.degree();
        if (da != db) return false;
        if (da < 0) return true;  // both the zero polynomial (degree -1);
                                   // casting da to size_t below would
                                   // otherwise underflow to SIZE_MAX and
                                   // walk off the end of coeffs_ -- found
                                   // 2026-08-02 via a real crash, not by
                                   // inspection.
        for (std::size_t i = 0; i <= static_cast<std::size_t>(da); ++i) {
            if (cmp(coeffs_[i], o.coeffs_[i]) != 0) return false;
        }
        return true;
    }
    bool operator!=(const PolyZ& o) const { return !(*this == o); }
};

inline PolyZ zero_z() { return PolyZ(); }
inline PolyZ constant_z(long long c) { return PolyZ(c); }
inline PolyZ constant_z(const BigInt& c) {
    PolyZ p;
    set(p.coeffs_[0], c);
    return p;
}

inline std::string str(const PolyZ& p) {
    long long d = p.degree();
    if (d < 0) return "0";
    std::string out;
    for (long long i = d; i >= 0; --i) {
        if (i < d) out += " + ";
        if (i == 0) {
            out += str(p.coeffs_[0]);
        } else if (i == 1) {
            if (cmp_si(p.coeffs_[1], 1) == 0) out += "x";
            else if (cmp_si(p.coeffs_[1], -1) == 0) out += "-x";
            else { out += str(p.coeffs_[1]); out += "*x"; }
        } else {
            if (cmp_si(p.coeffs_[i], 1) == 0) out += "x^";
            else if (cmp_si(p.coeffs_[i], -1) == 0) out += "-x^";
            else { out += str(p.coeffs_[i]); out += "*x^"; }
            out += std::to_string(i);
        }
    }
    return out;
}

inline PolyZ operator-(const PolyZ& a) {
    PolyZ r = a;
    for (auto& c : r.coeffs_) neg(c);
    return r;
}

inline PolyZ operator+(const PolyZ& a, const PolyZ& b) {
    PolyZ r;
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

inline PolyZ operator-(const PolyZ& a, const PolyZ& b) {
    PolyZ r;
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

inline PolyZ operator*(const PolyZ& a, const PolyZ& b) {
    if (a.is_zero() || b.is_zero()) return PolyZ();
    std::size_t n = a.coeffs_.size() + b.coeffs_.size() - 1;
    PolyZ r;
    r.coeffs_.resize(n);
    for (auto& c : r.coeffs_) set_ui(c, 0);
    for (std::size_t i = 0; i < a.coeffs_.size(); ++i) {
        if (sgn(a.coeffs_[i]) == 0) continue;
        for (std::size_t j = 0; j < b.coeffs_.size(); ++j) {
            if (sgn(b.coeffs_[j]) == 0) continue;
            BigInt tmp;
            mul(tmp, a.coeffs_[i], b.coeffs_[j]);
            add(r.coeffs_[i + j], r.coeffs_[i + j], tmp);
        }
    }
    r.trim();
    return r;
}

inline PolyZ operator*(const PolyZ& a, long long s) {
    PolyZ r = a;
    for (auto& c : r.coeffs_) {
        if (s >= 0) {
            mpz_mul_ui(c.get(), c.get(), static_cast<unsigned long>(s));
        } else {
            mpz_mul_si(c.get(), c.get(), s);
        }
    }
    return r;
}
inline PolyZ operator*(long long s, const PolyZ& a) { return a * s; }

inline PolyZ scale_poly(const PolyZ& p, const BigInt& s) {
    PolyZ r = p;
    for (auto& c : r.coeffs_) mul(c, c, s);
    return r;
}
inline PolyZ scale_poly(const PolyZ& p, long long s) {
    return p * s;
}

inline BigInt eval(const PolyZ& p, const BigInt& x) {
    BigInt r; set_ui(r, 0);
    for (long long i = p.degree(); i >= 0; --i) {
        mul(r, r, x);
        add(r, r, p.coeff(static_cast<std::size_t>(i)));
    }
    return r;
}

// ---- Pseudo-division in Z[x] ----
//
// Returns (q, r) with lc(b)^(da-db+1) * a = q*b + r.  Use this for
// Z[x] GCD via the primitive-part / pseudo-remainder loop (it always
// terminates in Z[x] because lc(b)^k forces leading-coefficient
// divisibility).
//
// Reference: Knuth, TAOCP Vol 2, §4.6.1; Cohen, CANT, §3.1.

struct DivModResultZ {
    PolyZ q;
    PolyZ r;
};

inline DivModResultZ divmod(const PolyZ& a, const PolyZ& b) {
    if (b.is_zero()) throw std::invalid_argument("PolyZ divmod by zero");
    long long da = a.degree(), db = b.degree();
    if (da < db) {
        return DivModResultZ{PolyZ(), a};
    }
    BigInt lc_b = b.coeff(static_cast<std::size_t>(db));
    // r := lc(b)^(da-db+1) * a
    PolyZ r = a;
    for (long long k = 0; k < da - db + 1; ++k) {
        r = scale_poly(r, lc_b);
    }
    PolyZ q;
    q.ensure_size(static_cast<std::size_t>(da - db + 1));
    for (long long i = da; i >= db; --i) {
        if (sgn(r.coeff(static_cast<std::size_t>(i))) == 0) continue;
        long long shift = i - db;
        // q[shift] += r[i] / lc_b
        BigInt qc;
        divexact(qc, r.coeff(static_cast<std::size_t>(i)), lc_b);
        add(q.coeff(static_cast<std::size_t>(shift)), q.coeff(static_cast<std::size_t>(shift)), qc);
        // r -= qc * x^shift * b
        for (long long j = 0; j <= db; ++j) {
            BigInt prod;
            mul(prod, qc, b.coeff(static_cast<std::size_t>(j)));
            sub(r.coeff(static_cast<std::size_t>(shift + j)),
                r.coeff(static_cast<std::size_t>(shift + j)), prod);
        }
    }
    r.trim();
    q.trim();
    return DivModResultZ{q, r};
}

// ---- GCD in Z[x] ----

inline BigInt content(const PolyZ& p) {
    BigInt g; set_ui(g, 0);
    for (const auto& c : p.coeffs_) {
        gcd(g, g, c);
    }
    BigInt sign;
    if (sgn(p.coeff(static_cast<std::size_t>(p.degree()))) < 0) set_si(sign, -1);
    else set_si(sign, 1);
    mul(g, g, sign);
    return g;
}

inline PolyZ primitive_part(const PolyZ& p) {
    if (p.is_zero()) return PolyZ();
    BigInt c = content(p);
    PolyZ r = p;
    for (auto& cc : r.coeffs_) {
        divexact(cc, cc, c);
    }
    return r;
}

inline PolyZ gcd(const PolyZ& a, const PolyZ& b) {
    if (a.is_zero()) return primitive_part(b);
    if (b.is_zero()) return primitive_part(a);
    PolyZ A = primitive_part(a);
    PolyZ B = primitive_part(b);
    while (!B.is_zero()) {
        DivModResultZ dm = divmod(A, B);
        A = B;
        B = primitive_part(dm.r);
    }
    return A;
}

}  // namespace mathlib
