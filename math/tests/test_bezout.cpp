// Test Tier 2: Bezout and Q(β) inverse.
//
// Reference values: pre-computed from SymPy (see verify_bezout_refs.py).
// All tests assert EXACT mpz_t / mpq_t equality.

#include <cstdio>
#include <cstdlib>
#include <string>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"
#include "math/bezout.hpp"

#include "test_common.hpp"

using namespace mathlib;






int main() {
    std::printf("== ext_gcd in Z[x]: (x+1)² vs (x-1) ==\n");
    {
        // a = x² + 2x + 1, b = x - 1.  gcd = x - 1.
        PolyZ a({1, 2, 1});
        PolyZ b({-1, 1});
        ExtGcdResultZ eg = ext_gcd(a, b);
        std::printf("    g = %s\n", str(eg.g).c_str());
        std::printf("    u = %s\n", str(eg.u).c_str());
        std::printf("    v = %s\n", str(eg.v).c_str());
        // Verify: u*a + v*b == g
        PolyZ lhs = eg.u * a + eg.v * b;
        EXPECT_EQ_PZ(lhs, eg.g);
    }

    std::printf("== ext_gcd in Z[x]: (2x+1) vs (x+1) ==\n");
    {
        // a = 2x+1, b = x+1.  gcd = 1 (since x = -1 makes 2x+1 = -1).
        PolyZ a({1, 2});
        PolyZ b({1, 1});
        ExtGcdResultZ eg = ext_gcd(a, b);
        std::printf("    g = %s\n", str(eg.g).c_str());
        std::printf("    u = %s\n", str(eg.u).c_str());
        std::printf("    v = %s\n", str(eg.v).c_str());
        PolyZ lhs = eg.u * a + eg.v * b;
        EXPECT_EQ_PZ(lhs, eg.g);
    }

    std::printf("== ext_gcd in Q[x]: (x³-1) vs (x²-1) ==\n");
    {
        PolyQ a({{-1,1}, {0,1}, {0,1}, {1,1}});
        PolyQ b({{-1,1}, {0,1}, {1,1}});
        ExtGcdResultQ eg = ext_gcd(a, b);
        std::printf("    g = %s\n", str(eg.g).c_str());
        std::printf("    u = %s\n", str(eg.u).c_str());
        std::printf("    v = %s\n", str(eg.v).c_str());
        PolyQ lhs = eg.u * a + eg.v * b;
        EXPECT(lhs == eg.g, "u*a + v*b == g (Q)");
    }

    std::printf("== Q(β) inverse: (1+β)^{-1} in Fibonacci ==\n");
    {
        QBetaRing R = QBetaRing::from_low_first({-1, -1});
        QElem one_plus_b = R.add(R.one(), R.beta_k(1));
        QBetaInverseResult ir = invert_in_qbeta(one_plus_b, R);
        EXPECT(ir.invertible, "1+β invertible in Fibonacci");
        std::printf("    inv = %s\n", str(ir.inverse).c_str());
        // (1+β)^{-1} = 2 - β
        QElem expected = qbeta_vec({{2,1}, {-1,1}});
        EXPECT(ir.inverse == expected, "(1+β)^{-1} = 2 - β");
        // Verify: (1+β) * inv = 1
        QElem prod = R.mul(one_plus_b, ir.inverse);
        EXPECT(prod == R.one(), "(1+β)·(1+β)^{-1} = 1");
    }

    std::printf("== Q(β) inverse: (1+β)^{-1} in Tribonacci ==\n");
    {
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});
        QElem one_plus_b = R.add(R.one(), R.beta_k(1));
        QBetaInverseResult ir = invert_in_qbeta(one_plus_b, R);
        EXPECT(ir.invertible, "1+β invertible in Tribonacci");
        std::printf("    inv = %s\n", str(ir.inverse).c_str());
        // (1+β)^{-1} = 1/2 - β + 1/2 β²
        QElem expected = qbeta_vec({{1,2}, {-1,1}, {1,2}});
        EXPECT(ir.inverse == expected, "(1+β)^{-1} = 1/2 - β + 1/2 β²");
        QElem prod = R.mul(one_plus_b, ir.inverse);
        EXPECT(prod == R.one(), "(1+β)·(1+β)^{-1} = 1");
    }

    std::printf("== Q(β) inverse: (1+β)^{-1} in σ_1 ==\n");
    {
        QBetaRing R = QBetaRing::from_low_first({-3, -2, -1});
        QElem one_plus_b = R.add(R.one(), R.beta_k(1));
        QBetaInverseResult ir = invert_in_qbeta(one_plus_b, R);
        EXPECT(ir.invertible, "1+β invertible in σ_1");
        std::printf("    inv = %s\n", str(ir.inverse).c_str());
        QElem expected = qbeta_vec({{2,3}, {-4,3}, {1,3}});
        EXPECT(ir.inverse == expected, "(1+β)^{-1} = 2/3 - 4/3 β + 1/3 β²");
        QElem prod = R.mul(one_plus_b, ir.inverse);
        EXPECT(prod == R.one(), "(1+β)·(1+β)^{-1} = 1");
    }

    std::printf("== Q(β) inverse: (1+β)^{-1} in TetrABONACCI ==\n");
    {
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1, -1});
        QElem one_plus_b = R.add(R.one(), R.beta_k(1));
        QBetaInverseResult ir = invert_in_qbeta(one_plus_b, R);
        EXPECT(ir.invertible, "1+β invertible in TetrABONACCI");
        std::printf("    inv = %s\n", str(ir.inverse).c_str());
        QElem expected = qbeta_vec({{2,1}, {-1,1}, {2,1}, {-1,1}});
        EXPECT(ir.inverse == expected, "(1+β)^{-1} = 2 - β + 2β² - β³");
        QElem prod = R.mul(one_plus_b, ir.inverse);
        EXPECT(prod == R.one(), "(1+β)·(1+β)^{-1} = 1");
    }

    std::printf("== Q(β) inverse: β^{-1} via Bezout (vs closed-form) ==\n");
    {
        // Verify Bezout-based β^{-1} matches the closed-form β^{-1}
        // for several Pisot substitutions.
        struct PisotTest { const char* name; std::initializer_list<long long> c; };
        PisotTest tests[] = {
            {"Fibonacci", {-1, -1}},
            {"Tribonacci", {-1, -1, -1}},
            {"sigma_1", {-3, -2, -1}},
            {"sigma_2", {-2, -1, -1}},
            {"TetrABONACCI", {-1, -1, -1, -1}},
        };
        for (const auto& t : tests) {
            QBetaRing R = QBetaRing::from_low_first(t.c);
            QElem b = R.beta_k(1);
            QElem b_inv_closed = R.beta_inverse();
            QBetaInverseResult ir = invert_in_qbeta(b, R);
            EXPECT(ir.invertible, "β invertible");
            if (ir.invertible) {
                EXPECT(ir.inverse == b_inv_closed, "Bezout β^{-1} == closed-form β^{-1}");
                QElem prod = R.mul(b, ir.inverse);
                EXPECT(prod == R.one(), "β·β^{-1} = 1");
            }
        }
    }

    std::printf("\n== summary ==\n");
    std::printf("%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
