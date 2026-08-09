import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem affine_block_translation
    {n : Type} [Fintype n] [DecidableEq n]
    (B : Matrix n n ℤ) (x t forcing : n → ℤ) :
    B *ᵥ (x + t) + forcing =
      (B *ᵥ x + forcing) + B *ᵥ t := by
  ext i
  simp [Matrix.mulVec_add]
  ring

theorem affine_block_same_translation_defect
    {n : Type} [Fintype n] [DecidableEq n]
    (B : Matrix n n ℤ) (x t forcing : n → ℤ) :
    (B *ᵥ (x + t) + forcing) -
        ((B *ᵥ x + forcing) + t)
      = B *ᵥ t - t := by
  rw [affine_block_translation]
  ext i
  simp

/-- Mechanically emitted: 2x2 n-bonacci-shaped block, dim 2 -- ravel::proof::certify_translation_defect independently
    computed this dimension-2 instance's translated output, base-plus-linear-translation value, and
    their difference from raw integer matrix/vector arithmetic. -/
def instanceB0 : Matrix (Fin 2) (Fin 2) ℤ :=
  !![2, 1; 0, 1]
def instanceX0 : Fin 2 → ℤ := ![(3 : ℤ), (-2 : ℤ)]
def instanceT0 : Fin 2 → ℤ := ![(5 : ℤ), (5 : ℤ)]
def instanceF0 : Fin 2 → ℤ := ![(1 : ℤ), (-1 : ℤ)]

theorem radial_translation_defect_instance_0 :
    ((instanceB0 *ᵥ (instanceX0 + instanceT0) + instanceF0) -
        ((instanceB0 *ᵥ instanceX0 + instanceF0) + instanceT0))
      = instanceB0 *ᵥ instanceT0 - instanceT0 :=
  affine_block_same_translation_defect instanceB0 instanceX0 instanceT0 instanceF0

theorem radial_translation_defect_instance_0_value :
    instanceB0 *ᵥ instanceT0 - instanceT0 = ![(10 : ℤ), (0 : ℤ)] := by decide

/-- Mechanically emitted: 3x3 upper-triangular block, dim 3 -- ravel::proof::certify_translation_defect independently
    computed this dimension-3 instance's translated output, base-plus-linear-translation value, and
    their difference from raw integer matrix/vector arithmetic. -/
def instanceB1 : Matrix (Fin 3) (Fin 3) ℤ :=
  !![1, 0, 2; 0, 1, 1; 0, 0, 1]
def instanceX1 : Fin 3 → ℤ := ![(1 : ℤ), (1 : ℤ), (1 : ℤ)]
def instanceT1 : Fin 3 → ℤ := ![(2 : ℤ), (-1 : ℤ), (3 : ℤ)]
def instanceF1 : Fin 3 → ℤ := ![(0 : ℤ), (0 : ℤ), (0 : ℤ)]

theorem radial_translation_defect_instance_1 :
    ((instanceB1 *ᵥ (instanceX1 + instanceT1) + instanceF1) -
        ((instanceB1 *ᵥ instanceX1 + instanceF1) + instanceT1))
      = instanceB1 *ᵥ instanceT1 - instanceT1 :=
  affine_block_same_translation_defect instanceB1 instanceX1 instanceT1 instanceF1

theorem radial_translation_defect_instance_1_value :
    instanceB1 *ᵥ instanceT1 - instanceT1 = ![(6 : ℤ), (3 : ℤ), (0 : ℤ)] := by decide

/- Semantic proof graph for: radial_translation_defect_batch
  [0] lean.radial_translation_defect_certificate :: dim=2 2x2 n-bonacci-shaped block, dim 2 -- instantiates affine_block_same_translation_defect
  [1] lean.radial_translation_defect_certificate :: dim=3 3x3 upper-triangular block, dim 3 -- instantiates affine_block_same_translation_defect
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
