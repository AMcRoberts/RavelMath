// Test: charpoly.hpp (Faddeev-LeVerrier characteristic polynomial).
//
// Checked against hand-computed closed-form charpolys, not against
// another implementation of the same algorithm (per
// docs/CPP_DESIGN_PHILOSOPHY.md Sec. 3: "test against a closed-form
// value where one exists").

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "math/charpoly.hpp"

#include "test_common.hpp"

using namespace mathlib;

int main() {
    std::printf("== charpoly: Faddeev-LeVerrier ==\n");
    {
        // Fibonacci companion matrix [[1,1],[1,0]]: det(xI-A) = x^2-x-1.
        std::vector<std::vector<long long>> fib = {{1, 1}, {1, 0}};
        PolyZ cp = charpoly_faddeev_leverrier(fib);
        EXPECT(cp.degree() == 2, "Fibonacci charpoly has degree 2");
        EXPECT_EQ_BI(cp.coeff(0), -1);
        EXPECT_EQ_BI(cp.coeff(1), -1);
        EXPECT_EQ_BI(cp.coeff(2), 1);
    }
    {
        // Tribonacci companion matrix [[1,1,1],[1,0,0],[0,1,0]]:
        // det(xI-A) = x^3 - x^2 - x - 1 (cross-checked elsewhere in
        // this project, e.g. tests/spectral_general_test.cpp and
        // math/tests/test_ball.cpp's beta ~= 1.839286755 reference).
        std::vector<std::vector<long long>> trib = {
            {1, 1, 1}, {1, 0, 0}, {0, 1, 0}};
        PolyZ cp = charpoly_faddeev_leverrier(trib);
        EXPECT(cp.degree() == 3, "Tribonacci charpoly has degree 3");
        EXPECT_EQ_BI(cp.coeff(0), -1);
        EXPECT_EQ_BI(cp.coeff(1), -1);
        EXPECT_EQ_BI(cp.coeff(2), -1);
        EXPECT_EQ_BI(cp.coeff(3), 1);
    }
    {
        // Identity matrix (any size): det(xI - I) = (x-1)^n. For n=3:
        // (x-1)^3 = x^3 - 3x^2 + 3x - 1.
        std::vector<std::vector<long long>> id3 = {
            {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        PolyZ cp = charpoly_faddeev_leverrier(id3);
        EXPECT_EQ_BI(cp.coeff(0), -1);
        EXPECT_EQ_BI(cp.coeff(1), 3);
        EXPECT_EQ_BI(cp.coeff(2), -3);
        EXPECT_EQ_BI(cp.coeff(3), 1);
    }
    {
        // A diagonal matrix diag(2,3): det(xI-A) = (x-2)(x-3) = x^2-5x+6.
        std::vector<std::vector<long long>> diag23 = {{2, 0}, {0, 3}};
        PolyZ cp = charpoly_faddeev_leverrier(diag23);
        EXPECT_EQ_BI(cp.coeff(0), 6);
        EXPECT_EQ_BI(cp.coeff(1), -5);
        EXPECT_EQ_BI(cp.coeff(2), 1);
    }
    {
        // A single 1x1 matrix [[7]]: det(x-7) = x-7.
        std::vector<std::vector<long long>> one = {{7}};
        PolyZ cp = charpoly_faddeev_leverrier(one);
        EXPECT(cp.degree() == 1, "1x1 charpoly has degree 1");
        EXPECT_EQ_BI(cp.coeff(0), -7);
        EXPECT_EQ_BI(cp.coeff(1), 1);
    }
    {
        // A matrix with a large-ish entry, to exercise BigInt (not
        // just small long long) coefficients: [[0,1],[100,0]],
        // det(xI-A) = x^2 - 100.
        std::vector<std::vector<long long>> m = {{0, 1}, {100, 0}};
        PolyZ cp = charpoly_faddeev_leverrier(m);
        EXPECT_EQ_BI(cp.coeff(0), -100);
        EXPECT_EQ_BI(cp.coeff(1), 0);
        EXPECT_EQ_BI(cp.coeff(2), 1);
    }
    {
        // Non-square input must throw, not silently misbehave.
        bool threw = false;
        try {
            std::vector<std::vector<long long>> bad = {{1, 2, 3}, {4, 5}};
            charpoly_faddeev_leverrier(bad);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        EXPECT(threw, "non-square input throws invalid_argument");
    }
    {
        // Empty input must throw.
        bool threw = false;
        try {
            std::vector<std::vector<long long>> empty;
            charpoly_faddeev_leverrier(empty);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        EXPECT(threw, "empty input throws invalid_argument");
    }

    std::printf("\n%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
