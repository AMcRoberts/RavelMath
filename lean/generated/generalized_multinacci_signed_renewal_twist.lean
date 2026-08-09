import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

inductive QRGenerator
  | q
  | r
  deriving DecidableEq, Repr

def defectWord : Int → List QRGenerator
  | 0 => [QRGenerator.q]
  | d => List.replicate d.natAbs QRGenerator.r

theorem defectWord_length (d : Int) :
    (defectWord d).length = if d = 0 then 1 else d.natAbs := by
  by_cases h : d = 0
  · simp [h, defectWord]
  · simp [h, defectWord]

theorem neg_defect_same_roof (d : Int) :
    (defectWord (-d)).length = (defectWord d).length := by
  by_cases h : d = 0
  · simp [h, defectWord]
  · simp [h, defectWord, Int.natAbs_neg]

theorem defect_roof_bounded (m : Nat) (d : Int)
    (h : d.natAbs ≤ m) :
    (defectWord d).length ≤ max 1 m := by
  rw [defectWord_length]
  split
  · exact Nat.le_max_left _ _
  · exact le_trans h (Nat.le_max_right _ _)

/-- Mechanically emitted: D=2 m=1 roof=1 D=2 m=1 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_0 :
    (defectWord (1 : ℤ)).length ≤ max 1 1 :=
  defect_roof_bounded 1 (1 : ℤ) (by decide)

/-- Mechanically emitted: D=3 m=1 roof=1 D=3 m=1 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_1 :
    (defectWord (1 : ℤ)).length ≤ max 1 1 :=
  defect_roof_bounded 1 (1 : ℤ) (by decide)

/-- Mechanically emitted: D=4 m=1 roof=1 D=4 m=1 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_2 :
    (defectWord (1 : ℤ)).length ≤ max 1 1 :=
  defect_roof_bounded 1 (1 : ℤ) (by decide)

/-- Mechanically emitted: D=5 m=1 roof=1 D=5 m=1 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_3 :
    (defectWord (1 : ℤ)).length ≤ max 1 1 :=
  defect_roof_bounded 1 (1 : ℤ) (by decide)

/-- Mechanically emitted: D=2 m=2 roof=0 D=2 m=2 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_4 :
    (defectWord (0 : ℤ)).length ≤ max 1 2 :=
  defect_roof_bounded 2 (0 : ℤ) (by decide)

/-- Mechanically emitted: D=3 m=2 roof=2 D=3 m=2 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_5 :
    (defectWord (2 : ℤ)).length ≤ max 1 2 :=
  defect_roof_bounded 2 (2 : ℤ) (by decide)

/-- Mechanically emitted: D=4 m=2 roof=2 D=4 m=2 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_6 :
    (defectWord (2 : ℤ)).length ≤ max 1 2 :=
  defect_roof_bounded 2 (2 : ℤ) (by decide)

/-- Mechanically emitted: D=5 m=2 roof=2 D=5 m=2 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_7 :
    (defectWord (2 : ℤ)).length ≤ max 1 2 :=
  defect_roof_bounded 2 (2 : ℤ) (by decide)

/-- Mechanically emitted: D=2 m=3 roof=0 D=2 m=3 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_8 :
    (defectWord (0 : ℤ)).length ≤ max 1 3 :=
  defect_roof_bounded 3 (0 : ℤ) (by decide)

/-- Mechanically emitted: D=3 m=3 roof=3 D=3 m=3 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_9 :
    (defectWord (3 : ℤ)).length ≤ max 1 3 :=
  defect_roof_bounded 3 (3 : ℤ) (by decide)

/-- Mechanically emitted: D=4 m=3 roof=3 D=4 m=3 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_10 :
    (defectWord (3 : ℤ)).length ≤ max 1 3 :=
  defect_roof_bounded 3 (3 : ℤ) (by decide)

/-- Mechanically emitted: D=5 m=3 roof=3 D=5 m=3 signed renewal twist instance -- ravel::proof::stage_generalized_multinacci_signed_renewal_twist
    independently reconfirmed maximum_return_time <= multiplicity. -/
theorem generalized_multinacci_signed_renewal_twist_instance_11 :
    (defectWord (3 : ℤ)).length ≤ max 1 3 :=
  defect_roof_bounded 3 (3 : ℤ) (by decide)

/- Semantic proof graph for: generalized_multinacci_signed_renewal_twist_batch
  [0] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=2 m=1 roof=1 D=2 m=1 signed renewal twist instance -- instantiates defect_roof_bounded
  [1] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=3 m=1 roof=1 D=3 m=1 signed renewal twist instance -- instantiates defect_roof_bounded
  [2] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=4 m=1 roof=1 D=4 m=1 signed renewal twist instance -- instantiates defect_roof_bounded
  [3] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=5 m=1 roof=1 D=5 m=1 signed renewal twist instance -- instantiates defect_roof_bounded
  [4] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=2 m=2 roof=0 D=2 m=2 signed renewal twist instance -- instantiates defect_roof_bounded
  [5] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=3 m=2 roof=2 D=3 m=2 signed renewal twist instance -- instantiates defect_roof_bounded
  [6] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=4 m=2 roof=2 D=4 m=2 signed renewal twist instance -- instantiates defect_roof_bounded
  [7] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=5 m=2 roof=2 D=5 m=2 signed renewal twist instance -- instantiates defect_roof_bounded
  [8] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=2 m=3 roof=0 D=2 m=3 signed renewal twist instance -- instantiates defect_roof_bounded
  [9] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=3 m=3 roof=3 D=3 m=3 signed renewal twist instance -- instantiates defect_roof_bounded
  [10] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=4 m=3 roof=3 D=4 m=3 signed renewal twist instance -- instantiates defect_roof_bounded
  [11] lean.generalized_multinacci_signed_renewal_twist_certificate :: D=5 m=3 roof=3 D=5 m=3 signed renewal twist instance -- instantiates defect_roof_bounded
-/

def reflectedNodeCount : Nat := 12

end RavelGenerated
