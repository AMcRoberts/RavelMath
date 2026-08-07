import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

structure AffineEdgeG where
  source : Nat
  target : Nat
  intercept : Int
  slope : Int
  deriving DecidableEq

def neighbor0EdgeListG : List AffineEdgeG :=
  [⟨0, 4, 1, 0⟩, ⟨0, 5, 1, 0⟩,
   ⟨1, 9, -2, 1⟩, ⟨1, 10, -3, 1⟩, ⟨1, 12, -2, 1⟩, ⟨1, 14, 1, 0⟩,
   ⟨2, 9, 1, 0⟩, ⟨2, 10, 1, 0⟩, ⟨2, 12, 1, 0⟩,
   ⟨3, 6, 1, 0⟩, ⟨3, 9, 1, 0⟩,
   ⟨4, 0, -2, 1⟩, ⟨4, 1, 0, 1⟩, ⟨4, 2, -1, 1⟩, ⟨4, 3, 1, 0⟩,
   ⟨4, 11, 1, 0⟩, ⟨5, 7, 1, 0⟩, ⟨5, 13, 1, 0⟩,
   ⟨6, 1, 1, 0⟩, ⟨6, 11, 1, 0⟩, ⟨7, 5, 1, 0⟩,
   ⟨8, 6, 1, 0⟩, ⟨8, 9, 0, 1⟩, ⟨8, 10, -2, 1⟩, ⟨8, 12, -1, 1⟩,
   ⟨8, 14, 1, 0⟩,
   ⟨9, 0, -3, 1⟩, ⟨9, 1, -2, 1⟩, ⟨9, 2, -2, 1⟩, ⟨9, 3, 1, 0⟩,
   ⟨10, 7, 1, 0⟩, ⟨10, 8, 1, 0⟩, ⟨11, 6, 1, 0⟩, ⟨11, 9, 1, 0⟩,
   ⟨12, 0, 1, 0⟩, ⟨12, 1, 1, 0⟩, ⟨12, 2, 1, 0⟩,
   ⟨13, 4, 1, 0⟩, ⟨13, 5, 1, 0⟩, ⟨14, 1, 1, 0⟩, ⟨14, 11, 1, 0⟩]

def neighbor1EdgeListG : List AffineEdgeG :=
  [⟨0, 9, 1, 0⟩, ⟨0, 12, 1, 0⟩,
   ⟨1, 9, -1, 1⟩, ⟨1, 12, -2, 1⟩, ⟨1, 14, 1, 0⟩,
   ⟨2, 15, 1, 0⟩, ⟨3, 9, 1, 0⟩, ⟨3, 12, 1, 0⟩,
   ⟨4, 15, 1, 0⟩, ⟨4, 16, 1, 0⟩, ⟨5, 10, 1, 0⟩,
   ⟨6, 7, 1, 0⟩, ⟨6, 11, 1, 0⟩, ⟨7, 8, 1, 0⟩, ⟨7, 13, 1, 0⟩,
   ⟨8, 12, 1, 0⟩,
   ⟨9, 0, -2, 1⟩, ⟨9, 1, -1, 1⟩, ⟨9, 2, -1, 1⟩, ⟨9, 3, 1, 0⟩,
   ⟨9, 4, 1, 0⟩, ⟨10, 0, 1, 0⟩, ⟨11, 5, 1, 0⟩, ⟨11, 6, 1, 0⟩,
   ⟨12, 0, 1, 0⟩, ⟨12, 1, 1, 0⟩, ⟨12, 2, 1, 0⟩, ⟨13, 9, 1, 0⟩,
   ⟨14, 0, 1, 0⟩, ⟨14, 1, 1, 0⟩, ⟨14, 2, 1, 0⟩, ⟨15, 10, 1, 0⟩,
   ⟨16, 7, 1, 0⟩, ⟨16, 11, 1, 0⟩]

def neighbor2EdgeListG : List AffineEdgeG :=
  [⟨0, 21, 1, 0⟩, ⟨1, 22, 1, 0⟩, ⟨1, 34, 1, 0⟩, ⟨2, 26, 1, 0⟩,
   ⟨3, 15, 0, 1⟩, ⟨3, 16, -1, 1⟩, ⟨3, 18, -2, 1⟩, ⟨3, 24, -1, 1⟩,
   ⟨3, 25, -1, 1⟩, ⟨3, 26, -2, 1⟩, ⟨3, 36, 1, 0⟩,
   ⟨4, 16, 1, 0⟩, ⟨4, 18, 1, 0⟩, ⟨5, 26, 1, 0⟩, ⟨6, 24, 1, 0⟩,
   ⟨7, 19, 1, 0⟩, ⟨7, 25, 1, 0⟩, ⟨7, 28, 1, 0⟩, ⟨7, 38, 1, 0⟩,
   ⟨8, 29, 1, 0⟩,
   ⟨9, 2, -1, 1⟩, ⟨9, 3, 0, 1⟩, ⟨9, 4, -1, 1⟩, ⟨9, 5, 1, 0⟩,
   ⟨9, 6, -1, 1⟩, ⟨9, 7, -2, 1⟩, ⟨9, 20, 0, 1⟩, ⟨9, 30, 1, 0⟩,
   ⟨9, 31, 1, 0⟩, ⟨9, 32, 1, 0⟩,
   ⟨10, 11, 1, 0⟩, ⟨10, 13, 1, 0⟩, ⟨11, 12, 1, 0⟩,
   ⟨12, 21, 1, 0⟩, ⟨12, 22, 1, 0⟩, ⟨12, 34, 1, 0⟩,
   ⟨13, 14, 1, 0⟩, ⟨13, 23, 1, 0⟩, ⟨13, 35, 1, 0⟩,
   ⟨14, 15, -1, 1⟩, ⟨14, 16, 0, 1⟩, ⟨14, 17, 1, 0⟩,
   ⟨14, 18, -1, 1⟩, ⟨14, 24, -2, 1⟩, ⟨14, 25, -1, 1⟩,
   ⟨14, 26, 0, 1⟩, ⟨14, 27, 1, 0⟩, ⟨14, 36, 1, 0⟩, ⟨14, 37, 1, 0⟩,
   ⟨15, 7, 1, 0⟩,
   ⟨16, 2, -2, 1⟩, ⟨16, 3, -1, 1⟩, ⟨16, 4, -1, 1⟩, ⟨16, 5, 1, 0⟩,
   ⟨16, 6, 0, 1⟩, ⟨16, 7, -1, 1⟩, ⟨16, 8, 1, 0⟩,
   ⟨16, 20, -2, 1⟩, ⟨16, 33, 1, 0⟩,
   ⟨17, 7, 1, 0⟩, ⟨18, 20, 1, 0⟩, ⟨19, 13, 1, 0⟩,
   ⟨20, 15, 1, 0⟩, ⟨21, 9, 1, 0⟩, ⟨22, 10, 1, 0⟩, ⟨23, 15, 1, 0⟩,
   ⟨24, 0, 1, 0⟩, ⟨24, 1, 1, 0⟩, ⟨24, 4, 1, 0⟩,
   ⟨25, 2, 1, 0⟩, ⟨25, 3, 1, 0⟩, ⟨26, 6, 1, 0⟩,
   ⟨27, 4, 1, 0⟩, ⟨27, 12, 1, 0⟩, ⟨28, 11, 1, 0⟩, ⟨29, 7, 1, 0⟩,
   ⟨30, 24, 1, 0⟩, ⟨31, 10, 1, 0⟩, ⟨31, 25, 1, 0⟩,
   ⟨32, 15, 1, 0⟩, ⟨33, 15, 1, 0⟩, ⟨34, 9, 1, 0⟩, ⟨34, 26, 1, 0⟩,
   ⟨35, 10, 1, 0⟩, ⟨35, 24, 1, 0⟩, ⟨35, 25, 1, 0⟩,
   ⟨36, 20, 1, 0⟩, ⟨37, 6, 1, 0⟩,
   ⟨38, 0, 1, 0⟩, ⟨38, 1, 1, 0⟩, ⟨38, 4, 1, 0⟩]

def neighbor0EdgesG : Finset AffineEdgeG := neighbor0EdgeListG.toFinset
def neighbor1EdgesG : Finset AffineEdgeG := neighbor1EdgeListG.toFinset
def neighbor2EdgesG : Finset AffineEdgeG := neighbor2EdgeListG.toFinset

def neighbor0BoundarySourceG : Finset Nat :=
  ({e ∈ neighbor0EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.source
def neighbor0BoundaryTargetG : Finset Nat :=
  ({e ∈ neighbor0EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.target
def neighbor1BoundarySourceG : Finset Nat :=
  ({e ∈ neighbor1EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.source
def neighbor1BoundaryTargetG : Finset Nat :=
  ({e ∈ neighbor1EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.target
def neighbor2BoundarySourceG : Finset Nat :=
  ({e ∈ neighbor2EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.source
def neighbor2BoundaryTargetG : Finset Nat :=
  ({e ∈ neighbor2EdgesG | e.slope ≠ 0} : Finset AffineEdgeG).image AffineEdgeG.target

/-- Reproduced from the independently kernel-checked
    `lean/class_ii_neighbor_d_support.lean` (not re-derived here). -/
theorem neighbor0BoundarySourceG_eq : neighbor0BoundarySourceG = {1, 4, 8, 9} := by
  unfold neighbor0BoundarySourceG neighbor0EdgesG neighbor0EdgeListG; native_decide
theorem neighbor0BoundaryTargetG_eq : neighbor0BoundaryTargetG = {0, 1, 2, 9, 10, 12} := by
  unfold neighbor0BoundaryTargetG neighbor0EdgesG neighbor0EdgeListG; native_decide
theorem neighbor1BoundarySourceG_eq : neighbor1BoundarySourceG = {1, 9} := by
  unfold neighbor1BoundarySourceG neighbor1EdgesG neighbor1EdgeListG; native_decide
theorem neighbor1BoundaryTargetG_eq : neighbor1BoundaryTargetG = {0, 1, 2, 9, 12} := by
  unfold neighbor1BoundaryTargetG neighbor1EdgesG neighbor1EdgeListG; native_decide
theorem neighbor2BoundarySourceG_eq : neighbor2BoundarySourceG = {3, 9, 14, 16} := by
  unfold neighbor2BoundarySourceG neighbor2EdgesG neighbor2EdgeListG; native_decide
theorem neighbor2BoundaryTargetG_eq :
    neighbor2BoundaryTargetG = {2, 3, 4, 6, 7, 15, 16, 18, 20, 24, 25, 26} := by
  unfold neighbor2BoundaryTargetG neighbor2EdgesG neighbor2EdgeListG; native_decide

/-- Mechanically emitted: the concrete source/target index sets C++
    actually computed for neighbor 0 equal neighbor0BoundarySourceG/neighbor0BoundaryTargetG exactly, which neighbor0BoundarySourceG_eq/neighbor0BoundaryTargetG_eq (above) prove equal the literal sets. -/
theorem class_ii_neighbor_d_support_instance_0 :
    ({1, 4, 8, 9} : Finset Nat) = neighbor0BoundarySourceG ∧ ({0, 1, 2, 9, 10, 12} : Finset Nat) = neighbor0BoundaryTargetG := by
  constructor <;> native_decide

/-- Mechanically emitted: the concrete source/target index sets C++
    actually computed for neighbor 1 equal neighbor1BoundarySourceG/neighbor1BoundaryTargetG exactly, which neighbor1BoundarySourceG_eq/neighbor1BoundaryTargetG_eq (above) prove equal the literal sets. -/
theorem class_ii_neighbor_d_support_instance_1 :
    ({1, 9} : Finset Nat) = neighbor1BoundarySourceG ∧ ({0, 1, 2, 9, 12} : Finset Nat) = neighbor1BoundaryTargetG := by
  constructor <;> native_decide

/-- Mechanically emitted: the concrete source/target index sets C++
    actually computed for neighbor 2 equal neighbor2BoundarySourceG/neighbor2BoundaryTargetG exactly, which neighbor2BoundarySourceG_eq/neighbor2BoundaryTargetG_eq (above) prove equal the literal sets. -/
theorem class_ii_neighbor_d_support_instance_2 :
    ({3, 9, 14, 16} : Finset Nat) = neighbor2BoundarySourceG ∧ ({2, 3, 4, 6, 7, 15, 16, 18, 20, 24, 25, 26} : Finset Nat) = neighbor2BoundaryTargetG := by
  constructor <;> native_decide

/- Semantic proof graph for: class_ii_neighbor_d_support_batch
  [0] lean.class_ii_neighbor_d_support_certificate :: neighbor=0 sources=4 targets=6 -- instantiates neighborXBoundarySource/TargetG
  [1] lean.class_ii_neighbor_d_support_certificate :: neighbor=1 sources=2 targets=5 -- instantiates neighborXBoundarySource/TargetG
  [2] lean.class_ii_neighbor_d_support_certificate :: neighbor=2 sources=4 targets=12 -- instantiates neighborXBoundarySource/TargetG
-/

def reflectedNodeCount : Nat := 3

end RavelGenerated
