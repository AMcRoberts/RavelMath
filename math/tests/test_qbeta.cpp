// Test Tier 1: qbeta arithmetic.
//
// Reference values: pre-computed from SymPy (see sympy_ref.py /
// verify_qbeta_refs.py).  All tests assert EXACT mpq_t equality.

#include <cstdio>
#include <cstdlib>
#include <string>

#include "math/bigint.hpp"
#include "math/qbeta.hpp"

#include "test_common.hpp"

using namespace mathlib;







int main() {
    std::printf("== qbeta::ring construction ==\n");
    {
        // Fibonacci: charpoly x^2 - x - 1, PolyZ form {-1, -1, 1}
        QBetaRing R(PolyZ({-1, -1, 1}));
        EXPECT(R.degree() == 2, "Fibonacci degree");
    }
    {
        // Tribonacci: x^3 - x^2 - x - 1, PolyZ form {-1, -1, -1, 1}
        QBetaRing R(PolyZ({-1, -1, -1, 1}));
        EXPECT(R.degree() == 3, "Tribonacci degree");
    }
    {
        // TetrABONACCI: x^4 - x^3 - x^2 - x - 1
        QBetaRing R(PolyZ({-1, -1, -1, -1, 1}));
        EXPECT(R.degree() == 4, "TetrABONACCI degree");
    }

    std::printf("== qbeta::from_low_first ==\n");
    {
        // Tribonacci: low_first = [-1, -1, -1] (i.e., c[0]=-1, c[1]=-1, c[2]=-1)
        // PolyZ form: {-1, -1, -1, 1}
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});
        EXPECT(R.degree() == 3, "Tribonacci low_first degree");
        EXPECT_EQ_BI(R.charpoly().coeff(0), -1);
        EXPECT_EQ_BI(R.charpoly().coeff(1), -1);
        EXPECT_EQ_BI(R.charpoly().coeff(2), -1);
        EXPECT_EQ_BI(R.charpoly().coeff(3), 1);
    }

    std::printf("== qbeta::add / sub / neg ==\n");
    {
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});
        QElem a = qbeta_vec({{1,1}, {2,1}, {3,1}});
        QElem b = qbeta_vec({{4,1}, {5,1}, {6,1}});
        QElem c = R.add(a, b);
        EXPECT_EQ_RAT(c.coeff(0), 5);
        EXPECT_EQ_RAT(c.coeff(1), 7);
        EXPECT_EQ_RAT(c.coeff(2), 9);
        QElem d = R.sub(a, b);
        EXPECT_EQ_RAT(d.coeff(0), -3);
        EXPECT_EQ_RAT(d.coeff(1), -3);
        EXPECT_EQ_RAT(d.coeff(2), -3);
        QElem na = R.neg(a);
        EXPECT_EQ_RAT(na.coeff(0), -1);
        EXPECT_EQ_RAT(na.coeff(1), -2);
        EXPECT_EQ_RAT(na.coeff(2), -3);
    }

    std::printf("== qbeta::mul reduction (β·β == β²) ==\n");
    {
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});  // Tribonacci
        QElem b = R.beta_k(1);
        QElem bb = R.mul(b, b);
        // β² = 0·1 + 0·β + 1·β²
        EXPECT_EQ_RAT(bb.coeff(0), 0);
        EXPECT_EQ_RAT(bb.coeff(1), 0);
        EXPECT_EQ_RAT(bb.coeff(2), 1);
    }

    std::printf("== qbeta::mul reduction (β³ = β² + β + 1 for Tribonacci) ==\n");
    {
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});  // Tribonacci
        QElem b = R.beta_k(1);
        QElem b2 = R.mul(b, b);
        QElem b3 = R.mul(b2, b);
        // β³ = β² + β + 1 in Tribonacci (coefficients: 1, 1, 1)
        EXPECT_EQ_RAT(b3.coeff(0), 1);
        EXPECT_EQ_RAT(b3.coeff(1), 1);
        EXPECT_EQ_RAT(b3.coeff(2), 1);
    }

    std::printf("== qbeta::β⁻¹ for Fibonacci (charpoly x² - x - 1) ==\n");
    {
        // β⁻¹ = -1 + β (low_first: [-1, 1])
        // Because β² = β + 1, so β(β - 1) = β² - β = 1.
        QBetaRing R = QBetaRing::from_low_first({-1, -1});
        QElem b_inv = R.beta_inverse();
        EXPECT_EQ_RAT(b_inv.coeff(0), -1);
        EXPECT_EQ_RAT(b_inv.coeff(1), 1);
        // Verify: β · β⁻¹ = 1
        QElem b = R.beta_k(1);
        QElem prod = R.mul(b, b_inv);
        QElem one = R.one();
        EXPECT_EQ_QELEM(prod, one);
    }

    std::printf("== qbeta::β⁻¹ for Tribonacci (charpoly x³ - x² - x - 1) ==\n");
    {
        // β⁻¹ = -1 - β + β² (low_first: [-1, -1, 1])
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});
        QElem b_inv = R.beta_inverse();
        EXPECT_EQ_RAT(b_inv.coeff(0), -1);
        EXPECT_EQ_RAT(b_inv.coeff(1), -1);
        EXPECT_EQ_RAT(b_inv.coeff(2), 1);
        QElem b = R.beta_k(1);
        QElem prod = R.mul(b, b_inv);
        QElem one = R.one();
        EXPECT_EQ_QELEM(prod, one);
    }

    std::printf("== qbeta::β⁻¹ for σ_1 (charpoly x³ - 3x² - 2x - 1) ==\n");
    {
        // β⁻¹ = -2 - 3β + β² (low_first: [-2, -3, 1])
        QBetaRing R = QBetaRing::from_low_first({-3, -2, -1});
        QElem b_inv = R.beta_inverse();
        EXPECT_EQ_RAT(b_inv.coeff(0), -2);
        EXPECT_EQ_RAT(b_inv.coeff(1), -3);
        EXPECT_EQ_RAT(b_inv.coeff(2), 1);
        QElem b = R.beta_k(1);
        QElem prod = R.mul(b, b_inv);
        QElem one = R.one();
        EXPECT_EQ_QELEM(prod, one);
    }

    std::printf("== qbeta::β⁻¹ for TetrABONACCI (charpoly x⁴ - x³ - x² - x - 1) ==\n");
    {
        // β⁻¹ = -1 - β - β² + β³ (low_first: [-1, -1, -1, 1])
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1, -1});
        QElem b_inv = R.beta_inverse();
        EXPECT_EQ_RAT(b_inv.coeff(0), -1);
        EXPECT_EQ_RAT(b_inv.coeff(1), -1);
        EXPECT_EQ_RAT(b_inv.coeff(2), -1);
        EXPECT_EQ_RAT(b_inv.coeff(3), 1);
        QElem b = R.beta_k(1);
        QElem prod = R.mul(b, b_inv);
        QElem one = R.one();
        EXPECT_EQ_QELEM(prod, one);
    }

    std::printf("== qbeta::mul distributive check ==\n");
    {
        // For Tribonacci: (1 + β)(1 - β + β²) should be 1 + β³ = 1 + (β² + β + 1) = 2 + β + β²
        QBetaRing R = QBetaRing::from_low_first({-1, -1, -1});
        QElem a = R.add(R.one(), R.beta_k(1));      // 1 + β
        QElem b = qbeta_vec({{1,1}, {-1,1}, {1,1}});  // 1 - β + β²
        QElem prod = R.mul(a, b);
        EXPECT_EQ_RAT(prod.coeff(0), 2);
        EXPECT_EQ_RAT(prod.coeff(1), 1);
        EXPECT_EQ_RAT(prod.coeff(2), 1);
    }

    std::printf("\n== summary ==\n");
    std::printf("%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
