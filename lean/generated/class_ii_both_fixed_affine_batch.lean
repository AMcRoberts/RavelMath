import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem affine_no_solution_at_or_above_threshold
    (const_ slope target a0 threshold : ℤ)
    (hslope : slope ≠ 0)
    (ha0 : const_ + a0 * slope = target)
    (hbelow : a0 < threshold) :
    ∀ a : ℤ, threshold ≤ a → const_ + a * slope ≠ target := by
  intro a ha hcontra
  have heq : a0 * slope = a * slope := by linarith [ha0, hcontra]
  have : a0 = a := by
    have := mul_right_cancel₀ hslope heq
    linarith [this]
  omega

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=-1, slope=-1, target=-1, a_required=0. -/
theorem both_fixed_affine_instance_0 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (-1 : ℤ) + a * (-1 : ℤ) ≠ (-1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (-1 : ℤ) (-1 : ℤ) (-1 : ℤ) (0 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=0, slope=-1, target=-1, a_required=1. -/
theorem both_fixed_affine_instance_1 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (0 : ℤ) + a * (-1 : ℤ) ≠ (-1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (0 : ℤ) (-1 : ℤ) (-1 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=-1, slope=1, target=0, a_required=1. -/
theorem both_fixed_affine_instance_2 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (-1 : ℤ) + a * (1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (-1 : ℤ) (1 : ℤ) (0 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=-1, target=0, a_required=1. -/
theorem both_fixed_affine_instance_3 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (-1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (-1 : ℤ) (0 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=1, target=0, a_required=-1. -/
theorem both_fixed_affine_instance_4 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (1 : ℤ) (0 : ℤ) (-1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=-1, target=0, a_required=1. -/
theorem both_fixed_affine_instance_5 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (-1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (-1 : ℤ) (0 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=1, target=-1, a_required=-2. -/
theorem both_fixed_affine_instance_6 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (1 : ℤ) ≠ (-1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (1 : ℤ) (-1 : ℤ) (-2 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=2, slope=-1, target=-1, a_required=3. -/
theorem both_fixed_affine_instance_7 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (2 : ℤ) + a * (-1 : ℤ) ≠ (-1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (2 : ℤ) (-1 : ℤ) (-1 : ℤ) (3 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=-1, slope=-1, target=-2, a_required=1. -/
theorem both_fixed_affine_instance_8 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (-1 : ℤ) + a * (-1 : ℤ) ≠ (-2 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (-1 : ℤ) (-1 : ℤ) (-2 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=-1, slope=-1, target=-1, a_required=0. -/
theorem both_fixed_affine_instance_9 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (-1 : ℤ) + a * (-1 : ℤ) ≠ (-1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (-1 : ℤ) (-1 : ℤ) (-1 : ℤ) (0 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=0, slope=-1, target=0, a_required=0. -/
theorem both_fixed_affine_instance_10 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (0 : ℤ) + a * (-1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (0 : ℤ) (-1 : ℤ) (0 : ℤ) (0 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=1, target=2, a_required=1. -/
theorem both_fixed_affine_instance_11 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (1 : ℤ) ≠ (2 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (1 : ℤ) (2 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=1, target=2, a_required=1. -/
theorem both_fixed_affine_instance_12 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (1 : ℤ) ≠ (2 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (1 : ℤ) (2 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=-3, slope=-1, target=-1, a_required=-2. -/
theorem both_fixed_affine_instance_13 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (-3 : ℤ) + a * (-1 : ℤ) ≠ (-1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (-3 : ℤ) (-1 : ℤ) (-1 : ℤ) (-2 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=-1, slope=-1, target=0, a_required=-1. -/
theorem both_fixed_affine_instance_14 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (-1 : ℤ) + a * (-1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (-1 : ℤ) (-1 : ℤ) (0 : ℤ) (-1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=-2, slope=1, target=0, a_required=2. -/
theorem both_fixed_affine_instance_15 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (-2 : ℤ) + a * (1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (-2 : ℤ) (1 : ℤ) (0 : ℤ) (2 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=-1, target=-1, a_required=2. -/
theorem both_fixed_affine_instance_16 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (-1 : ℤ) ≠ (-1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (-1 : ℤ) (-1 : ℤ) (2 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=0, slope=-1, target=0, a_required=0. -/
theorem both_fixed_affine_instance_17 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (0 : ℤ) + a * (-1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (0 : ℤ) (-1 : ℤ) (0 : ℤ) (0 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=1, slope=-1, target=0, a_required=1. -/
theorem both_fixed_affine_instance_18 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (1 : ℤ) + a * (-1 : ℤ) ≠ (0 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (1 : ℤ) (-1 : ℤ) (0 : ℤ) (1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=0, slope=-1, target=1, a_required=-1. -/
theorem both_fixed_affine_instance_19 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (0 : ℤ) + a * (-1 : ℤ) ≠ (1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (0 : ℤ) (-1 : ℤ) (1 : ℤ) (-1 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/-- Mechanically emitted: instantiates the general lemma above for
    CONST=3, slope=1, target=1, a_required=-2. -/
theorem both_fixed_affine_instance_20 :
    ∀ a : ℤ, (7 : ℤ) ≤ a → (3 : ℤ) + a * (1 : ℤ) ≠ (1 : ℤ) := by
  apply affine_no_solution_at_or_above_threshold (3 : ℤ) (1 : ℤ) (1 : ℤ) (-2 : ℤ) (7 : ℤ)
  · norm_num
  · norm_num
  · norm_num

/- Semantic proof graph for: both_fixed_affine_batch
  [0] lean.both_fixed_affine_certificate :: const=-1 slope=-1 target=-1 a_required=0 -- instantiates affine_no_solution_at_or_above_threshold
  [1] lean.both_fixed_affine_certificate :: const=0 slope=-1 target=-1 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [2] lean.both_fixed_affine_certificate :: const=-1 slope=1 target=0 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [3] lean.both_fixed_affine_certificate :: const=1 slope=-1 target=0 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [4] lean.both_fixed_affine_certificate :: const=1 slope=1 target=0 a_required=-1 -- instantiates affine_no_solution_at_or_above_threshold
  [5] lean.both_fixed_affine_certificate :: const=1 slope=-1 target=0 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [6] lean.both_fixed_affine_certificate :: const=1 slope=1 target=-1 a_required=-2 -- instantiates affine_no_solution_at_or_above_threshold
  [7] lean.both_fixed_affine_certificate :: const=2 slope=-1 target=-1 a_required=3 -- instantiates affine_no_solution_at_or_above_threshold
  [8] lean.both_fixed_affine_certificate :: const=-1 slope=-1 target=-2 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [9] lean.both_fixed_affine_certificate :: const=-1 slope=-1 target=-1 a_required=0 -- instantiates affine_no_solution_at_or_above_threshold
  [10] lean.both_fixed_affine_certificate :: const=0 slope=-1 target=0 a_required=0 -- instantiates affine_no_solution_at_or_above_threshold
  [11] lean.both_fixed_affine_certificate :: const=1 slope=1 target=2 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [12] lean.both_fixed_affine_certificate :: const=1 slope=1 target=2 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [13] lean.both_fixed_affine_certificate :: const=-3 slope=-1 target=-1 a_required=-2 -- instantiates affine_no_solution_at_or_above_threshold
  [14] lean.both_fixed_affine_certificate :: const=-1 slope=-1 target=0 a_required=-1 -- instantiates affine_no_solution_at_or_above_threshold
  [15] lean.both_fixed_affine_certificate :: const=-2 slope=1 target=0 a_required=2 -- instantiates affine_no_solution_at_or_above_threshold
  [16] lean.both_fixed_affine_certificate :: const=1 slope=-1 target=-1 a_required=2 -- instantiates affine_no_solution_at_or_above_threshold
  [17] lean.both_fixed_affine_certificate :: const=0 slope=-1 target=0 a_required=0 -- instantiates affine_no_solution_at_or_above_threshold
  [18] lean.both_fixed_affine_certificate :: const=1 slope=-1 target=0 a_required=1 -- instantiates affine_no_solution_at_or_above_threshold
  [19] lean.both_fixed_affine_certificate :: const=0 slope=-1 target=1 a_required=-1 -- instantiates affine_no_solution_at_or_above_threshold
  [20] lean.both_fixed_affine_certificate :: const=3 slope=1 target=1 a_required=-2 -- instantiates affine_no_solution_at_or_above_threshold
-/

def reflectedNodeCount : Nat := 21

end RavelGenerated
