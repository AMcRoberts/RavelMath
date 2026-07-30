// tests/dual_format_test.cpp
//
// Tests for include/ravel/dual_format.hpp -- the rational-assessment
// + dual BigFloat/Ball<Rat> view plumbing used by smooth-relaxation.
//
// The assess(bf, ball) function reports:
//   - exactly_rational: ball has zero width (degenerate to a point)
//   - consistent_rational_digits: ~log2(width^-1), how many rational digits
//     the ball is consistent with
//   - appears_irrational: width is too large to be plausibly rational
//   - description: human-readable summary
//
// BigFloat is the "converges on irrationals" representation; Ball<Rat>
// is the exact/certified representation.  At output time we pair them
// and ask: "what do these two views, taken together, say about whether
// the value is rational?" -- per the project's smooth-relaxation policy
// (DEVELOPER_POLICY for this work item) we always do both views.
//
// Test ladder:
//   1. Rational inputs: ball collapses, exactly_rational = true
//   2. Irrational sqrt: ball stays wide, appears_irrational = true
//   3. Higher-precision BigFloat converges into the same Ball<Rat>
//   4. BigFloat and Ball endpoints agree to double precision (sanity)

#include <cmath>
#include <cstdio>

#include "math/bigint.hpp"
#include "math/ball.hpp"
#include "math/bigfloat.hpp"

#include "ravel/dual_format.hpp"

using namespace mathlib;

static int n_pass = 0, n_fail = 0;

#define EXPECT(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; std::fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

int main() {
    std::printf("== Rational assessment: degenerate (= rational) cases ==\n");
    {
        // 3/4 = 0.75 is rational.  The Ball<Rat> collapses to a single
        // point [3/4, 3/4]; assess() should report exactly_rational.
        BigFloat bf = bigfloat_div(BigFloat(3), BigFloat(4), 64);
        Rat r; set_si(r, 3, 4);
        Ball ball(r, r);
        auto asmt = ravel::dual_format::RationalAssessment::assess(bf, ball);
        EXPECT(asmt.exactly_rational,
               "ball [3/4, 3/4] degenerates: exactly rational");
        EXPECT(!asmt.appears_irrational,
               "degenerate ball is not 'appears irrational'");
        EXPECT(asmt.description.find("EXACTLY RATIONAL") != std::string::npos,
               "description flags the exactly-rational case by name");
    }
    {
        // 27 (an integer) is exactly rational.  BigFloat constructor with
        // long long always produces an exact value, so its ball-side
        // companion is also a point.
        BigFloat bf(27);
        Rat r; set_si(r, 27);
        Ball ball(r, r);
        auto asmt = ravel::dual_format::RationalAssessment::assess(bf, ball);
        EXPECT(asmt.exactly_rational, "ball [27, 27] degenerates: exactly rational");
    }

    std::printf("\n== Rational assessment: irrational cases (Ball<Rat> widens) ==\n");
    {
        // sqrt(2) via BigFloat (convergent irrational) + a Ball<Rat>
        // that strictly brackets 1.4-1.5: the ball has positive width
        // because sqrt(2) is irrational, and assess() should NOT call
        // it "exactly rational".
        BigFloat sqrt2_bf = bigfloat_sqrt(BigFloat(2), 128);
        Rat lo_irr, hi_irr;
        set_si(lo_irr, 7, 5);   // 1.4
        set_si(hi_irr, 3, 2);   // 1.5
        Ball ball(lo_irr, hi_irr);
        auto asmt = ravel::dual_format::RationalAssessment::assess(sqrt2_bf, ball);
        EXPECT(!asmt.exactly_rational,
               "Ball<Rat> with nonzero width is NOT exactly rational");
        // consistent rational digits: log2(width^-1) where width = 0.1,
        // gives log2(10) ~ 3.32 -> consistent with at most 3 digits.
        EXPECT(asmt.consistent_rational_digits <= 5,
               "sqrt(2) in a 0.1-wide ball is consistent with at most ~3-4 rational digits");
        EXPECT(!asmt.appears_irrational,
               "sqrt(2) in a 0.1-wide ball: ~3 rational digits is reasonable (not 'irrational')");
    }
    {
        // sqrt(2) in a [1, 5] ball: that's a 4-wide ball around midpoint
        // 3 -- ratio = 4/3 ~ 1.33, log2(1/(4/3)) = -log2(4/3) ~ -0.4.
        // So consistent rational digits is ~0 (i.e. consistent with
        // essentially any rational at that scale).
        BigFloat sqrt2_bf = bigfloat_sqrt(BigFloat(2), 64);
        Rat w_lo, w_hi; set_si(w_lo, 1); set_si(w_hi, 5);
        Ball wide_ball(w_lo, w_hi);
        auto asmt = ravel::dual_format::RationalAssessment::assess(sqrt2_bf, wide_ball);
        EXPECT(!asmt.exactly_rational, "wide ball is not exactly rational");
        EXPECT(asmt.consistent_rational_digits <= 1,
               "ball of width 4 around midpoint 3 is consistent with ~0-1 rational digits");
        EXPECT(asmt.appears_irrational,
               "wide ball: assess() should report 'appears irrational' (digits <= 2)");
    }

    std::printf("\n== Round-trip agreement: BigFloat ↔ Ball<Rat> ==\n");
    {
        // Convert BigFloat to double, convert Ball endpoints to double,
        // verify BigFloat lies inside the ball.  Limited by double
        // precision (the operands are 64-bit BigFloat and exact Rat).
        BigFloat three_quarter = bigfloat_div(BigFloat(3), BigFloat(4), 64);
        Rat lo; set_si(lo, 7, 10);   // 0.7 -- below 0.75
        Rat hi; set_si(hi, 8, 10);   // 0.8 -- above 0.75
        Ball ball(lo, hi);
        double bf_d = bigfloat_to_double(three_quarter);
        double lo_d = mpq_get_d(lo.get());
        double hi_d = mpq_get_d(hi.get());
        EXPECT(lo_d <= bf_d && bf_d <= hi_d,
               "BigFloat(0.75) value (via bigfloat_to_double) lies inside Ball<Rat> [0.7, 0.8]");
    }
    {
        // A higher-precision BigFloat @ 256 bits, post-rounded, lands
        // at the same value as BigFloat @ 64 bits rounded.  This is
        // a sanity check that the precision knob only widens the
        // mantissa, doesn't perturb the result.
        BigFloat lo_prec = bigfloat_div(BigFloat(1), BigFloat(3), 64);
        BigFloat hi_prec = bigfloat_div(BigFloat(1), BigFloat(3), 256);
        EXPECT(bigfloat_to_double(lo_prec) == bigfloat_to_double(hi_prec),
               "1/3 at 64 bits and 256 bits, both converted to double, agree exactly (53-bit double precision is the bottleneck, not BigFloat's mantissa)");
    }

    std::printf("\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
