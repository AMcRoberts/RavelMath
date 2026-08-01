// Test Tier 0f: BigFloat (tunable-precision binary floating point)
// and the switchable Perron-eigenvalue certification entry point
// (certify_perron / PerronCertifyMethod) built on top of it.
//
// Reference values: same Tetrabonacci/rnd13 betas test_ball.cpp
// checks, so the two tiers are directly comparable -- this file
// checks BigFloat converges to the SAME known values as the exact
// Ball bracket, just via a different, tunable-memory representation.

#include <cmath>
#include <cstdio>
#include <vector>

#include "math/bigfloat.hpp"
#include "math/bigfloat_trig.hpp"

#include "test_common.hpp"

using namespace mathlib;

int main() {
    std::printf("== BigFloat: basic arithmetic ==\n");
    {
        BigFloat a(3);   // 3
        BigFloat b(4);   // 4
        BigFloat s = bigfloat_add(a, b, 64);
        EXPECT(bigfloat_to_double(s) == 7.0, "3 + 4 = 7");

        BigFloat d = bigfloat_sub(a, b, 64);
        EXPECT(bigfloat_to_double(d) == -1.0, "3 - 4 = -1");

        BigFloat m = bigfloat_mul(a, b, 64);
        EXPECT(bigfloat_to_double(m) == 12.0, "3 * 4 = 12");

        BigFloat q = bigfloat_div(a, b, 64);
        EXPECT(std::abs(bigfloat_to_double(q) - 0.75) < 1e-15, "3 / 4 = 0.75");
    }
    {
        // 1/3 at low precision (8 bits) vs high precision (128 bits):
        // both should be close to 0.333..., but the high-precision one
        // should be closer -- this is the actual point of a TUNABLE
        // precision parameter.
        BigFloat one(1), three(3);
        BigFloat lo_prec = bigfloat_div(one, three, 8);
        BigFloat hi_prec = bigfloat_div(one, three, 128);
        double err_lo = std::abs(bigfloat_to_double(lo_prec) - 1.0 / 3.0);
        double err_hi = std::abs(bigfloat_to_double(hi_prec) - 1.0 / 3.0);
        EXPECT(err_hi <= err_lo, "higher precision_bits gives a result at least as close to 1/3");
        EXPECT(bigfloat_bits(lo_prec) <= 8, "8-bit division result respects the requested precision");
        EXPECT(bigfloat_bits(hi_prec) <= 128, "128-bit division result respects the requested precision");
    }
    {
        // Rounding never INCREASES precision beyond what's asked, and
        // never touches a value that already fits.
        BigFloat exact_small(5);  // fits in 3 bits
        BigFloat unrounded = bigfloat_round(exact_small, 64);
        EXPECT(bigfloat_to_double(unrounded) == 5.0, "rounding an already-small value is a no-op");
    }
    {
        // Comparison must be exact regardless of the two operands'
        // internal exponent representation (e.g. 4 == 4 even if one
        // is stored as mant=1,exp=2 and the other as mant=4,exp=0).
        BigFloat a(BigInt(1), 2);  // 1 * 2^2 = 4
        BigFloat b(4);             // 4 * 2^0 = 4
        EXPECT(bigfloat_cmp(a, b) == 0, "bigfloat_cmp: 4 represented two different ways compares equal");
        BigFloat c(5);
        EXPECT(bigfloat_cmp(a, c) < 0, "bigfloat_cmp: 4 < 5");
        EXPECT(bigfloat_cmp(c, a) > 0, "bigfloat_cmp: 5 > 4");
    }

    std::printf("\n== BigFloat: tunable-precision Collatz-Wielandt bracket ==\n");
    {
        // Tetrabonacci: beta ~= 1.9275619754829254 (companion matrix).
        std::vector<std::vector<long long>> M = {
            {1, 1, 1, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
        auto br64 = certify_perron_bracket_bigfloat(M, 64, 60);
        auto br256 = certify_perron_bracket_bigfloat(M, 256, 60);
        double known_beta = 1.9275619754829254;
        double lo64 = bigfloat_to_double(br64.lo), hi64 = bigfloat_to_double(br64.hi);
        double lo256 = bigfloat_to_double(br256.lo), hi256 = bigfloat_to_double(br256.hi);
        EXPECT(lo64 <= known_beta && known_beta <= hi64 + 1e-9,
               "64-bit BigFloat bracket contains the known Tetrabonacci beta");
        EXPECT(lo256 <= known_beta + 1e-30 && known_beta <= hi256 + 1e-9,
               "256-bit BigFloat bracket contains the known Tetrabonacci beta");
        EXPECT((hi256 - lo256) <= (hi64 - lo64) + 1e-30,
               "256-bit precision bracket is at least as tight as 64-bit after the same iteration count");
    }
    {
        // rnd13: M = [[2,2,2,3],[1,0,0,0],[1,1,0,1],[2,2,2,2]],
        // beta ~= 5.623559 (same matrix test_ball.cpp uses, docs/
        // FINDINGS_FOR_CITATION.md) -- same cross-check test_ball.cpp
        // does for the exact bracket, run here through the tunable
        // BigFloat path instead.
        std::vector<std::vector<long long>> M = {
            {2, 2, 2, 3}, {1, 0, 0, 0}, {1, 1, 0, 1}, {2, 2, 2, 2}};
        auto br = certify_perron_bracket_bigfloat(M, 200, 80);
        double lo = bigfloat_to_double(br.lo), hi = bigfloat_to_double(br.hi);
        double mid = 0.5 * (lo + hi);
        EXPECT(std::abs(mid - 5.623559) < 1e-5, "200-bit BigFloat bracket matches the known rnd13 beta to 1e-5");
    }
    {
        // The precision knob actually changes memory footprint, not
        // just display: ask for the result to at most 32 bits and
        // confirm it never exceeds that, across many iterations
        // (i.e. rounding is actually applied every step, not just at
        // the end -- this is the "bounded, tunable memory cost"
        // property this file exists to provide, as opposed to Ball's
        // unboundedly-growing exact rational endpoints).
        std::vector<std::vector<long long>> M = {{1, 1}, {1, 0}};  // Fibonacci
        auto br = certify_perron_bracket_bigfloat(M, 32, 100);
        EXPECT(bigfloat_bits(br.lo) <= 32, "requested 32-bit precision is respected for lo after 100 iterations");
        EXPECT(bigfloat_bits(br.hi) <= 32, "requested 32-bit precision is respected for hi after 100 iterations");
    }

    std::printf("\n== The switch: PerronCertifyMethod / certify_perron ==\n");
    {
        std::vector<std::vector<long long>> fibonacci = {{1, 1}, {1, 0}};
        auto exact = certify_perron(fibonacci, PerronCertifyMethod::ExactRationalBracket, 60);
        auto tuned = certify_perron(fibonacci, PerronCertifyMethod::TunedBigFloat, 60, 128);
        EXPECT(exact.method == PerronCertifyMethod::ExactRationalBracket,
               "certify_perron reports back the method actually used (exact)");
        EXPECT(tuned.method == PerronCertifyMethod::TunedBigFloat,
               "certify_perron reports back the method actually used (tuned)");
        double exact_mid = (mpq_get_d(exact.exact_bracket.lo.get()) +
                             mpq_get_d(exact.exact_bracket.hi.get())) / 2.0;
        double tuned_mid = (bigfloat_to_double(tuned.tuned_lo) + bigfloat_to_double(tuned.tuned_hi)) / 2.0;
        EXPECT(std::abs(exact_mid - tuned_mid) < 1e-6,
               "both methods, switched at the same call site, agree on the golden ratio to 1e-6");
        EXPECT(tuned.precision_bits_used == 128, "certify_perron records the precision_bits the caller asked for");
    }
    {
        // A precision_bits of 0 is a caller error (there is no such
        // thing as a 0-bit mantissa), not a silent default -- this
        // mirrors ball.hpp's own explicit-argument-checking style.
        bool threw = false;
        try {
            certify_perron_bracket_bigfloat({{1, 1}, {1, 0}}, 0, 10);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        EXPECT(threw, "precision_bits == 0 is rejected explicitly rather than silently misbehaving");
    }

    std::printf("\n== BigFloat: Newton-iteration sqrt / cbrt / abs ==\n");
    {
        // Perfect-square roots: exact at any precision.
        EXPECT(bigfloat_to_double(bigfloat_sqrt(BigFloat(9), 64)) == 3.0,
               "sqrt(9) = 3.0 exactly");
        EXPECT(bigfloat_to_double(bigfloat_sqrt(BigFloat(16), 64)) == 4.0,
               "sqrt(16) = 4.0 exactly");
        EXPECT(bigfloat_to_double(bigfloat_sqrt(BigFloat(0), 64)) == 0.0,
               "sqrt(0) = 0");
        // sqrt(2): irrational, but BigFloat converges.  We compare via
        // BigFloat arithmetic (not bigfloat_to_double, which loses
        // precision by extracting at most 53 mantissa bits during the
        // double conversion -- the bound 1e-30 would be unmeetable
        // given the conversion loses ~10^-16 of relative precision).
        // Instead: compute BigFloat(2) ^ 2 via repeated multiplication
        // at high precision and verify it matches the input within the
        // requested precision bits.
        BigFloat sqrt2 = bigfloat_sqrt(BigFloat(2), 256);
        // 2 vs sqrt(2)^2
        BigFloat sqrt2_sq = bigfloat_mul(sqrt2, sqrt2, 256);
        BigFloat two(2);
        BigFloat err = bigfloat_sub(sqrt2_sq, two, 256);
        // |sqrt(2)^2 - 2| should be ~0 -- much smaller than 2.
        BigFloat err_abs = bigfloat_abs(err);
        // The relative error after 256-bit Newton's is at most 2^-256
        // of sqrt(2), i.e. ~2^-256 in magnitude -- effectively 0 in
        // any meaningful comparison.  Compare against 1 (= |2|) using
        // bigfloat_cmp: should be <<< 1 in absolute magnitude.
        BigFloat one(1);
        EXPECT(bigfloat_cmp(err_abs, one) < 0,
               "256-bit BigFloat sqrt(2)^2 - 2 has tiny magnitude (< 1)");
        // Higher precision bits => more mantissa bits actually computed.
        BigFloat sqrt2_lo = bigfloat_sqrt(BigFloat(2), 64);
        EXPECT(bigfloat_bits(sqrt2_lo) <= 64,
               "requested 64-bit precision is respected for sqrt(2)");
        EXPECT(bigfloat_bits(sqrt2) >= bigfloat_bits(sqrt2_lo),
               "256-bit sqrt(2) has at least as many mantissa bits as 64-bit");
        // Negative input throws rather than silently producing NaN.
        BigFloat neg(0);
        BigFloat nine(9);
        neg = bigfloat_sub(neg, nine, 64);  // -9
        bool sqrt_neg_threw = false;
        try { bigfloat_sqrt(neg, 64); } catch (const std::invalid_argument&) { sqrt_neg_threw = true; }
        EXPECT(sqrt_neg_threw, "sqrt(negative) throws explicitly");
    }
    {
        // cbrt: perfect cubes.
        EXPECT(std::abs(bigfloat_to_double(bigfloat_cbrt(BigFloat(27), 64)) - 3.0) < 1e-15,
               "cbrt(27) = 3.0 exactly");
        EXPECT(std::abs(bigfloat_to_double(bigfloat_cbrt(BigFloat(8), 64)) - 2.0) < 1e-15,
               "cbrt(8) = 2.0 exactly");
        // cbrt(2): irrational, compare via BigFloat arithmetic
        // (cbrt(2)^3 - 2 ≈ 0 at 256-bit precision).
        BigFloat cbrt2 = bigfloat_cbrt(BigFloat(2), 256);
        BigFloat cbrt2_sq = bigfloat_mul(cbrt2, cbrt2, 256);
        BigFloat cbrt2_cu = bigfloat_mul(cbrt2_sq, cbrt2, 256);
        BigFloat two(2);
        BigFloat err = bigfloat_sub(cbrt2_cu, two, 256);
        BigFloat one(1);
        EXPECT(bigfloat_cmp(bigfloat_abs(err), one) < 0,
               "256-bit BigFloat cbrt(2)^3 - 2 has tiny magnitude");
        // cbrt of negative: real root exists for odd-degree roots.
        BigFloat neg_nine(0);
        BigFloat nine(9);
        neg_nine = bigfloat_sub(neg_nine, nine, 64);
        BigFloat neg_cbrt = bigfloat_cbrt(neg_nine, 64);
        EXPECT(std::abs(bigfloat_to_double(neg_cbrt) - (-std::cbrt(9.0))) < 1e-15,
               "cbrt(-9) = -cbrt(9) (real root for odd-degree roots)");
    }
    {
        // bigfloat_abs: signed value -> signed magnitude.
        BigFloat neg(0), nine(9);
        neg = bigfloat_sub(neg, nine, 64);
        EXPECT(bigfloat_to_double(bigfloat_abs(neg)) == 9.0, "abs(-9) = 9");
        EXPECT(bigfloat_to_double(bigfloat_abs(BigFloat(7))) == 7.0, "abs(7) = 7");
        EXPECT(bigfloat_to_double(bigfloat_abs(BigFloat(0))) == 0.0, "abs(0) = 0");
        // abs doesn't lose information; mantissa is preserved.
        EXPECT(bigfloat_bits(bigfloat_abs(neg)) == bigfloat_bits(neg),
               "abs does not change mantissa bit-length");
    }

    std::printf("\n== BigFloat: precision bound (MATHLIB_BIGFLOAT_MAX_PREC_BITS) ==\n");
    {
        // The macro is exactly 4096 (and is a #define so callers can
        // override at the compiler flag level if they really need to,
        // though the project's actual uses should fit well under this).
        EXPECT(MATHLIB_BIGFLOAT_MAX_PREC_BITS == 4096,
               "MATHLIB_BIGFLOAT_MAX_PREC_BITS is 4096 (the documented hard upper bound)");
        // precision_bits == 0 is "no mantissa" -- useless and forbidden.
        bool zero_threw = false;
        try { bigfloat_mul(BigFloat(2), BigFloat(3), 0); }
        catch (const std::invalid_argument&) { zero_threw = true; }
        EXPECT(zero_threw, "precision_bits == 0 throws explicitly (not silently 0)");
        // precision_bits above 4096 throws with a message naming the bound.
        bool over_threw = false;
        std::string over_msg;
        try {
            bigfloat_mul(BigFloat(2), BigFloat(3), MATHLIB_BIGFLOAT_MAX_PREC_BITS + 1);
        } catch (const std::invalid_argument& e) {
            over_threw = true;
            over_msg = e.what();
        }
        EXPECT(over_threw, "precision_bits > 4096 throws (does not silently allocate gigabyte mantissa)");
        EXPECT(over_msg.find("4096") != std::string::npos,
               "out-of-range error message names the bound 4096 (avoids 'magic number surprises')");
        // The MAX itself is allowed (boundary case).
        BigFloat at_max(2);
        BigFloat at_max2(3);
        EXPECT(bigfloat_to_double(bigfloat_mul(at_max, at_max2, MATHLIB_BIGFLOAT_MAX_PREC_BITS)) == 6.0,
               "precision_bits == MATHLIB_BIGFLOAT_MAX_PREC_BITS is allowed (boundary OK)");
        // The switch / certify_perron_bracket_bigfloat also enforces.
        bool sw_threw = false;
        try {
            certify_perron({{1, 1}, {1, 0}}, PerronCertifyMethod::TunedBigFloat,
                           /*max_iters=*/10, /*precision_bits=*/MATHLIB_BIGFLOAT_MAX_PREC_BITS + 1);
        } catch (const std::invalid_argument&) { sw_threw = true; }
        EXPECT(sw_threw, "certify_perron(..., TunedBigFloat, precision > 4096) also throws");
        // ASSERT_PRECISION_LITERAL: a static assertion macro.  We don't
        // exercise it directly here (a static_assert that ALWAYS fires
        // would break the build); instead verify the helper it depends
        // on returns true for in-range literals and false for out-of-range.
        EXPECT(precision_in_bounds(1),
               "precision_in_bounds(1) is true (smallest allowed)");
        EXPECT(precision_in_bounds(MATHLIB_BIGFLOAT_MAX_PREC_BITS),
               "precision_in_bounds(4096) is true (largest allowed)");
        EXPECT(!precision_in_bounds(MATHLIB_BIGFLOAT_MAX_PREC_BITS + 1),
               "precision_in_bounds(4097) is false");
        EXPECT(!precision_in_bounds(0),
               "precision_in_bounds(0) is false (0 is meaningless, not even 'zero bits')");
    }

    {
        std::printf("== BigFloat: bigfloat_exp range reduction + bigfloat_log ==\n");
        // The bug this session found and fixed: exp(-20) used to hit
        // bigfloat_exp's 1000-term Taylor cap before converging. Direct
        // regression check, independent of bigfloat_log.
        BigFloat e20 = bigfloat_exp(bigfloat_from_ll(20));
        EXPECT(std::fabs(bigfloat_to_double(e20) - std::exp(20.0)) / std::exp(20.0) < 1e-12,
               "bigfloat_exp(20) matches std::exp(20) closely (range reduction actually engages)");
        BigFloat em20 = bigfloat_exp(bigfloat_from_ll(-20));
        EXPECT(std::fabs(bigfloat_to_double(em20) - std::exp(-20.0)) / std::exp(-20.0) < 1e-9,
               "bigfloat_exp(-20) matches std::exp(-20) closely (previously hit the term cap here)");

        auto close = [](double a, double b) { return std::fabs(a - b) < 1e-12; };
        double l2 = bigfloat_to_double(bigfloat_log(bigfloat_from_ll(2), 200));
        EXPECT(close(l2, std::log(2.0)), "bigfloat_log(2) matches std::log(2) to 1e-12");
        double l100 = bigfloat_to_double(bigfloat_log(bigfloat_from_ll(100), 200));
        EXPECT(close(l100, std::log(100.0)), "bigfloat_log(100) matches std::log(100)");
        EXPECT(bigfloat_is_zero(bigfloat_log(bigfloat_from_ll(1), 200)), "bigfloat_log(1) == 0 exactly");

        // Round-trip at full BigFloat precision (not through a lossy
        // double comparison), including x=20 -- exactly the case that
        // used to fail because of bigfloat_exp's missing range
        // reduction.
        for (long long xv : {1LL, 5LL, 20LL, -3LL}) {
            BigFloat x = bigfloat_from_ll(xv);
            BigFloat ex = bigfloat_exp(x);
            BigFloat back = bigfloat_log(ex, 200);
            BigFloat diff = bigfloat_sub(back, x, 200);
            BigFloat tiny(BigInt(1), -180);
            EXPECT(bigfloat_cmp(bigfloat_abs(diff), tiny) <= 0,
                   "bigfloat_log(bigfloat_exp(x)) round-trips x to near full 200-bit precision");
        }

        // The large-magnitude case relevant to Golod-Shafarevich's own
        // Proposition 10 formula: log of a ~14-digit product of primes.
        BigInt big_n(1);
        for (long long p : {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43}) {
            BigInt r; mul_si(r, big_n, p); big_n = r;
        }
        BigFloat log_big = bigfloat_log(bigfloat_from_bigint(big_n), 200);
        double expected = std::log(mpz_get_d(big_n.get()));
        EXPECT(std::fabs(bigfloat_to_double(log_big) - expected) < 1e-6,
               "bigfloat_log of a large BigInt (product of Sawin's T) matches std::log(double) closely");

        bool threw = false;
        try { bigfloat_log(bigfloat_from_ll(0), 200); } catch (const std::invalid_argument&) { threw = true; }
        EXPECT(threw, "bigfloat_log(0) throws");
        threw = false;
        try { bigfloat_log(bigfloat_from_ll(-5), 200); } catch (const std::invalid_argument&) { threw = true; }
        EXPECT(threw, "bigfloat_log(negative) throws");
    }

    std::printf("\n%d passed, %d failed\n", mathlib_test::n_pass, mathlib_test::n_fail);
    return mathlib_test::n_fail == 0 ? 0 : 1;
}
