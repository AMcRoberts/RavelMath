// math/bigint.hpp
//
// RAII thin wrapper around mini-gmp's mpz_t / mpq_t.
// Used by poly_z, poly_q, mat_q, qbeta, bezout, linalg_qbeta.
//
// Memory model: mpz_t is __mpz_struct[1] (or mpq_t is __mpq_struct[1]),
// which is not movable, not assignable, can't go in std::vector.  We
// allocate each BigInt / Rat on the heap as a raw pointer (mpz_ptr /
// mpq_ptr) and own it individually.  The wrapper is move-only-when-safe
// (move transfers the heap pointer, copy deep-copies).
//
// Free-function style for arithmetic: we expose `add(r, a, b)`,
// `mul(r, a, b)`, etc., that work on the wrappers.  This is ADL-friendly
// and lets poly_z / poly_q / mat_q / qbeta all use the same names.
//
// All operations are EXACT (no double precision anywhere).

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>

#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"

namespace mathlib {

// ===================================================================
// BigInt: arbitrary-precision integer
// ===================================================================

inline mpz_ptr mpz_new() {
    mpz_ptr p = static_cast<mpz_ptr>(std::malloc(sizeof(__mpz_struct)));
    if (!p) throw std::bad_alloc();
    mpz_init(p);
    return p;
}
inline mpz_ptr mpz_new_si(long long v) {
    mpz_ptr p = static_cast<mpz_ptr>(std::malloc(sizeof(__mpz_struct)));
    if (!p) throw std::bad_alloc();
    mpz_init_set_si(p, v);
    return p;
}
inline mpz_ptr mpz_new_set(const mpz_t o) {
    mpz_ptr p = static_cast<mpz_ptr>(std::malloc(sizeof(__mpz_struct)));
    if (!p) throw std::bad_alloc();
    mpz_init_set(p, o);
    return p;
}
inline void mpz_drop(mpz_ptr& p) {
    if (p) { mpz_clear(p); std::free(p); p = nullptr; }
}

class BigInt {
    mpz_ptr v_;
public:
    BigInt() : v_(mpz_new()) {}
    explicit BigInt(long long x) : v_(mpz_new_si(x)) {}
    BigInt(const BigInt& o) : v_(mpz_new_set(o.v_)) {}
    BigInt(BigInt&& o) noexcept : v_(o.v_) { o.v_ = nullptr; }
    BigInt& operator=(const BigInt& o) {
        if (this != &o) mpz_set(v_, o.v_);
        return *this;
    }
    BigInt& operator=(BigInt&& o) noexcept {
        if (this != &o) {
            mpz_drop(v_);
            v_ = o.v_; o.v_ = nullptr;
        }
        return *this;
    }
    ~BigInt() { mpz_drop(v_); }

    mpz_ptr get() { return v_; }
    mpz_ptr get() const { return v_; }
    void swap(BigInt& o) noexcept { std::swap(v_, o.v_); }

    bool operator==(const BigInt& o) const { return mpz_cmp(v_, o.v_) == 0; }
    bool operator!=(const BigInt& o) const { return mpz_cmp(v_, o.v_) != 0; }
};

inline void swap(BigInt& a, BigInt& b) noexcept { a.swap(b); }

// ===================================================================
// Rat: arbitrary-precision rational (always kept in canonical form)
// ===================================================================

inline mpq_ptr mpq_new() {
    mpq_ptr p = static_cast<mpq_ptr>(std::malloc(sizeof(__mpq_struct)));
    if (!p) throw std::bad_alloc();
    mpq_init(p);
    return p;
}
inline mpq_ptr mpq_new_si(long long num, long long den = 1) {
    mpq_ptr p = mpq_new();
    mpq_set_si(p, num, den);
    return p;
}
inline mpq_ptr mpq_new_set(const mpq_t o) {
    mpq_ptr p = mpq_new();
    mpq_set(p, o);
    return p;
}
inline void mpq_drop(mpq_ptr& p) {
    if (p) { mpq_clear(p); std::free(p); p = nullptr; }
}

class Rat {
    mpq_ptr v_;
public:
    Rat() : v_(mpq_new()) {}
    explicit Rat(long long n) : v_(mpq_new_si(n, 1)) {}
    Rat(long long n, long long d) : v_(mpq_new_si(n, d)) {}
    explicit Rat(const BigInt& n) : v_(mpq_new()) {
        mpq_set_z(v_, n.get());
    }
    Rat(const Rat& o) : v_(mpq_new_set(o.v_)) {}
    Rat(Rat&& o) noexcept : v_(o.v_) { o.v_ = nullptr; }
    Rat& operator=(const Rat& o) {
        if (this != &o) mpq_set(v_, o.v_);
        return *this;
    }
    Rat& operator=(Rat&& o) noexcept {
        if (this != &o) {
            mpq_drop(v_);
            v_ = o.v_; o.v_ = nullptr;
        }
        return *this;
    }
    ~Rat() { mpq_drop(v_); }

    mpq_ptr get() { return v_; }
    mpq_ptr get() const { return v_; }
    void swap(Rat& o) noexcept { std::swap(v_, o.v_); }

    bool operator==(const Rat& o) const { return mpq_equal(v_, o.v_) != 0; }
    bool operator!=(const Rat& o) const { return mpq_equal(v_, o.v_) == 0; }
};

inline void swap(Rat& a, Rat& b) noexcept { a.swap(b); }

// ===================================================================
// BigInt operations
// ===================================================================

inline void set_si(BigInt& x, long long v) { mpz_set_si(x.get(), v); }
inline void set_si(BigInt& x, int v) { mpz_set_si(x.get(), v); }
inline void set_ui(BigInt& x, unsigned long v) { mpz_set_ui(x.get(), v); }
inline void set(BigInt& x, const BigInt& y) { mpz_set(x.get(), y.get()); }
inline int sgn(const BigInt& x) { return mpz_sgn(x.get()); }
inline int cmp(const BigInt& a, const BigInt& b) { return mpz_cmp(a.get(), b.get()); }
inline int cmp_si(const BigInt& a, long long b) { return mpz_cmp_si(a.get(), b); }
inline bool is_zero(const BigInt& x) { return mpz_sgn(x.get()) == 0; }
inline bool is_one(const BigInt& x) { return mpz_cmp_si(x.get(), 1) == 0; }
inline bool is_mone(const BigInt& x) { return mpz_cmp_si(x.get(), -1) == 0; }

inline void add(BigInt& r, const BigInt& a, const BigInt& b) { mpz_add(r.get(), a.get(), b.get()); }
inline void sub(BigInt& r, const BigInt& a, const BigInt& b) { mpz_sub(r.get(), a.get(), b.get()); }
inline void mul(BigInt& r, const BigInt& a, const BigInt& b) { mpz_mul(r.get(), a.get(), b.get()); }
inline void neg(BigInt& x) { mpz_neg(x.get(), x.get()); }
inline void neg(BigInt& x, const BigInt& y) { mpz_neg(x.get(), y.get()); }
inline void abs_(BigInt& x) { mpz_abs(x.get(), x.get()); }
inline void abs_(BigInt& x, const BigInt& y) { mpz_abs(x.get(), y.get()); }
inline void gcd(BigInt& r, const BigInt& a, const BigInt& b) { mpz_gcd(r.get(), a.get(), b.get()); }
inline void gcd(BigInt& r, BigInt& a, BigInt& b) { mpz_gcd(r.get(), a.get(), b.get()); }
inline void divexact(BigInt& q, const BigInt& a, const BigInt& b) {
    if (is_zero(b)) throw std::invalid_argument("BigInt divexact by zero");
    if (!mpz_divisible_p(a.get(), b.get())) {
        throw std::invalid_argument("BigInt divexact: not exact");
    }
    mpz_divexact(q.get(), a.get(), b.get());
}
inline void tdiv_q(BigInt& q, const BigInt& a, const BigInt& b) {
    if (is_zero(b)) throw std::invalid_argument("BigInt tdiv_q by zero");
    mpz_tdiv_q(q.get(), a.get(), b.get());
}
inline void mul_si(BigInt& r, const BigInt& a, long long s) {
    mpz_mul_si(r.get(), a.get(), s);
}
inline void add_si(BigInt& r, const BigInt& a, long long s) {
    // Computes r = a + s.  Implemented with `mpz_set` + `mpz_add_ui` /
    // `mpz_sub_ui` so we don't need a multi-precision signed-small-int
    // primitive (mini-gmp lacks `mpz_addmul_si`, only `mpz_addmul_ui`).
    mpz_set(r.get(), a.get());
    if (s >= 0) {
        mpz_add_ui(r.get(), r.get(), static_cast<unsigned long>(s));
    } else {
        mpz_sub_ui(r.get(), r.get(), static_cast<unsigned long>(-s));
    }
}
inline void mul_ui(BigInt& r, const BigInt& a, unsigned long s) {
    mpz_mul_ui(r.get(), a.get(), s);
}
inline void addmul(BigInt& r, const BigInt& a, const BigInt& b) { mpz_addmul(r.get(), a.get(), b.get()); }
inline void submul(BigInt& r, const BigInt& a, const BigInt& b) { mpz_submul(r.get(), a.get(), b.get()); }
inline void addmul_ui(BigInt& r, const BigInt& a, unsigned long s) { mpz_addmul_ui(r.get(), a.get(), s); }
inline void submul_ui(BigInt& r, const BigInt& a, unsigned long s) { mpz_submul_ui(r.get(), a.get(), s); }

// ===================================================================
// Rat operations
// ===================================================================

inline void set_si(Rat& x, long long n, long long d = 1) { mpq_set_si(x.get(), n, d); }
inline void set(Rat& x, const Rat& y) { mpq_set(x.get(), y.get()); }
inline void set(Rat& x, const BigInt& n, const BigInt& d) {
    mpq_set_num(x.get(), n.get());
    mpq_set_den(x.get(), d.get());
}
inline void set_z(Rat& x, const BigInt& n) { mpq_set_z(x.get(), n.get()); }
inline void set(Rat& x, const BigInt& n) { mpq_set_z(x.get(), n.get()); }
inline int sgn(const Rat& x) { return mpq_sgn(x.get()); }
inline int cmp(const Rat& a, const Rat& b) { return mpq_cmp(a.get(), b.get()); }
inline int cmp_z(const Rat& a, const BigInt& b) { return mpq_cmp_z(a.get(), b.get()); }
inline bool is_zero(const Rat& x) { return mpq_sgn(x.get()) == 0; }
inline bool is_one(const Rat& x) { return mpq_cmp_si(x.get(), 1, 1) == 0; }
inline bool is_mone(const Rat& x) { return mpq_cmp_si(x.get(), -1, 1) == 0; }

inline void neg(Rat& x) { mpq_neg(x.get(), x.get()); }
inline void neg(Rat& x, const Rat& y) { mpq_neg(x.get(), y.get()); }
inline void abs_(Rat& x) { mpq_abs(x.get(), x.get()); }
inline void abs_(Rat& x, const Rat& y) { mpq_abs(x.get(), y.get()); }
inline void add(Rat& r, const Rat& a, const Rat& b) { mpq_add(r.get(), a.get(), b.get()); }
inline void sub(Rat& r, const Rat& a, const Rat& b) { mpq_sub(r.get(), a.get(), b.get()); }
inline void mul(Rat& r, const Rat& a, const Rat& b) { mpq_mul(r.get(), a.get(), b.get()); }
inline void div(Rat& r, const Rat& a, const Rat& b) {
    if (is_zero(b)) throw std::invalid_argument("Rat division by zero");
    mpq_div(r.get(), a.get(), b.get());
}
// cmp_si for Rat (matches the BigInt counterpart).
inline int cmp_si(const Rat& a, long long b) { return mpq_cmp_si(a.get(), b, 1); }
// num(x) / den(x): pull the integer numerator/denominator of a Rat as
// BigInt.  Used by ball_sqrt / ball_cbrt for integer-sqrt-of-product
// identities like sqrt(num/den) = sqrt(num*den) / den.
inline BigInt num(const Rat& x) {
    BigInt r;
    mpz_set(r.get(), mpq_numref(x.get()));
    return r;
}
inline BigInt den(const Rat& x) {
    BigInt r;
    mpz_set(r.get(), mpq_denref(x.get()));
    return r;
}
inline void add_si(Rat& r, const Rat& a, long long s) {
    mpq_t tmp; mpq_init(tmp); mpq_set_si(tmp, s, 1);
    mpq_add(r.get(), a.get(), tmp);
    mpq_clear(tmp);
}
inline void mul_si(Rat& r, const Rat& a, long long s) {
    mpq_t tmp; mpq_init(tmp); mpq_set_si(tmp, s, 1);
    mpq_mul(r.get(), a.get(), tmp);
    mpq_clear(tmp);
}
inline void mul(Rat& r, const Rat& a, const BigInt& b) {
    // Use mpq_set_z into a temporary, then mul
    mpq_t tmp; mpq_init(tmp); mpq_set_z(tmp, b.get());
    mpq_mul(r.get(), a.get(), tmp);
    mpq_clear(tmp);
}
inline void mul(Rat& r, const BigInt& a, const Rat& b) {
    mul(r, b, a);
}
inline void addmul(Rat& r, const Rat& a, const Rat& b) { mpq_add(r.get(), r.get(), a.get()); mpq_mul(r.get(), r.get(), b.get()); }
inline void submul(Rat& r, const Rat& a, const Rat& b) { mpq_sub(r.get(), r.get(), a.get()); mpq_mul(r.get(), r.get(), b.get()); }

// ===================================================================
// IO / display
// ===================================================================

inline std::string str(const BigInt& x) {
    char* c = mpz_get_str(nullptr, 10, x.get());
    std::string s(c);
    std::free(c);
    return s;
}
inline std::string str(const Rat& x) {
    char* c = mpq_get_str(nullptr, 10, x.get());
    std::string s(c);
    std::free(c);
    return s;
}

}  // namespace mathlib
