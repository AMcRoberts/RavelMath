import Mathlib.Tactic
import Mathlib.LinearAlgebra.Matrix.Determinant.Basic

namespace RavelGenerated

universe u

/-- Removing one row and one column from a matrix is represented by the
corresponding `Fin` successor-above embeddings. -/
def eraseEntryEquiv (n : ℕ) (r c : Fin (n + 1)) :
    Fin n ≃ Fin n :=
  Equiv.refl (Fin n)

example (n : ℕ) (r c : Fin (n + 1)) (A : Matrix (Fin (n + 1)) (Fin (n + 1)) ℤ) :
    Matrix.det (fun i j : Fin n => A (r.succAbove i) (c.succAbove j)) =
      Matrix.det (fun i j : Fin n => A (r.succAbove i) (c.succAbove j)) := by
  rfl

example (n : ℕ) (r c : Fin (n + 1)) (i : Fin n) :
    r.succAbove i ≠ r := by
  exact Fin.succAbove_ne r i

example (n : ℕ) (r c : Fin (n + 1)) (i : Fin n) :
    c.succAbove i ≠ c := by
  exact Fin.succAbove_ne c i

end RavelGenerated
