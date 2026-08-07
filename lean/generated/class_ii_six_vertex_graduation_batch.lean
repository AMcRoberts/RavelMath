import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure Node5G where
  i : Int
  x0 : Int
  x1 : Int
  x2 : Int
  j : Int
  deriving DecidableEq

def promotedNodesG (q : Int) : List Node5G :=
  [⟨0, q - 2, -(q - 2), 2, 1⟩,
   ⟨2, -(q - 2), q - 2, -2, 0⟩,
   ⟨2, -(q - 1), q - 1, -1, 1⟩,
   ⟨2, -q, q, -1, 1⟩,
   ⟨2, -q, q, -2, 0⟩,
   ⟨2, q - 2, -(q - 2), 1, 1⟩]

def transferredNodeG (q : Int) : Node5G :=
  ⟨2, -(q - 1), q - 1, -2, 0⟩

/-- Reproduced from the independently kernel-checked
    `lean/class_ii_six_vertex_graduation.lean` (not re-derived here). -/
theorem promotedNodesG_nodup (q : Int) (_hq : 4 ≤ q) :
    (promotedNodesG q).Nodup := by
  unfold promotedNodesG; simp [Node5G.mk.injEq]

theorem promoted_disjoint_transferredG (q : Int) (_hq : 4 ≤ q) :
    (promotedNodesG q).all (· ≠ transferredNodeG q) := by
  unfold promotedNodesG transferredNodeG; simp [Node5G.mk.injEq] <;> omega

/-- Mechanically emitted: the concrete promoted/transferred nodes C++
    actually built at a=5 equal promotedNodesG/transferredNodeG
    at q=4 exactly, so the already-proven general Nodup/disjoint
    facts (above) transfer to this concrete data. -/
theorem class_ii_six_vertex_graduation_instance_0 :
    ([(⟨0,2,-2,2,1⟩ : Node5G), (⟨2,-4,4,-2,0⟩ : Node5G), (⟨2,-4,4,-1,1⟩ : Node5G), (⟨2,-3,3,-1,1⟩ : Node5G), (⟨2,-2,2,-2,0⟩ : Node5G), (⟨2,2,-2,1,1⟩ : Node5G)] : List Node5G).toFinset = (promotedNodesG (4 : Int)).toFinset ∧
    (⟨2,-3,3,-2,0⟩ : Node5G) = transferredNodeG (4 : Int) ∧
    (promotedNodesG (4 : Int)).Nodup ∧
    (promotedNodesG (4 : Int)).all (· ≠ transferredNodeG (4 : Int)) := by
  refine ⟨by decide, by decide, ?_, ?_⟩
  · exact promotedNodesG_nodup (4 : Int) (by norm_num)
  · exact promoted_disjoint_transferredG (4 : Int) (by norm_num)

/-- Mechanically emitted: the concrete promoted/transferred nodes C++
    actually built at a=11 equal promotedNodesG/transferredNodeG
    at q=10 exactly, so the already-proven general Nodup/disjoint
    facts (above) transfer to this concrete data. -/
theorem class_ii_six_vertex_graduation_instance_1 :
    ([(⟨0,8,-8,2,1⟩ : Node5G), (⟨2,-10,10,-2,0⟩ : Node5G), (⟨2,-10,10,-1,1⟩ : Node5G), (⟨2,-9,9,-1,1⟩ : Node5G), (⟨2,-8,8,-2,0⟩ : Node5G), (⟨2,8,-8,1,1⟩ : Node5G)] : List Node5G).toFinset = (promotedNodesG (10 : Int)).toFinset ∧
    (⟨2,-9,9,-2,0⟩ : Node5G) = transferredNodeG (10 : Int) ∧
    (promotedNodesG (10 : Int)).Nodup ∧
    (promotedNodesG (10 : Int)).all (· ≠ transferredNodeG (10 : Int)) := by
  refine ⟨by decide, by decide, ?_, ?_⟩
  · exact promotedNodesG_nodup (10 : Int) (by norm_num)
  · exact promoted_disjoint_transferredG (10 : Int) (by norm_num)

/- Semantic proof graph for: class_ii_six_vertex_graduation_batch
  [0] lean.class_ii_six_vertex_graduation_certificate :: a=5 -- instantiates promotedNodes/transferredNode at q=4
  [1] lean.class_ii_six_vertex_graduation_certificate :: a=11 -- instantiates promotedNodes/transferredNode at q=10
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
