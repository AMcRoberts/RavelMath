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

/- Semantic proof graph for: class_ii_face_candidate_batch
  [0] lean.lemma_application :: class_ii_dCont_face_candidate_valid_iff proves for any a>=2 and its actual Class-II Perron root, exactly the nine flagged face candidates lie in the restricted stepped hyperplane -- an iff, not just a one-direction check
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
