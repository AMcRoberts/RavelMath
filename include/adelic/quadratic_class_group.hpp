// include/adelic/quadratic_class_group.hpp
//
// Class group computation for imaginary quadratic fields Q(sqrt(D)),
// D < 0, via binary quadratic forms (Gauss/Cohen, "A Course in
// Computational Algebraic Number Theory," Ch. 5). This is task #4 of
// the class-field-tower/CM-field initiative: "class number / class
// group computation for small number fields" -- deliberately scoped to
// the imaginary quadratic case (the CM fields this project's own
// number-field work already touches, e.g. tests/number_field_parity_test.cpp's
// Q(sqrt(-5))), not the general-degree-n case, which would need a much
// larger algorithm (Buchmann-style, LLL-dependent) this project has no
// present use for.
//
// Background: for a fundamental discriminant D < 0, ideal classes of
// the maximal order of Q(sqrt(D)) correspond bijectively to SL(2,Z)-
// equivalence classes of primitive, positive-definite binary quadratic
// forms ax^2+bxy+cy^2 of discriminant D = b^2-4ac. Each equivalence
// class has a UNIQUE reduced representative (Cohen Def 5.3.2: a form
// (a,b,c) is reduced iff |b| <= a <= c, and b >= 0 whenever |b| = a or
// a = c), so counting reduced forms directly gives the class number,
// with no relation-finding, no LLL, no Minkowski-bound prime
// enumeration needed -- the imaginary quadratic case is special in
// exactly the way that makes it tractable to build correctly in one
// sitting and check against textbook values.
//
// Validated against the nine Heegner numbers (the complete list of
// D < 0 with class number 1: -3, -4, -7, -8, -11, -19, -43, -67, -163)
// plus several small h > 1 discriminants, in
// tests/quadratic_class_group_test.cpp.
//
// Group structure (composition of forms, giving the actual group law
// rather than just its order) is a separate, not-yet-built piece --
// this file computes the class NUMBER (and enumerates the reduced
// forms themselves, which is most of what's needed for structure too)
// but does not yet implement Gauss composition.

#pragma once

#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "mini-gmp/mini-gmp.h"

namespace adelic {

struct QuadForm {
    mathlib::BigInt a, b, c;
};

inline mathlib::BigInt qform_discriminant(const QuadForm& f) {
    mathlib::BigInt b2, ac4, D;
    mathlib::mul(b2, f.b, f.b);
    mathlib::mul(ac4, f.a, f.c);
    mathlib::BigInt four(4);
    mathlib::mul(ac4, four, ac4);
    mathlib::sub(D, b2, ac4);
    return D;
}

// Cohen Definition 5.3.2: (a,b,c) is reduced iff |b| <= a <= c, and
// b >= 0 whenever |b| == a or a == c.
inline bool qform_is_reduced(const QuadForm& f) {
    mathlib::BigInt abs_b;
    mathlib::abs_(abs_b, f.b);
    if (mathlib::cmp(abs_b, f.a) > 0) return false;
    if (mathlib::cmp(f.a, f.c) > 0) return false;
    bool boundary = (mathlib::cmp(abs_b, f.a) == 0) || (mathlib::cmp(f.a, f.c) == 0);
    if (boundary && mathlib::sgn(f.b) < 0) return false;
    return true;
}

namespace detail {
// Floor division that rounds toward -infinity (needed for the b mod 2a
// step below; mpz_fdiv_q already does this for mini-gmp).
inline mathlib::BigInt fdiv_q(const mathlib::BigInt& a, const mathlib::BigInt& b) {
    mathlib::BigInt q;
    mpz_fdiv_q(q.get(), a.get(), b.get());
    return q;
}
inline mathlib::BigInt fdiv_r(const mathlib::BigInt& a, const mathlib::BigInt& b) {
    mathlib::BigInt r;
    mpz_fdiv_r(r.get(), a.get(), b.get());
    return r;
}
}  // namespace detail

// Reduce a primitive positive-definite form to its unique reduced
// representative equivalent under SL(2,Z) (Cohen Algorithm 5.4.2,
// the "rho" reduction step iterated to a fixed point).
inline QuadForm qform_reduce(QuadForm f) {
    if (mathlib::sgn(f.a) <= 0) {
        throw std::invalid_argument("qform_reduce: a must be positive (positive-definite form)");
    }
    mathlib::BigInt D = qform_discriminant(f);
    if (mathlib::sgn(D) >= 0) {
        throw std::invalid_argument("qform_reduce: discriminant must be negative");
    }

    mathlib::BigInt two(2), four(4);
    for (int iter = 0; iter < 100000; ++iter) {
        // Step 1: bring b into (-a, a] via b -> b - 2a*floor((b+a-1)/(2a))
        // (the "-1" shift is needed so the right endpoint b==a maps to
        // itself rather than spilling over to the excluded b==-a; found
        // by a failing cross-check against enumerate_reduced_forms's
        // independent search, not by inspection), recomputing c to
        // preserve b^2 - 4ac = D exactly.
        mathlib::BigInt two_a;
        mathlib::mul(two_a, two, f.a);
        mathlib::BigInt shifted;
        mathlib::add(shifted, f.b, f.a);
        mathlib::add_si(shifted, shifted, -1);
        mathlib::BigInt k = detail::fdiv_q(shifted, two_a);
        mathlib::BigInt new_b;
        {
            mathlib::BigInt t;
            mathlib::mul(t, two_a, k);
            mathlib::sub(new_b, f.b, t);
        }
        bool b_changed = (mathlib::cmp(new_b, f.b) != 0);
        if (b_changed) {
            f.b = new_b;
            mathlib::BigInt b2, num;
            mathlib::mul(b2, f.b, f.b);
            mathlib::sub(num, b2, D);
            mathlib::BigInt new_c;
            mathlib::tdiv_q(new_c, num, four);  // num/4 exact since D==b^2 mod4 always
            mathlib::divexact(new_c, new_c, f.a);
            f.c = new_c;
        }

        if (mathlib::cmp(f.a, f.c) > 0) {
            std::swap(f.a, f.c);
            mathlib::neg(f.b);
            continue;
        }

        // a <= c and b in (-a, a] now. Apply the boundary tie-break and stop.
        if ((mathlib::cmp(f.a, f.c) == 0 || mathlib::cmp(f.b, f.a) == 0) && mathlib::sgn(f.b) < 0) {
            mathlib::neg(f.b);
        }
        return f;
    }
    throw std::runtime_error("qform_reduce: did not converge (should be impossible)");
}

// All primitive reduced forms of discriminant D (D < 0, D == 0 or 1
// mod 4). This IS the class group's underlying set; its size is the
// class number h(D).
inline std::vector<QuadForm> enumerate_reduced_forms(const mathlib::BigInt& D) {
    if (mathlib::sgn(D) >= 0) {
        throw std::invalid_argument("enumerate_reduced_forms: D must be negative");
    }
    {
        mathlib::BigInt r = detail::fdiv_r(D, mathlib::BigInt(4));
        if (!(mathlib::is_zero(r) || mathlib::cmp_si(r, 1) == 0)) {
            throw std::invalid_argument("enumerate_reduced_forms: D must be 0 or 1 mod 4");
        }
    }

    std::vector<QuadForm> result;

    // Reduced forms satisfy a <= sqrt(|D|/3) (from a<=c and |b|<=a
    // forcing 3a^2 <= b^2 - 4ac + 4a^2 ... <= -D + a^2, i.e. 3a^2<=-D
    // when a<=c and using D=b^2-4ac<=a^2-4a^2=-3a^2... standard bound,
    // Cohen Sec 5.3).
    mathlib::BigInt neg_D;
    mathlib::neg(neg_D, D);
    mathlib::BigInt bound_sq;
    mathlib::tdiv_q(bound_sq, neg_D, mathlib::BigInt(3));
    mathlib::BigInt a_max;
    mpz_sqrt(a_max.get(), bound_sq.get());

    mathlib::BigInt a(1);
    while (mathlib::cmp(a, a_max) <= 0) {
        mathlib::BigInt four_a;
        mathlib::mul(four_a, mathlib::BigInt(4), a);
        // b ranges over (-a, a], i.e. b in [-a+1, a] as an inclusive
        // integer range (Cohen's |b|<=a with the boundary tie-break
        // applied afterward via the b>=0 filter below).
        mathlib::BigInt b;
        mathlib::neg(b, a);
        mathlib::add_si(b, b, 1);
        mathlib::BigInt b_end = a;
        while (mathlib::cmp(b, b_end) <= 0) {
            mathlib::BigInt b2, num;
            mathlib::mul(b2, b, b);
            mathlib::sub(num, b2, D);
            if (mpz_divisible_p(num.get(), four_a.get())) {
                mathlib::BigInt c;
                mathlib::divexact(c, num, four_a);
                if (mathlib::cmp(a, c) <= 0) {
                    // b ranges over {-a+1, ..., a} here, so |b| == a
                    // can only happen at the right endpoint b == a.
                    bool boundary = (mathlib::cmp(b, a) == 0) || (mathlib::cmp(a, c) == 0);
                    bool keep = !boundary || mathlib::sgn(b) >= 0;
                    if (keep) {
                        mathlib::BigInt g1, g2;
                        mathlib::gcd(g1, a, b);
                        mathlib::gcd(g2, g1, c);
                        if (mathlib::is_one(g2)) {
                            result.push_back(QuadForm{a, b, c});
                        }
                    }
                }
            }
            mathlib::add_si(b, b, 1);
        }
        mathlib::add_si(a, a, 1);
    }

    return result;
}

inline long long quadratic_class_number(const mathlib::BigInt& D) {
    return static_cast<long long>(enumerate_reduced_forms(D).size());
}

inline long long quadratic_class_number(long long D) {
    return quadratic_class_number(mathlib::BigInt(D));
}

}  // namespace adelic
