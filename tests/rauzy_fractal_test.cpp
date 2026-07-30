// rauzy_fractal_test.cpp
//
// Self-tests for the Rauzy fractal construction.
// Verifies on Tetrabonacci (4-letter unimodular Pisot) that the
// IFS chaos game produces a valid 3D Rauzy fractal in the contracting
// hyperplane.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <array>

#include "ravel/rauzy_fractal.hpp"

using namespace ravel;

static int total_tests = 0;
static int failed = 0;

#define CHECK(expr, label) do {                              \
    ++total_tests;                                          \
    if (!(expr)) {                                          \
        printf("  [FAIL] %s\n", label);                     \
        ++failed;                                           \
    } else {                                                \
        printf("  [ok]   %s\n", label);                     \
    }                                                       \
} while (0)

int main() {
    // Tetrabonacci: σ(0) = (0,1), σ(1) = (0,2), σ(2) = (0,3), σ(3) = (0).
    RauzyFractal<4>::ImageList tetrabonacci_images;
    tetrabonacci_images[0] = {0, 1};
    tetrabonacci_images[1] = {0, 2};
    tetrabonacci_images[2] = {0, 3};
    tetrabonacci_images[3] = {0};

    RauzyFractal<4> rf(tetrabonacci_images);

    printf("Tetrabonacci: β=%.6f  (known unimodular)\n", rf.beta());
    printf("  u =");
    for (double x : rf.u()) printf(" %.6g", x);
    printf("\n  v =");
    for (double x : rf.v()) printf(" %.6g", x);
    printf("\n");

    // Check that M v = β v (approximately).
    double right_residual = 0;
    for (int i = 0; i < 4; ++i) {
        double row = 0;
        for (int j = 0; j < 4; ++j) row += rf.M()[i][j] * rf.v()[j];
        right_residual = std::max(
            right_residual, std::abs(row - rf.beta() * rf.v()[i]));
    }
    CHECK(right_residual < 1e-9,
          "M v ≈ β v (right Perron eigenvector check)");

    // Check that M^T u = β u.
    double left_residual = 0;
    for (int i = 0; i < 4; ++i) {
        double row = 0;
        for (int j = 0; j < 4; ++j) row += rf.M()[j][i] * rf.u()[j];
        left_residual = std::max(
            left_residual, std::abs(row - rf.beta() * rf.u()[i]));
    }
    CHECK(left_residual < 1e-9,
          "M^T u ≈ β u (left Perron eigenvector check)");

    // Check that M * M^{-1} = I (approximately).
    double max_err = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            double s = 0;
            for (int k = 0; k < 4; ++k) s += rf.M()[i][k] * rf.M_inv()[k][j];
            double err = std::abs(s - (i == j ? 1 : 0));
            if (err > max_err) max_err = err;
        }
    }
    CHECK(max_err < 1e-9, "M · M^{-1} = I (matrix inverse check)");

    // Check that u is in W^⊥ -- i.e., projecting any u gives 0.
    RauzyFractal<4>::Vec u_test{};
    for (int i = 0; i < 4; ++i) u_test[i] = rf.u()[i];
    auto u_proj = rf.project_to_W(u_test);
    double u_proj_norm = 0;
    for (int i = 0; i < 3; ++i) u_proj_norm += u_proj[i] * u_proj[i];
    CHECK(u_proj_norm < 1e-15, "project_to_W(u) ≈ 0 (u is in the kernel)");

    // Run chaos game and check orbit.
    auto pts = rf.chaos_game(10000, 42);
    CHECK(pts.size() == 10000, "chaos_game returns 10000 points");

    // Check that all points lie in the affine hyperplane H = {<x, u> = c}.
    double u_dot_v = 0;
    for (int i = 0; i < 4; ++i) u_dot_v += rf.u()[i] * rf.v()[i];
    double expected_height = u_dot_v / (rf.beta() - 1.0);
    double max_height_error = 0;
    for (const auto& p : pts) {
        double h = 0;
        for (int i = 0; i < 4; ++i) h += p[i] * rf.u()[i];
        max_height_error =
            std::max(max_height_error, std::abs(h - expected_height));
    }
    printf("  expected c = %.6f, max height error = %.3e\n",
           expected_height, max_height_error);
    CHECK(max_height_error < 1e-9,
          "chaos game orbit stays in the affine hyperplane");

    // Finite-resolution smoke check only. This estimator is biased
    // downward at 10k samples and six scales; it does not certify the
    // theoretical dimension.
    double dim = rf.box_dimension(6);
    printf("  finite-sample box-counting slope: %.3f\n", dim);
    CHECK(std::isfinite(dim) && dim > 2.0 && dim <= 3.0,
          "box-counting smoke estimate is finite and dimensionally plausible");

    printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
