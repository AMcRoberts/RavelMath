/-
  class_ii_round234_shape_closure.lean

  Author: Ravel.
  Date:   2026-07-31.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Formalizes the arithmetic core shared by every closed-form
          argument in this session's Rounds 2/3/4 shape-classification
          proof (hybrid category's window monotonicity, both-fixed
          category's "no slope-nonzero edge survives for a>=7"
          closed-form proof): an affine integer function with nonzero
          slope takes any fixed target value at most once, so if its
          unique solution is below a threshold, it never re-hits that
          target above the threshold. Also formalizes the exhaustive
          occurrence-type slope enumeration (5 types, slope in {0,1})
          that both closed-form arguments build on.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  Scope, stated precisely: this file formalizes the ARITHMETIC core
  that both closed-form proofs (app/class_ii_hybrid_window_slope_
  derivation.cpp, app/class_ii_both_fixed_full_proof.cpp) rely on. It
  does NOT re-derive, inside Lean, that the specific 5 occurrence
  types are exhaustive over tau_a's word structure (that combinatorial
  fact rests on parent_decompositions applied to tau_a's fixed images
  sigma(0)=0^a 1 2, sigma(1)=0^(a-1) 2 0, sigma(2)=0, established and
  checked in the C++ files cited above), nor does it re-derive which
  specific (node, shape, target) triples arise in Rounds 2/3/4's
  pre_red catalogues (an a-independent but large finite set, computed
  via the corona/backward_closure/red_anode C++ pipeline, not ported
  to Lean). What Lean adds here is a kernel-checked proof of the two
  general facts that make the whole "solve for the required a, show
  it's below the threshold" strategy valid in the first place -- the
  same scope convention class_ii_round1_red_pruning.lean already uses
  for Round 1's arithmetic core.
-/

import Mathlib.Tactic

/-- The core arithmetic fact behind every closed-form argument in this
session's Round 2/3/4 work: an affine integer function with nonzero
slope can equal a fixed target for at most one input. If that unique
solution lies below a threshold, the function never equals the target
again at or above the threshold. This is exactly what turns "the
unique required `a` is `< 7`" (checked directly, in closed form, for
every one of 410 slope-nonzero cases in
`app/class_ii_both_fixed_full_proof.cpp`) into "this edge is never
valid for any integer `a >= 7`", without inspecting each such `a`
individually. -/
theorem affine_no_solution_at_or_above_threshold
    (const slope target a0 threshold : ℤ)
    (hslope : slope ≠ 0)
    (ha0 : const + a0 * slope = target)
    (hbelow : a0 < threshold) :
    ∀ a : ℤ, threshold ≤ a → const + a * slope ≠ target := by
  intro a ha hcontra
  have heq : a0 * slope = a * slope := by linarith [ha0, hcontra]
  have : a0 = a := by
    have := mul_right_cancel₀ hslope heq
    linarith [this]
  omega

/-- The exhaustively-enumerated occurrence types that give a FIXED
(non-ranging) occurrence length in `tau_a`'s images, reproduced
exactly from `app/class_ii_both_fixed_full_proof.cpp`'s
`fixed_types()`: `(inner letter, parent letter, l(p)[1], l(p)[2],
slope, intercept)`, where the occurrence length as a function of `a`
is `slope * a + intercept`. -/
inductive FixedOccurrenceKind
  | trivialSigma2        -- inner=0, parent=2: sigma(2)="0", len=0
  | sigma0Letter1        -- inner=1, parent=0: the single '1', len=a
  | sigma0Letter2        -- inner=2, parent=0: the single '2', len=a+1
  | sigma1Letter2        -- inner=2, parent=1: the single '2', len=a-1
  | sigma1AfterMarker    -- inner=0, parent=1, past the '2': len=a
  deriving DecidableEq, Fintype

/-- Each fixed occurrence type's `(slope, intercept)` pair, matching
`app/class_ii_both_fixed_full_proof.cpp`'s `fixed_types()` table
exactly. -/
def occurrenceSlopeIntercept : FixedOccurrenceKind → ℤ × ℤ
  | .trivialSigma2     => (0, 0)
  | .sigma0Letter1     => (1, 0)
  | .sigma0Letter2     => (1, 1)
  | .sigma1Letter2     => (1, -1)
  | .sigma1AfterMarker => (1, 0)

/-- The exhaustive slope enumeration: every fixed occurrence type has
slope exactly `0` or exactly `1`, no other value -- checked
computationally in `app/class_ii_hybrid_window_slope_derivation.cpp`
(against every occurrence type that actually arises, not just these
five by assumption); this is the finite catalogue-level fact those
checks establish, now kernel-checked directly from the definition
above. -/
theorem occurrenceSlope_is_zero_or_one (k : FixedOccurrenceKind) :
    (occurrenceSlopeIntercept k).1 = 0 ∨ (occurrenceSlopeIntercept k).1 = 1 := by
  cases k <;> simp [occurrenceSlopeIntercept]

/-- There are exactly five fixed occurrence kinds, matching the
exhaustive enumeration in the C++ source. -/
theorem fixedOccurrenceKind_card :
    Fintype.card FixedOccurrenceKind = 5 := by native_decide

/-- Combining the two facts above: for any pair of fixed occurrence
kinds contributing to a both-fixed edge's `x2'(a) = const + a * [(
slope_q - slope_p) - rhs2]` formula (`app/
class_ii_both_fixed_full_proof.cpp`), the overall slope
`(slope_q - slope_p) - rhs2` is an integer in `{-2,-1,0,1,2}` shifted
by `-rhs2` -- in particular, if this combined slope is nonzero, the
`affine_no_solution_at_or_above_threshold` lemma above applies
directly: the resulting `x2'(a)` can match any fixed target for at
most one integer `a`. -/
theorem both_fixed_slope_bounded (p q : FixedOccurrenceKind) :
    let sp := (occurrenceSlopeIntercept p).1
    let sq := (occurrenceSlopeIntercept q).1
    sq - sp = -1 ∨ sq - sp = 0 ∨ sq - sp = 1 := by
  have hp := occurrenceSlope_is_zero_or_one p
  have hq := occurrenceSlope_is_zero_or_one q
  simp only
  rcases hp with hp | hp <;> rcases hq with hq | hq <;> omega
