import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- If `f^[L] x = x`, then `f^[m*L] x = x` for every `m`. Reproduced from the
    independently kernel-checked `lean/periodic_point_repetition.lean` (not
    re-derived here). -/
theorem periodic_point_iterate_mul {α : Type*} (f : α → α) (L : ℕ) (x : α)
    (h : f^[L] x = x) : ∀ m : ℕ, f^[L * m] x = x := by
  intro m
  induction m with
  | zero => simp
  | succ m ih =>
      rw [Nat.mul_succ, Function.iterate_add_apply, h, ih]

/-- Mechanically emitted: instantiates the general lemma above for
    leftmost-branch cycle at junction 0, length 4, m=0. -/
theorem leftmost_loop_instance_0 :
    (fun (i : Fin 4) => i + 1)^[4 * 0] (0 : Fin 4) = 0 :=
  periodic_point_iterate_mul (fun (i : Fin 4) => i + 1) 4 (0 : Fin 4) (by decide) 0

/-- Mechanically emitted: instantiates the general lemma above for
    leftmost-branch cycle at junction 0, length 4, m=1. -/
theorem leftmost_loop_instance_1 :
    (fun (i : Fin 4) => i + 1)^[4 * 1] (0 : Fin 4) = 0 :=
  periodic_point_iterate_mul (fun (i : Fin 4) => i + 1) 4 (0 : Fin 4) (by decide) 1

/-- Mechanically emitted: instantiates the general lemma above for
    leftmost-branch cycle at junction 0, length 4, m=2. -/
theorem leftmost_loop_instance_2 :
    (fun (i : Fin 4) => i + 1)^[4 * 2] (0 : Fin 4) = 0 :=
  periodic_point_iterate_mul (fun (i : Fin 4) => i + 1) 4 (0 : Fin 4) (by decide) 2

/-- Mechanically emitted: instantiates the general lemma above for
    leftmost-branch cycle at junction 0, length 4, m=3. -/
theorem leftmost_loop_instance_3 :
    (fun (i : Fin 4) => i + 1)^[4 * 3] (0 : Fin 4) = 0 :=
  periodic_point_iterate_mul (fun (i : Fin 4) => i + 1) 4 (0 : Fin 4) (by decide) 3

/- Semantic proof graph for: leftmost_loop_batch
  [0] lean.leftmost_loop_certificate :: start=0 L=4 max_m=3 leftmost-branch cycle at junction 0, length 4 -- instantiates periodic_point_iterate_mul
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
