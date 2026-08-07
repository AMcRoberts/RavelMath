import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure ClassIINodeG where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

inductive ContactKindG
  | c00 | c01 | c02 | c03 | c04 | c05 | c06
  | c07 | c08 | c09 | c10 | c11 | c12 | c13
  deriving DecidableEq, Fintype

def contactNodeG : ContactKindG → ClassIINodeG
  | .c00 => ⟨0, -1,  1,  1, 1⟩ | .c01 => ⟨0,  0,  0,  0, 1⟩
  | .c02 => ⟨0,  0,  0,  0, 2⟩ | .c03 => ⟨0,  0,  0,  1, 0⟩
  | .c04 => ⟨0,  0,  0,  1, 1⟩ | .c05 => ⟨0,  0,  1,  0, 0⟩
  | .c06 => ⟨0,  1, -1,  0, 0⟩ | .c07 => ⟨1,  0,  0,  0, 2⟩
  | .c08 => ⟨1,  1, -1,  0, 0⟩ | .c09 => ⟨1,  1, -1,  0, 2⟩
  | .c10 => ⟨1,  1,  0, -1, 0⟩ | .c11 => ⟨2,  0,  1, -1, 0⟩
  | .c12 => ⟨2,  0,  1,  0, 0⟩ | .c13 => ⟨2,  1,  0, -1, 0⟩

inductive PreContactKindG
  | p00 | p01 | p02 | p03 | p04 | p05 | p06 | p07
  | p08 | p09 | p10 | p11 | p12 | p13 | p14 | p15
  deriving DecidableEq, Fintype

def preContactNodeG : PreContactKindG → ClassIINodeG
  | .p00 => ⟨0, -1,  1,  1, 1⟩ | .p01 => ⟨0,  0,  0,  0, 1⟩
  | .p02 => ⟨0,  0,  0,  0, 2⟩ | .p03 => ⟨0,  0,  0,  1, 0⟩
  | .p04 => ⟨0,  0,  0,  1, 1⟩ | .p05 => ⟨0,  0,  1,  0, 0⟩
  | .p06 => ⟨0,  1, -1,  0, 0⟩ | .p07 => ⟨1,  0,  0,  0, 2⟩
  | .p08 => ⟨1,  0,  0,  1, 1⟩ | .p09 => ⟨1,  1, -1,  0, 0⟩
  | .p10 => ⟨1,  1, -1,  0, 2⟩ | .p11 => ⟨1,  1,  0, -1, 0⟩
  | .p12 => ⟨2,  0,  1, -1, 0⟩ | .p13 => ⟨2,  0,  1, -1, 1⟩
  | .p14 => ⟨2,  0,  1,  0, 0⟩ | .p15 => ⟨2,  1,  0, -1, 0⟩

inductive DContKindG
  | d00 | d01 | d02 | d03 | d04 | d05 | d06 | d07 | d08
  deriving DecidableEq, Fintype

def dContNodeG : DContKindG → ClassIINodeG
  | .d00 => ⟨0, 0,  0,  1, 0⟩ | .d01 => ⟨0, 0,  1,  0, 0⟩
  | .d02 => ⟨0, 0,  0,  0, 1⟩ | .d03 => ⟨0, 0,  0,  0, 2⟩
  | .d04 => ⟨1, 1, -1,  0, 0⟩ | .d05 => ⟨1, 0,  0,  1, 1⟩
  | .d06 => ⟨1, 0,  0,  0, 2⟩ | .d07 => ⟨2, 1,  0, -1, 0⟩
  | .d08 => ⟨2, 0,  1, -1, 1⟩

inductive DContFaceCandidateKindG
  | f00 | f01 | f02 | f03 | f04 | f05 | f06 | f07 | f08 | f09
  | f10 | f11 | f12 | f13 | f14 | f15 | f16 | f17 | f18 | f19
  | f20 | f21 | f22 | f23 | f24 | f25 | f26 | f27 | f28 | f29
  | f30 | f31 | f32
  deriving DecidableEq, Fintype

def dContFaceCandidateNodeG : DContFaceCandidateKindG → ClassIINodeG
  | .f00 => ⟨0,  0, -1,  0, 0⟩ | .f01 => ⟨0,  0,  0, -1, 0⟩
  | .f02 => ⟨0,  0,  0,  1, 0⟩ | .f03 => ⟨0,  0,  1,  0, 0⟩
  | .f04 => ⟨0, -1,  0,  0, 1⟩ | .f05 => ⟨0, -1,  1,  0, 1⟩
  | .f06 => ⟨0,  0,  0,  0, 1⟩ | .f07 => ⟨0,  0,  1,  0, 1⟩
  | .f08 => ⟨0, -1,  0,  0, 2⟩ | .f09 => ⟨0, -1,  0,  1, 2⟩
  | .f10 => ⟨0,  0,  0,  0, 2⟩ | .f11 => ⟨0,  0,  0,  1, 2⟩
  | .f12 => ⟨1,  0, -1,  0, 0⟩ | .f13 => ⟨1,  1, -1,  0, 0⟩
  | .f14 => ⟨1,  1,  0,  0, 0⟩ | .f15 => ⟨1, -1,  0,  0, 1⟩
  | .f16 => ⟨1,  0,  0, -1, 1⟩ | .f17 => ⟨1,  0,  0,  1, 1⟩
  | .f18 => ⟨1,  1,  0,  0, 1⟩ | .f19 => ⟨1,  0, -1,  0, 2⟩
  | .f20 => ⟨1,  0, -1,  1, 2⟩ | .f21 => ⟨1,  0,  0,  0, 2⟩
  | .f22 => ⟨1,  0,  0,  1, 2⟩ | .f23 => ⟨2,  0,  0, -1, 0⟩
  | .f24 => ⟨2,  1,  0, -1, 0⟩ | .f25 => ⟨2,  1,  0,  0, 0⟩
  | .f26 => ⟨2,  0,  0, -1, 1⟩ | .f27 => ⟨2,  0,  1, -1, 1⟩
  | .f28 => ⟨2,  0,  1,  0, 1⟩ | .f29 => ⟨2, -1,  0,  0, 2⟩
  | .f30 => ⟨2,  0, -1,  0, 2⟩ | .f31 => ⟨2,  0,  1,  0, 2⟩
  | .f32 => ⟨2,  1,  0,  0, 2⟩

/-- Mechanically emitted: every node C++ actually built for the "contact" table is in contactNodeG's range. -/
theorem class_ii_fixed_table_instance_0 :
    ∀ node ∈ [(⟨0,-1,1,1,1⟩ : ClassIINodeG), (⟨0,0,0,0,1⟩ : ClassIINodeG), (⟨0,0,0,0,2⟩ : ClassIINodeG), (⟨0,0,0,1,0⟩ : ClassIINodeG), (⟨0,0,0,1,1⟩ : ClassIINodeG), (⟨0,0,1,0,0⟩ : ClassIINodeG), (⟨0,1,-1,0,0⟩ : ClassIINodeG), (⟨1,0,0,0,2⟩ : ClassIINodeG), (⟨1,1,-1,0,0⟩ : ClassIINodeG), (⟨1,1,-1,0,2⟩ : ClassIINodeG), (⟨1,1,0,-1,0⟩ : ClassIINodeG), (⟨2,0,1,-1,0⟩ : ClassIINodeG), (⟨2,0,1,0,0⟩ : ClassIINodeG), (⟨2,1,0,-1,0⟩ : ClassIINodeG)], ∃ k : ContactKindG, contactNodeG k = node := by
  decide

/-- Mechanically emitted: every node C++ actually built for the "pre_contact" table is in preContactNodeG's range. -/
theorem class_ii_fixed_table_instance_1 :
    ∀ node ∈ [(⟨0,-1,1,1,1⟩ : ClassIINodeG), (⟨0,0,0,0,1⟩ : ClassIINodeG), (⟨0,0,0,0,2⟩ : ClassIINodeG), (⟨0,0,0,1,0⟩ : ClassIINodeG), (⟨0,0,0,1,1⟩ : ClassIINodeG), (⟨0,0,1,0,0⟩ : ClassIINodeG), (⟨0,1,-1,0,0⟩ : ClassIINodeG), (⟨1,0,0,0,2⟩ : ClassIINodeG), (⟨1,0,0,1,1⟩ : ClassIINodeG), (⟨1,1,-1,0,0⟩ : ClassIINodeG), (⟨1,1,-1,0,2⟩ : ClassIINodeG), (⟨1,1,0,-1,0⟩ : ClassIINodeG), (⟨2,0,1,-1,0⟩ : ClassIINodeG), (⟨2,0,1,-1,1⟩ : ClassIINodeG), (⟨2,0,1,0,0⟩ : ClassIINodeG), (⟨2,1,0,-1,0⟩ : ClassIINodeG)], ∃ k : PreContactKindG, preContactNodeG k = node := by
  decide

/-- Mechanically emitted: every node C++ actually built for the "d_cont" table is in dContNodeG's range. -/
theorem class_ii_fixed_table_instance_2 :
    ∀ node ∈ [(⟨0,0,0,0,1⟩ : ClassIINodeG), (⟨0,0,0,0,2⟩ : ClassIINodeG), (⟨0,0,0,1,0⟩ : ClassIINodeG), (⟨0,0,1,0,0⟩ : ClassIINodeG), (⟨1,0,0,0,2⟩ : ClassIINodeG), (⟨1,0,0,1,1⟩ : ClassIINodeG), (⟨1,1,-1,0,0⟩ : ClassIINodeG), (⟨2,0,1,-1,1⟩ : ClassIINodeG), (⟨2,1,0,-1,0⟩ : ClassIINodeG)], ∃ k : DContKindG, dContNodeG k = node := by
  decide

/-- Mechanically emitted: every node C++ actually built for the "d_cont_face_candidates" table is in dContFaceCandidateNodeG's range. -/
theorem class_ii_fixed_table_instance_3 :
    ∀ node ∈ [(⟨0,-1,0,0,1⟩ : ClassIINodeG), (⟨0,-1,0,0,2⟩ : ClassIINodeG), (⟨0,-1,0,1,2⟩ : ClassIINodeG), (⟨0,-1,1,0,1⟩ : ClassIINodeG), (⟨0,0,-1,0,0⟩ : ClassIINodeG), (⟨0,0,0,-1,0⟩ : ClassIINodeG), (⟨0,0,0,0,1⟩ : ClassIINodeG), (⟨0,0,0,0,2⟩ : ClassIINodeG), (⟨0,0,0,1,0⟩ : ClassIINodeG), (⟨0,0,0,1,2⟩ : ClassIINodeG), (⟨0,0,1,0,0⟩ : ClassIINodeG), (⟨0,0,1,0,1⟩ : ClassIINodeG), (⟨1,-1,0,0,1⟩ : ClassIINodeG), (⟨1,0,-1,0,0⟩ : ClassIINodeG), (⟨1,0,-1,0,2⟩ : ClassIINodeG), (⟨1,0,-1,1,2⟩ : ClassIINodeG), (⟨1,0,0,-1,1⟩ : ClassIINodeG), (⟨1,0,0,0,2⟩ : ClassIINodeG), (⟨1,0,0,1,1⟩ : ClassIINodeG), (⟨1,0,0,1,2⟩ : ClassIINodeG), (⟨1,1,-1,0,0⟩ : ClassIINodeG), (⟨1,1,0,0,0⟩ : ClassIINodeG), (⟨1,1,0,0,1⟩ : ClassIINodeG), (⟨2,-1,0,0,2⟩ : ClassIINodeG), (⟨2,0,-1,0,2⟩ : ClassIINodeG), (⟨2,0,0,-1,0⟩ : ClassIINodeG), (⟨2,0,0,-1,1⟩ : ClassIINodeG), (⟨2,0,1,-1,1⟩ : ClassIINodeG), (⟨2,0,1,0,1⟩ : ClassIINodeG), (⟨2,0,1,0,2⟩ : ClassIINodeG), (⟨2,1,0,-1,0⟩ : ClassIINodeG), (⟨2,1,0,0,0⟩ : ClassIINodeG), (⟨2,1,0,0,2⟩ : ClassIINodeG)], ∃ k : DContFaceCandidateKindG, dContFaceCandidateNodeG k = node := by
  decide

/- Semantic proof graph for: class_ii_fixed_tables_batch
  [0] lean.class_ii_fixed_table_certificate :: contact table, 14 concrete nodes
  [1] lean.class_ii_fixed_table_certificate :: pre_contact table, 16 concrete nodes
  [2] lean.class_ii_fixed_table_certificate :: d_cont table, 9 concrete nodes
  [3] lean.class_ii_fixed_table_certificate :: d_cont_face_candidates table, 33 concrete nodes
-/

def reflectedNodeCount : Nat := 4

end RavelGenerated
