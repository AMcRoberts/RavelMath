import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure ClassIINodeITG where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

def neighbor2InteriorTipG (r : Int) : ClassIINodeITG :=
  ⟨2, -r, r, -1, 0⟩

/-- Reproduced from the independently kernel-checked
    `lean/class_ii_neighbor2_extensions.lean` (not re-derived here). -/
theorem neighbor2InteriorTipG_injective :
    Function.Injective neighbor2InteriorTipG := by
  intro r s h
  simp [neighbor2InteriorTipG, ClassIINodeITG.mk.injEq] at h
  exact h

/-- Mechanically emitted: the concrete node C++ actually built at r=2 equals neighbor2InteriorTipG at that r. -/
theorem class_ii_interior_tip_instance_0 :
    (⟨2,-2,2,-1,0⟩ : ClassIINodeITG) = neighbor2InteriorTipG (2 : Int) := by
  decide

/-- Mechanically emitted: the concrete node C++ actually built at r=20 equals neighbor2InteriorTipG at that r. -/
theorem class_ii_interior_tip_instance_1 :
    (⟨2,-20,20,-1,0⟩ : ClassIINodeITG) = neighbor2InteriorTipG (20 : Int) := by
  decide

/- Semantic proof graph for: class_ii_interior_tip_batch
  [0] lean.class_ii_interior_tip_certificate :: r=2 -- instantiates neighbor2InteriorTip
  [1] lean.class_ii_interior_tip_certificate :: r=20 -- instantiates neighbor2InteriorTip
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
