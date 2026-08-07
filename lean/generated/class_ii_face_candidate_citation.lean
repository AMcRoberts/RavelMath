import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure ClassIINodeE where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

inductive DContFaceCandidateKindE
  | f00 | f01 | f02 | f03 | f04 | f05 | f06 | f07 | f08 | f09
  | f10 | f11 | f12 | f13 | f14 | f15 | f16 | f17 | f18 | f19
  | f20 | f21 | f22 | f23 | f24 | f25 | f26 | f27 | f28 | f29
  | f30 | f31 | f32
  deriving DecidableEq, Fintype

def dContFaceCandidateNodeE : DContFaceCandidateKindE → ClassIINodeE
  | .f00 => ⟨0,  0, -1,  0, 0⟩
  | .f01 => ⟨0,  0,  0, -1, 0⟩
  | .f02 => ⟨0,  0,  0,  1, 0⟩
  | .f03 => ⟨0,  0,  1,  0, 0⟩
  | .f04 => ⟨0, -1,  0,  0, 1⟩
  | .f05 => ⟨0, -1,  1,  0, 1⟩
  | .f06 => ⟨0,  0,  0,  0, 1⟩
  | .f07 => ⟨0,  0,  1,  0, 1⟩
  | .f08 => ⟨0, -1,  0,  0, 2⟩
  | .f09 => ⟨0, -1,  0,  1, 2⟩
  | .f10 => ⟨0,  0,  0,  0, 2⟩
  | .f11 => ⟨0,  0,  0,  1, 2⟩
  | .f12 => ⟨1,  0, -1,  0, 0⟩
  | .f13 => ⟨1,  1, -1,  0, 0⟩
  | .f14 => ⟨1,  1,  0,  0, 0⟩
  | .f15 => ⟨1, -1,  0,  0, 1⟩
  | .f16 => ⟨1,  0,  0, -1, 1⟩
  | .f17 => ⟨1,  0,  0,  1, 1⟩
  | .f18 => ⟨1,  1,  0,  0, 1⟩
  | .f19 => ⟨1,  0, -1,  0, 2⟩
  | .f20 => ⟨1,  0, -1,  1, 2⟩
  | .f21 => ⟨1,  0,  0,  0, 2⟩
  | .f22 => ⟨1,  0,  0,  1, 2⟩
  | .f23 => ⟨2,  0,  0, -1, 0⟩
  | .f24 => ⟨2,  1,  0, -1, 0⟩
  | .f25 => ⟨2,  1,  0,  0, 0⟩
  | .f26 => ⟨2,  0,  0, -1, 1⟩
  | .f27 => ⟨2,  0,  1, -1, 1⟩
  | .f28 => ⟨2,  0,  1,  0, 1⟩
  | .f29 => ⟨2, -1,  0,  0, 2⟩
  | .f30 => ⟨2,  0, -1,  0, 2⟩
  | .f31 => ⟨2,  0,  1,  0, 2⟩
  | .f32 => ⟨2,  1,  0,  0, 2⟩

def dContFaceCandidateAcceptedE : DContFaceCandidateKindE → Bool
  | .f02 | .f03 | .f06 | .f10 | .f13
  | .f17 | .f21 | .f24 | .f27 => true
  | _ => false

def nodeHeightE (node : ClassIINodeE) (b c : ℝ) : ℝ :=
  node.x0 * b + node.x1 * c + node.x2

def rightWidthE (node : ClassIINodeE) (b c : ℝ) : ℝ :=
  if node.right = 0 then b else if node.right = 1 then c else 1

def InRestrictedHE (node : ClassIINodeE) (b c : ℝ) : Prop :=
  0 ≤ nodeHeightE node b c ∧ nodeHeightE node b c < rightWidthE node b c

theorem dContFaceCandidate_window_iffE
    (kind : DContFaceCandidateKindE) (b c : ℝ)
    (hc : 1 < c) (hcb : c < b) :
    InRestrictedHE (dContFaceCandidateNodeE kind) b c ↔
      dContFaceCandidateAcceptedE kind = true := by
  cases kind <;>
    simp [InRestrictedHE, rightWidthE, nodeHeightE,
      dContFaceCandidateNodeE, dContFaceCandidateAcceptedE] <;>
    first
    | linarith
    | (constructor <;> linarith)
    | (intro h; linarith)

/-- Universal Perron-window classification of the fixed `D_cont`
    face-candidate table: for ANY `a >= 2` and its actual Class-II Perron root
    `beta`, a candidate lies in the restricted stepped hyperplane IFF it is one
    of the nine flagged candidates. Reproduced from the independently
    kernel-checked `lean/class_ii_affine_shells.lean` (not re-derived here). -/
theorem class_ii_dCont_face_candidate_valid_iff
    (kind : DContFaceCandidateKindE) (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic : beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    InRestrictedHE (dContFaceCandidateNodeE kind) beta (a + 1 / beta) ↔
      dContFaceCandidateAcceptedE kind = true := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hprod : 0 < beta^2 * (beta - (a + 1 / beta)) := by
    rw [hd]
    positivity
  have hbc : a + 1 / beta < beta := by
    have hgap : 0 < beta - (a + 1 / beta) :=
      pos_of_mul_pos_right hprod (sq_nonneg beta)
    linarith
  have hc : (1 : ℝ) < a + 1 / beta := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    linarith
  exact dContFaceCandidate_window_iffE kind beta (a + 1 / beta) hc hbc

structure ClassIINodeF where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

def nodeHeightF (node : ClassIINodeF) (b c : ℝ) : ℝ :=
  node.x0 * b + node.x1 * c + node.x2

def rightWidthF (node : ClassIINodeF) (b c : ℝ) : ℝ :=
  if node.right = 0 then b else if node.right = 1 then c else 1

def InRestrictedHF (node : ClassIINodeF) (b c : ℝ) : Prop :=
  0 ≤ nodeHeightF node b c ∧ nodeHeightF node b c < rightWidthF node b c

/-- A raw contact-envelope category has `x1 ∈ [-2,2]` and `x2 ∈ [-1,1]`. Once
    the Perron gap exceeds one half, restricted window membership forces its
    remaining integer coordinate into the five slices `x0 ∈ [-2,2]`. -/
theorem restrictedHF_forces_bounded_x0
    (node : ClassIINodeF) (b c : ℝ)
    (hc : 1 < c) (hcb : c < b) (hgap : 1 / 2 < b - c)
    (hx1lo : -2 ≤ node.x1) (hx1hi : node.x1 ≤ 2)
    (hx2lo : -1 ≤ node.x2) (hx2hi : node.x2 ≤ 1)
    (hright : node.right = 0 ∨ node.right = 1 ∨ node.right = 2)
    (hwindow : InRestrictedHF node b c) :
    -2 ≤ node.x0 ∧ node.x0 ≤ 2 := by
  have hb : 0 < b := by linarith
  constructor
  · by_contra hnot
    have hx0 : node.x0 ≤ -3 := by omega
    have hx0r : (node.x0 : ℝ) ≤ -3 := by exact_mod_cast hx0
    have hx1r : (node.x1 : ℝ) ≤ 2 := by exact_mod_cast hx1hi
    have hx2r : (node.x2 : ℝ) ≤ 1 := by exact_mod_cast hx2hi
    have h0 : (node.x0 : ℝ) * b ≤ -3 * b :=
      mul_le_mul_of_nonneg_right hx0r hb.le
    have h1 : (node.x1 : ℝ) * c ≤ 2 * c :=
      mul_le_mul_of_nonneg_right hx1r (by linarith)
    have hheight : nodeHeightF node b c < 0 := by
      simp [nodeHeightF]
      linarith
    linarith [hwindow.1]
  · by_contra hnot
    have hx0 : 3 ≤ node.x0 := by omega
    have hx0r : (3 : ℝ) ≤ node.x0 := by exact_mod_cast hx0
    have hx1r : (-2 : ℝ) ≤ node.x1 := by exact_mod_cast hx1lo
    have hx2r : (-1 : ℝ) ≤ node.x2 := by exact_mod_cast hx2lo
    have h0 : 3 * b ≤ (node.x0 : ℝ) * b :=
      mul_le_mul_of_nonneg_right hx0r hb.le
    have h1 : -2 * c ≤ (node.x1 : ℝ) * c :=
      mul_le_mul_of_nonneg_right hx1r (by linarith)
    have hheight : b < nodeHeightF node b c := by
      simp [nodeHeightF]
      linarith
    rcases hright with hr | hr | hr <;>
      simp [InRestrictedHF, rightWidthF, hr] at hwindow <;>
      linarith

theorem class_ii_perron_gap_gt_halfF
    (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic : beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    1 / 2 < beta - (a + 1 / beta) := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd : beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  by_contra hnot
  have hgap_le : beta - (a + 1 / beta) ≤ 1 / 2 := le_of_not_gt hnot
  have hscaled : 2 * (a * beta + 1) ≤ beta^2 := by
    have hmul := mul_le_mul_of_nonneg_left hgap_le (sq_nonneg beta)
    rw [hd] at hmul
    nlinarith
  have hbeta_two_a : 2 * a < beta := by
    by_contra hle
    have hbeta_le : beta ≤ 2 * a := le_of_not_gt hle
    have hproduct : 0 ≤ beta * (2 * a - beta) :=
      mul_nonneg hbeta.le (by linarith)
    nlinarith
  have heq : beta^2 * (beta - a) = (a + 1) * beta + 1 := by nlinarith [hcubic]
  have hleft : a * beta^2 < beta^2 * (beta - a) := by
    have hdelta : a < beta - a := by linarith
    simpa [mul_comm] using mul_lt_mul_of_pos_left hdelta (sq_pos_of_pos hbeta)
  have hfactor : 1 < a * beta - a - 1 := by
    nlinarith [mul_nonneg (show 0 ≤ a by linarith) (show 0 ≤ beta - 2 * a by linarith)]
  have hright : (a + 1) * beta + 1 < a * beta^2 := by
    have hproduct : beta < beta * (a * beta - a - 1) := by
      simpa using mul_lt_mul_of_pos_left hfactor hbeta
    nlinarith
  nlinarith

/-- Universal Class-II specialization: for ANY `a >= 2` and its actual
    Perron root `beta`, a raw-contact-envelope node's `x0` coordinate is
    forced into `[-2,2]` -- exactly the search range this project's
    `class_ii_d_cont_face_candidates` uses. Reproduced from the
    independently kernel-checked `lean/class_ii_affine_shells.lean` (not
    re-derived here). -/
theorem class_ii_rawContact_x0_bounded
    (node : ClassIINodeF) (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic : beta^3 = a * beta^2 + (a + 1) * beta + 1)
    (hx1lo : -2 ≤ node.x1) (hx1hi : node.x1 ≤ 2)
    (hx2lo : -1 ≤ node.x2) (hx2hi : node.x2 ≤ 1)
    (hright : node.right = 0 ∨ node.right = 1 ∨ node.right = 2)
    (hwindow : InRestrictedHF node beta (a + 1 / beta)) :
    -2 ≤ node.x0 ∧ node.x0 ≤ 2 := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd : beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hprod : 0 < beta^2 * (beta - (a + 1 / beta)) := by
    rw [hd]
    positivity
  have hbc : a + 1 / beta < beta := by
    have hgap : 0 < beta - (a + 1 / beta) :=
      pos_of_mul_pos_right hprod (sq_nonneg beta)
    linarith
  have hc : (1 : ℝ) < a + 1 / beta := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    linarith
  exact restrictedHF_forces_bounded_x0
    node beta (a + 1 / beta) hc hbc
    (class_ii_perron_gap_gt_halfF a beta ha hbeta hcubic)
    hx1lo hx1hi hx2lo hx2hi hright hwindow

/- Semantic proof graph for: class_ii_face_candidate_batch
  [0] lean.lemma_application :: class_ii_dCont_face_candidate_valid_iff proves for any a>=2 and its actual Class-II Perron root, exactly the nine flagged face candidates lie in the restricted stepped hyperplane -- an iff, not just a one-direction check
  [1] lean.lemma_application :: class_ii_rawContact_x0_bounded proves the search range x0 in [-2,2] used above is exactly the bound the restricted-window Perron-gap argument forces, for any a>=2 -- not a heuristic search-space choice
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
