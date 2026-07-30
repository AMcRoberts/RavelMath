/-
  class_ii_neighbor2_extensions.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Affine correction geometry for the third fixed-light Class-II neighbor (catalogue-shape theorems only).
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  Affine correction geometry for the third fixed-light Class-II neighbor.

  These are catalogue-shape theorems only. They do not assert contact
  membership, corona propagation, or Red survival.
-/

import Mathlib.Tactic

abbrev NeighborNode := ℤ × ℤ × ℤ × ℤ × ℤ

def neighbor2TerminalSextet (a : ℤ) : Finset NeighborNode :=
  let q := a - 2
  [
    (0, q, -(q+1), 2, 2),
    (0, q, -q, 2, 1),
    (1, -q, q, -2, 0),
    (2, -q, q, -2, 0),
    (2, -q, q+1, -2, 0),
    (2, q, -q, 1, 1)
  ].toFinset

theorem neighbor2TerminalSextet_card (a : ℤ) :
    (neighbor2TerminalSextet a).card = 6 := by
  simp [neighbor2TerminalSextet]

def neighbor2PenultimatePair (a : ℤ) : Finset NeighborNode :=
  [
    (2, -(a-1), a-1, -1, 0),
    (2, -(a-2), a-2, -2, 0)
  ].toFinset

theorem neighbor2PenultimatePair_card (a : ℤ) :
    (neighbor2PenultimatePair a).card = 2 := by
  simp [neighbor2PenultimatePair]

def neighbor2InteriorTip (r : ℤ) : NeighborNode :=
  (2, -r, r, -1, 0)

theorem neighbor2InteriorTip_injective :
    Function.Injective neighbor2InteriorTip := by
  intro r s h
  simp [neighbor2InteriorTip] at h
  exact h

theorem neighbor2InteriorTips_infinite :
    Set.Infinite (Set.range neighbor2InteriorTip) :=
  Set.infinite_range_of_injective neighbor2InteriorTip_injective

/-- The moving correction tip propagates by the fixed center contact
hop `[0,(-1,1,0),0]`; this is the affine core of the relative corona
induction. -/
theorem neighbor2InteriorTip_propagates (r : ℤ) :
    (2, -(r-1)-1, (r-1)+1, -1, 0) = neighbor2InteriorTip r := by
  simp [neighbor2InteriorTip]

/-- The only moving state in an interior neighbor-2 layer lies in the
open signed strip as soon as the center-family window margin is known.
For the Class-II Perron coordinates that margin follows by applying
`class_ii_window_bounds` at `q=r+1`. -/
theorem neighbor2InteriorTip_in_open_strip
    (r b c : ℝ)
    (hr : 0 ≤ r) (hcb : c < b)
    (hgap : r * (b - c) + 1 < c) :
    -b < -r*b + r*c - 1 ∧ -r*b + r*c - 1 < b := by
  constructor <;> nlinarith

inductive Neighbor2FixedKind
  | n00 | n01 | n02 | n03 | n04 | n05 | n06 | n07
  | n08 | n09 | n10 | n11 | n12 | n13 | n14 | n15
  | n16 | n17 | n18 | n19 | n20 | n21 | n22 | n23
  deriving DecidableEq, Fintype

/-- The fixed twenty-four-state part of
`class_ii_neighbor2_fixed_extension_states`. -/
def neighbor2FixedNode : Neighbor2FixedKind → NeighborNode
  | .n00 => (0, -2,  2,  1, 1)
  | .n01 => (0, -1,  0,  1, 0)
  | .n02 => (0, -1,  1,  1, 0)
  | .n03 => (0, -1,  1,  1, 2)
  | .n04 => (0, -1,  2,  1, 0)
  | .n05 => (0,  1, -2, -1, 0)
  | .n06 => (0,  1, -1, -1, 0)
  | .n07 => (0,  1, -1,  0, 2)
  | .n08 => (0,  1,  0, -1, 0)
  | .n09 => (1, -1,  1,  0, 1)
  | .n10 => (1,  0,  1,  0, 0)
  | .n11 => (1,  1, -2, -1, 0)
  | .n12 => (1,  1, -1,  0, 1)
  | .n13 => (1,  2, -2, -1, 0)
  | .n14 => (1,  2, -1, -1, 0)
  | .n15 => (2, -2,  2,  0, 1)
  | .n16 => (2, -1,  1,  0, 0)
  | .n17 => (2, -1,  1,  0, 2)
  | .n18 => (2, -1,  2,  0, 0)
  | .n19 => (2, -1,  2,  0, 1)
  | .n20 => (2,  1, -2, -1, 0)
  | .n21 => (2,  1, -1, -1, 0)
  | .n22 => (2,  1, -1,  0, 2)
  | .n23 => (2,  2, -1, -1, 0)

def neighborNodeHeight (node : NeighborNode) (b c : ℝ) : ℝ :=
  node.2.1 * b + node.2.2.1 * c + node.2.2.2.1

def neighborRightWidth (node : NeighborNode) (b c : ℝ) : ℝ :=
  if node.2.2.2.2 = 0 then b
  else if node.2.2.2.2 = 1 then c else 1

def NeighborInOpenSignedStrip
    (node : NeighborNode) (b c : ℝ) : Prop :=
  -neighborRightWidth node b c < neighborNodeHeight node b c ∧
    neighborNodeHeight node b c < neighborRightWidth node b c

/-- Every fixed correction endpoint satisfies `same_letter_H`'s open
strip inequality from three coarse Class-II Perron-gap bounds. This
turns the twenty-four endpoint checks into one finite kernel-checked
table; deriving the three gap bounds from the cubic is separate. -/
theorem neighbor2FixedNode_in_open_strip
    (kind : Neighbor2FixedKind) (b c : ℝ)
    (hc : 3 < c)
    (hgapLower : 1 / 2 < b - c)
    (hgapUpper : b - c < 1) :
    NeighborInOpenSignedStrip (neighbor2FixedNode kind) b c := by
  cases kind <;>
    constructor <;>
    simp [neighborRightWidth,
      neighborNodeHeight, neighbor2FixedNode] <;>
    norm_num at * <;> linarith

/-- The coarse gap assumptions used by
`neighbor2FixedNode_in_open_strip` follow from the Class-II Perron
relation once the elementary root bound `beta < 2*a` is available. -/
theorem class_ii_neighbor2_perron_gap_bounds
    (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (_hbeta_a : a < beta) (hbeta_2a : beta < 2 * a)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    3 < a + 1 / beta ∧
      1 / 2 < beta - (a + 1 / beta) ∧
      beta - (a + 1 / beta) < 1 := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hgap :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hc : 3 < a + 1 / beta := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    nlinarith
  have hlowerScaled : beta^2 < 2 * (a * beta + 1) := by
    have hproduct : 0 < beta * (2 * a - beta) :=
      mul_pos hbeta (sub_pos.mpr hbeta_2a)
    nlinarith
  have hgapPos : 0 < beta - (a + 1 / beta) := by
    have hab : 0 < a * beta + 1 := by positivity
    nlinarith [sq_pos_of_pos hbeta]
  have hlower : 1 / 2 < beta - (a + 1 / beta) := by
    nlinarith [sq_pos_of_pos hbeta]
  have hidentity :
      beta * (beta - a) = a + 1 + 1 / beta := by
    field_simp [hne]
    nlinarith [hcubic]
  have hab1 : a * beta + 1 < beta^2 := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    nlinarith
  have hupper : beta - (a + 1 / beta) < 1 := by
    nlinarith [sq_pos_of_pos hbeta]
  exact ⟨hc, hlower, hupper⟩

/-- A deliberately coarse but sufficient Class-II Perron-root bound.
It is proved directly from the cubic, without numerical root
isolation. -/
theorem class_ii_neighbor2_perron_lt_two_a
    (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    beta < 2 * a := by
  by_contra hnot
  have htwo : 2 * a ≤ beta := le_of_not_gt hnot
  have hdelta : a ≤ beta - a := by linarith
  have hleft :
      a * beta^2 ≤ beta^2 * (beta - a) :=
    by
      simpa [mul_comm] using
        mul_le_mul_of_nonneg_left hdelta (sq_nonneg beta)
  have heq :
      beta^2 * (beta - a) = (a + 1) * beta + 1 := by
    nlinarith [hcubic]
  have hfactor : 1 < a * beta - a - 1 := by
    nlinarith [mul_nonneg
      (show 0 ≤ a by linarith) (show 0 ≤ beta - 2 * a by linarith)]
  have hstrict : (a + 1) * beta + 1 < a * beta^2 := by
    have hproduct : beta < beta * (a * beta - a - 1) :=
      by
        simpa using mul_lt_mul_of_pos_left hfactor hbeta
    nlinarith
  nlinarith

/-- The sharper coarse bound needed by the universal stepped-window
exclusion certificate.  It is still deliberately much weaker than
root isolation, but improves `beta < 2*a` enough to force the gap
`beta-(a+1/beta)` above `2/3`. -/
theorem class_ii_neighbor2_perron_lt_three_halves_a
    (a beta : ℝ)
    (ha : 7 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    beta < (3 / 2 : ℝ) * a := by
  by_contra hnot
  have hthree : (3 / 2 : ℝ) * a ≤ beta := le_of_not_gt hnot
  have haPos : 0 < a := by linarith
  have hdelta : a / 2 ≤ beta - a := by linarith
  have heq :
      beta^2 * (beta - a) = (a + 1) * beta + 1 := by
    nlinarith [hcubic]
  have hlarge : 2 < a * beta / 2 - a - 1 := by
    nlinarith [mul_nonneg
      (show 0 ≤ a by linarith)
      (show 0 ≤ beta - (3 / 2 : ℝ) * a by linarith)]
  have hstrict :
      (a + 1) * beta + 1 < beta^2 * (beta - a) := by
    have hbetaSq : 0 < beta^2 := sq_pos_of_pos hbeta
    have hlower :
        beta^2 * (a / 2) ≤ beta^2 * (beta - a) :=
      mul_le_mul_of_nonneg_left hdelta hbetaSq.le
    have hmiddle :
        (a + 1) * beta + 1 < beta^2 * (a / 2) := by
      nlinarith
    exact lt_of_lt_of_le hmiddle hlower
  nlinarith

/-- The exact scalar inequality used by the C++ affine
Fourier--Motzkin certificate. -/
theorem class_ii_neighbor2_perron_gap_gt_two_thirds
    (a beta : ℝ)
    (ha : 7 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    2 / 3 < beta - (a + 1 / beta) := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hgap :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hbound :=
    class_ii_neighbor2_perron_lt_three_halves_a
      a beta ha hbeta hcubic
  have hscaled : 2 * beta^2 < 3 * (a * beta + 1) := by
    have hproduct : 0 < beta * (3 * a - 2 * beta) :=
      mul_pos hbeta (by nlinarith)
    nlinarith
  nlinarith [sq_pos_of_pos hbeta]

/-- Kernel-checked endpoint validity for the fixed correction directly
from the Perron relation and one coarse root bound. -/
theorem neighbor2FixedNode_perron_valid
    (kind : Neighbor2FixedKind) (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (hbeta_a : a < beta) (hbeta_2a : beta < 2 * a)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (neighbor2FixedNode kind) beta (a + 1 / beta) := by
  have bounds := class_ii_neighbor2_perron_gap_bounds
    a beta ha hbeta hbeta_a hbeta_2a hcubic
  exact neighbor2FixedNode_in_open_strip
    kind beta (a + 1 / beta) bounds.1 bounds.2.1 bounds.2.2

/-- Fully cubic-driven fixed-correction endpoint validity. -/
theorem neighbor2FixedNode_cubic_valid
    (kind : Neighbor2FixedKind) (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (hbeta_a : a < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (neighbor2FixedNode kind) beta (a + 1 / beta) := by
  exact neighbor2FixedNode_perron_valid
    kind a beta ha hbeta hbeta_a
    (class_ii_neighbor2_perron_lt_two_a a beta ha hbeta hcubic)
    hcubic

inductive Neighbor2TerminalKind
  | n00 | n01 | n02 | n03 | n04 | n05
  deriving DecidableEq, Fintype

/-- The terminal sextet has exactly six kinds. -/
theorem neighbor2TerminalKind_card :
    Fintype.card Neighbor2TerminalKind = 6 := by
  native_decide

/-- The fixed twenty-four-state correction has exactly 24 kinds. -/
theorem neighbor2FixedKind_card :
    Fintype.card Neighbor2FixedKind = 24 := by
  native_decide

/-- The six affine terminal roles, with `q=a-2`. -/
def neighbor2TerminalNode
    (kind : Neighbor2TerminalKind) (a : ℤ) : NeighborNode :=
  let q := a - 2
  match kind with
  | .n00 => (0, q, -(q + 1),  2, 2)
  | .n01 => (0, q, -q,        2, 1)
  | .n02 => (1, -q, q,       -2, 0)
  | .n03 => (2, -q, q,       -2, 0)
  | .n04 => (2, -q, q + 1,   -2, 0)
  | .n05 => (2, q, -q,        1, 1)

/-- Two scalar endpoint margins suffice for all six terminal roles.
Keeping these margins explicit isolates the only unbounded arithmetic
from the finite endpoint table. -/
theorem neighbor2TerminalNode_in_open_strip
    (kind : Neighbor2TerminalKind) (a : ℤ) (b c : ℝ)
    (ha : 3 ≤ a)
    (hgapPos : 0 < b - c)
    (hgapUpper : b - c < 1)
    (hlower : c - 3 < ((a : ℝ) - 2) * (b - c))
    (hupper : ((a : ℝ) - 2) * (b - c) + 2 < c) :
    NeighborInOpenSignedStrip (neighbor2TerminalNode kind a) b c := by
  have haReal : (2 : ℝ) ≤ (a : ℝ) := by exact_mod_cast (show (2 : ℤ) ≤ a by omega)
  have hqd : 0 ≤ ((a : ℝ) - 2) * (b - c) :=
    mul_nonneg (by linarith) hgapPos.le
  have hcPos : 0 < c := by nlinarith
  cases kind <;>
    constructor <;>
    simp [neighborRightWidth, neighborNodeHeight,
      neighbor2TerminalNode] <;>
    norm_num at * <;> ring_nf at * <;> nlinarith

/-- The second penultimate affine state shares the terminal upper
margin. The first is the moving-tip formula and uses the weaker
center-shell edge margin already available in
`class_ii_window_bounds`. -/
theorem neighbor2PenultimateExtra_in_open_strip
    (a : ℤ) (b c : ℝ)
    (ha : 3 ≤ a)
    (hgapPos : 0 < b - c)
    (hupper : ((a : ℝ) - 2) * (b - c) + 2 < c) :
    NeighborInOpenSignedStrip
      (2, -(a - 2), a - 2, -2, 0) b c := by
  have haReal : (2 : ℝ) ≤ (a : ℝ) := by exact_mod_cast (show (2 : ℤ) ≤ a by omega)
  have hqd : 0 ≤ ((a : ℝ) - 2) * (b - c) :=
    mul_nonneg (by linarith) hgapPos.le
  have hcPos : 0 < c := by nlinarith
  constructor <;>
    simp [neighborRightWidth, neighborNodeHeight] <;>
    norm_num at * <;> ring_nf at * <;> nlinarith

/-- The two terminal margins follow from the Class-II cubic. The
identity `(beta-a-1) * (beta^2+beta) = 1` makes the lower margin's
apparently parameter-sized cancellation uniformly positive. -/
theorem class_ii_neighbor2_terminal_margins
    (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta) (hbeta_a : a < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    (a + 1 / beta) - 3 <
        (a - 2) * (beta - (a + 1 / beta)) ∧
      (a - 2) * (beta - (a + 1 / beta)) + 2 <
        a + 1 / beta := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hgap :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hbeta_a1 : a + 1 < beta := by
    by_contra hnot
    have hle : beta ≤ a + 1 := le_of_not_gt hnot
    have hleft :
        beta^2 * (beta - a) ≤ beta^2 :=
      mul_le_of_le_one_right (sq_nonneg beta) (by linarith)
    have heq :
        beta^2 * (beta - a) = (a + 1) * beta + 1 := by
      nlinarith [hcubic]
    have hright : beta^2 + 1 ≤ (a + 1) * beta + 1 := by
      have := mul_le_mul_of_nonneg_right hle hbeta.le
      nlinarith
    nlinarith
  let u := beta - a - 1
  have hu : 0 < u := by dsimp [u]; linarith
  have hueq : u * (beta^2 + beta) = 1 := by
    dsimp [u]
    nlinarith [hcubic]
  have hbeta3 : 3 < beta := by linarith
  have hden : 1 < beta^2 + beta := by nlinarith [sq_nonneg beta]
  have hu1 : u < 1 := by
    have hmul : u < u * (beta^2 + beta) :=
      by simpa using mul_lt_mul_of_pos_left hden hu
    nlinarith
  have haPos : 0 < a := by linarith
  have ha2beta2 : a^2 < beta^2 := by
    nlinarith [mul_pos (sub_pos.mpr hbeta_a)
      (show 0 < beta + a by linarith)]
  have ha2u : a^2 * u < 1 := by
    have hmul := mul_lt_mul_of_pos_right ha2beta2 hu
    have hbu : 0 < beta * u := mul_pos hbeta hu
    nlinarith
  have hau2 : a * u^2 < 1 := by
    have huu : u^2 < u := by nlinarith [mul_pos hu (sub_pos.mpr hu1)]
    have hau : a * u^2 < a * u :=
      mul_lt_mul_of_pos_left huu haPos
    have haa : a < a^2 := by nlinarith
    have haau : a * u < a^2 * u :=
      mul_lt_mul_of_pos_right haa hu
    linarith
  have hlowerScaled :
      ((a - 3) * beta^2 + beta) <
        (a - 2) * (a * beta + 1) := by
    have hubeta : beta = a + 1 + u := by dsimp [u]; ring
    nlinarith
  have hlower : (a + 1 / beta) - 3 <
      (a - 2) * (beta - (a + 1 / beta)) := by
    have hb2 : 0 < beta^2 := sq_pos_of_pos hbeta
    have hscaled :
        beta^2 * ((a + 1 / beta) - 3) <
          beta^2 * ((a - 2) * (beta - (a + 1 / beta))) := by
      field_simp [hne] at *
      nlinarith
    nlinarith
  have hupperScaled :
      (a - 2) * (a * beta + 1) + 2 * beta^2 <
        a * beta^2 + beta := by
    have hfactor : 0 < (a - 2) * beta + 1 := by
      have : 0 ≤ (a - 2) * beta :=
        mul_nonneg (by linarith) hbeta.le
      linarith
    have hproduct :
        0 < ((a - 2) * beta + 1) * (beta - a) :=
      mul_pos hfactor (sub_pos.mpr hbeta_a)
    nlinarith
  have hupper : (a - 2) * (beta - (a + 1 / beta)) + 2 <
      a + 1 / beta := by
    have hb2 : 0 < beta^2 := sq_pos_of_pos hbeta
    have hscaled :
        beta^2 *
            ((a - 2) * (beta - (a + 1 / beta)) + 2) <
          beta^2 * (a + 1 / beta) := by
      field_simp [hne] at *
      nlinarith
    nlinarith
  exact ⟨hlower, hupper⟩

/-- Two sharper terminal inequalities needed only by four endpoint
roles.  They retain the correlation between `d = beta-c` and
`e = (a-2)d` that the coarse independent scalar bounds forget. -/
theorem class_ii_neighbor2_terminal_refined_margins
    (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta) (hbeta_a : a < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    a + 1 / beta <
        (beta - (a + 1 / beta)) +
          (a - 2) * (beta - (a + 1 / beta)) + 2 ∧
      (a - 2) * (beta - (a + 1 / beta)) + 3 <
        a + 1 / beta + (beta - (a + 1 / beta)) := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hbeta_a1 : a + 1 < beta := by
    by_contra hnot
    have hle : beta ≤ a + 1 := le_of_not_gt hnot
    have hleft :
        beta^2 * (beta - a) ≤ beta^2 :=
      mul_le_of_le_one_right (sq_nonneg beta) (by linarith)
    have heq :
        beta^2 * (beta - a) = (a + 1) * beta + 1 := by
      nlinarith [hcubic]
    have hright : beta^2 + 1 ≤ (a + 1) * beta + 1 := by
      have := mul_le_mul_of_nonneg_right hle hbeta.le
      nlinarith
    nlinarith
  have hden : 0 < beta * (beta + 1) :=
    mul_pos hbeta (by linarith)
  have hfirstIdentity :
      ((beta - (a + 1 / beta)) +
          (a - 2) * (beta - (a + 1 / beta)) + 2 -
          (a + 1 / beta)) *
          (beta * (beta + 1))
        = beta^2 - (a - 1) * beta - 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hfirstRhs : 0 < beta^2 - (a - 1) * beta - 1 := by
    have hfactor : 2 < beta - (a - 1) := by linarith
    have hmul :
        2 * beta < beta * (beta - (a - 1)) :=
      by simpa [mul_comm] using
        (mul_lt_mul_of_pos_left hfactor hbeta)
    nlinarith
  have hfirstProduct :
      0 <
        ((beta - (a + 1 / beta)) +
          (a - 2) * (beta - (a + 1 / beta)) + 2 -
          (a + 1 / beta)) *
          (beta * (beta + 1)) := by
    rw [hfirstIdentity]
    exact hfirstRhs
  have hfirst :
      0 <
        (beta - (a + 1 / beta)) +
          (a - 2) * (beta - (a + 1 / beta)) + 2 -
          (a + 1 / beta) := by
    rcases (mul_pos_iff.mp hfirstProduct) with h | h
    · exact h.1
    · exfalso
      linarith [h.2, hden]
  have hsecondIdentity :
      ((a + 1 / beta + (beta - (a + 1 / beta))) -
          ((a - 2) * (beta - (a + 1 / beta)) + 3)) *
          (beta * (beta + 1))
        = (a - 2) * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hsecondRhs : 0 < (a - 2) * beta + 1 := by
    have : 0 ≤ (a - 2) * beta :=
      mul_nonneg (by linarith) hbeta.le
    linarith
  have hsecondProduct :
      0 <
        ((a + 1 / beta + (beta - (a + 1 / beta))) -
          ((a - 2) * (beta - (a + 1 / beta)) + 3)) *
          (beta * (beta + 1)) := by
    rw [hsecondIdentity]
    exact hsecondRhs
  have hsecond :
      0 <
        (a + 1 / beta + (beta - (a + 1 / beta))) -
          ((a - 2) * (beta - (a + 1 / beta)) + 3) := by
    rcases (mul_pos_iff.mp hsecondProduct) with h | h
    · exact h.1
    · exfalso
      linarith [h.2, hden]
  constructor <;> linarith

/-- Three correlated margins used by the repeated-terminal fixed
layer. They are elementary consequences of the same Perron cubic but
are invisible to independent interval bounds on `c`, `d`, and `e`. -/
theorem class_ii_neighbor2_fixed_refined_margins
    (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    (a + 1) * (beta - (a + 1 / beta)) < a + 1 / beta ∧
      a + 1 / beta <
        (a - 2) * (beta - (a + 1 / beta)) +
          2 * (beta - (a + 1 / beta)) + 1 ∧
      (a - 2) * (beta - (a + 1 / beta)) +
          2 * (beta - (a + 1 / beta)) <
        a + 1 / beta + 1 := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hden : 0 < beta * (beta + 1) :=
    mul_pos hbeta (by linarith)
  have hbeta_a1 : a + 1 < beta := by
    by_contra hnot
    have hle : beta ≤ a + 1 := le_of_not_gt hnot
    have hleft :
        beta^2 * (beta - a) ≤ beta^2 :=
      mul_le_of_le_one_right (sq_nonneg beta) (by linarith)
    have heq :
        beta^2 * (beta - a) = (a + 1) * beta + 1 := by
      nlinarith [hcubic]
    have hright : beta^2 + 1 ≤ (a + 1) * beta + 1 := by
      have := mul_le_mul_of_nonneg_right hle hbeta.le
      nlinarith
    nlinarith
  have hfirstIdentity :
      ((a + 1 / beta) -
          (a + 1) * (beta - (a + 1 / beta))) *
          (beta * (beta + 1))
        = (a + 1) * beta + 1 - beta^2 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hfirstRhs : 0 < (a + 1) * beta + 1 - beta^2 := by
    have hid :
        ((a + 1) * beta + 1 - beta^2) * (beta + 1) = beta := by
      nlinarith [hcubic]
    have hprod :
        0 < ((a + 1) * beta + 1 - beta^2) * (beta + 1) := by
      rw [hid]
      exact hbeta
    rcases (mul_pos_iff.mp hprod) with h | h
    · exact h.1
    · exfalso
      linarith [h.2, hbeta]
  have hfirstProduct :
      0 <
        ((a + 1 / beta) -
          (a + 1) * (beta - (a + 1 / beta))) *
          (beta * (beta + 1)) := by
    rw [hfirstIdentity]
    exact hfirstRhs
  have hfirst :
      0 < (a + 1 / beta) -
          (a + 1) * (beta - (a + 1 / beta)) := by
    rcases (mul_pos_iff.mp hfirstProduct) with h | h
    · exact h.1
    · exfalso
      linarith [h.2, hden]
  have hmiddleIdentity :
      ((a - 2) * (beta - (a + 1 / beta)) +
          2 * (beta - (a + 1 / beta)) + 1 -
          (a + 1 / beta)) *
          (beta * (beta + 1))
        = beta^2 - a * beta - 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hmiddleRhs : 0 < beta^2 - a * beta - 1 := by
    have hgap : 1 < beta - a := by linarith [hbeta_a1]
    have hmul : beta < beta * (beta - a) :=
      by simpa using (mul_lt_mul_of_pos_left hgap hbeta)
    nlinarith
  have hmiddleProduct :
      0 <
        ((a - 2) * (beta - (a + 1 / beta)) +
          2 * (beta - (a + 1 / beta)) + 1 -
          (a + 1 / beta)) *
          (beta * (beta + 1)) := by
    rw [hmiddleIdentity]
    exact hmiddleRhs
  have hmiddle :
      0 <
        (a - 2) * (beta - (a + 1 / beta)) +
          2 * (beta - (a + 1 / beta)) + 1 -
          (a + 1 / beta) := by
    rcases (mul_pos_iff.mp hmiddleProduct) with h | h
    · exact h.1
    · exfalso
      linarith [h.2, hden]
  have hlastIdentity :
      ((a + 1 / beta + 1) -
          ((a - 2) * (beta - (a + 1 / beta)) +
            2 * (beta - (a + 1 / beta)))) *
          (beta * (beta + 1))
        = beta^2 + (a + 2) * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hlastRhs : 0 < beta^2 + (a + 2) * beta + 1 := by
    have : 0 ≤ (a + 2) * beta :=
      mul_nonneg (by linarith) hbeta.le
    nlinarith [sq_nonneg beta]
  have hlastProduct :
      0 <
        ((a + 1 / beta + 1) -
          ((a - 2) * (beta - (a + 1 / beta)) +
            2 * (beta - (a + 1 / beta)))) *
          (beta * (beta + 1)) := by
    rw [hlastIdentity]
    exact hlastRhs
  have hlast :
      0 <
        (a + 1 / beta + 1) -
          ((a - 2) * (beta - (a + 1 / beta)) +
            2 * (beta - (a + 1 / beta))) := by
    rcases (mul_pos_iff.mp hlastProduct) with h | h
    · exact h.1
    · exfalso
      linarith [h.2, hden]
  constructor
  · linarith
  constructor <;> linarith

/-- The terminal sextet is universally window-valid for the Class-II
Perron root. -/
theorem neighbor2TerminalNode_cubic_valid
    (kind : Neighbor2TerminalKind) (a : ℤ) (beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (hbeta_a : (a : ℝ) < beta)
    (hcubic :
      beta^3 = (a : ℝ) * beta^2 +
        ((a : ℝ) + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (neighbor2TerminalNode kind a)
      beta ((a : ℝ) + 1 / beta) := by
  have haReal : (3 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
  have margins := class_ii_neighbor2_terminal_margins
    (a : ℝ) beta haReal hbeta hbeta_a hcubic
  have gapBounds := class_ii_neighbor2_perron_gap_bounds
    (a : ℝ) beta haReal hbeta hbeta_a
    (class_ii_neighbor2_perron_lt_two_a
      (a : ℝ) beta haReal hbeta hcubic)
    hcubic
  exact neighbor2TerminalNode_in_open_strip
    kind a beta ((a : ℝ) + 1 / beta) ha
    (by linarith [gapBounds.2.1])
    gapBounds.2.2 margins.1 margins.2

/-- The extra penultimate affine endpoint is universally window-valid.
The other penultimate affine endpoint is the moving-tip shape and is
covered by the existing center-shell edge margin. -/
theorem neighbor2PenultimateExtra_cubic_valid
    (a : ℤ) (beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (hbeta_a : (a : ℝ) < beta)
    (hcubic :
      beta^3 = (a : ℝ) * beta^2 +
        ((a : ℝ) + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (2, -(a - 2), a - 2, -2, 0)
      beta ((a : ℝ) + 1 / beta) := by
  have haReal : (3 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
  have margins := class_ii_neighbor2_terminal_margins
    (a : ℝ) beta haReal hbeta hbeta_a hcubic
  have gapBounds := class_ii_neighbor2_perron_gap_bounds
    (a : ℝ) beta haReal hbeta hbeta_a
    (class_ii_neighbor2_perron_lt_two_a
      (a : ℝ) beta haReal hbeta hcubic)
    hcubic
  exact neighbor2PenultimateExtra_in_open_strip
    a beta ((a : ℝ) + 1 / beta) ha
    (by linarith [gapBounds.2.1]) margins.2

/-- The moving penultimate endpoint uses exactly the second
center-window bound at `q=a-1`. -/
theorem neighbor2PenultimateTip_cubic_valid
    (a : ℤ) (beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta)
    (hbeta_a : (a : ℝ) < beta)
    (hcubic :
      beta^3 = (a : ℝ) * beta^2 +
        ((a : ℝ) + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (2, -(a - 1), a - 1, -1, 0)
      beta ((a : ℝ) + 1 / beta) := by
  have haReal : (3 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
  have margins := class_ii_neighbor2_terminal_margins
    (a : ℝ) beta haReal hbeta hbeta_a hcubic
  have hgapPos : 0 < beta - ((a : ℝ) + 1 / beta) := by
    have gapBounds := class_ii_neighbor2_perron_gap_bounds
      (a : ℝ) beta (by exact_mod_cast ha) hbeta hbeta_a
      (class_ii_neighbor2_perron_lt_two_a
        (a : ℝ) beta (by exact_mod_cast ha) hbeta hcubic)
      hcubic
    linarith [gapBounds.2.1]
  have hqnonneg :
      0 ≤ ((a : ℝ) - 1) *
        (beta - ((a : ℝ) + 1 / beta)) :=
    mul_nonneg (by linarith) hgapPos.le
  constructor <;>
    simp [neighborRightWidth, neighborNodeHeight] <;>
    norm_num at * <;> ring_nf at * <;> nlinarith

/-- Direct cubic-driven validity for every moving correction tip up to
the penultimate round, including the exceptional second-round tip at
`a=3`. -/
theorem neighbor2InteriorTip_cubic_valid
    (a r : ℤ) (beta : ℝ)
    (ha : 3 ≤ a) (hr : 0 ≤ r) (hra : r ≤ a - 1)
    (hbeta : 0 < beta) (hbeta_a : (a : ℝ) < beta)
    (hcubic :
      beta^3 = (a : ℝ) * beta^2 +
        ((a : ℝ) + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (2, -r, r, -1, 0)
      beta ((a : ℝ) + 1 / beta) := by
  have haReal : (3 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
  have hrReal : (0 : ℝ) ≤ (r : ℝ) := by exact_mod_cast hr
  have hraReal : (r : ℝ) ≤ (a : ℝ) - 1 := by
    exact_mod_cast (show r ≤ a - 1 by omega)
  have margins := class_ii_neighbor2_terminal_margins
    (a : ℝ) beta haReal hbeta hbeta_a hcubic
  have gapBounds := class_ii_neighbor2_perron_gap_bounds
    (a : ℝ) beta haReal hbeta hbeta_a
    (class_ii_neighbor2_perron_lt_two_a
      (a : ℝ) beta haReal hbeta hcubic)
    hcubic
  have hgapPos :
      0 < beta - ((a : ℝ) + 1 / beta) := by
    linarith [gapBounds.2.1]
  have hbound :
      ((r : ℝ) - 1) *
          (beta - ((a : ℝ) + 1 / beta)) + 1 <
        (a : ℝ) + 1 / beta := by
    have hmul :
        ((r : ℝ) - 1) *
            (beta - ((a : ℝ) + 1 / beta)) ≤
          ((a : ℝ) - 2) *
            (beta - ((a : ℝ) + 1 / beta)) :=
      mul_le_mul_of_nonneg_right (by linarith) hgapPos.le
    linarith [margins.2]
  have hrd :
      0 ≤ (r : ℝ) *
        (beta - ((a : ℝ) + 1 / beta)) :=
    mul_nonneg hrReal hgapPos.le
  constructor <;>
    simp [neighborRightWidth, neighborNodeHeight] <;>
    norm_num at * <;> ring_nf at * <;> nlinarith

/-- The one state replacing a fixed correction state in round two is
also universally inside the open window. -/
theorem neighbor2SecondExtra_cubic_valid
    (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta) (hbeta_a : a < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (0, 0, 1, -1, 0) beta (a + 1 / beta) := by
  have gapBounds := class_ii_neighbor2_perron_gap_bounds
    a beta ha hbeta hbeta_a
    (class_ii_neighbor2_perron_lt_two_a
      a beta ha hbeta hcubic)
    hcubic
  constructor <;>
    simp [neighborRightWidth, neighborNodeHeight] <;>
    norm_num at * <;> linarith

inductive CenterInterfaceSupportKind
  | n00 | n01 | n02 | n03 | n04 | n05 | n06 | n07
  | n08 | n09 | n10 | n11 | n12 | n13 | n14 | n15
  | n16 | n17 | n18 | n19 | n20 | n21 | n22 | n23
  deriving DecidableEq, Fintype

/-- The center-interface support catalogue has 24 kinds. -/
theorem centerInterfaceSupportKind_card :
    Fintype.card CenterInterfaceSupportKind = 24 := by
  native_decide

/-- The thirteen signed-contact support states, seven round-two
support states, and four round-three interface targets used by the
bounded center-interface occurrence table. -/
def centerInterfaceSupportNode :
    CenterInterfaceSupportKind → NeighborNode
  | .n00 => (0, -1,  0,  1, 1)
  | .n01 => (0, -1,  1,  0, 1)
  | .n02 => (0,  0, -1,  0, 0)
  | .n03 => (0,  0,  0,  0, 1)
  | .n04 => (1,  1, -1, -1, 0)
  | .n05 => (1,  1, -1,  0, 0)
  | .n06 => (2,  0,  0,  0, 0)
  | .n07 => (2,  0,  0,  0, 1)
  | .n08 => (0, -1,  1,  0, 0)
  | .n09 => (1,  0,  0, -1, 0)
  | .n10 => (1,  0,  0,  0, 0)
  | .n11 => (2, -1,  1,  0, 1)
  | .n12 => (2,  0,  1, -1, 0)
  | .n13 => (0, -2,  2,  0, 0)
  | .n14 => (1, -1,  1, -1, 0)
  | .n15 => (1,  0, -1,  0, 0)
  | .n16 => (1,  1, -2,  0, 0)
  | .n17 => (2, -1,  2, -1, 0)
  | .n18 => (2,  0, -1,  0, 0)
  | .n19 => (2, -1,  1, -1, 0)
  | .n20 => (0, -2,  3,  0, 0)
  | .n21 => (1, -1,  2, -1, 0)
  | .n22 => (2, -2,  2, -1, 0)
  | .n23 => (2, -2,  3, -1, 0)

theorem centerInterfaceSupportNode_in_open_strip
    (kind : CenterInterfaceSupportKind) (b c : ℝ)
    (hc : 3 < c)
    (hgapLower : 1 / 2 < b - c)
    (hgapUpper : b - c < 1) :
    NeighborInOpenSignedStrip
      (centerInterfaceSupportNode kind) b c := by
  cases kind <;>
    constructor <;>
    simp [neighborRightWidth, neighborNodeHeight,
      centerInterfaceSupportNode] <;>
    norm_num at * <;> linarith

/-- All center-interface support endpoints are universally valid in
the Class-II Perron window. -/
theorem centerInterfaceSupportNode_cubic_valid
    (kind : CenterInterfaceSupportKind) (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta) (hbeta_a : a < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (centerInterfaceSupportNode kind) beta (a + 1 / beta) := by
  have gapBounds := class_ii_neighbor2_perron_gap_bounds
    a beta ha hbeta hbeta_a
    (class_ii_neighbor2_perron_lt_two_a
      a beta ha hbeta hcubic)
    hcubic
  exact centerInterfaceSupportNode_in_open_strip
    kind beta (a + 1 / beta)
    gapBounds.1 gapBounds.2.1 gapBounds.2.2

inductive CenterBaseRedExtraKind
  | n00 | n01 | n02 | n03 | n04
  | n05 | n06 | n07 | n08 | n09
  deriving DecidableEq, Fintype

/-- The center-base Red-survival extras have 10 kinds. -/
theorem centerBaseRedExtraKind_card :
    Fintype.card CenterBaseRedExtraKind = 10 := by
  native_decide

/-- Extra nodes needed to close the functional Red-survival
subgraphs for center rounds two and three. -/
def centerBaseRedExtraNode : CenterBaseRedExtraKind → NeighborNode
  | .n00 => (0, -1,  1, -1, 0)
  | .n01 => (0,  1, -2,  0, 0)
  | .n02 => (0,  2, -2,  0, 0)
  | .n03 => (1,  2, -2,  0, 0)
  | .n04 => (0, -3,  3,  0, 0)
  | .n05 => (0, -3,  3,  0, 1)
  | .n06 => (0, -2,  2, -1, 0)
  | .n07 => (0,  2, -3,  0, 0)
  | .n08 => (0,  3, -3,  0, 0)
  | .n09 => (1,  3, -3,  0, 0)

theorem centerBaseRedExtraNode_in_open_strip
    (kind : CenterBaseRedExtraKind) (b c : ℝ)
    (hc : 3 < c)
    (hgapLower : 1 / 2 < b - c)
    (hgapUpper : b - c < 1) :
    NeighborInOpenSignedStrip
      (centerBaseRedExtraNode kind) b c := by
  cases kind <;>
    constructor <;>
    simp [neighborRightWidth, neighborNodeHeight,
      centerBaseRedExtraNode] <;>
    norm_num at * <;> linarith

theorem centerBaseRedExtraNode_cubic_valid
    (kind : CenterBaseRedExtraKind) (a beta : ℝ)
    (ha : 3 ≤ a) (hbeta : 0 < beta) (hbeta_a : a < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    NeighborInOpenSignedStrip
      (centerBaseRedExtraNode kind) beta (a + 1 / beta) := by
  have gapBounds := class_ii_neighbor2_perron_gap_bounds
    a beta ha hbeta hbeta_a
    (class_ii_neighbor2_perron_lt_two_a
      a beta ha hbeta hcubic)
    hcubic
  exact centerBaseRedExtraNode_in_open_strip
    kind beta (a + 1 / beta)
    gapBounds.1 gapBounds.2.1 gapBounds.2.2
