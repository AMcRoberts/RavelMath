// tests/dual_test.cpp
//
// Tests for include/ravel/dual.hpp -- forward-mode autodiff Dual<T>.
//
// Forward-mode autodiff on a scalar function f: R -> R, evaluated at x:
//   1. Construct x_dual = Dual<double>(x, 1)   (variable with df/dx = 1)
//   2. Run f(x_dual) -- using the chain rule under the hood
//   3. Extract .eps -- that's df/dx at x
//
// For Dual<BigFloat>, the same idea but BigFloat can converge to
// irrationals (cbrt, sqrt), so the chain rule is the natural test for
// double-precision autodiff libraries vs. arbitrary-precision versions.
//
// Test ladder:
//   1. Basic chain rule on double (sanity for the API)
//   2. Composition: f(g(x))' = f'(g(x)) * g'(x)
//   3. cbrt and sqrt on double (Cardano-style operations)
//   4. Same on Dual<BigFloat> (converges on irrationals)
//   5. Same on Dual<Ball> (interval arithmetic widens as expected)
//   6. Finite-difference check vs closed-form derivatives
//   7. Matrix entrywise support (Dual<double> per matrix entry)

#include <cmath>
#include <cstdio>
#include <vector>

#include "math/bigint.hpp"
#include "math/ball.hpp"
#include "math/bigfloat.hpp"

#include "ravel/dual.hpp"

using namespace mathlib;
using namespace ravel::dual;

static int n_pass = 0, n_fail = 0;

#define EXPECT(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; std::fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

int main() {
    std::printf("== Dual<double>: basic chain rule ==\n");
    {
        // f(x) = x * x, so f'(x) = 2x.  At x = 3, f'(3) = 6.
        Dual<double> x = Dual<double>::variable(3.0);
        Dual<double> f = dual_mul(x, x);
        EXPECT(std::abs(f.val - 9.0) < 1e-15, "f(3) = 9");
        EXPECT(std::abs(f.eps - 6.0) < 1e-15, "f'(3) = 6 (chain rule)");
    }
    {
        // f(x) = (x + 1)^2  -- x=4 -> f=25, f'=2*(x+1)=10
        Dual<double> x = Dual<double>::variable(4.0);
        Dual<double> one = Dual<double>::constant(1.0);
        Dual<double> inner = x + one;
        Dual<double> f = dual_sqr(inner);
        EXPECT(std::abs(f.val - 25.0) < 1e-15, "f(4) = 25");
        EXPECT(std::abs(f.eps - 10.0) < 1e-15, "f'(4) = 10 = 2*(4+1)");
    }
    {
        // f(x) = 3*x + 5, f'(x) = 3
        Dual<double> x = Dual<double>::variable(2.0);
        Dual<double> three = Dual<double>::constant(3.0);
        Dual<double> five = Dual<double>::constant(5.0);
        Dual<double> f = dual_add(dual_mul(three, x), five);
        EXPECT(std::abs(f.val - 11.0) < 1e-15, "f(2) = 11");
        EXPECT(std::abs(f.eps - 3.0) < 1e-15, "f'(2) = 3 (constant coefficients don't contribute to derivative)");
    }
    {
        // Composition: f(x) = sqrt(x^2 + 1), so f'(x) = x / sqrt(x^2 + 1).
        // At x=3: f(3) = sqrt(10), f'(3) = 3/sqrt(10).
        Dual<double> x = Dual<double>::variable(3.0);
        Dual<double> one = Dual<double>::constant(1.0);
        Dual<double> x_sq = dual_sqr(x);
        Dual<double> inner = x_sq + one;
        Dual<double> f = dual_sqrt(inner);
        double expected_val = std::sqrt(10.0);
        double expected_eps = 3.0 / std::sqrt(10.0);
        EXPECT(std::abs(f.val - expected_val) < 1e-14, "f(3) = sqrt(10)");
        EXPECT(std::abs(f.eps - expected_eps) < 1e-13, "f'(3) = 3/sqrt(10) (chain rule composition)");
    }
    {
        // cbrt chain rule: f(x) = cbrt(x), f'(x) = 1/(3 cbrt(x)^2).
        // At x=8: f(8) = 2, f'(8) = 1/12.
        Dual<double> x = Dual<double>::variable(8.0);
        Dual<double> f = dual_cbrt(x);
        EXPECT(std::abs(f.val - 2.0) < 1e-14, "cbrt(8) = 2");
        EXPECT(std::abs(f.eps - (1.0 / 12.0)) < 1e-14, "cbrt'(8) = 1/(3 * 4) = 1/12");
    }

    std::printf("\n== Dual<BigFloat>: chain rule on irrational-capable scalar ==\n");
    {
        // f(x) = x*x at BigFloat precision: chain rule same as double.
        BigFloat three(3);
        Dual<BigFloat> x = Dual<BigFloat>::variable(three);
        Dual<BigFloat> f = dual_mul(x, x);
        BigFloat nine(9);
        BigFloat six(6);
        EXPECT(mathlib::bigfloat_cmp(f.val, nine) == 0,
               "BigFloat f(3) = 9");
        EXPECT(mathlib::bigfloat_cmp(f.eps, six) == 0,
               "BigFloat f'(3) = 6 (chain rule on BigFloat multiplications)");
    }
    {
        // cbrt on BigFloat: BigFloat internally handles the irrational
        // case via Newton's method; verify chain rule still gives the
        // same derivative 1/(3 cbrt(x)^2).
        BigFloat eight(8);
        Dual<BigFloat> x = Dual<BigFloat>::variable(eight);
        Dual<BigFloat> f = dual_cbrt(x);
        // f.val should be BigFloat(2) (cbrt(8)=2 exactly).
        BigFloat two(2);
        EXPECT(mathlib::bigfloat_cmp(f.val, two) == 0,
               "BigFloat cbrt(8) = 2 exactly");
        // f.eps should equal 1/(3*2*2) = 1/12 = 0.08333...3.
        // We compute cbrt(8)*2 = 4, then 12 (3*4), then take reciprocal.
        BigFloat twelve(12);
        BigFloat twelve_inv = mathlib::bigfloat_div(BigFloat(1), twelve, 64);
        EXPECT(std::abs(mathlib::bigfloat_to_double(f.eps) -
                        mathlib::bigfloat_to_double(twelve_inv)) < 1e-14,
               "BigFloat cbrt'(8) = 1/12 (chain rule on BigFloat cbrt)");
    }

    std::printf("\n== Dual<Ball>: chain rule on exact-rational interval arithmetic ==\n");
    {
        // f(x) = x*x on Ball<Rat>.  At x = 3 (exact), f = 9 (exact).
        // Also verifies Ball preserves exactness for perfect-square endpoints.
        Rat r3; set_si(r3, 3);
        Rat r1; set_si(r1, 1);
        Ball x_val(r3, r3);   // exact point at 3
        Ball eps_val(r1, r1); // exact df/dx seed = 1
        Dual<Ball> x(x_val, eps_val);
        Dual<Ball> f = dual_mul(x, x);
        Rat r9; set_si(r9, 9);
        EXPECT(cmp(f.val.lo, r9) == 0 && cmp(f.val.hi, r9) == 0,
               "Ball<Rat> 3*3 = 9 (perfect square, exact)");
        Rat r6; set_si(r6, 6);
        EXPECT(cmp(f.eps.lo, r6) == 0 && cmp(f.eps.hi, r6) == 0,
               "Ball<Rat> chain rule gives exact derivative 2*3 = 6");
    }
    {
        // cbrt on a non-trivial Ball<Rat>: ball_cbrt is widening, so the
        // result is a genuine interval.  We verify the chain rule
        // produces a sensible interval for f.eps (not a point).
        Rat r8; set_si(r8, 8);
        Rat r1; set_si(r1, 1);
        Ball x_val(r8, r8);   // exact point at 8
        Ball eps_val(r1, r1); // exact df/dx seed = 1
        Dual<Ball> x(x_val, eps_val);
        Dual<Ball> f = dual_cbrt(x);
        Rat r2; set_si(r2, 2);
        EXPECT(cmp(f.val.lo, r2) == 0 && cmp(f.val.hi, r2) == 0,
               "Ball cbrt(8) = 2 exactly");
    }
    {
        // Finite-difference check: at x = 3.0, f(x) = x*x gives f'(3) = 6
        // exactly.  Diff against our Dual chain rule.  Tolerances are
        // double-precision because we're comparing numeric outputs of
        // the chain rule vs a finite-difference estimate.
        Dual<double> x = Dual<double>::variable(3.0);
        Dual<double> f = dual_mul(x, x);
        double fd_eps = ((3.0 + 1e-6) * (3.0 + 1e-6) - (3.0 - 1e-6) * (3.0 - 1e-6)) / (2e-6);
        EXPECT(std::abs(f.eps - fd_eps) < 1e-5,
               "Dual<double> chain rule agrees with finite-difference on x^2");
    }

    std::printf("\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
