// tilt.hpp
//
// Hall & Branciard (2020) tilt model: closed-form binary KL divergence
// at the Tsirelson bound and the cos(x-y) correlator for the four
// settings {0, pi/2, pi, 3*pi/2}.
//
// Provenance note: this file is named for its origin in the
// causal-vs-retrocausal simulation cost literature for CHSH
// (Hall & Branciard, Phys. Rev. A 102, 052228, 2020).  The 0.046274
// bits floor and the cos(x-y) correlator are closed-form literature
// values, not Spectre discoveries.  The model is reused here as a
// *correlation diagnostic* on substitution sequences, NOT a claim
// about quantum mechanics in the substitution.  See
// `docs/RESEARCH_STATUS.md` for the original context
// and `thermometer.hpp` for the related Pisot-conjecture use.
//
// The 0.046274 bits floor is a CLOSED-FORM LITERATURE VALUE, not a
// Spectre discovery. Source of truth:
//
//     Hall, M. J. W. & Branciard, C.
//     "Quantitative trade-off between locality and freedom of choice
//      in a hidden-variable model of a Bell scenario"
//     Phys. Rev. A 102, 052228 (2020), Eq. (29):
//
//         IR(S) = 2 - h((4-S)/8) - ((4+S)/8) log2(3)
//
//     At S = 2*sqrt(2)  =>  IR(2*sqrt(2)) = KL_2((2+sqrt(2))/4 || 3/4)
//                          = 0.046273846... bits.
//
// See docs/THEOREM_STATUS.md section 7 and
// docs/THEOREM_STATUS.md "Hall-Branciard values".
//
// PROVENANCE NOTE (do not mistake this for a quantum-mechanics claim):
// the "Tsirelson bound" and "Hall-Branciard KL" helpers below
// (`tsirelson_bound`, `hb_mi_floor`, `tilt_correlator`,
// `tilt_silver_cost_per_angle`) are LITERATURE CONSTANTS evaluated
// from the formulae cited above.  They are exposed here because the
// `thermometer.hpp` four-term correlation statistic shares the
// CHSH algebraic form, and the tilt model gives a clean closed-form
// target to compare the correlation against.  Nothing in this
// header makes any claim that Pisot substitutions violate Bell
// inequalities or exhibit quantum non-locality; the helpers are
// correlation diagnostics borrowed from the Bell-inequality
// literature for their algebraic convenience.  See also
// `thermometer.hpp` for the same caveat on the CHSH-named
// observables.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>

namespace ravel {

inline constexpr std::size_t kTiltSettings = 4;

inline double tsirelson_bound() {
    return 2.0 * std::sqrt(2.0);
}

inline double kl2_bits(double p, double q) {
    if (p <= 0.0 || p >= 1.0) return 0.0;
    if (q <= 0.0 || q >= 1.0) return 0.0;
    return p * std::log2(p / q) + (1.0 - p) * std::log2((1.0 - p) / (1.0 - q));
}

// Hall & Branciard Eq. (29): IR(S) = 2 - h((4-S)/8) - ((4+S)/8) log2(3).
// At S = 2*sqrt(2), this is algebraically equal to
// KL_2((2+sqrt(2))/4 || 3/4). HB and the earlier Friedman-Guth-Hall-
// Kaiser-Gallicchio AR 1809.01307 conjecture the result; HB prove
// it analytically. The local implementation here evaluates the KL
// form, which is the same numeric.
inline double hb_mi_floor() {
    double p = (2.0 + std::sqrt(2.0)) / 4.0;
    double q = 3.0 / 4.0;
    return kl2_bits(p, q);
}

// Numerical constant for tests and provenance cross-checks.
// Hall & Branciard Eq. (29) at S = 2*sqrt(2), reproduced symbolically
// in scripts/01_proof_pipeline.py and isolated here so tests can
// compare without re-deriving.
inline constexpr double kHallBranciardFloor = 0.046273846853407;

inline double tilt_correlator(std::size_t i, std::size_t j) {
    double ai = static_cast<double>(i) * (M_PI / 2.0);
    double aj = static_cast<double>(j) * (M_PI / 2.0);
    return std::cos(ai - aj);
}

// Closed-form per-angle cost for the cos(x-y) tilt model
// at angle theta: KL_2((1 + cos theta)/2 || 3/4).
// Used to compute the 0.0349 bits per-angle mean cited in
// docs/THEOREM_STATUS.md as Open.
inline double tilt_silver_cost_per_angle(double theta) {
    double p = (1.0 + std::cos(theta)) / 2.0;
    double q = 3.0 / 4.0;
    return kl2_bits(p, q);
}

}  // namespace ravel
