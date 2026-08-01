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
// Group structure: qform_compose (below) gives the actual group law,
// via the ideal correspondence rather than a hand-derived Gauss/
// Dirichlet composition formula -- a form (a,b,c) corresponds to the
// O_D-ideal I = a*Z + ((-b+sqrt(D))/2)*Z, and composing two forms is
// multiplying their ideals (reusing ideal_arithmetic.hpp's already-
// tested, general-purpose ideal multiplication over the order's power
// basis {1, omega}, omega = sqrt(D)/2 or (1+sqrt(D))/2 depending on
// D mod 4) and reading the product ideal's own form back out via the
// norm form Q(x,y) = N(x*v1+y*v2)/N(I) for a Z-basis {v1,v2} of the
// ideal. This route was chosen over implementing Cohen's Algorithm
// 5.4.7 (a three-way extended-gcd composition formula) directly,
// because a half-remembered version of that formula is exactly the
// kind of thing likely to ship a subtle, hard-to-notice bug -- reusing
// already-tested ideal arithmetic and cross-checking against the group
// axioms (identity, inverses, associativity) directly is more robust.
// A real bug WAS caught this way during development: the naive
// extraction of (a,b,c) from a product ideal came out systematically
// conjugated (every composition was silently computing inverses),
// caught by testing that composing with the identity form left the
// other operand unchanged -- see qform_compose's implementation
// comment for the fix (a sign flip on the extracted b).

#pragma once

#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "mini-gmp/mini-gmp.h"

#include "adelic/ideal_arithmetic.hpp"

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

// ===================================================================
// Group structure: qform_compose, via the ideal correspondence.
//
// Order O_D = Z[omega], omega = sqrt(D)/2 (D == 0 mod 4, D = 4m,
// omega = sqrt(m)) or (1+sqrt(D))/2 (D == 1 mod 4). Its trace/norm:
//   D == 0 mod 4: Tr(omega) = 0,  N(omega) = -m = -D/4
//   D == 1 mod 4: Tr(omega) = 1,  N(omega) = (1-D)/4
// (both read directly off the minimal polynomial x^2 - Tr*x + N).
// ===================================================================

namespace detail {

inline mathlib::PolyZ quadratic_order_charpoly(const mathlib::BigInt& D,
                                                mathlib::BigInt& Tr, mathlib::BigInt& Nm) {
    mathlib::PolyZ p;
    p.ensure_size(3);
    mathlib::BigInt r = fdiv_r(D, mathlib::BigInt(4));
    if (mathlib::is_zero(r)) {
        mathlib::BigInt m; mathlib::tdiv_q(m, D, mathlib::BigInt(4));
        mathlib::BigInt neg_m; mathlib::neg(neg_m, m);
        mathlib::set(p.coeff(0), neg_m);
        mathlib::set_si(p.coeff(1), 0);
        mathlib::set_si(p.coeff(2), 1);
        mathlib::set_si(Tr, 0);
        mathlib::set(Nm, neg_m);
    } else {
        mathlib::BigInt k; mathlib::sub(k, mathlib::BigInt(1), D); mathlib::tdiv_q(k, k, mathlib::BigInt(4));
        mathlib::set(p.coeff(0), k);
        mathlib::set_si(p.coeff(1), -1);
        mathlib::set_si(p.coeff(2), 1);
        mathlib::set_si(Tr, 1);
        mathlib::set(Nm, k);
    }
    return p;
}

// The O_D-ideal a*Z + ((-b+sqrt(D))/2)*Z corresponding to form (a,b,c).
inline IdealHNF form_to_ideal(const QuadForm& f, const mathlib::BigInt& D,
                               const mathlib::PolyZ& charpoly) {
    mathlib::PolyZ gen1;
    gen1.ensure_size(1);
    mathlib::set(gen1.coeff(0), f.a);

    mathlib::PolyZ gen2;
    gen2.ensure_size(2);
    mathlib::BigInt coeff0;
    mathlib::BigInt r = fdiv_r(D, mathlib::BigInt(4));
    if (mathlib::is_zero(r)) {
        // -b/2 (b is forced even when D == 0 mod 4)
        mathlib::BigInt half;
        mathlib::tdiv_q(half, f.b, mathlib::BigInt(2));
        mathlib::neg(coeff0, half);
    } else {
        // (-b-1)/2 (b is forced odd when D == 1 mod 4)
        mathlib::BigInt t;
        mathlib::add_si(t, f.b, 1);
        mathlib::neg(t, t);
        mathlib::tdiv_q(coeff0, t, mathlib::BigInt(2));
    }
    mathlib::set(gen2.coeff(0), coeff0);
    mathlib::set_si(gen2.coeff(1), 1);

    std::vector<mathlib::PolyZ> gens = {gen1, gen2};
    return ideal_from_generators(gens, charpoly, 2);
}

inline mathlib::BigInt elt_norm(const mathlib::BigInt& p, const mathlib::BigInt& q,
                                 const mathlib::BigInt& Tr, const mathlib::BigInt& Nm) {
    // N(p + q*omega) = p^2 + p*q*Tr(omega) + q^2*N(omega)
    mathlib::BigInt t1, t2, t3, r;
    mathlib::mul(t1, p, p);
    mathlib::mul(t2, p, q); mathlib::mul(t2, t2, Tr);
    mathlib::mul(t3, q, q); mathlib::mul(t3, t3, Nm);
    mathlib::add(r, t1, t2);
    mathlib::add(r, r, t3);
    return r;
}

// Tr(v1 * conj(v2)), v1=(p1,q1), v2=(p2,q2) as p+q*omega;
// conj(p+q*omega) = (p+q*Tr(omega)) - q*omega.
inline mathlib::BigInt trace_v1_conjv2(const mathlib::BigInt& p1, const mathlib::BigInt& q1,
                                        const mathlib::BigInt& p2, const mathlib::BigInt& q2,
                                        const mathlib::BigInt& Tr, const mathlib::BigInt& Nm) {
    mathlib::BigInt p2c, q2c;
    { mathlib::BigInt t; mathlib::mul(t, q2, Tr); mathlib::add(p2c, p2, t); }
    mathlib::neg(q2c, q2);

    mathlib::BigInt constpart;
    { mathlib::BigInt a; mathlib::mul(a, p1, p2c);
      mathlib::BigInt b; mathlib::mul(b, q1, q2c); mathlib::mul(b, b, Nm);
      mathlib::sub(constpart, a, b); }
    mathlib::BigInt omegapart;
    { mathlib::BigInt a; mathlib::mul(a, p1, q2c);
      mathlib::BigInt b; mathlib::mul(b, q1, p2c);
      mathlib::BigInt c; mathlib::mul(c, q1, q2c); mathlib::mul(c, c, Tr);
      mathlib::add(omegapart, a, b); mathlib::add(omegapart, omegapart, c); }

    // Tr(const + omegapart*omega) = 2*const + omegapart*Tr(omega)
    mathlib::BigInt result;
    { mathlib::BigInt t1; mathlib::mul(t1, mathlib::BigInt(2), constpart);
      mathlib::BigInt t2; mathlib::mul(t2, omegapart, Tr);
      mathlib::add(result, t1, t2); }
    return result;
}

// Extract the (unreduced) primitive form Q(x,y) = N(x*v1+y*v2)/N(I)
// from a Z-basis {v1,v2} of ideal I. The final neg(f.b) corrects a
// systematic conjugation found empirically: form_to_ideal's generator
// uses -b (not +b), so the naive norm-form extraction here recovers
// the CONJUGATE class (equivalently the group-theoretic inverse) of
// the true product form -- caught by testing that composing with the
// identity form should be a no-op, and it wasn't, before this fix.
inline QuadForm ideal_to_form(const IdealHNF& I, const mathlib::BigInt& Tr, const mathlib::BigInt& Nm) {
    const mathlib::BigInt& p1 = I.basis[0][0];
    const mathlib::BigInt& q1 = I.basis[0][1];
    const mathlib::BigInt& p2 = I.basis[1][0];
    const mathlib::BigInt& q2 = I.basis[1][1];

    mathlib::BigInt NI;
    { mathlib::BigInt t1, t2; mathlib::mul(t1, p1, q2); mathlib::mul(t2, q1, p2);
      mathlib::sub(NI, t1, t2); mathlib::abs_(NI, NI); }

    mathlib::BigInt A = elt_norm(p1, q1, Tr, Nm);
    mathlib::BigInt C = elt_norm(p2, q2, Tr, Nm);
    mathlib::BigInt B = trace_v1_conjv2(p1, q1, p2, q2, Tr, Nm);

    QuadForm f;
    mathlib::divexact(f.a, A, NI);
    mathlib::divexact(f.b, B, NI);
    mathlib::divexact(f.c, C, NI);
    mathlib::neg(f.b);
    return f;
}

}  // namespace detail

// Composes two primitive forms of the same discriminant D, returning
// the unique reduced representative of the product class.
inline QuadForm qform_compose(const QuadForm& f1, const QuadForm& f2, const mathlib::BigInt& D) {
    mathlib::BigInt Tr, Nm;
    mathlib::PolyZ charpoly = detail::quadratic_order_charpoly(D, Tr, Nm);
    IdealHNF I1 = detail::form_to_ideal(f1, D, charpoly);
    IdealHNF I2 = detail::form_to_ideal(f2, D, charpoly);
    IdealHNF I3 = ideal_mul(I1, I2, charpoly);
    QuadForm f3 = detail::ideal_to_form(I3, Tr, Nm);
    return qform_reduce(f3);
}

// The principal (identity) form, in closed form (no search needed):
// (1, 0, -D/4) if D == 0 mod 4, (1, 1, (1-D)/4) if D == 1 mod 4.
inline QuadForm qform_principal(const mathlib::BigInt& D) {
    mathlib::BigInt r = detail::fdiv_r(D, mathlib::BigInt(4));
    if (mathlib::is_zero(r)) {
        mathlib::BigInt c; mathlib::neg(c, D); mathlib::tdiv_q(c, c, mathlib::BigInt(4));
        return QuadForm{mathlib::BigInt(1), mathlib::BigInt(0), c};
    } else {
        mathlib::BigInt c; mathlib::sub(c, mathlib::BigInt(1), D); mathlib::tdiv_q(c, c, mathlib::BigInt(4));
        return QuadForm{mathlib::BigInt(1), mathlib::BigInt(1), c};
    }
}

// The order of a form's ideal class in the class group (smallest k>=1
// with f composed with itself k times reducing to the principal form).
inline long long qform_order(const QuadForm& f, const mathlib::BigInt& D, long long max_order) {
    QuadForm principal = qform_reduce(qform_principal(D));
    QuadForm cur = qform_reduce(f);
    for (long long k = 1; k <= max_order; ++k) {
        if (mathlib::cmp(cur.a, principal.a) == 0 && mathlib::cmp(cur.b, principal.b) == 0 &&
            mathlib::cmp(cur.c, principal.c) == 0) {
            return k;
        }
        cur = qform_compose(cur, f, D);
    }
    throw std::runtime_error("qform_order: did not find identity within max_order compositions");
}

}  // namespace adelic
