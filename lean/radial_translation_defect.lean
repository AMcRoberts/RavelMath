import Mathlib.Tactic
import Mathlib.LinearAlgebra.Matrix

namespace RavelGenerated

open Matrix

/-- Affine block dynamics transport translations by the linear block map,
not by the original translation vector. -/
theorem affine_block_translation
    {n : Type} [Fintype n] [DecidableEq n]
    (B : Matrix n n ℤ) (x t forcing : n → ℤ) :
    B *ᵥ (x + t) + forcing =
      (B *ᵥ x + forcing) + B *ᵥ t := by
  ext i
  simp [Matrix.mulVec_add]
  ring

/-- The obstruction to naive radial translation is exactly `B*t-t`. -/
theorem affine_block_same_translation_defect
    {n : Type} [Fintype n] [DecidableEq n]
    (B : Matrix n n ℤ) (x t forcing : n → ℤ) :
    (B *ᵥ (x + t) + forcing) -
        ((B *ᵥ x + forcing) + t)
      = B *ᵥ t - t := by
  rw [affine_block_translation]
  ext i
  simp
  ring

/-- For the n-bonacci block map `2A-I`, the translation defect is
`2(A*t-t)`. This is the extra phase datum required by any uniform radial
transport theorem. -/
theorem nbonacci_block_translation_defect
    {n : Type} [Fintype n] [DecidableEq n]
    (A : Matrix n n ℤ) (x t forcing : n → ℤ) :
    (((2 : ℤ) • A - 1) *ᵥ (x + t) + forcing) -
        ((((2 : ℤ) • A - 1) *ᵥ x + forcing) + t)
      = 2 • (A *ᵥ t - t) := by
  rw [affine_block_same_translation_defect]
  ext i
  simp [Matrix.sub_mulVec, Matrix.smul_mulVec]
  ring

end RavelGenerated
