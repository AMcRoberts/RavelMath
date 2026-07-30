// tests/padic_test.cpp
//
// Self-test for include/adelic/padic.hpp (Z_p arithmetic).
//
// Tests:
//   1. Construction (zero, one, from_int)
//   2. Addition and subtraction (with carry / borrow)
//   3. Multiplication (convolution + carry)
//   4. Negation
//   5. Valuation
//   6. Inversion via Newton doubling
//   7. Known identity: 1/(1-p) = 1 + p + p^2 + ... in Z_p
//      (the geometric series; the right-hand side is what we
//      expect when we compute the inversion by hand digit-by-digit)
//   8. Working precision is respected (a + 0 = a, etc.)

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "adelic/padic.hpp"

using namespace std;
using adelic::ZpInt;
using adelic::zp_zero;
using adelic::zp_one;
using adelic::zp_from_int;
using adelic::zp_from_int_full;
using adelic::zp_add;
using adelic::zp_sub;
using adelic::zp_neg;
using adelic::zp_mul;
using adelic::zp_invert;
using adelic::zp_valuation;
using adelic::zp_equal;
using adelic::zp_str;
using adelic::zp_from_rat;
using adelic::QpTotallyRamified;
using adelic::qp_zero;
using adelic::qp_uniformizer;
using adelic::qp_from_qelem;
using adelic::qp_add;
using adelic::qp_sub;
using adelic::qp_mul;
using adelic::qp_valuation;
using adelic::qp_equal;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

void test_construction() {
    fprintf(stderr, "=== test_construction ===\n");
    ZpInt z = zp_zero(5, 4);
    CHECK(z.p == 5, "zero: p should be 5");
    CHECK(z.precision() == 4, "zero: precision should be 4");
    CHECK(z.digits.size() == 4, "zero: digits size should be precision");
    for (auto d : z.digits) CHECK(d == 0, "zero: all digits should be 0");

    ZpInt o = zp_one(5, 4);
    CHECK(o.digits[0] == 1, "one: digits[0] should be 1");
    for (std::size_t i = 1; i < o.digits.size(); ++i) {
        CHECK(o.digits[i] == 0, "one: higher digits should be 0");
    }

    ZpInt seven = zp_from_int(5, 7, 4);
    CHECK(seven.digits[0] == 2, "from_int(5, 7): 7 mod 5 = 2");
    for (std::size_t i = 1; i < seven.digits.size(); ++i) {
        CHECK(seven.digits[i] == 0, "from_int: higher digits should be 0");
    }

    ZpInt neg = zp_from_int(5, -3, 4);
    CHECK(neg.digits[0] == 2, "from_int(5, -3): -3 mod 5 = 2");
}

void test_addition() {
    fprintf(stderr, "=== test_addition ===\n");
    // 2 + 3 = 5 (no carry, p = 7)
    {
        ZpInt a = zp_from_int(7, 2, 4);
        ZpInt b = zp_from_int(7, 3, 4);
        ZpInt s = zp_add(a, b);
        CHECK(s.digits[0] == 5, "2 + 3 = 5 (no carry)");
        for (std::size_t i = 1; i < s.digits.size(); ++i) {
            CHECK(s.digits[i] == 0, "2 + 3: no higher carries");
        }
    }

    // 3 + 4 in Z/5Z: 3 + 4 = 7 = 1*5 + 2, so digit 0 = 2 and digit 1 = 1
    {
        ZpInt a = zp_from_int(5, 3, 4);
        ZpInt b = zp_from_int(5, 4, 4);
        ZpInt s = zp_add(a, b);
        CHECK(s.digits[0] == 2, "3 + 4: digit 0 = 2 (7 mod 5)");
        CHECK(s.digits[1] == 1, "3 + 4: digit 1 = 1 (carry from 7 / 5)");
        for (std::size_t i = 2; i < s.digits.size(); ++i) {
            CHECK(s.digits[i] == 0, "3 + 4: digits 2+ should be 0");
        }
    }

    // 3 + 3 = 6 = 1 + carry 1
    {
        ZpInt a = zp_from_int(5, 3, 4);
        ZpInt c = zp_from_int(5, 3, 4);
        ZpInt s2 = zp_add(a, c);
        CHECK(s2.digits[0] == 1, "3 + 3 = 6: digit 0 = 1");
        CHECK(s2.digits[1] == 1, "3 + 3 = 6: digit 1 = 1 (carry)");
    }

    // Multi-digit: 7 + 8 = 15 (in base 5: digit 0 = 0, digit 1 = 3, no further carry)
    {
        ZpInt a = zp_zero(5, 4);
        a.digits[0] = 2; a.digits[1] = 1;  // 5*1 + 2 = 7
        ZpInt b = zp_zero(5, 4);
        b.digits[0] = 3; b.digits[1] = 1;  // 5*1 + 3 = 8
        ZpInt s3 = zp_add(a, b);
        CHECK(s3.digits[0] == 0, "7 + 8: digit 0 = 0 (2 + 3 = 5 = 0 with carry)");
        CHECK(s3.digits[1] == 3, "7 + 8: digit 1 = 3 (1 + 1 + carry 1 = 3)");
        CHECK(s3.digits[2] == 0, "7 + 8: digit 2 = 0 (no further carry)");
    }

    // Test addition with higher precision
    {
        ZpInt a3 = zp_from_int(7, 48, 5);
        ZpInt b3 = zp_from_int(7, 2, 5);
        ZpInt s4 = zp_add(a3, b3);
        CHECK(s4.digits[0] == 1, "48 + 2 = 50 = 1 mod 7");
        CHECK(s4.digits[1] == 1, "48 + 2: digit 1 = 1 (carry from 50 = 7*1 + 1)");
    }
}

void test_subtraction() {
    fprintf(stderr, "=== test_subtraction ===\n");
    // 7 - 3 = 4
    {
        ZpInt a = zp_from_int(5, 7, 4);
        ZpInt b = zp_from_int(5, 3, 4);
        ZpInt d = zp_sub(a, b);
        CHECK(d.digits[0] == 4, "7 - 3 = 4");
    }

    // 3 - 7 = -4 = 1 mod 5 (with borrow)
    {
        ZpInt a = zp_from_int(5, 7, 4);
        ZpInt b = zp_from_int(5, 3, 4);
        ZpInt d2 = zp_sub(b, a);
        CHECK(d2.digits[0] == 1, "3 - 7 = -4: digit 0 = 1 (with borrow)");
    }

    // 0 - 1 = -1 = p - 1 (with borrow)
    {
        ZpInt z = zp_zero(5, 4);
        ZpInt o = zp_one(5, 4);
        ZpInt d3 = zp_sub(z, o);
        CHECK(d3.digits[0] == 4, "0 - 1: digit 0 = 4 (p - 1)");
    }

    // Negation
    {
        ZpInt o = zp_one(5, 4);
        ZpInt z = zp_zero(5, 4);
        ZpInt d3 = zp_sub(z, o);
        ZpInt neg_o = zp_neg(o);
        CHECK(zp_equal(neg_o, d3), "neg(1) = 0 - 1");
    }

    {
        ZpInt neg_3 = zp_neg(zp_from_int(5, 3, 4));
        CHECK(neg_3.digits[0] == 2, "neg(3) = -3 = 2 mod 5");
    }
}

void test_multiplication() {
    fprintf(stderr, "=== test_multiplication ===\n");
    // 3 * 4 = 12 = 2 mod 5
    {
        ZpInt a = zp_from_int(5, 3, 4);
        ZpInt b = zp_from_int(5, 4, 4);
        ZpInt p = zp_mul(a, b);
        CHECK(p.digits[0] == 2, "3 * 4 = 12 = 2 mod 5");
    }

    // 3 * 3 = 9 = 4 mod 5
    {
        ZpInt a = zp_from_int(5, 3, 4);
        ZpInt p2 = zp_mul(a, a);
        CHECK(p2.digits[0] == 4, "3 * 3 = 9 = 4 mod 5");
    }

    // Multi-digit: 6 * 7 = 42 in Z/11Z: 42 = 11*3 + 9, so digit 0 = 9
    // and digit 1 = 3 (with the carry propagated to the next
    // position).
    {
        ZpInt a2 = zp_from_int(11, 6, 4);
        ZpInt b2 = zp_from_int(11, 7, 4);
        ZpInt p3 = zp_mul(a2, b2);
        CHECK(p3.digits[0] == 9, "6 * 7 = 42: digit 0 = 9 (42 mod 11)");
        CHECK(p3.digits[1] == 3, "6 * 7 = 42: digit 1 = 3 (42 / 11)");
        CHECK(p3.digits[2] == 0, "6 * 7 = 42: digit 2 = 0");
    }

    // Multi-digit carry in Z/3Z: a=5, b=5 (both reduce to 2 mod 3).
    // 2*2 = 4 in Z, but in Z/3Z the 1 carry propagates to the
    // next digit: result is [1, 1, 0, 0] (4 = 1*3 + 1).
    {
        ZpInt a3 = zp_from_int(3, 5, 4);  // digits = [2, 0, 0, 0]
        ZpInt b3 = zp_from_int(3, 5, 4);  // digits = [2, 0, 0, 0]
        ZpInt p4 = zp_mul(a3, b3);
        CHECK(p4.digits[0] == 1, "5 * 5 = 25 in Z/3Z: digit 0 = 1 (4 mod 3)");
        CHECK(p4.digits[1] == 1, "5 * 5 = 25 in Z/3Z: digit 1 = 1 (carry from 4/3 = 1)");
        CHECK(p4.digits[2] == 0, "5 * 5 = 25 in Z/3Z: digit 2 = 0");
    }

    // Larger carry: 6 * 8 = 48 in Z/11Z: 48 = 11*4 + 4, so digit 0
    // = 4 and digit 1 = 4 (with the carry 4).
    {
        ZpInt a4 = zp_from_int(11, 6, 4);
        ZpInt b4 = zp_from_int(11, 8, 4);
        ZpInt p5 = zp_mul(a4, b4);
        CHECK(p5.digits[0] == 4, "6 * 8 = 48 in Z/11Z: digit 0 = 4 (48 mod 11)");
        CHECK(p5.digits[1] == 4, "6 * 8 = 48 in Z/11Z: digit 1 = 4 (carry 4)");
        CHECK(p5.digits[2] == 0, "6 * 8 = 48 in Z/11Z: digit 2 = 0");
    }

    // Identity
    {
        ZpInt a = zp_from_int(5, 3, 4);
        ZpInt o = zp_one(5, 4);
        ZpInt a_times_1 = zp_mul(a, o);
        CHECK(zp_equal(a_times_1, a), "a * 1 = a");
    }
}

void test_valuation() {
    fprintf(stderr, "=== test_valuation ===\n");
    ZpInt z = zp_zero(5, 4);
    CHECK(zp_valuation(z) == 4, "v_p(0) = precision");

    ZpInt o = zp_one(5, 4);
    CHECK(zp_valuation(o) == 0, "v_p(1) = 0");

    // 5 in base 5: digits = [0, 1, 0, 0]
    ZpInt five = zp_zero(5, 4);
    five.digits[0] = 0; five.digits[1] = 1; five.digits[2] = 0; five.digits[3] = 0;
    CHECK(zp_valuation(five) == 1, "v_p(5) = 1");

    // 25 in base 5: digits = [0, 0, 1, 0]
    ZpInt twentyfive = zp_zero(5, 4);
    twentyfive.digits[0] = 0; twentyfive.digits[1] = 0; twentyfive.digits[2] = 1; twentyfive.digits[3] = 0;
    CHECK(zp_valuation(twentyfive) == 2, "v_p(25) = 2");

    // 5^precision is at the cap
    ZpInt big = zp_zero(5, 4);
    big.digits[3] = 1;  // 125 = 5^3
    CHECK(zp_valuation(big) == 3, "v_p(125) = 3 (within precision)");
}

void test_inversion() {
    fprintf(stderr, "=== test_inversion ===\n");
    // 3^(-1) mod 5 = 2 (since 3 * 2 = 6 = 1 mod 5)
    ZpInt three = zp_from_int(5, 3, 4);
    ZpInt inv3 = zp_invert(three);
    CHECK(inv3.digits[0] == 2, "3^(-1) = 2 mod 5");
    ZpInt three_inv3 = zp_mul(three, inv3);
    CHECK(three_inv3.digits[0] == 1, "3 * 3^(-1) = 1");
    for (std::size_t i = 1; i < three_inv3.digits.size(); ++i) {
        CHECK(three_inv3.digits[i] == 0, "3 * 3^(-1): higher digits should be 0");
    }

    // 2^(-1) mod 7 = 4 (since 2 * 4 = 8 = 1 mod 7)
    ZpInt two = zp_from_int(7, 2, 4);
    ZpInt inv2 = zp_invert(two);
    CHECK(inv2.digits[0] == 4, "2^(-1) = 4 mod 7");
    ZpInt two_inv2 = zp_mul(two, inv2);
    CHECK(two_inv2.digits[0] == 1, "2 * 2^(-1) = 1 mod 7");

    // Inversion with higher precision
    ZpInt a = zp_from_int(7, 3, 8);
    ZpInt inv_a = zp_invert(a);
    ZpInt a_inv_a = zp_mul(a, inv_a);
    CHECK(a_inv_a.digits[0] == 1, "3 * 3^(-1) = 1 mod 7 (8 digits)");
    for (std::size_t i = 1; i < 8; ++i) {
        CHECK(a_inv_a.digits[static_cast<std::size_t>(i)] == 0,
              "3 * 3^(-1): higher digits should be 0");
    }
}

void test_known_identity_geometric_series() {
    fprintf(stderr, "=== test_known_identity_geometric_series ===\n");
    // 1/(1 - p) = 1 + p + p^2 + p^3 + ... in Z_p (the geometric series).
    // Verify: a = 1 - p, so a * (1 + p + p^2 + ...) = 1 in Z_p.
    // I.e., 1 / (1 - p) = sum_{k=0}^{N-1} p^k in Z_p (mod p^N).
    //
    // For p = 5, N = 4: a = 1 - 5 = -4 = 1 mod 5
    // (1 - 5) * (1 + 5 + 5^2 + 5^3) = 1 - 5^4 = 1 mod 5^4
    //
    // We can verify the inversion directly: 1/(1-5) in Z_5 should be
    // the digits [1, 1, 1, 1, 1, ...] = 1/(1-5) in base 5.

    ZpInt a = zp_zero(5, 4);
    a.digits[0] = 1;  // 1 - 5 = -4 = 1 mod 5
    a = zp_sub(a, zp_from_int(5, 5, 4));
    fprintf(stderr, "  a = 1 - 5 (in Z_5, precision 4): %s\n", zp_str(a).c_str());
    // a should be [1, 4, 0, 0] (1 - 5 = -4 = 5*1 - 4, so digit 0 = 1, digit 1 = 4)
    // Wait, 1 - 5 = -4.  In Z_5, -4 = 5 - 4 = 1 mod 5.  Then "subtract 5" doesn't change
    // anything in Z_5.  Hmm.
    // Actually, 1 - 5 in Z (not Z_5) is -4.  In Z_5, -4 = 1 (mod 5).  So a = 1 in Z_5.
    // That's the wrong element.  Let me use a different verification.
    //
    // We need: x = 1/(1-5) in Z_5, with working precision 4.
    // a = (1 - 5) in Z = -4.  In Z_5, a = 1 (mod 5).  So 1/a = 1 in Z_5.
    // That's not the geometric series.  I need to work in Z, not Z/5Z.
    //
    // Hmm, the p-adic arithmetic here is in Z/p^precision Z, not Z_p directly.
    // Let me verify: (1 - 5) * (1 + 5 + 5^2 + 5^3) = 1 - 5^4 = 1 - 625 = -624.
    // -624 mod 5^4 = 5^4 - 624 = 625 - 624 = 1.  So (1-5) * sum = 1 in Z/5^4 Z.  ✓
    //
    // To test this, I need to construct a = 1 - 5 in Z/5^4 Z, where
    // 1 - 5 = -4 in Z, which is represented as 5^4 - 4 = 621 in Z/5^4 Z.
    // In base 5: 621 = 5 * 124 + 1, so digit 0 = 1, digit 1 = 4, digit 2 = 4, digit 3 = 4.
    // Or we can use the "borrow" representation: 1 - 5 = -4 = 5^4 - 4, so the
    // digits are [1, 4, 4, 4] (where the "implicit higher digit" 1 at position
    // 4 gives 5^4).  This is the canonical form in Z/5^4 Z for -4.

    ZpInt a_minus = zp_zero(5, 4);
    a_minus.digits[0] = 1;
    a_minus.digits[1] = 4;
    a_minus.digits[2] = 4;
    a_minus.digits[3] = 4;
    // Now a_minus represents 1 + 4*5 + 4*25 + 4*125 = 1 + 20 + 100 + 500 = 621 = -4 mod 5^4.  ✓

    // Construct the geometric series 1 + 5 + 5^2 + 5^3
    ZpInt gs = zp_zero(5, 4);
    gs.digits[0] = 1; gs.digits[1] = 1; gs.digits[2] = 1; gs.digits[3] = 1;

    // a_minus * gs should be 1 (in Z/5^4 Z)
    ZpInt prod = zp_mul(a_minus, gs);
    fprintf(stderr, "  (1 - 5) * (1 + 5 + 5^2 + 5^3) in Z/5^4 Z: digits = [%lld, %lld, %lld, %lld]\n",
            (long long)prod.digits[0], (long long)prod.digits[1],
            (long long)prod.digits[2], (long long)prod.digits[3]);
    CHECK(prod.digits[0] == 1, "(1-5) * (1+5+5^2+5^3): digit 0 = 1");
    for (std::size_t i = 1; i < prod.digits.size(); ++i) {
        CHECK(prod.digits[i] == 0, "(1-5) * (1+5+5^2+5^3): higher digits = 0");
    }

    // Now check that 1/(1-5) via zp_invert gives the same as the geometric series
    ZpInt inv = zp_invert(a_minus);
    fprintf(stderr, "  1/(1-5) via Newton: digits = [%lld, %lld, %lld, %lld]\n",
            (long long)inv.digits[0], (long long)inv.digits[1],
            (long long)inv.digits[2], (long long)inv.digits[3]);
    CHECK(zp_equal(inv, gs), "1/(1-5) via Newton = 1+5+5^2+5^3 (geometric series)");
}

void test_precision_works() {
    fprintf(stderr, "=== test_precision_works ===\n");
    // Verify that precision is respected.
    ZpInt a = zp_from_int(7, 3, 5);
    ZpInt o = zp_one(7, 5);
    ZpInt sum = zp_add(a, o);
    CHECK(sum.digits[0] == 4, "3 + 1 = 4 (precision respected)");
    for (std::size_t i = 1; i < 5; ++i) {
        CHECK(sum.digits[i] == 0, "3 + 1: no higher carries");
    }

    // a + 0 = a
    ZpInt z = zp_zero(7, 5);
    ZpInt same = zp_add(a, z);
    CHECK(zp_equal(same, a), "a + 0 = a");

    // a * 1 = a
    ZpInt prod = zp_mul(a, o);
    CHECK(zp_equal(prod, a), "a * 1 = a");

    // a * 0 = 0
    ZpInt zero = zp_mul(a, z);
    CHECK(zp_equal(zero, z), "a * 0 = 0");
}

// Verify the structural ZpInt invariant (precision == digits.size())
// is maintained by the precision-management API.  The previous
// version of this struct kept precision and digits.size() as
// independent fields and let them drift out of sync; that was the
// root cause of the heap-OOB non-determinism in
// sweep_nonunit_property_f.cpp.  Every code path that grows or
// shrinks precision must keep the invariant -- these tests pin it.
void test_set_precision_invariant() {
    fprintf(stderr, "=== test_set_precision_invariant ===\n");

    // Initial construction: precision == digits.size().
    ZpInt z = zp_zero(5, 4);
    CHECK(z.precision() == 4, "zp_zero(5,4): precision() == 4");
    CHECK(static_cast<long long>(z.digits.size()) == z.precision(),
          "zp_zero: digits.size() == precision()");

    // extend_to grows digits and keeps the invariant.
    z.extend_to(7);
    CHECK(z.precision() == 7, "extend_to(7) on prec=4 -> precision() == 7");
    CHECK(static_cast<long long>(z.digits.size()) == z.precision(),
          "extend_to: digits.size() == precision() after grow");
    for (std::size_t i = 4; i < z.digits.size(); ++i) {
        CHECK(z.digits[i] == 0, "extend_to: new high digits zero-padded");
    }

    // extend_to with a smaller n is a no-op (does not shrink).
    z.extend_to(5);
    CHECK(z.precision() == 7, "extend_to(5) on prec=7 -> still 7 (no-op)");

    // truncate_to shrinks digits and keeps the invariant.
    z.truncate_to(3);
    CHECK(z.precision() == 3, "truncate_to(3) on prec=7 -> precision() == 3");
    CHECK(static_cast<long long>(z.digits.size()) == z.precision(),
          "truncate_to: digits.size() == precision() after shrink");

    // truncate_to with a larger n is a no-op.
    z.truncate_to(5);
    CHECK(z.precision() == 3, "truncate_to(5) on prec=3 -> still 3 (no-op)");

    // set_precision grows when larger.
    z.set_precision(10);
    CHECK(z.precision() == 10, "set_precision(10) on prec=3 -> precision() == 10");
    CHECK(static_cast<long long>(z.digits.size()) == z.precision(),
          "set_precision: digits.size() == precision() after grow");

    // set_precision shrinks when smaller.
    z.set_precision(4);
    CHECK(z.precision() == 4, "set_precision(4) on prec=10 -> precision() == 4");
    CHECK(static_cast<long long>(z.digits.size()) == z.precision(),
          "set_precision: digits.size() == precision() after shrink");

    // set_precision same is a no-op.
    z.set_precision(4);
    CHECK(z.precision() == 4, "set_precision(4) on prec=4 -> still 4");

    // set_precision rejects negatives (defensive; the previous
    // code would silently leave the field in a bad state).
    bool threw = false;
    try { z.set_precision(-1); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw, "set_precision(-1) throws");
    CHECK(z.precision() == 4, "set_precision(-1) did not change precision");

    // After all the resize operations, arithmetic still produces
    // the correct result -- the invariant matters at the API
    // boundary, not just structurally.
    ZpInt a = zp_from_int(3, 2, 4);
    a.set_precision(8);  // grow with zero-padding
    a.digits[4] = 1;     // sprinkle a 1 in the middle
    ZpInt b = zp_one(3, 8);
    ZpInt s = zp_add(a, b);
    CHECK(s.precision() == 8, "add after set_precision: precision preserved");
    CHECK(s.digits[0] == 0, "add: digits[0] = 0 (2+1 in base 3 = 10)");
    CHECK(s.digits[1] == 1, "add: digits[1] = 1");
    for (std::size_t i = 2; i < 4; ++i) CHECK(s.digits[i] == 0, "add: middle digits 0");
    CHECK(s.digits[4] == 1, "add: high digit preserved through add");
}

void test_precision_boundary_valuation() {
    fprintf(stderr, "=== test_precision_boundary_valuation ===\n");
    // Multiply two elements each with valuation ~precision/2 and
    // check the result doesn't spuriously look exact-zero when it
    // shouldn't, and does look zero-to-precision when the true
    // valuation exceeds the working precision.
    ZpInt a = zp_zero(3, 6);
    a.digits[3] = 1;  // a = 3^3 (valuation 3)
    ZpInt b = zp_zero(3, 6);
    b.digits[3] = 1;  // b = 3^3 (valuation 3)
    ZpInt prod = zp_mul(a, b);  // true value 3^6, which is 0 mod 3^6 (precision 6)
    CHECK(zp_valuation(prod) == 6, "3^3 * 3^3 = 3^6 looks like valuation >= precision (6)");
}

void test_qp_totally_ramified_rnd13() {
    fprintf(stderr, "=== test_qp_totally_ramified_rnd13 ===\n");
    // rnd13's quartic charpoly: x^4 - 4x^3 - 8x^2 - 6x - 2.
    // dedekind_factorization_test / rnd13_factor_probe already
    // confirmed: p=2 is 2-maximal, (2) = p^4 (single prime, e=4,
    // f=1) -- so K_p = Q_2[beta]/(charpoly) directly, per the
    // QpTotallyRamified doc comment.  This test checks the defining
    // relation actually holds numerically in the Z_p arithmetic,
    // and that beta itself has p-adic valuation exactly 1 (as
    // derived from the norm: |N(beta)| = |constant term| = 2 = 2^1,
    // and f=1 means v_2(N(beta)) = v_p(beta) directly).
    mathlib::PolyZ charpoly;
    charpoly.ensure_size(5);
    mathlib::set_si(charpoly.coeff(0), -2);
    mathlib::set_si(charpoly.coeff(1), -6);
    mathlib::set_si(charpoly.coeff(2), -8);
    mathlib::set_si(charpoly.coeff(3), -4);
    mathlib::set_si(charpoly.coeff(4), 1);

    long long p = 2, n = 4, prec = 30;
    QpTotallyRamified beta = qp_uniformizer(p, n, charpoly, prec);

    // beta^4 should equal 4*beta^3 + 8*beta^2 + 6*beta + 2 (i.e. the
    // relation from the charpoly: beta^4 - 4beta^3 - 8beta^2 - 6beta - 2 = 0).
    QpTotallyRamified beta2 = qp_mul(beta, beta);
    QpTotallyRamified beta3 = qp_mul(beta2, beta);
    QpTotallyRamified beta4 = qp_mul(beta3, beta);

    QpTotallyRamified four_beta3 = beta3;
    for (auto& c : four_beta3.coeffs) c = zp_mul(c, zp_from_int_full(p, 4, prec));
    QpTotallyRamified eight_beta2 = beta2;
    for (auto& c : eight_beta2.coeffs) c = zp_mul(c, zp_from_int_full(p, 8, prec));
    QpTotallyRamified six_beta = beta;
    for (auto& c : six_beta.coeffs) c = zp_mul(c, zp_from_int_full(p, 6, prec));
    QpTotallyRamified two = qp_zero(p, n, charpoly, prec);
    two.coeffs[0] = zp_from_int_full(p, 2, prec);

    QpTotallyRamified rhs = qp_add(qp_add(four_beta3, eight_beta2), qp_add(six_beta, two));
    CHECK(qp_equal(beta4, rhs), "beta^4 == 4beta^3 + 8beta^2 + 6beta + 2 in K_2 (defining relation holds)");

    // v_2(beta) should be exactly 1.
    CHECK(qp_valuation(beta) == 1, "v_2(beta) = 1 (beta is a uniformizer for the totally ramified prime)");

    // v_2(beta^4) should be exactly 4 (= v_2(2), since (beta) and (2)
    // generate the same prime power up to a unit: (2) = p^4 = (beta)^4).
    CHECK(qp_valuation(beta4) == 4, "v_2(beta^4) = 4, matching (2) = p^4");

    // A rational digit example: embed 1/2 * beta^2 (an actual digit
    // shape from the prefix-automaton eigenvector, per
    // prefix_automaton_test's worked-example output) and check its
    // valuation is well-defined (1/2 has valuation -1 at p=2, so the
    // whole term has valuation 2 + 4*(-1) = -2 -- i.e. it is NOT in
    // Z_p, it has a genuine pole at p=2; this is an expected, not a
    // bug, since eigenvector entries need not be p-integral).
    std::vector<mathlib::Rat> half_beta2(4);
    mathlib::set_si(half_beta2[2], 1, 2);
    bool threw = false;
    try {
        qp_from_qelem(p, n, charpoly, half_beta2, prec);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw, "1/2 is correctly rejected by zp_from_rat at p=2 (denominator not coprime to p)");
}

void test_zp_from_int_full() {
    fprintf(stderr, "=== test_zp_from_int_full ===\n");
    // zp_from_int_full(2, 2, prec) should be "10" in base 2, i.e.
    // digits = [0, 1, 0, ...] -- unlike zp_from_int(2, 2, prec)
    // which deliberately only stores 2 mod 2 = 0 in digit 0.
    ZpInt two = zp_from_int_full(2, 2, 10);
    CHECK(two.digits[0] == 0 && two.digits[1] == 1, "zp_from_int_full(2,2): base-2 digits = [0,1]");

    // Cross-check against zp_from_rat(p, num=8, den=1) for a
    // multi-digit value: 8 = 1000 in base 2.
    mathlib::BigInt num, den;
    mathlib::set_si(num, 8);
    mathlib::set_si(den, 1);
    ZpInt eight_via_rat = zp_from_rat(2, num, den, 10);
    ZpInt eight_via_full = zp_from_int_full(2, 8, 10);
    CHECK(zp_equal(eight_via_rat, eight_via_full),
          "zp_from_int_full(2,8) agrees with zp_from_rat(2, 8/1) (two independent paths)");

    // Negative: -3 in Z_5 at precision 4 should equal 5^4 - 3 = 622,
    // i.e. digits (base 5, low first) of 622: 622 = 4 + 4*5 + 4*25 + 4*125? 
    // check via zp_neg(zp_from_int_full(5,3,4)) instead of hand arithmetic.
    ZpInt neg3 = zp_from_int_full(5, -3, 4);
    ZpInt three = zp_from_int_full(5, 3, 4);
    ZpInt sum = zp_add(neg3, three);
    ZpInt zero = zp_zero(5, 4);
    CHECK(zp_equal(sum, zero), "zp_from_int_full(5,-3) + zp_from_int_full(5,3) = 0");
}

}  // namespace

int main() {
    test_construction();
    test_addition();
    test_subtraction();
    test_multiplication();
    test_valuation();
    test_inversion();
    test_known_identity_geometric_series();
    test_precision_works();
    test_set_precision_invariant();
    test_precision_boundary_valuation();
    test_zp_from_int_full();
    test_qp_totally_ramified_rnd13();
    fprintf(stderr, "\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
