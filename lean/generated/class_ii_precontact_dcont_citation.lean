import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure ClassIINodeD where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

inductive DContKindD
  | d00 | d01 | d02 | d03 | d04 | d05 | d06 | d07 | d08
  deriving DecidableEq, Fintype

def dContNodeD : DContKindD → ClassIINodeD
  | .d00 => ⟨0, 0,  0,  1, 0⟩
  | .d01 => ⟨0, 0,  1,  0, 0⟩
  | .d02 => ⟨0, 0,  0,  0, 1⟩
  | .d03 => ⟨0, 0,  0,  0, 2⟩
  | .d04 => ⟨1, 1, -1,  0, 0⟩
  | .d05 => ⟨1, 0,  0,  1, 1⟩
  | .d06 => ⟨1, 0,  0,  0, 2⟩
  | .d07 => ⟨2, 1,  0, -1, 0⟩
  | .d08 => ⟨2, 0,  1, -1, 1⟩

inductive PreContactKindD
  | p00 | p01 | p02 | p03 | p04 | p05 | p06 | p07
  | p08 | p09 | p10 | p11 | p12 | p13 | p14 | p15
  deriving DecidableEq, Fintype

def preContactNodeD : PreContactKindD → ClassIINodeD
  | .p00 => ⟨0, -1,  1,  1, 1⟩
  | .p01 => ⟨0,  0,  0,  0, 1⟩
  | .p02 => ⟨0,  0,  0,  0, 2⟩
  | .p03 => ⟨0,  0,  0,  1, 0⟩
  | .p04 => ⟨0,  0,  0,  1, 1⟩
  | .p05 => ⟨0,  0,  1,  0, 0⟩
  | .p06 => ⟨0,  1, -1,  0, 0⟩
  | .p07 => ⟨1,  0,  0,  0, 2⟩
  | .p08 => ⟨1,  0,  0,  1, 1⟩
  | .p09 => ⟨1,  1, -1,  0, 0⟩
  | .p10 => ⟨1,  1, -1,  0, 2⟩
  | .p11 => ⟨1,  1,  0, -1, 0⟩
  | .p12 => ⟨2,  0,  1, -1, 0⟩
  | .p13 => ⟨2,  0,  1, -1, 1⟩
  | .p14 => ⟨2,  0,  1,  0, 0⟩
  | .p15 => ⟨2,  1,  0, -1, 0⟩

theorem dContNode_in_preContact (kind : DContKindD) :
    ∃ pre : PreContactKindD, dContNodeD kind = preContactNodeD pre := by
  cases kind <;> native_decide

inductive ContactKindD
  | c00 | c01 | c02 | c03 | c04 | c05 | c06
  | c07 | c08 | c09 | c10 | c11 | c12 | c13
  deriving DecidableEq, Fintype

def contactNodeD : ContactKindD → ClassIINodeD
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

inductive ContactRedExcludedKindD
  | e00 | e01
  deriving DecidableEq, Fintype

def contactRedExcludedNodeD : ContactRedExcludedKindD → ClassIINodeD
  | .e00 => ⟨1, 0, 0,  1, 1⟩
  | .e01 => ⟨2, 0, 1, -1, 1⟩

/-- The pre-contact catalogue is exactly the disjoint union of the fourteen
    contact states and the two displayed Red exclusions. Reproduced from the
    independently kernel-checked `lean/class_ii_affine_shells.lean` (not
    re-derived here). -/
theorem preContactNode_partition (kind : PreContactKindD) :
    (∃ contact : ContactKindD, preContactNodeD kind = contactNodeD contact) ∨
    (∃ excluded : ContactRedExcludedKindD,
      preContactNodeD kind = contactRedExcludedNodeD excluded) := by
  cases kind <;> native_decide

/- Semantic proof graph for: class_ii_precontact_dcont_batch
  [0] lean.lemma_application :: dContNode_in_preContact proves every one of the nine D_cont states is a genuine member of the sixteen-state pre-contact catalogue
  [1] lean.lemma_application :: preContactNode_partition proves every one of the sixteen pre-contact states is either one of the fourteen contact states or one of the two Red exclusions
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
