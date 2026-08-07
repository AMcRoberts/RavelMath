import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure ClassIINodeC where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

inductive ContactKind
  | c00 | c01 | c02 | c03 | c04 | c05 | c06
  | c07 | c08 | c09 | c10 | c11 | c12 | c13
  deriving DecidableEq, Fintype

def contactNode : ContactKind → ClassIINodeC
  | .c00 => ⟨0, -1,  1,  1, 1⟩
  | .c01 => ⟨0,  0,  0,  0, 1⟩
  | .c02 => ⟨0,  0,  0,  0, 2⟩
  | .c03 => ⟨0,  0,  0,  1, 0⟩
  | .c04 => ⟨0,  0,  0,  1, 1⟩
  | .c05 => ⟨0,  0,  1,  0, 0⟩
  | .c06 => ⟨0,  1, -1,  0, 0⟩
  | .c07 => ⟨1,  0,  0,  0, 2⟩
  | .c08 => ⟨1,  1, -1,  0, 0⟩
  | .c09 => ⟨1,  1, -1,  0, 2⟩
  | .c10 => ⟨1,  1,  0, -1, 0⟩
  | .c11 => ⟨2,  0,  1, -1, 0⟩
  | .c12 => ⟨2,  0,  1,  0, 0⟩
  | .c13 => ⟨2,  1,  0, -1, 0⟩

def nodeHeight (node : ClassIINodeC) (b c : ℝ) : ℝ :=
  node.x0 * b + node.x1 * c + node.x2

def rightWidth (node : ClassIINodeC) (b c : ℝ) : ℝ :=
  if node.right = 0 then b else if node.right = 1 then c else 1

def InRestrictedH (node : ClassIINodeC) (b c : ℝ) : Prop :=
  0 ≤ nodeHeight node b c ∧ nodeHeight node b c < rightWidth node b c

theorem contactNode_in_restrictedH
    (kind : ContactKind) (b c : ℝ)
    (hc : 1 < c) (hcb : c < b) (hgap : b - c < 1) :
    InRestrictedH (contactNode kind) b c := by
  cases kind <;>
    simp [InRestrictedH, nodeHeight, rightWidth, contactNode] <;>
    first
    | (constructor <;> linarith)
    | linarith

theorem class_ii_perron_gap_lt_one
    (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic : beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    beta - (a + 1 / beta) < 1 := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hscaled :
      beta * (beta - a) = a + 1 + 1 / beta := by
    field_simp [hne]
    nlinarith [hcubic]
  have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
  have hunit : 1 < beta * (beta - a) := by
    rw [hscaled]
    linarith
  have hcompare : a * beta + 1 < beta^2 := by
    nlinarith
  have hb2 : 0 < beta^2 := sq_pos_of_pos hbeta
  have hmul :
      beta^2 * (beta - (a + 1 / beta)) < beta^2 * 1 := by
    rw [hd]
    simpa using hcompare
  exact lt_of_mul_lt_mul_left hmul hb2.le

/-- Universal Class-II form of contact-catalogue validity: EVERY one of the
    fourteen contact states lies in the restricted stepped hyperplane, for ANY
    `a >= 2` and its actual Class-II Perron root `beta`. Reproduced from the
    independently kernel-checked `lean/class_ii_affine_shells.lean` (not
    re-derived here). -/
theorem class_ii_contactNode_valid
    (kind : ContactKind) (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic : beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    InRestrictedH (contactNode kind) beta (a + 1 / beta) := by
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
  exact contactNode_in_restrictedH kind beta (a + 1 / beta)
    hc hbc (class_ii_perron_gap_lt_one a beta ha hbeta hcubic)

/- Semantic proof graph for: class_ii_contact_batch
  [0] lean.lemma_application :: class_ii_contactNode_valid proves every state in the fourteen-state contact catalogue lies in the restricted stepped hyperplane, for any a>=2 and its actual Class-II Perron root -- not a per-a numeric check
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
