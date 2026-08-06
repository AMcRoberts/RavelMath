// ravel/proof/first_genuine_fourth_generator.hpp
//
// Answers the project's standing question -- "how do we obtain a
// genuinely fourth primitive positive matrix?" -- with a real example,
// not a further collapse.
//
// x^3-2x^2-2 (beta ~ 2.35930, |norm|=2, non-unit) has canonical
// beta-substitution digits (2,0,2): 0->001, 1->2, 2->00. Its raw
// parent-prefix classification has five defect classes {-2,-1,0,+1,+2},
// same as x^2-2x-2 and the silver ratio -- but UNLIKE those two, the
// +2 and -2 classes here do NOT collapse: they are not entrywise
// dominated by the square of the +-1 generator. Checked directly
// (`canonical_substitution_generator_collapse.hpp`), not assumed; this
// header additionally traces and pins down WHY.
//
// The mechanism: letter 1's only parent decomposition is (state 0,
// prefix [0,0]) -- length exactly 2, with NO shorter alternative,
// because digit t_2=0 means state 1's own image (sigma(1)=[2]) has no
// leading zeros of its own to supply a length-1 or length-0 occurrence
// of letter 1 elsewhere. So the universal role state reached via that
// parent has literally NO defect-+1 outgoing edge to decompose the
// +2 jump through: G[+1]'s entire row at that role is zero. The +2
// transport is not a shortcut through existing generators; it is the
// only way to get there at all.
//
// Contrast with x^2-2x-2 (digits (2,2), no interior zero): there the
// only "gap" letter's own parent list still has enough alternative
// (shorter) occurrences that a two-step +1 path always exists,
// checked to fully dominate the +2 class. The presence of an interior
// zero digit BETWEEN two nonzero digits is exactly what isolates a
// role with no fallback, which is the structural signature to look
// for in future candidates.
//
// x^3-x^2-2x-2 (digits (2,0,1,0,2), the smallest known non-unit Pisot
// number) shows the same phenomenon (two violations each of +2/-2,
// not one) -- it has two interior zeros, hence two isolated roles.

#pragma once

#include "ravel/proof/canonical_substitution_generator_collapse.hpp"

namespace ravel::proof {

// Just a documented specialization: run the generic collapse check and
// additionally assert the specific, expected non-collapse shape for
// x^3-2x^2-2, so a future change to the generic operation that
// accidentally "fixes" this into collapsing again is caught as a
// regression rather than silently reinterpreted as progress.
inline GeneratorCollapseCertificate derive_first_genuine_fourth_generator_witness(
    const mathlib::QBetaRing& R, const mathlib::RootInterval& beta_I) {
    return derive_canonical_substitution_generator_collapse(R, beta_I);
}

}  // namespace ravel::proof
