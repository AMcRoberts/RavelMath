// spectral_general_test.cpp
//
// Self-test for spectral_invariants_general (spectral.hpp) and its
// wiring into classify_matrix_spectral (survey.hpp). See
// TODO_GENERALIZATION.md items 1-2: before this, any matrix with
// n != 2, 3 made classify_matrix_spectral silently return
// beta=0/pisot=false. This test is a regression guard against that
// coming back.
//
// Expected values cross-checked against:
//   * Tribonacci: AUDIT.md's numerical-cross-check table
//     (beta=1.839286755214161, |b2|=0.737352705760328).
//   * Tetrabonacci, rnd13: verified independently via
//     an unrelated standalone power iteration in cylinder_measure.cpp
//     (beta=1.927562, beta=5.623559 respectively).

#include "ravel/survey.hpp"
#include <cstdio>
#include <cmath>

using namespace ravel;

static int total_tests = 0;
static int failed = 0;

#define CHECK_NEAR(actual, expected, tol, label) do {                 \
    ++total_tests;                                                    \
    double diff = std::abs((actual) - (expected));                    \
    if (diff > (tol)) {                                                \
        std::printf("  [FAIL] %s (got %.6f, expected %.6f, diff %.2e)\n", \
                    label, (double)(actual), (double)(expected), diff); \
        ++failed;                                                     \
    } else {                                                          \
        std::printf("  [ok]   %s (%.6f)\n", label, (double)(actual)); \
    }                                                                 \
} while (0)

#define CHECK(expr, label) do {                              \
    ++total_tests;                                            \
    if (!(expr)) { std::printf("  [FAIL] %s\n", label); ++failed; } \
    else         { std::printf("  [ok]   %s\n", label); }     \
} while (0)

int main() {
    std::printf("spectral_invariants_general self-tests:\n");

    {
        // n=3 must still take the closed-form Cardano path (spectral.hpp
        // dispatches n==3 to spectral_invariants_3x3, unaffected by this
        // change) -- this checks the dispatch didn't regress it.
        std::vector<std::vector<long long>> M = {{1,1,1},{1,0,0},{0,1,0}};
        auto r = classify_matrix_spectral(M);
        CHECK_NEAR(r.beta, 1.839286755214161, 1e-9, "Tribonacci (n=3, closed-form path) beta");
        CHECK_NEAR(r.b2, 0.737352705760328, 1e-9, "Tribonacci (n=3, closed-form path) b2");
        CHECK(r.pisot, "Tribonacci classified as Pisot");
    }
    {
        // n=4: this is the case that used to silently fail.
        std::vector<std::vector<long long>> M = {{1,1,1,1},{1,0,0,0},{0,1,0,0},{0,0,1,0}};
        auto r = classify_matrix_spectral(M);
        CHECK_NEAR(r.beta, 1.927562, 1e-5, "Tetrabonacci (n=4, general path) beta");
        CHECK(r.pisot, "Tetrabonacci classified as Pisot (used to be false unconditionally)");
    }
    {
        // rnd13, non-unimodular (|det|=2), n=4.
        std::vector<std::vector<long long>> M = {{2,2,2,3},{1,0,0,0},{1,1,0,1},{2,2,2,2}};
        auto r = classify_matrix_spectral(M);
        CHECK_NEAR(r.beta, 5.623559, 1e-5, "rnd13 (n=4, general path) beta");
        CHECK(std::abs(r.b2) < r.beta, "rnd13 beta2 modulus < beta (Perron-Frobenius sanity)");
        CHECK(r.pisot, "rnd13 classified as Pisot");
    }
    {
        // A deliberately non-Pisot 4x4 (identity-like, all eigenvalues
        // modulus 1) should NOT be misreported as Pisot by the general path.
        std::vector<std::vector<long long>> M = {{0,1,0,0},{0,0,1,0},{0,0,0,1},{1,0,0,0}};
        auto r = classify_matrix_spectral(M);
        CHECK(!r.pisot, "4-cycle permutation matrix correctly rejected as non-Pisot");
    }

    std::printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
