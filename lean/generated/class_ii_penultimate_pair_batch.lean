import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure ClassIINodePPG where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

def neighbor2PenultimatePairG (a : Int) : List ClassIINodePPG :=
  [⟨2, -(a-1), a-1, -1, 0⟩, ⟨2, -(a-2), a-2, -2, 0⟩]

/-- Reproduced from the independently kernel-checked
    `lean/class_ii_neighbor2_extensions.lean` (not re-derived here). -/
theorem neighbor2PenultimatePairG_length (a : Int) :
    (neighbor2PenultimatePairG a).length = 2 := by
  unfold neighbor2PenultimatePairG; rfl

/-- Mechanically emitted: the concrete penultimate pair C++ actually
    built at a=4 equals neighbor2PenultimatePairG at that a. -/
theorem class_ii_penultimate_pair_instance_0 :
    ([(⟨2,-3,3,-1,0⟩ : ClassIINodePPG), (⟨2,-2,2,-2,0⟩ : ClassIINodePPG)] : List ClassIINodePPG).toFinset = (neighbor2PenultimatePairG (4 : Int)).toFinset := by
  decide

/-- Mechanically emitted: the concrete penultimate pair C++ actually
    built at a=20 equals neighbor2PenultimatePairG at that a. -/
theorem class_ii_penultimate_pair_instance_1 :
    ([(⟨2,-19,19,-1,0⟩ : ClassIINodePPG), (⟨2,-18,18,-2,0⟩ : ClassIINodePPG)] : List ClassIINodePPG).toFinset = (neighbor2PenultimatePairG (20 : Int)).toFinset := by
  decide

/- Semantic proof graph for: class_ii_penultimate_pair_batch
  [0] lean.class_ii_penultimate_pair_certificate :: a=4 -- instantiates neighbor2PenultimatePair
  [1] lean.class_ii_penultimate_pair_certificate :: a=20 -- instantiates neighbor2PenultimatePair
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
