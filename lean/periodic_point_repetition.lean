-- Author: Ravel
-- Date: 2026-08-07
--
-- The general fact behind Finding 27's leftmost-loop corollary: a
-- deterministic walk that returns to its start after L steps returns
-- to its start after every multiple of L steps too. Applied (in C++,
-- coincidence_converse_leftmost_loop.hpp) to the "always take
-- child_index=0" walk, whose every step contributes the zero landmark
-- vector by construction -- so this periodicity fact is exactly what
-- proves "gap = m*L coincides at K = m*L" for every m >= 0, no search
-- needed.

import Mathlib

namespace RavelGenerated

/-- If `f^[L] x = x`, then `f^[m*L] x = x` for every `m` -- repeating a
closed loop any number of times returns to the start. Proved by
induction on `m`. -/
theorem periodic_point_iterate_mul {α : Type*} (f : α → α) (L : ℕ) (x : α)
    (h : f^[L] x = x) : ∀ m : ℕ, f^[L * m] x = x := by
  intro m
  induction m with
  | zero => simp
  | succ m ih =>
      rw [Nat.mul_succ, Function.iterate_add_apply, h, ih]

end RavelGenerated
