import Mathlib.LinearAlgebra.Matrix.Determinant.Basic
import Mathlib.Tactic

namespace RavelMath

open Matrix

/-- The reflected r-family: `X` is on the strict subdiagonal and `-1` is
on the diagonal. This corrects the old superdiagonal condition. -/
noncomputable def rMatrix (n : ℕ) :
    Matrix (Fin (n - 1)) (Fin (n - 1)) (Polynomial ℤ) :=
  fun i j =>
    if j.val + 1 = i.val then Polynomial.X
    else if i.val = j.val then Polynomial.C (-1)
    else 0

lemma rMatrix_lowerTriangular (n : ℕ) :
    (rMatrix n).BlockTriangular toDual := by
  intro i j hij
  simp only [rMatrix]
  have hlt : i.val < j.val := by
    simpa [Fin.lt_iff_val_lt_val] using hij
  have hsub : j.val + 1 ≠ i.val := by omega
  have hdiag : i.val ≠ j.val := Nat.ne_of_lt hlt
  simp [hsub, hdiag]

theorem rMatrix_det (n : ℕ) :
    (rMatrix n).det = (Polynomial.C (-1) : Polynomial ℤ) ^ (n - 1) := by
  rw [Matrix.det_of_lowerTriangular (rMatrix_lowerTriangular n)]
  simp [rMatrix, Finset.prod_const, Finset.card_univ, Fintype.card_fin]

end RavelMath
