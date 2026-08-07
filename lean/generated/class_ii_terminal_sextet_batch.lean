import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure ClassIINodeTSG where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

def neighbor2TerminalSextetG (a : Int) : List ClassIINodeTSG :=
  let q := a - 2
  [⟨0, q, -(q+1), 2, 2⟩, ⟨0, q, -q, 2, 1⟩, ⟨1, -q, q, -2, 0⟩,
   ⟨2, -q, q, -2, 0⟩, ⟨2, -q, q+1, -2, 0⟩, ⟨2, q, -q, 1, 1⟩]

/-- Reproduced from the independently kernel-checked
    `lean/class_ii_neighbor2_extensions.lean` (not re-derived here). -/
theorem neighbor2TerminalSextetG_length (a : Int) :
    (neighbor2TerminalSextetG a).length = 6 := by
  unfold neighbor2TerminalSextetG; rfl

/-- Mechanically emitted: the concrete terminal sextet C++ actually
    built at a=3 equals neighbor2TerminalSextetG at that a. -/
theorem class_ii_terminal_sextet_instance_0 :
    [(⟨0,1,-2,2,2⟩ : ClassIINodeTSG), (⟨0,1,-1,2,1⟩ : ClassIINodeTSG), (⟨1,-1,1,-2,0⟩ : ClassIINodeTSG), (⟨2,-1,1,-2,0⟩ : ClassIINodeTSG), (⟨2,-1,2,-2,0⟩ : ClassIINodeTSG), (⟨2,1,-1,1,1⟩ : ClassIINodeTSG)] = neighbor2TerminalSextetG (3 : Int) := by
  decide

/-- Mechanically emitted: the concrete terminal sextet C++ actually
    built at a=15 equals neighbor2TerminalSextetG at that a. -/
theorem class_ii_terminal_sextet_instance_1 :
    [(⟨0,13,-14,2,2⟩ : ClassIINodeTSG), (⟨0,13,-13,2,1⟩ : ClassIINodeTSG), (⟨1,-13,13,-2,0⟩ : ClassIINodeTSG), (⟨2,-13,13,-2,0⟩ : ClassIINodeTSG), (⟨2,-13,14,-2,0⟩ : ClassIINodeTSG), (⟨2,13,-13,1,1⟩ : ClassIINodeTSG)] = neighbor2TerminalSextetG (15 : Int) := by
  decide

/- Semantic proof graph for: class_ii_terminal_sextet_batch
  [0] lean.class_ii_terminal_sextet_certificate :: a=3 -- instantiates neighbor2TerminalSextet
  [1] lean.class_ii_terminal_sextet_certificate :: a=15 -- instantiates neighbor2TerminalSextet
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
