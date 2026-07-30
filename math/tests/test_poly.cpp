// Test Tier 0: poly_z and poly_q.
//
// Reference values: pre-computed from SymPy (see sympy_ref.py).
// Each test asserts exact equality via mpz/mpq coercion.
//
// Convention reminder: mathlib::PolyZ uses LOWEST-degree first.
// PolyZ({7, -5, 2, 1}) is the polynomial 7 - 5x + 2x^2 + x^3.

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"

#include "test_common.hpp"

using namespace mathlib;







int main() {
    std::printf("== poly_z::add ==\n");
    // (x^3 + 2x^2 - 5x + 7) + (3x^2 - x + 4) = x^3 + 5x^2 - 6x + 11
    // Lowest-degree first: {7, -5, 2, 1} + {4, -1, 3} = {11, -6, 5, 1}
    {
        PolyZ a({7, -5, 2, 1});
        PolyZ b({4, -1, 3});
        PolyZ c = a + b;
        EXPECT_EQ_PZ(c.coeff(0), B(11));
        EXPECT_EQ_PZ(c.coeff(1), B(-6));
        EXPECT_EQ_PZ(c.coeff(2), B(5));
        EXPECT_EQ_PZ(c.coeff(3), B(1));
    }

    std::printf("== poly_z::sub ==\n");
    // (x^3 + 2x^2 - 5x + 7) - (3x^2 - x + 4) = x^3 - x^2 - 4x + 3
    // {7, -5, 2, 1} - {4, -1, 3} = {3, -4, -1, 1}
    {
        PolyZ a({7, -5, 2, 1});
        PolyZ b({4, -1, 3});
        PolyZ c = a - b;
        EXPECT_EQ_PZ(c.coeff(0), B(3));
        EXPECT_EQ_PZ(c.coeff(1), B(-4));
        EXPECT_EQ_PZ(c.coeff(2), B(-1));
        EXPECT_EQ_PZ(c.coeff(3), B(1));
    }

    std::printf("== poly_z::mul ==\n");
    // (x^3 + 2x^2 - 5x + 7) * (3x^2 - x + 4) = 3x^5 + 5x^4 - 13x^3 + 34x^2 - 27x + 28
    // {7, -5, 2, 1} * {4, -1, 3} = {28, -27, 34, -13, 5, 3}
    {
        PolyZ a({7, -5, 2, 1});
        PolyZ b({4, -1, 3});
        PolyZ c = a * b;
        EXPECT_EQ_PZ(c.coeff(0), B(28));
        EXPECT_EQ_PZ(c.coeff(1), B(-27));
        EXPECT_EQ_PZ(c.coeff(2), B(34));
        EXPECT_EQ_PZ(c.coeff(3), B(-13));
        EXPECT_EQ_PZ(c.coeff(4), B(5));
        EXPECT_EQ_PZ(c.coeff(5), B(3));
    }

    std::printf("== poly_z::divmod monic / monic exact ==\n");
    // (x^3 + 2x^2 - 5x + 7) / (x^2 - 1)  in Q: q = x + 2, r = -4x + 9
    // (x^2 - 1) is monic, so pseudo-rem is exact division.  q = 2 + x, r = 9 - 4x.
    {
        PolyZ a({7, -5, 2, 1});
        PolyZ b({-1, 0, 1});
        DivModResultZ dm = divmod(a, b);
        EXPECT_EQ_PZ(dm.q.coeff(0), B(2));
        EXPECT_EQ_PZ(dm.q.coeff(1), B(1));
        EXPECT_EQ_PZ(dm.r.coeff(0), B(9));
        EXPECT_EQ_PZ(dm.r.coeff(1), B(-4));
        // Identity: lc(b)^(da-db+1) * a = q * b + r
        PolyZ a2 = a;
        PolyZ qb = dm.q * b;
        PolyZ sum = qb + dm.r;
        EXPECT(a2 == sum, "divmod identity");
    }

    std::printf("== poly_z::divmod non-monic pseudo-remainder ==\n");
    // a = x^3 + 2x^2 - 5x + 7, b = 2x^2 - 1, lc(b) = 2
    // Pseudo-rem: lc(b)^(da-db+1) * a = 2^2 * a = 4a
    // q = 2x + 4, r = -18x + 32
    // Check: 4a = q*b + r
    //   4a = 4x^3 + 8x^2 - 20x + 28
    //   q*b = (2x + 4)(2x^2 - 1) = 4x^3 + 8x^2 - 2x - 4
    //   q*b + r = 4x^3 + 8x^2 - 2x - 4 + (-18x + 32) = 4x^3 + 8x^2 - 20x + 28 = 4a ✓
    {
        PolyZ a({7, -5, 2, 1});
        PolyZ b({-1, 0, 2});
        DivModResultZ dm = divmod(a, b);
        std::printf("    q = %s\n", str(dm.q).c_str());
        std::printf("    r = %s\n", str(dm.r).c_str());
        EXPECT_EQ_PZ(dm.q.coeff(0), B(4));
        EXPECT_EQ_PZ(dm.q.coeff(1), B(2));
        EXPECT_EQ_PZ(dm.r.coeff(0), B(32));
        EXPECT_EQ_PZ(dm.r.coeff(1), B(-18));
    }

    std::printf("== poly_z::gcd x^4-1 vs x^6-1 ==\n");
    // gcd(x^4 - 1, x^6 - 1) = x^2 - 1
    {
        PolyZ a({-1, 0, 0, 0, 1});
        PolyZ b({-1, 0, 0, 0, 0, 0, 1});
        PolyZ g = gcd(a, b);
        std::printf("    g = %s\n", str(g).c_str());
        EXPECT_EQ_PZ(g.coeff(0), B(-1));
        EXPECT_EQ_PZ(g.coeff(1), B(0));
        EXPECT_EQ_PZ(g.coeff(2), B(1));
    }

    std::printf("== poly_z::gcd 2x^2+3x+1 vs x^2+x ==\n");
    // gcd(2x^2 + 3x + 1, x^2 + x) = x + 1
    {
        PolyZ a({1, 3, 2});
        PolyZ b({0, 1, 1});
        PolyZ g = gcd(a, b);
        std::printf("    g = %s\n", str(g).c_str());
        EXPECT_EQ_PZ(g.coeff(0), B(1));
        EXPECT_EQ_PZ(g.coeff(1), B(1));
    }

    std::printf("== poly_z::eval ==\n");
    // p(x) = 2x^2 - 3x + 1, p(5) = 50 - 15 + 1 = 36
    {
        PolyZ p({1, -3, 2});
        BigInt r = eval(p, B(5));
        EXPECT_EQ_PZ(r, B(36));
    }

    std::printf("== poly_q::add ==\n");
    {
        PolyQ a({{7,1}, {-5,1}, {2,1}, {1,1}});
        PolyQ b({{4,1}, {-1,1}, {3,1}});
        PolyQ c = a + b;
        EXPECT_EQ_Q(c.coeff(0), Q_(11));
        EXPECT_EQ_Q(c.coeff(1), Q_(-6));
        EXPECT_EQ_Q(c.coeff(2), Q_(5));
        EXPECT_EQ_Q(c.coeff(3), Q_(1));
    }

    std::printf("== poly_q::divmod ==\n");
    // (x^3 + 2x^2 - 5x + 7) / (x^2 - 1)  in Q: q = x + 2, r = -4x + 9
    {
        PolyQ a({{7,1}, {-5,1}, {2,1}, {1,1}});
        PolyQ b({{-1,1}, {0,1}, {1,1}});
        DivModResultQ dm = divmod(a, b);
        EXPECT_EQ_Q(dm.q.coeff(0), Q_(2));
        EXPECT_EQ_Q(dm.q.coeff(1), Q_(1));
        EXPECT_EQ_Q(dm.r.coeff(0), Q_(9));
        EXPECT_EQ_Q(dm.r.coeff(1), Q_(-4));
    }

    std::printf("== poly_q::gcd ==\n");
    // gcd(x^2 - 1, x^2 - 2x + 1) = x - 1
    {
        PolyQ a({{-1,1}, {0,1}, {1,1}});
        PolyQ b({{1,1}, {-2,1}, {1,1}});
        PolyQ g = gcd(a, b);
        std::printf("    g = %s\n", str(g).c_str());
        EXPECT_EQ_Q(g.coeff(0), Q_(-1));
        EXPECT_EQ_Q(g.coeff(1), Q_(1));
    }

    std::printf("\n== summary ==\n");
    std::printf("%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
