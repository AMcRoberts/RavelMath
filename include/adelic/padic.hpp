// include/adelic/padic.hpp
//
// p-adic arithmetic, per the adelic tiling classifier
// (Minervino-Thuswaldner §3.2).  All arithmetic is exact in the
// "finite-precision p-adic" sense: we work modulo p^precision
// (i.e., to N digits of p-adic precision).  This is enough to
// distinguish the finitely many translation classes that arise
// in the strong-coincidence and property-(F) checks.
//
// Two layers:
//
//   1. Z_p: p-adic integers, digits in [0, p), base p, working
//      precision p^precision.  All arithmetic stays in
//      [0, p^precision).  Valuation is the exponent of p
//      dividing the element (>= 0 for Z_p elements).
//
//   2. K_p: the completion K_p = Q_p(π) / g(π), where g is an
//      irreducible factor of the char poly of β modulo p
//      (so π is the uniformizer in the ramified case, or π = p
//      itself in the unramified case).  An element is represented
//      in the basis (1, π, π^2, ..., π^{ef-1}) with ef = deg(g),
//      each coefficient being a Z_p element.  Multiplication is
//      via the relation g(π) = 0.
//
// The default working precision is 40 digits (configurable at
// construction).  The Minervino-Thuswaldner spec recommends
// 40-60 digits; 40 is enough for all the graph-based checks
// since the zero-expansion graph has only finitely many nodes
// in the explicit bound M.

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/qbeta.hpp"

namespace adelic {

// ===================================================================
// Z_p: p-adic integers at finite precision
// ===================================================================
//
// Normalized so that v_p(x) >= 0 (i.e., x in Z_p, not p^(-1) Z_p).
// A Z_p element is a vector of `precision` digits in [0, p), with
// the implicit understanding that higher digits are zero (truncated
// to the working precision).  Valuation of the element is the
// smallest index k such that digits[k] != 0, or `precision` if
// the element is zero (in which case v_p(x) = +infinity, capped
// at `precision` here).
//
// Memory: digits are long long (small range).  The actual
// working precision is bounded by what long long can hold;
// 40 digits of p-adic precision (the default) is well within
// range for any p we care about (the rational primes that
// divide |det M| in the adelic classifier are small).
//
// Invariant: a.precision is ALWAYS equal to a.digits.size().  The
// previous design kept these as two independent fields and let
// them drift out of sync (a ZpInt default-constructed with empty
// digits, then assigned a `precision` value, would silently have
// digits.size() == 0 and any indexed read would be a heap-OOB --
// the actual non-determinism source behind the intermittently-
// crashing make_combined_padic_bound).  This version enforces
// the invariant structurally: `precision` is a const accessor
// returning digits.size(), and the only way to grow precision is
// the `extend_to(n)` method, which resizes the digits vector.
// `truncate_to(n)` shrinks if requested, and is the only way to
// drop precision.

struct ZpInt {
    long long p;                       // The prime (must be > 1)
    std::vector<long long> digits;     // digits[i] in [0, p), low-order first

    // Current working precision.  Identical to digits.size() by
    // invariant; never independently settable.  This accessor
    // is the only public read of "precision".
    long long precision() const {
        return static_cast<long long>(digits.size());
    }

    // Grow precision in place to exactly n digits.  If n <= current
    // size, this is a no-op.  If n > current size, new digits are
    // zero-padded.  Use this as the SOLE way to grow precision --
    // it keeps the invariant digits.size() == precision.
    ZpInt& extend_to(long long n) {
        if (n < 0) throw std::invalid_argument("ZpInt::extend_to: negative size");
        if (static_cast<long long>(digits.size()) < n) {
            digits.resize(static_cast<std::size_t>(n), 0);
        }
        return *this;
    }

    // Shrink precision to n digits.  If n >= current size, no-op.
    // (Distinct from extend_to, which never shrinks.)
    ZpInt& truncate_to(long long n) {
        if (n < 0) throw std::invalid_argument("ZpInt::truncate_to: negative size");
        if (static_cast<long long>(digits.size()) > n) {
            digits.resize(static_cast<std::size_t>(n));
        }
        return *this;
    }

    // Unified precision setter: grow OR shrink to exactly n digits
    // in one call.  When n > current size, new high digits are
    // zero-padded (extend); when n < current size, the high digits
    // are dropped (truncate); when n == current size, no-op.  This
    // is the recommended API for the common "I want this many digits
    // of precision" case -- it picks the right direction automatically
    // and never leaves the digits vector out of sync with the new
    // precision value.  Same invariants as extend_to / truncate_to,
    // expressed as a single resize.
    ZpInt& set_precision(long long n) {
        if (n < 0) throw std::invalid_argument("ZpInt::set_precision: negative size");
        long long cur = static_cast<long long>(digits.size());
        if (cur == n) return *this;
        if (cur < n) {
            digits.resize(static_cast<std::size_t>(n), 0);  // grow: zero-pad
        } else {
            digits.resize(static_cast<std::size_t>(n));        // shrink: drop high
        }
        return *this;
    }
};

// Forward declaration: zp_neg is defined further below; zp_from_int
// needs it to represent negative integers.
inline ZpInt zp_neg(const ZpInt& a);

// Construction helpers.  All return a value with `precision`
// digits; the caller chooses the precision.  Default precision
// is 40, per the Minervino-Thuswaldner recommendation.
inline ZpInt zp_zero(long long p, long long precision = 40) {
    if (p < 2) throw std::invalid_argument("zp_zero: p must be prime and >= 2");
    if (precision < 1) throw std::invalid_argument("zp_zero: precision must be >= 1");
    ZpInt z;
    z.p = p;
    z.extend_to(precision);
    return z;
}
inline ZpInt zp_one(long long p, long long precision = 40) {
    ZpInt z = zp_zero(p, precision);
    z.digits[0] = 1 % p;
    return z;
}
inline ZpInt zp_from_int(long long p, long long n, long long precision = 40) {
    ZpInt z = zp_zero(p, precision);
    long long v = ((n % p) + p) % p;  // normalize to [0, p)
    z.digits[0] = v;
    return z;
}

// Full base-p expansion of an arbitrary integer n (|n| can be >= p,
// unlike zp_from_int above which deliberately only stores n mod p
// in digit 0 -- see its own tests).  Needed wherever an actual
// multi-digit integer constant (e.g. a charpoly coefficient) must
// be embedded as a genuine Z_p value, not just a residue.  Handles
// negative n via p^precision + n (zp_neg on the |n| expansion).
inline ZpInt zp_from_int_full(long long p, long long n, long long precision = 40) {
    ZpInt z = zp_zero(p, precision);
    long long av = n < 0 ? -n : n;
    for (long long i = 0; i < precision && av != 0; ++i) {
        z.digits[static_cast<std::size_t>(i)] = av % p;
        av /= p;
    }
    return n < 0 ? zp_neg(z) : z;
}

// p-adic valuation: smallest k with digits[k] != 0, or `precision`
// (capped infinity) if the element is zero.  With the new ZpInt
// invariant (precision == digits.size()), this is just digits.size()
// when the element is zero.
inline long long zp_valuation(const ZpInt& a) {
    long long n = a.precision();
    for (long long k = 0; k < n; ++k) {
        if (a.digits[static_cast<std::size_t>(k)] != 0) return k;
    }
    return n;
}

// Equality.
inline bool zp_equal(const ZpInt& a, const ZpInt& b) {
    if (a.p != b.p) return false;
    long long n = std::min(a.precision(), b.precision());
    for (long long k = 0; k < n; ++k) {
        if (a.digits[static_cast<std::size_t>(k)] !=
            b.digits[static_cast<std::size_t>(k)]) return false;
    }
    // The "tail" past min(precision_a, precision_b) is zero
    // by construction (digits are always padded to precision),
    // so the values are equal iff the shared prefix agrees.
    return true;
}

// Addition with carry.  Both operands must have the same p and
// the same precision (or be "broadcast" to the larger of the
// two precisions by zero-padding — this implementation assumes
// the caller has already done that).
inline ZpInt zp_add(const ZpInt& a, const ZpInt& b) {
    if (a.p != b.p) {
        throw std::invalid_argument("zp_add: mismatched primes");
    }
    if (a.precision() != b.precision()) {
        throw std::invalid_argument("zp_add: mismatched precisions (caller must broadcast)");
    }
    ZpInt r = zp_zero(a.p, a.precision());
    long long carry = 0;
    for (long long k = 0; k < r.precision(); ++k) {
        long long sum = a.digits[static_cast<std::size_t>(k)]
                      + b.digits[static_cast<std::size_t>(k)]
                      + carry;
        r.digits[static_cast<std::size_t>(k)] = sum % r.p;
        carry = sum / r.p;
    }
    // Any final carry beyond `precision` is dropped (truncation).
    return r;
}

// Subtraction with borrow.  Result is (a - b) mod p^precision,
// which is well-defined since we're working in Z/p^precision Z.
inline ZpInt zp_sub(const ZpInt& a, const ZpInt& b) {
    if (a.p != b.p) {
        throw std::invalid_argument("zp_sub: mismatched primes");
    }
    if (a.precision() != b.precision()) {
        throw std::invalid_argument("zp_sub: mismatched precisions");
    }
    ZpInt r = zp_zero(a.p, a.precision());
    long long borrow = 0;
    for (long long k = 0; k < r.precision(); ++k) {
        long long diff = a.digits[static_cast<std::size_t>(k)]
                       - b.digits[static_cast<std::size_t>(k)]
                       - borrow;
        if (diff < 0) {
            diff += r.p;
            borrow = 1;
        } else {
            borrow = 0;
        }
        r.digits[static_cast<std::size_t>(k)] = diff;
    }
    return r;
}

// Negation: just (p - digit) mod p for each digit, with carry.
inline ZpInt zp_neg(const ZpInt& a) {
    ZpInt r = zp_zero(a.p, a.precision());
    long long b = 0;
    for (long long k = 0; k < r.precision(); ++k) {
        long long diff = 0 - a.digits[static_cast<std::size_t>(k)] - b;
        if (diff < 0) {
            diff += r.p;
            b = 1;
        } else {
            b = 0;
        }
        r.digits[static_cast<std::size_t>(k)] = diff;
    }
    return r;
}

// Multiplication.  Modulo p^precision.  The convolution
// accumulates into a wider temporary (length 2*precision) and
// the carries are propagated through the whole product; the
// result is then truncated to `precision` digits.  This handles
// the case where the convolution generates carries that need
// to propagate past the original precision boundary (e.g.,
// 6 * 7 = 42 in Z/5Z: digit 0 = 2, digit 1 = 3, digit 2 = 1,
// which needs a 3-digit wide product array).
inline ZpInt zp_mul(const ZpInt& a, const ZpInt& b) {
    if (a.p != b.p) {
        throw std::invalid_argument("zp_mul: mismatched primes");
    }
    if (a.precision() != b.precision()) {
        throw std::invalid_argument("zp_mul: mismatched precisions");
    }
    long long p = a.p;
    long long prec = a.precision();
    // Wide product array: 2*prec digits, with carries propagated.
    std::vector<long long> wide(2 * prec, 0);
    for (long long i = 0; i < prec; ++i) {
        if (a.digits[static_cast<std::size_t>(i)] == 0) continue;
        long long carry = 0;
        for (long long j = 0; j < prec; ++j) {
            long long prod = a.digits[static_cast<std::size_t>(i)]
                           * b.digits[static_cast<std::size_t>(j)]
                           + wide[static_cast<std::size_t>(i + j)]
                           + carry;
            wide[static_cast<std::size_t>(i + j)] = prod % p;
            carry = prod / p;
        }
        // Propagate any remaining carry into the higher positions.
        // The carry accumulates at position i + prec.
        long long pos = i + prec;
        while (carry > 0 && pos < 2 * prec) {
            long long sum = wide[static_cast<std::size_t>(pos)] + carry;
            wide[static_cast<std::size_t>(pos)] = sum % p;
            carry = sum / p;
            ++pos;
        }
        // Any carry beyond 2*prec is dropped (truncation).
    }
    // Truncate to precision.
    ZpInt r = zp_zero(p, prec);
    for (long long k = 0; k < prec; ++k) {
        r.digits[static_cast<std::size_t>(k)] = wide[static_cast<std::size_t>(k)];
    }
    return r;
}

// Inversion: assumes a is invertible, i.e., digits[0] != 0.
// Computes a^(-1) mod p^precision using Hensel-like lifting:
// start with r0 = (a mod p)^(-1) mod p (exists iff a is
// invertible), then iteratively lift to higher precisions via
// the Newton iteration
//   r_{k+1} = r_k · (2 - a · r_k) mod p^{2^{k+1}}.
// (Doubling scheme: each iteration doubles the effective
// precision.)  The ZpInt invariant digits.size() == precision
// means the only way to grow precision is extend_to, which
// keeps the invariant automatically -- no manual digit-count
// bookkeeping is needed, and no heap-OOB is possible.
inline ZpInt zp_invert(const ZpInt& a) {
    if (a.precision() < 1) {
        throw std::invalid_argument("zp_invert: input ZpInt has precision < 1");
    }
    if (a.digits[0] == 0) {
        throw std::invalid_argument("zp_invert: element is not invertible (digits[0] = 0)");
    }
    long long p = a.p;
    // Start with r0 = (a mod p)^(-1) mod p.
    long long inv = 0;
    for (long long k = 1; k < p; ++k) {
        if ((a.digits[0] * k) % p == 1) { inv = k; break; }
    }
    ZpInt r = zp_zero(p, a.precision());
    r.digits[0] = inv;

    long long eff_prec = 1;
    while (eff_prec < a.precision()) {
        long long next_prec = std::min(2 * eff_prec, a.precision());
        // Truncate a and r to next_prec (the high digits are
        // dropped; the low digits are kept).  extend_to grows
        // them back to full next_prec digits (zero-pad if the
        // current size is already >= next_prec, this is a no-op).
        ZpInt a_trunc = zp_zero(p, next_prec);
        ZpInt r_trunc = zp_zero(p, next_prec);
        long long copy_n = std::min(next_prec, a.precision());
        for (long long i = 0; i < copy_n; ++i) {
            a_trunc.digits[static_cast<std::size_t>(i)] = a.digits[static_cast<std::size_t>(i)];
            r_trunc.digits[static_cast<std::size_t>(i)] = r.digits[static_cast<std::size_t>(i)];
        }
        // ar = a * r at precision next_prec.
        ZpInt ar = zp_mul(a_trunc, r_trunc);
        // 2 - ar at precision next_prec. Use zp_from_int_full so the
        // base-p expansion of 2 is correctly represented (e.g., 2 in
        // base 2 is "10", digits[0]=0, digits[1]=1; setting just
        // digits[0] = 2 % p = 0 gives 0 instead of 2, breaking
        // Newton iteration at p=2).
        ZpInt two = zp_from_int_full(p, 2, next_prec);
        ZpInt two_minus_ar = zp_sub(two, ar);
        // new_r = r * (2 - ar) at precision next_prec.
        ZpInt new_r = zp_mul(r_trunc, two_minus_ar);
        // Promote r to full a.precision() (the result of zp_zero
        // is zero-padded to a.precision() digits) and copy the
        // next_prec digits from new_r.  Higher digits of r stay
        // 0 -- they get refined on subsequent iterations as
        // next_prec doubles.  extend_to is a no-op here since r
        // was just zero-padded to a.precision(); the comment is
        // kept to make the design explicit.
        r = zp_zero(p, a.precision());
        r.extend_to(next_prec);  // no-op after zp_zero; kept for clarity
        for (long long i = 0; i < next_prec; ++i) {
            r.digits[static_cast<std::size_t>(i)] = new_r.digits[static_cast<std::size_t>(i)];
        }
        eff_prec = next_prec;
    }
    return r;
}

// Stream a Z_p value as a string in base p (for debugging).
inline std::string zp_str(const ZpInt& a) {
    std::string out = "...";
    for (long long k = a.precision() - 1; k >= 0; --k) {
        out += std::to_string(a.digits[static_cast<std::size_t>(k)]);
    }
    return out + " (base " + std::to_string(a.p) + ")";
}

// Embed a rational n/d into Z_p, valid when gcd(d, p) = 1 (the
// generic case for the digit sets this classifier deals with;
// throws otherwise -- a denominator divisible by p means the
// rational is not integral at p and genuinely doesn't live in
// Z_p, which would be a real precision/design issue upstream, not
// something to silently paper over here).
inline ZpInt zp_from_rat(long long p, const mathlib::BigInt& num,
                          const mathlib::BigInt& den, long long precision = 40) {
    mathlib::BigInt pk; mathlib::set_si(pk, 1);
    mathlib::BigInt pb; mathlib::set_si(pb, p);
    for (long long i = 0; i < precision; ++i) mathlib::mul(pk, pk, pb);
    // Reduce den mod p^precision, check invertibility (gcd(den, p) = 1).
    mathlib::BigInt den_mod;
    mpz_mod(den_mod.get(), den.get(), pk.get());
    mathlib::BigInt g;
    mathlib::gcd(g, den_mod, pb);
    if (mathlib::cmp_si(g, 1) != 0) {
        throw std::invalid_argument("zp_from_rat: denominator not coprime to p "
                                     "(rational is not in Z_p at this prime)");
    }
    // den_inv = den_mod^{-1} mod p^precision, via extended gcd.
    mathlib::BigInt den_inv, t;
    mpz_t g2; mpz_init(g2);
    mpz_gcdext(g2, den_inv.get(), t.get(), den_mod.get(), pk.get());
    mpz_clear(g2);
    mathlib::BigInt num_mod, prod, prod_mod;
    mpz_mod(num_mod.get(), num.get(), pk.get());
    mathlib::mul(prod, num_mod, den_inv);
    mpz_mod(prod_mod.get(), prod.get(), pk.get());
    // Unpack prod_mod into base-p digits.
    ZpInt z = zp_zero(p, precision);
    mathlib::BigInt cur = prod_mod;
    for (long long i = 0; i < precision; ++i) {
        mathlib::BigInt q, r;
        mpz_tdiv_qr(q.get(), r.get(), cur.get(), pb.get());
        z.digits[static_cast<std::size_t>(i)] =
            static_cast<long long>(mpz_get_si(r.get()));
        cur = q;
    }
    return z;
}

// ===================================================================
// K_p: the p-adic completion at a specific prime above p.
// ===================================================================
//
// GENERAL CASE (arbitrary e, f with ef < n, several primes above p):
// NOT implemented here.  Constructing the local field for a general
// prime ideal (p, g(β)) requires either Hensel-lifting an unramified
// factor (straightforward when e_i = 1) or a ramified local-field
// construction (Montes/Ore-type algorithm, or an explicit Eisenstein
// generator) when e_i > 1 -- genuinely more machinery than this
// session builds.
//
// SPECIAL CASE implemented here: a SINGLE prime ideal above p with
// e = n (totally ramified, the whole degree absorbed by
// ramification -- f = 1, ef = n).  This is exactly rnd13's own case
// at p = 2 (dedekind_factorization_test / rnd13_factor_probe: one
// prime, e=4, f=1, n=4).  In this case K ⊗_Q Q_p is already a field
// (there being only one prime above p forces f(x), the *global*
// charpoly, to stay irreducible over Q_p even though it is NOT
// squarefree mod p) -- a standard local-global fact, not re-derived
// here -- so K_p = Q_p[x]/(f(x)) directly, using the SAME defining
// polynomial as the global field, just with Z_p coefficients instead
// of Q coefficients.  Concretely: represent elements in the power
// basis (1, β, ..., β^{n-1}) with each coefficient a ZpInt, and
// reduce products using the same recurrence β^n = -(c_{n-1}β^{n-1}
// + ... + c_0) the global QBetaRing already uses (just carried out
// in Z_p instead of Q).
//
// Consequence used by qp_valuation below: since e = n, the n power-
// basis exponents 0, 1, ..., n-1 are already pairwise DISTINCT mod
// e -- so for a "generic" element (no coefficient exactly 0 to full
// working precision), the terms a_i * β^i have pairwise distinct
// valuations i + n*v_p(a_i), and the ultrametric inequality is an
// equality: v(sum) = min_i (i + n*v_p(a_i)).  No cancellation is
// possible between terms at different valuations mod e.  This only
// holds because e = n here; it is NOT a general-e formula.
struct QpTotallyRamified {
    long long p;
    long long precision;
    long long n;                 // = e = ef (degree of the extension)
    mathlib::PolyZ defining_poly; // the global charpoly, reused as-is
    std::vector<ZpInt> coeffs;    // coeffs[i] = coefficient of beta^i, i=0..n-1
};

inline QpTotallyRamified qp_zero(long long p, long long n,
                                  const mathlib::PolyZ& defining_poly,
                                  long long precision = 40) {
    QpTotallyRamified z;
    z.p = p; z.precision = precision; z.n = n; z.defining_poly = defining_poly;
    z.coeffs.assign(static_cast<std::size_t>(n), zp_zero(p, precision));
    return z;
}

inline QpTotallyRamified qp_uniformizer(long long p, long long n,
                                         const mathlib::PolyZ& defining_poly,
                                         long long precision = 40) {
    QpTotallyRamified z = qp_zero(p, n, defining_poly, precision);
    z.coeffs[1] = zp_one(p, precision);  // beta^1
    return z;
}

inline QpTotallyRamified qp_from_qelem(long long p, long long n,
                                        const mathlib::PolyZ& defining_poly,
                                        const std::vector<mathlib::Rat>& rat_coeffs,
                                        long long precision = 40) {
    QpTotallyRamified z = qp_zero(p, n, defining_poly, precision);
    for (std::size_t i = 0; i < rat_coeffs.size() && static_cast<long long>(i) < n; ++i) {
        const mpz_ptr num = mpq_numref(rat_coeffs[i].get());
        const mpz_ptr den = mpq_denref(rat_coeffs[i].get());
        mathlib::BigInt num_bi, den_bi;
        mpz_set(num_bi.get(), num);
        mpz_set(den_bi.get(), den);
        z.coeffs[i] = zp_from_rat(p, num_bi, den_bi, precision);
    }
    return z;
}

inline QpTotallyRamified qp_add(const QpTotallyRamified& a, const QpTotallyRamified& b) {
    QpTotallyRamified r = qp_zero(a.p, a.n, a.defining_poly, a.precision);
    for (long long i = 0; i < a.n; ++i) {
        r.coeffs[static_cast<std::size_t>(i)] =
            zp_add(a.coeffs[static_cast<std::size_t>(i)], b.coeffs[static_cast<std::size_t>(i)]);
    }
    return r;
}

inline QpTotallyRamified qp_sub(const QpTotallyRamified& a, const QpTotallyRamified& b) {
    QpTotallyRamified r = qp_zero(a.p, a.n, a.defining_poly, a.precision);
    for (long long i = 0; i < a.n; ++i) {
        r.coeffs[static_cast<std::size_t>(i)] =
            zp_sub(a.coeffs[static_cast<std::size_t>(i)], b.coeffs[static_cast<std::size_t>(i)]);
    }
    return r;
}

// Multiply, reducing modulo the defining polynomial via the
// recurrence beta^n = -(c_{n-1} beta^{n-1} + ... + c_0), where
// defining_poly = beta^n + c_{n-1} beta^{n-1} + ... + c_0
// (defining_poly.coeff(n) = 1, monic).
inline QpTotallyRamified qp_mul(const QpTotallyRamified& a, const QpTotallyRamified& b) {
    long long n = a.n;
    long long p = a.p, prec = a.precision;
    std::vector<ZpInt> wide(static_cast<std::size_t>(2 * n - 1), zp_zero(p, prec));
    for (long long i = 0; i < n; ++i) {
        for (long long j = 0; j < n; ++j) {
            ZpInt prod = zp_mul(a.coeffs[static_cast<std::size_t>(i)],
                                 b.coeffs[static_cast<std::size_t>(j)]);
            wide[static_cast<std::size_t>(i + j)] =
                zp_add(wide[static_cast<std::size_t>(i + j)], prod);
        }
    }
    // Reduce degrees n .. 2n-2 down using beta^k = -(sum c_j beta^{j+k-n})
    // for k from 2n-2 down to n.
    for (long long k = 2 * n - 2; k >= n; --k) {
        ZpInt coeff_k = wide[static_cast<std::size_t>(k)];
        if (zp_valuation(coeff_k) >= prec) continue;  // zero to precision; nothing to fold in
        wide[static_cast<std::size_t>(k)] = zp_zero(p, prec);
        for (long long j = 0; j < n; ++j) {
            long long cj = mpz_get_si(a.defining_poly.coeff(static_cast<std::size_t>(j)).get());
            if (cj == 0) continue;
            ZpInt cj_zp = zp_from_int_full(p, -cj, prec);
            ZpInt term = zp_mul(coeff_k, cj_zp);
            long long pos = j + (k - n);
            wide[static_cast<std::size_t>(pos)] = zp_add(wide[static_cast<std::size_t>(pos)], term);
        }
    }
    QpTotallyRamified r = qp_zero(p, n, a.defining_poly, prec);
    for (long long i = 0; i < n; ++i) r.coeffs[static_cast<std::size_t>(i)] = wide[static_cast<std::size_t>(i)];
    return r;
}

// Valuation, per the "generic element" formula documented above the
// struct: v(sum a_i beta^i) = min_i (i + n * v_p(a_i)), valid because
// e = n makes the n candidate valuations pairwise distinct mod e.
// Returns n * precision (a large sentinel) if every coefficient is
// zero to full working precision (can't distinguish "exactly zero"
// from "zero beyond current precision" -- caller should raise
// precision and retry if this matters).
inline long long qp_valuation(const QpTotallyRamified& a) {
    long long best = a.n * a.precision + 1;  // sentinel
    for (long long i = 0; i < a.n; ++i) {
        long long vi = zp_valuation(a.coeffs[static_cast<std::size_t>(i)]);
        if (vi >= a.precision) continue;  // coefficient is zero to precision; skip
        long long cand = i + a.n * vi;
        if (cand < best) best = cand;
    }
    return best;
}

inline bool qp_equal(const QpTotallyRamified& a, const QpTotallyRamified& b) {
    for (long long i = 0; i < a.n; ++i) {
        if (!zp_equal(a.coeffs[static_cast<std::size_t>(i)], b.coeffs[static_cast<std::size_t>(i)])) return false;
    }
    return true;
}

}  // namespace adelic
