import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

def adjacentUnequalCount (words : List (List Nat)) : Nat :=
  (words.map (fun w => ((w.zip w.tail).filter (fun p => p.1 ≠ p.2)).length)).sum

/-- Mechanically emitted: Class-II sigma_{2,3}: 0->0^2 1^3 2, 1->0^2 2, 2->0 (class_ii). -/
theorem adjacent_swap_count_instance_0 :
    adjacentUnequalCount [[0, 0, 1, 1, 1, 2], [0, 0, 2], [0]] = 3 := by decide

/-- Mechanically emitted: Class-II sigma_{1,1}: 0->0 1 2, 1->0 2, 2->0 (class_ii). -/
theorem adjacent_swap_count_instance_1 :
    adjacentUnequalCount [[0, 1, 2], [0, 2], [0]] = 3 := by decide

/-- Mechanically emitted: tribonacci (n=3) (nbonacci). -/
theorem adjacent_swap_count_instance_2 :
    adjacentUnequalCount [[0, 1], [0, 2], [0]] = 2 := by decide

/-- Mechanically emitted: 5-bonacci (n=5) (nbonacci). -/
theorem adjacent_swap_count_instance_3 :
    adjacentUnequalCount [[0, 1], [0, 2], [0, 3], [0, 4], [0]] = 4 := by decide

/- Semantic proof graph for: adjacent_swap_count_batch
  [0] lean.adjacent_swap_count_certificate :: class_ii count=3 Class-II sigma_{2,3}: 0->0^2 1^3 2, 1->0^2 2, 2->0 -- instantiates adjacentUnequalCount
  [1] lean.adjacent_swap_count_certificate :: class_ii count=3 Class-II sigma_{1,1}: 0->0 1 2, 1->0 2, 2->0 -- instantiates adjacentUnequalCount
  [2] lean.adjacent_swap_count_certificate :: nbonacci count=2 tribonacci (n=3) -- instantiates adjacentUnequalCount
  [3] lean.adjacent_swap_count_certificate :: nbonacci count=4 5-bonacci (n=5) -- instantiates adjacentUnequalCount
-/

def reflectedNodeCount : Nat := 4

end RavelGenerated
