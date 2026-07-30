// include/ravel/dual_format.hpp
//
// Dual-format utilities for the smooth-relaxation search.
//
// Per the project's SMOOTH_RELAXATION_POLICY (DEVELOPER_POLICY for this
// work item):
//   "use multiple types of number representation to see both views, that
//    way you can differentiate at runtime or post-output whether some
//    component is rational or not"
//   "when using IBA to also include a different representation that
//    converges on irrationals, in case that's what you find you're
//    dealing with"
//
// Concretely: when an autodiff computation produces a BigFloat value
// (convergent on irrationals), we ALSO compute the same expression in
// Ball<Rat> (exact-certified rational interval), then call
// RationalAssessment::assess() at output time to report:
//
//   - "EXACTLY RATIONAL" iff the Ball<Rat> collapses to a single point
//     (width = 0) -- proven by exact rational comparison
//   - "APPEARS RATIONAL AT N DIGITS" iff width/center <= 2^{-N}
//     (the Ball's contents are consistent with a rational at N digits)
//   - "APPEARS IRRATIONAL" iff width is too wide for ANY consistent
//     rational (consistent_rational_digits <= 2)
//
// The dual-view approach surfaces rational-vs-irrational nature as
// observable information at runtime, rather than silently letting
// "the number is irrational but the doubled precision isn't enough to
// tell" hide behind tolerance comparisons.  Several pieces of project
// machinery (Pisot-conjecture machinery, scale-invariant check
// pipelines) already have this requirement latent; this header turns
// it into an explicit, tested utility.
//
// =====================================================================
// Why this is paired with the smooth-relaxation search specifically
// =====================================================================
// smooth-relaxation (RESEARCH_VECTORS.md vector 2) treats the matrix
// entries as continuous variables and uses gradient information to
// walk matrix space toward regions of "good Pisot-quality" candidates.
// The spectral invariants depend on cbrt (Cardano), which can converge
// to irrational roots; we want to know WHEN the cbrt-of-cubic is
// returning rational numbers (those are the structural cases worth
// reporting separately) versus genuinely irrational ones (which need
// the BigFloat autograd path to converge at all).

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

#include "math/bigint.hpp"
#include "math/ball.hpp"
#include "math/bigfloat.hpp"

namespace ravel::dual_format {

// ===================================================================
// Rational assessment
// ===================================================================
//
// `RationalAssessment::assess(bf, ball)` classifies the joint view of
// a BigFloat `bf` and a Ball<Rat> `ball` that, ideally, both describe
// the SAME underlying value (or approximate each other very closely).
// The classification tells the caller whether this value is plausible
// rational at N digits, exactly rational, or appears irrational.
struct RationalAssessment {
    // True iff `ball_width(ball) == 0` -- the Ball collapses to a single
    // rational point.  This is a PROVEN certification at this point
    // (exact rational comparison, zero tolerance).
    bool exactly_rational = false;

    // Estimate of "how many consistent rational digits" the ball is.
    // If the ball has width w and a midpoint c, consistent with a
    // rational at ~-log2(w/c) digits of precision.  Computed via
    // double conversion of the rational width/c ratio.  A non-rational
    // value will see width near c and this number near 0; a tightly
    // bracketed rational will see width 0 (handled by
    // exactly_rational above).
    long consistent_rational_digits = 0;

    // True iff `consistent_rational_digits <= 2`.  The cutoff "2" is
    // deliberate: any ball where width > 25% of midpoint is too wide
    // to be consistent with any meaningful exact rational, and a
    // caller checking "is this rational at all" should err on the side
    // of "irrational, please use high-precision arithmetic".
    bool appears_irrational = false;

    // Human-readable summary; for logging.
    std::string description;

    static RationalAssessment assess(const mathlib::BigFloat& bf, const mathlib::Ball& ball);
};

inline RationalAssessment RationalAssessment::assess(
        const mathlib::BigFloat& bf, const mathlib::Ball& ball) {
    RationalAssessment out;
    (void)bf;  // not used in the assessment directly; BigFloat and Ball
               // are presumed to agree (modulo precision) by construction
               // -- the assessment is purely from the Ball<Rat> side.

    // Width exactly zero => exactly rational (the Ball is a point).
    mathlib::Rat width = mathlib::ball_width(ball);
    if (mathlib::cmp_si(width, 0) == 0) {
        out.exactly_rational = true;
        out.consistent_rational_digits = -1;  // sentinel for "any N"
        out.appears_irrational = false;
        out.description = "EXACTLY RATIONAL: ball degenerates to a single rational point";
        return out;
    }
    out.exactly_rational = false;

    // width/midpoint ratio as a rational; converted to double for an
    // order-of-magnitude estimate of -log2(ratio).
    mathlib::Rat midpoint = mathlib::ball_midpoint(ball);
    mathlib::Rat ratio;
    int midpoint_sgn = mpq_sgn(midpoint.get());
    if (midpoint_sgn == 0) {
        // midpoint is exactly 0 -- width IS the ratio, but log2 of a
        // pure-width assumes scale 1.  We pick 1 as the reference and
        // width is taken as-is.
        ratio = width;
    } else {
        // |width/midpoint|.
        mathlib::Rat abs_width; mathlib::abs_(abs_width, width);
        mathlib::Rat abs_mid; mathlib::abs_(abs_mid, midpoint);
        mathlib::div(ratio, abs_width, abs_mid);
    }
    // Convert to double for the log2 estimate.  For values far from 1
    // this loses accuracy, but the assessment is qualitative
    // ("consistent with N digits") and the error here is O(log2 of
    // the double-precision epsilon) which doesn't change the verdict.
    double r_d = mpq_get_d(ratio.get());
    if (r_d <= 0.0) {
        // Width zero or some other impossible case: should have been
        // caught by the exactly_rational branch above.  Defensive.
        out.consistent_rational_digits = 100;
    } else {
        out.consistent_rational_digits = static_cast<long>(-std::log2(r_d));
    }
    out.appears_irrational = (out.consistent_rational_digits <= 2);

    char buf[256];
    std::snprintf(buf, sizeof(buf),
                  "ball width/midpoint ~ %.3g ~ 2^{-%ld}; consistent with %s",
                  r_d, out.consistent_rational_digits,
                  out.appears_irrational ? "irrational (no tight rational)"
                                         : "rational at the stated digits");
    out.description = buf;
    return out;
}

// ===================================================================
// Round-trip helpers
// ===================================================================
//
// Convenience: pair a BigFloat with a Ball<Rat> that the caller has
// computed independently (e.g. computing the same Cardano cbrt twice
// in different scalar types) into a "dual view" struct, useful for
// reporting the assess() verdict alongside both underlying values.
struct DualView {
    mathlib::BigFloat bf;       // the convergent-on-irrationals scalar value
    mathlib::Ball ball;         // the exact-certified rational view of the same

    RationalAssessment assessment() const {
        return RationalAssessment::assess(bf, ball);
    }
};

// Convert a Ball<Rat> to a printable string of its midpoint (the
// canonical "the ball represents approximately this rational" reading).
// Useful for logging when the assess() verdict is "irrational" -- in
// that case the midpoint is still a useful number to print.
inline std::string ball_midpoint_str(const mathlib::Ball& ball) {
    mathlib::Rat mid = mathlib::ball_midpoint(ball);
    return mathlib::str(mid);
}

}  // namespace ravel::dual_format
