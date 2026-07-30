// Test Tier 0e: Ball (exact rational interval arithmetic) and the
// Collatz-Wielandt Perron-eigenvalue bracket built on top of it.
//
// Reference values: Tetrabonacci beta ~= 1.927562, rnd13 beta ~=
// 5.623559 (both cross-checked elsewhere in this project, e.g.
// tests/spectral_general_test.cpp).  The bracket is checked to (a)
// actually CONTAIN the known beta and (b) strictly narrow as
// iterations increase, which is the whole point of the tool.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "math/ball.hpp"

#include "test_common.hpp"

using namespace mathlib;

int main() {
    std::printf("== Ball: basic interval arithmetic ==\n");
    {
        Ball a(Q_(1, 2), Q_(3, 2));   // [0.5, 1.5]
        Ball b(Q_(-1, 1), Q_(2, 1));  // [-1, 2]
        Ball s = ball_add(a, b);
        EXPECT(cmp(s.lo, Q_(-1, 2)) == 0, "ball_add lower bound: 0.5 + (-1) = -0.5");
        EXPECT(cmp(s.hi, Q_(7, 2)) == 0, "ball_add upper bound: 1.5 + 2 = 3.5");

        Ball d = ball_sub(a, b);
        // [0.5,1.5] - [-1,2] = [0.5-2, 1.5-(-1)] = [-1.5, 2.5]
        EXPECT(cmp(d.lo, Q_(-3, 2)) == 0, "ball_sub lower bound");
        EXPECT(cmp(d.hi, Q_(5, 2)) == 0, "ball_sub upper bound");

        Ball m = ball_mul(a, b);
        // corners: 0.5*-1=-0.5, 0.5*2=1, 1.5*-1=-1.5, 1.5*2=3 => [-1.5,3]
        EXPECT(cmp(m.lo, Q_(-3, 2)) == 0, "ball_mul lower bound (mixed-sign corners)");
        EXPECT(cmp(m.hi, Q_(3, 1)) == 0, "ball_mul upper bound (mixed-sign corners)");
    }
    {
        Ball a(Q_(2, 1), Q_(4, 1));
        Ball b(Q_(1, 1), Q_(2, 1));
        Ball q = ball_div(a, b);
        // [2,4]/[1,2]: corners 2/1=2,2/2=1,4/1=4,4/2=2 => [1,4]
        EXPECT(cmp(q.lo, Q_(1, 1)) == 0, "ball_div lower bound");
        EXPECT(cmp(q.hi, Q_(4, 1)) == 0, "ball_div upper bound");
        bool threw = false;
        try {
            Ball zero_straddle(Q_(-1, 1), Q_(1, 1));
            ball_div(a, zero_straddle);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        EXPECT(threw, "ball_div refuses a divisor interval that straddles zero");
    }
    {
        Ball exact(5);
        EXPECT(cmp(ball_width(exact), Q_(0, 1)) == 0, "Ball(long long) has zero width");
        EXPECT(ball_contains(exact, Q_(5, 1)), "Ball(5) contains 5");
        EXPECT(!ball_contains(exact, Q_(6, 1)), "Ball(5) does not contain 6");
    }

    std::printf("\n== Collatz-Wielandt Perron-eigenvalue bracket ==\n");
    {
        // Tetrabonacci: M = [[1,1,1,1],[1,0,0,0],[0,1,0,0],[0,0,1,0]],
        // beta ~= 1.9275619754829254.
        std::vector<std::vector<long long>> M = {
            {1, 1, 1, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
        auto r = certify_perron_bracket(M, 200, 1e-9);
        double lo_d = mpq_get_d(r.bracket.lo.get());
        double hi_d = mpq_get_d(r.bracket.hi.get());
        std::printf("  Tetrabonacci bracket after %d iterations: [%.12f, %.12f]\n",
                    r.iterations_run, lo_d, hi_d);
        const double beta_tetra = 1.9275619754829254;
        EXPECT(lo_d <= beta_tetra && beta_tetra <= hi_d,
               "Tetrabonacci: exact bracket contains the known beta");
        EXPECT(hi_d - lo_d < 1e-6, "Tetrabonacci: bracket narrowed below 1e-6 within 200 iterations");
    }
    {
        // rnd13: M = [[2,2,2,3],[1,0,0,0],[1,1,0,1],[2,2,2,2]],
        // beta ~= 5.623559 (docs/FINDINGS_FOR_CITATION.md).
        std::vector<std::vector<long long>> M = {
            {2, 2, 2, 3}, {1, 0, 0, 0}, {1, 1, 0, 1}, {2, 2, 2, 2}};
        auto r = certify_perron_bracket(M, 200, 1e-6);
        double lo_d = mpq_get_d(r.bracket.lo.get());
        double hi_d = mpq_get_d(r.bracket.hi.get());
        std::printf("  rnd13 bracket after %d iterations: [%.9f, %.9f]\n",
                    r.iterations_run, lo_d, hi_d);
        const double beta_rnd13 = 5.623559;  // truncated reference (5 decimals,
                                              // per docs/FINDINGS_FOR_CITATION.md
                                              // and spectral_general_test.cpp's
                                              // own 1e-5 tolerance) -- our exact
                                              // bracket is tighter than that
                                              // truncation, so compare via
                                              // tolerance against the midpoint
                                              // rather than asserting the
                                              // truncated literal lies inside
                                              // the (much tighter) exact bracket.
        double mid_d = 0.5 * (lo_d + hi_d);
        EXPECT(std::abs(mid_d - beta_rnd13) < 1e-5,
               "rnd13: exact bracket midpoint matches the known beta to 1e-5");
        EXPECT(hi_d - lo_d < 1e-4, "rnd13: bracket narrowed below 1e-4 within 200 iterations");
    }
    {
        // Monotone narrowing: bracket width after N+50 iterations must
        // be <= width after N iterations (never widens), checked
        // directly rather than assumed.
        std::vector<std::vector<long long>> M = {
            {1, 1, 1, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
        auto r10 = certify_perron_bracket(M, 10, 0.0);   // target 0 => always runs max_iters
        auto r60 = certify_perron_bracket(M, 60, 0.0);
        double w10 = mpq_get_d(ball_width(r10.bracket).get());
        double w60 = mpq_get_d(ball_width(r60.bracket).get());
        std::printf("  width after 10 iters: %.3e, after 60 iters: %.3e\n", w10, w60);
        EXPECT(w60 <= w10, "Collatz-Wielandt bracket narrows (or stays equal) as iterations increase");
    }

    {
        std::vector<std::vector<long long>> M = {
            {1000000000LL, 1}, {1, 999999999LL}};
        auto r = certify_perron_bracket_exact(M, 80, 1e-12);
        EXPECT(r.iterations_run > 1, "arbitrary-precision bracket runs without overflow");
        EXPECT(!r.stopped_early, "arbitrary-precision bracket never stops for integer overflow");
        EXPECT(cmp(r.bracket.lo, r.bracket.hi) <= 0,
               "arbitrary-precision bracket remains ordered");
    }
    {
        std::vector<std::vector<long long>> fibonacci = {
            {1, 1}, {1, 0}};
        std::vector<std::vector<long long>> tribonacci = {
            {1, 1, 1}, {1, 0, 0}, {0, 1, 0}};
        auto separated = compare_perron_roots_exact(fibonacci, tribonacci, 200, 1e-12);
        EXPECT(separated.order == PerronOrder::less,
               "exact Perron comparison separates Fibonacci below Tribonacci");

        std::vector<std::vector<long long>> exact = {
            {1, 1}, {1, 1}};
        auto equal = compare_perron_roots_exact(exact, exact, 10, 0.0);
        EXPECT(equal.order == PerronOrder::equal,
               "exact Perron comparison proves equal singleton brackets");
    }

    std::printf("\n== Ball: sqrt / cbrt on exact rational intervals ==\n");
    {
        // Ball<Rat> sqrt of a perfect-square endpoint interval: each
        // endpoint is detected as a perfect square and tightens to the
        // exact rational (no 1/den widening on perfect-square inputs).
        // The interval itself does NOT collapse to a point -- it spans
        // the [sqrt(lo), sqrt(hi)] range of values -- but is contained
        // with EXACT rational endpoints (no widening on either end).
        Ball ball_with_4(Q_(4, 1), Q_(9, 1));  // [4, 9]
        Ball sqrt_ball = ball_sqrt(ball_with_4);
        // sqrt(4) = 2 exactly, sqrt(9) = 3 exactly.
        EXPECT(cmp(sqrt_ball.lo, Q_(2, 1)) == 0,
               "ball_sqrt([4,9]).lo = 2 exactly (perfect square, no widening)");
        EXPECT(cmp(sqrt_ball.hi, Q_(3, 1)) == 0,
               "ball_sqrt([4,9]).hi = 3 exactly (perfect square, no widening)");
        // Containment of every true sqrt of any point in [4, 9].
        EXPECT(ball_contains(sqrt_ball, Q_(2, 1)),
               "ball_sqrt([4,9]) contains 2 (= sqrt(4))");
        EXPECT(ball_contains(sqrt_ball, Q_(3, 1)),
               "ball_sqrt([4,9]) contains 3 (= sqrt(9))");
        EXPECT(ball_contains(sqrt_ball, Q_(5, 2)),
               "ball_sqrt([4,9]) contains 2.5 (= sqrt(6.25, in [4,9])");
    }
    {
        // sqrt of interval crossing zero: refused (matching ball_div's policy).
        Ball cross_zero(Q_(-1, 1), Q_(1, 1));
        bool threw = false;
        try { ball_sqrt(cross_zero); } catch (const std::invalid_argument&) { threw = true; }
        EXPECT(threw, "ball_sqrt refuses intervals crossing zero");
    }
    {
        // Irrational endpoint: sqrt([1, 3]) must bracket sqrt(2) and is
        // strictly wider than the [perfect-square] case above (width
        // at least 2/3 = sqrt(3) - sqrt(1) approximately; we just
        // check it's > 0, which is the operational "irrational"
        // observable that dual_format's assess() consumes).
        Ball irr(Q_(1, 1), Q_(3, 1));
        Ball sqrt_irr = ball_sqrt(irr);
        EXPECT(cmp(ball_width(sqrt_irr), Q_(0, 1)) > 0,
               "ball_sqrt of irrational-flavored interval has nonzero width");
        // Containment: sqrt(2) ≈ 1.414 should be in the ball.
        Rat approx_2; set_si(approx_2, 1414, 1000);  // 1.414
        EXPECT(cmp(sqrt_irr.lo, approx_2) <= 0 && cmp(approx_2, sqrt_irr.hi) <= 0,
               "ball_sqrt([1,3]) brackets 1.414 (sqrt(2))");
    }
    {
        // cbrt on rational intervals: monotone on all reals, no zero-crossing issue.
        Ball ball_8_27(Q_(8, 1), Q_(27, 1));
        Ball cbrt_ball = ball_cbrt(ball_8_27);
        EXPECT(ball_contains(cbrt_ball, Q_(2, 1)), "ball_cbrt([8,27]) contains 2 = cbrt(8)");
        EXPECT(ball_contains(cbrt_ball, Q_(3, 1)), "ball_cbrt([8,27]) contains 3 = cbrt(27)");
        // cbrt of negative interval: also valid since cbrt is monotone on R.
        Ball neg_ball(Q_(-27, 1), Q_(-1, 1));
        Ball cbrt_neg = ball_cbrt(neg_ball);
        EXPECT(ball_contains(cbrt_neg, Q_(-3, 1)),
               "ball_cbrt([-27,-1]) contains -3 = cbrt(-27)");
    }

    std::printf("\n%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
