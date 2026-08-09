import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

inductive PrimitiveGenerator
  | q
  | r
  deriving DecidableEq, Repr

def defectWord : ℕ → List PrimitiveGenerator
  | 0 => [PrimitiveGenerator.q]
  | d + 1 => List.replicate (d + 1) PrimitiveGenerator.r

def schedulerCoefficient (m d : ℕ) : ℕ :=
  if d = 0 then m + 1
  else if d ≤ m then 2 * (m + 1 - d)
  else 0

theorem schedulerCoefficient_zero (m : ℕ) :
    schedulerCoefficient m 0 = m + 1 := by
  simp [schedulerCoefficient]

theorem schedulerCoefficient_positive
    (m d : ℕ) (hd0 : 0 < d) (hdm : d ≤ m) :
    schedulerCoefficient m d = 2 * (m + 1 - d) := by
  simp [schedulerCoefficient, Nat.ne_of_gt hd0, hdm]

/-- Mechanically emitted: m=1 m=1 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_0 :
    schedulerCoefficient 1 0 = 1 + 1 ∧
      ∀ d, 0 < d → d ≤ 1 → schedulerCoefficient 1 d = 2 * (1 + 1 - d) :=
  ⟨schedulerCoefficient_zero 1, fun d hd0 hdm => schedulerCoefficient_positive 1 d hd0 hdm⟩

/-- Mechanically emitted: m=2 m=2 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_1 :
    schedulerCoefficient 2 0 = 2 + 1 ∧
      ∀ d, 0 < d → d ≤ 2 → schedulerCoefficient 2 d = 2 * (2 + 1 - d) :=
  ⟨schedulerCoefficient_zero 2, fun d hd0 hdm => schedulerCoefficient_positive 2 d hd0 hdm⟩

/-- Mechanically emitted: m=3 m=3 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_2 :
    schedulerCoefficient 3 0 = 3 + 1 ∧
      ∀ d, 0 < d → d ≤ 3 → schedulerCoefficient 3 d = 2 * (3 + 1 - d) :=
  ⟨schedulerCoefficient_zero 3, fun d hd0 hdm => schedulerCoefficient_positive 3 d hd0 hdm⟩

/-- Mechanically emitted: m=4 m=4 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_3 :
    schedulerCoefficient 4 0 = 4 + 1 ∧
      ∀ d, 0 < d → d ≤ 4 → schedulerCoefficient 4 d = 2 * (4 + 1 - d) :=
  ⟨schedulerCoefficient_zero 4, fun d hd0 hdm => schedulerCoefficient_positive 4 d hd0 hdm⟩

/-- Mechanically emitted: m=5 m=5 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_4 :
    schedulerCoefficient 5 0 = 5 + 1 ∧
      ∀ d, 0 < d → d ≤ 5 → schedulerCoefficient 5 d = 2 * (5 + 1 - d) :=
  ⟨schedulerCoefficient_zero 5, fun d hd0 hdm => schedulerCoefficient_positive 5 d hd0 hdm⟩

/-- Mechanically emitted: m=6 m=6 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_5 :
    schedulerCoefficient 6 0 = 6 + 1 ∧
      ∀ d, 0 < d → d ≤ 6 → schedulerCoefficient 6 d = 2 * (6 + 1 - d) :=
  ⟨schedulerCoefficient_zero 6, fun d hd0 hdm => schedulerCoefficient_positive 6 d hd0 hdm⟩

/-- Mechanically emitted: m=7 m=7 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_6 :
    schedulerCoefficient 7 0 = 7 + 1 ∧
      ∀ d, 0 < d → d ≤ 7 → schedulerCoefficient 7 d = 2 * (7 + 1 - d) :=
  ⟨schedulerCoefficient_zero 7, fun d hd0 hdm => schedulerCoefficient_positive 7 d hd0 hdm⟩

/-- Mechanically emitted: m=8 m=8 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_7 :
    schedulerCoefficient 8 0 = 8 + 1 ∧
      ∀ d, 0 < d → d ≤ 8 → schedulerCoefficient 8 d = 2 * (8 + 1 - d) :=
  ⟨schedulerCoefficient_zero 8, fun d hd0 hdm => schedulerCoefficient_positive 8 d hd0 hdm⟩

/-- Mechanically emitted: m=9 m=9 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_8 :
    schedulerCoefficient 9 0 = 9 + 1 ∧
      ∀ d, 0 < d → d ≤ 9 → schedulerCoefficient 9 d = 2 * (9 + 1 - d) :=
  ⟨schedulerCoefficient_zero 9, fun d hd0 hdm => schedulerCoefficient_positive 9 d hd0 hdm⟩

/-- Mechanically emitted: m=10 m=10 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_9 :
    schedulerCoefficient 10 0 = 10 + 1 ∧
      ∀ d, 0 < d → d ≤ 10 → schedulerCoefficient 10 d = 2 * (10 + 1 - d) :=
  ⟨schedulerCoefficient_zero 10, fun d hd0 hdm => schedulerCoefficient_positive 10 d hd0 hdm⟩

/-- Mechanically emitted: m=11 m=11 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_10 :
    schedulerCoefficient 11 0 = 11 + 1 ∧
      ∀ d, 0 < d → d ≤ 11 → schedulerCoefficient 11 d = 2 * (11 + 1 - d) :=
  ⟨schedulerCoefficient_zero 11, fun d hd0 hdm => schedulerCoefficient_positive 11 d hd0 hdm⟩

/-- Mechanically emitted: m=12 m=12 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_11 :
    schedulerCoefficient 12 0 = 12 + 1 ∧
      ∀ d, 0 < d → d ≤ 12 → schedulerCoefficient 12 d = 2 * (12 + 1 - d) :=
  ⟨schedulerCoefficient_zero 12, fun d hd0 hdm => schedulerCoefficient_positive 12 d hd0 hdm⟩

/-- Mechanically emitted: m=13 m=13 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_12 :
    schedulerCoefficient 13 0 = 13 + 1 ∧
      ∀ d, 0 < d → d ≤ 13 → schedulerCoefficient 13 d = 2 * (13 + 1 - d) :=
  ⟨schedulerCoefficient_zero 13, fun d hd0 hdm => schedulerCoefficient_positive 13 d hd0 hdm⟩

/-- Mechanically emitted: m=14 m=14 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_13 :
    schedulerCoefficient 14 0 = 14 + 1 ∧
      ∀ d, 0 < d → d ≤ 14 → schedulerCoefficient 14 d = 2 * (14 + 1 - d) :=
  ⟨schedulerCoefficient_zero 14, fun d hd0 hdm => schedulerCoefficient_positive 14 d hd0 hdm⟩

/-- Mechanically emitted: m=15 m=15 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_14 :
    schedulerCoefficient 15 0 = 15 + 1 ∧
      ∀ d, 0 < d → d ≤ 15 → schedulerCoefficient 15 d = 2 * (15 + 1 - d) :=
  ⟨schedulerCoefficient_zero 15, fun d hd0 hdm => schedulerCoefficient_positive 15 d hd0 hdm⟩

/-- Mechanically emitted: m=16 m=16 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_15 :
    schedulerCoefficient 16 0 = 16 + 1 ∧
      ∀ d, 0 < d → d ≤ 16 → schedulerCoefficient 16 d = 2 * (16 + 1 - d) :=
  ⟨schedulerCoefficient_zero 16, fun d hd0 hdm => schedulerCoefficient_positive 16 d hd0 hdm⟩

/-- Mechanically emitted: m=17 m=17 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_16 :
    schedulerCoefficient 17 0 = 17 + 1 ∧
      ∀ d, 0 < d → d ≤ 17 → schedulerCoefficient 17 d = 2 * (17 + 1 - d) :=
  ⟨schedulerCoefficient_zero 17, fun d hd0 hdm => schedulerCoefficient_positive 17 d hd0 hdm⟩

/-- Mechanically emitted: m=18 m=18 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_17 :
    schedulerCoefficient 18 0 = 18 + 1 ∧
      ∀ d, 0 < d → d ≤ 18 → schedulerCoefficient 18 d = 2 * (18 + 1 - d) :=
  ⟨schedulerCoefficient_zero 18, fun d hd0 hdm => schedulerCoefficient_positive 18 d hd0 hdm⟩

/-- Mechanically emitted: m=19 m=19 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_18 :
    schedulerCoefficient 19 0 = 19 + 1 ∧
      ∀ d, 0 < d → d ≤ 19 → schedulerCoefficient 19 d = 2 * (19 + 1 - d) :=
  ⟨schedulerCoefficient_zero 19, fun d hd0 hdm => schedulerCoefficient_positive 19 d hd0 hdm⟩

/-- Mechanically emitted: m=20 m=20 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_19 :
    schedulerCoefficient 20 0 = 20 + 1 ∧
      ∀ d, 0 < d → d ≤ 20 → schedulerCoefficient 20 d = 2 * (20 + 1 - d) :=
  ⟨schedulerCoefficient_zero 20, fun d hd0 hdm => schedulerCoefficient_positive 20 d hd0 hdm⟩

/-- Mechanically emitted: m=21 m=21 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_20 :
    schedulerCoefficient 21 0 = 21 + 1 ∧
      ∀ d, 0 < d → d ≤ 21 → schedulerCoefficient 21 d = 2 * (21 + 1 - d) :=
  ⟨schedulerCoefficient_zero 21, fun d hd0 hdm => schedulerCoefficient_positive 21 d hd0 hdm⟩

/-- Mechanically emitted: m=22 m=22 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_21 :
    schedulerCoefficient 22 0 = 22 + 1 ∧
      ∀ d, 0 < d → d ≤ 22 → schedulerCoefficient 22 d = 2 * (22 + 1 - d) :=
  ⟨schedulerCoefficient_zero 22, fun d hd0 hdm => schedulerCoefficient_positive 22 d hd0 hdm⟩

/-- Mechanically emitted: m=23 m=23 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_22 :
    schedulerCoefficient 23 0 = 23 + 1 ∧
      ∀ d, 0 < d → d ≤ 23 → schedulerCoefficient 23 d = 2 * (23 + 1 - d) :=
  ⟨schedulerCoefficient_zero 23, fun d hd0 hdm => schedulerCoefficient_positive 23 d hd0 hdm⟩

/-- Mechanically emitted: m=24 m=24 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_23 :
    schedulerCoefficient 24 0 = 24 + 1 ∧
      ∀ d, 0 < d → d ≤ 24 → schedulerCoefficient 24 d = 2 * (24 + 1 - d) :=
  ⟨schedulerCoefficient_zero 24, fun d hd0 hdm => schedulerCoefficient_positive 24 d hd0 hdm⟩

/-- Mechanically emitted: m=25 m=25 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_24 :
    schedulerCoefficient 25 0 = 25 + 1 ∧
      ∀ d, 0 < d → d ≤ 25 → schedulerCoefficient 25 d = 2 * (25 + 1 - d) :=
  ⟨schedulerCoefficient_zero 25, fun d hd0 hdm => schedulerCoefficient_positive 25 d hd0 hdm⟩

/-- Mechanically emitted: m=26 m=26 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_25 :
    schedulerCoefficient 26 0 = 26 + 1 ∧
      ∀ d, 0 < d → d ≤ 26 → schedulerCoefficient 26 d = 2 * (26 + 1 - d) :=
  ⟨schedulerCoefficient_zero 26, fun d hd0 hdm => schedulerCoefficient_positive 26 d hd0 hdm⟩

/-- Mechanically emitted: m=27 m=27 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_26 :
    schedulerCoefficient 27 0 = 27 + 1 ∧
      ∀ d, 0 < d → d ≤ 27 → schedulerCoefficient 27 d = 2 * (27 + 1 - d) :=
  ⟨schedulerCoefficient_zero 27, fun d hd0 hdm => schedulerCoefficient_positive 27 d hd0 hdm⟩

/-- Mechanically emitted: m=28 m=28 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_27 :
    schedulerCoefficient 28 0 = 28 + 1 ∧
      ∀ d, 0 < d → d ≤ 28 → schedulerCoefficient 28 d = 2 * (28 + 1 - d) :=
  ⟨schedulerCoefficient_zero 28, fun d hd0 hdm => schedulerCoefficient_positive 28 d hd0 hdm⟩

/-- Mechanically emitted: m=29 m=29 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_28 :
    schedulerCoefficient 29 0 = 29 + 1 ∧
      ∀ d, 0 < d → d ≤ 29 → schedulerCoefficient 29 d = 2 * (29 + 1 - d) :=
  ⟨schedulerCoefficient_zero 29, fun d hd0 hdm => schedulerCoefficient_positive 29 d hd0 hdm⟩

/-- Mechanically emitted: m=30 m=30 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_29 :
    schedulerCoefficient 30 0 = 30 + 1 ∧
      ∀ d, 0 < d → d ≤ 30 → schedulerCoefficient 30 d = 2 * (30 + 1 - d) :=
  ⟨schedulerCoefficient_zero 30, fun d hd0 hdm => schedulerCoefficient_positive 30 d hd0 hdm⟩

/-- Mechanically emitted: m=31 m=31 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_30 :
    schedulerCoefficient 31 0 = 31 + 1 ∧
      ∀ d, 0 < d → d ≤ 31 → schedulerCoefficient 31 d = 2 * (31 + 1 - d) :=
  ⟨schedulerCoefficient_zero 31, fun d hd0 hdm => schedulerCoefficient_positive 31 d hd0 hdm⟩

/-- Mechanically emitted: m=32 m=32 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_31 :
    schedulerCoefficient 32 0 = 32 + 1 ∧
      ∀ d, 0 < d → d ≤ 32 → schedulerCoefficient 32 d = 2 * (32 + 1 - d) :=
  ⟨schedulerCoefficient_zero 32, fun d hd0 hdm => schedulerCoefficient_positive 32 d hd0 hdm⟩

/-- Mechanically emitted: m=33 m=33 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_32 :
    schedulerCoefficient 33 0 = 33 + 1 ∧
      ∀ d, 0 < d → d ≤ 33 → schedulerCoefficient 33 d = 2 * (33 + 1 - d) :=
  ⟨schedulerCoefficient_zero 33, fun d hd0 hdm => schedulerCoefficient_positive 33 d hd0 hdm⟩

/-- Mechanically emitted: m=34 m=34 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_33 :
    schedulerCoefficient 34 0 = 34 + 1 ∧
      ∀ d, 0 < d → d ≤ 34 → schedulerCoefficient 34 d = 2 * (34 + 1 - d) :=
  ⟨schedulerCoefficient_zero 34, fun d hd0 hdm => schedulerCoefficient_positive 34 d hd0 hdm⟩

/-- Mechanically emitted: m=35 m=35 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_34 :
    schedulerCoefficient 35 0 = 35 + 1 ∧
      ∀ d, 0 < d → d ≤ 35 → schedulerCoefficient 35 d = 2 * (35 + 1 - d) :=
  ⟨schedulerCoefficient_zero 35, fun d hd0 hdm => schedulerCoefficient_positive 35 d hd0 hdm⟩

/-- Mechanically emitted: m=36 m=36 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_35 :
    schedulerCoefficient 36 0 = 36 + 1 ∧
      ∀ d, 0 < d → d ≤ 36 → schedulerCoefficient 36 d = 2 * (36 + 1 - d) :=
  ⟨schedulerCoefficient_zero 36, fun d hd0 hdm => schedulerCoefficient_positive 36 d hd0 hdm⟩

/-- Mechanically emitted: m=37 m=37 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_36 :
    schedulerCoefficient 37 0 = 37 + 1 ∧
      ∀ d, 0 < d → d ≤ 37 → schedulerCoefficient 37 d = 2 * (37 + 1 - d) :=
  ⟨schedulerCoefficient_zero 37, fun d hd0 hdm => schedulerCoefficient_positive 37 d hd0 hdm⟩

/-- Mechanically emitted: m=38 m=38 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_37 :
    schedulerCoefficient 38 0 = 38 + 1 ∧
      ∀ d, 0 < d → d ≤ 38 → schedulerCoefficient 38 d = 2 * (38 + 1 - d) :=
  ⟨schedulerCoefficient_zero 38, fun d hd0 hdm => schedulerCoefficient_positive 38 d hd0 hdm⟩

/-- Mechanically emitted: m=39 m=39 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_38 :
    schedulerCoefficient 39 0 = 39 + 1 ∧
      ∀ d, 0 < d → d ≤ 39 → schedulerCoefficient 39 d = 2 * (39 + 1 - d) :=
  ⟨schedulerCoefficient_zero 39, fun d hd0 hdm => schedulerCoefficient_positive 39 d hd0 hdm⟩

/-- Mechanically emitted: m=40 m=40 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_39 :
    schedulerCoefficient 40 0 = 40 + 1 ∧
      ∀ d, 0 < d → d ≤ 40 → schedulerCoefficient 40 d = 2 * (40 + 1 - d) :=
  ⟨schedulerCoefficient_zero 40, fun d hd0 hdm => schedulerCoefficient_positive 40 d hd0 hdm⟩

/-- Mechanically emitted: m=41 m=41 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_40 :
    schedulerCoefficient 41 0 = 41 + 1 ∧
      ∀ d, 0 < d → d ≤ 41 → schedulerCoefficient 41 d = 2 * (41 + 1 - d) :=
  ⟨schedulerCoefficient_zero 41, fun d hd0 hdm => schedulerCoefficient_positive 41 d hd0 hdm⟩

/-- Mechanically emitted: m=42 m=42 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_41 :
    schedulerCoefficient 42 0 = 42 + 1 ∧
      ∀ d, 0 < d → d ≤ 42 → schedulerCoefficient 42 d = 2 * (42 + 1 - d) :=
  ⟨schedulerCoefficient_zero 42, fun d hd0 hdm => schedulerCoefficient_positive 42 d hd0 hdm⟩

/-- Mechanically emitted: m=43 m=43 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_42 :
    schedulerCoefficient 43 0 = 43 + 1 ∧
      ∀ d, 0 < d → d ≤ 43 → schedulerCoefficient 43 d = 2 * (43 + 1 - d) :=
  ⟨schedulerCoefficient_zero 43, fun d hd0 hdm => schedulerCoefficient_positive 43 d hd0 hdm⟩

/-- Mechanically emitted: m=44 m=44 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_43 :
    schedulerCoefficient 44 0 = 44 + 1 ∧
      ∀ d, 0 < d → d ≤ 44 → schedulerCoefficient 44 d = 2 * (44 + 1 - d) :=
  ⟨schedulerCoefficient_zero 44, fun d hd0 hdm => schedulerCoefficient_positive 44 d hd0 hdm⟩

/-- Mechanically emitted: m=45 m=45 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_44 :
    schedulerCoefficient 45 0 = 45 + 1 ∧
      ∀ d, 0 < d → d ≤ 45 → schedulerCoefficient 45 d = 2 * (45 + 1 - d) :=
  ⟨schedulerCoefficient_zero 45, fun d hd0 hdm => schedulerCoefficient_positive 45 d hd0 hdm⟩

/-- Mechanically emitted: m=46 m=46 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_45 :
    schedulerCoefficient 46 0 = 46 + 1 ∧
      ∀ d, 0 < d → d ≤ 46 → schedulerCoefficient 46 d = 2 * (46 + 1 - d) :=
  ⟨schedulerCoefficient_zero 46, fun d hd0 hdm => schedulerCoefficient_positive 46 d hd0 hdm⟩

/-- Mechanically emitted: m=47 m=47 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_46 :
    schedulerCoefficient 47 0 = 47 + 1 ∧
      ∀ d, 0 < d → d ≤ 47 → schedulerCoefficient 47 d = 2 * (47 + 1 - d) :=
  ⟨schedulerCoefficient_zero 47, fun d hd0 hdm => schedulerCoefficient_positive 47 d hd0 hdm⟩

/-- Mechanically emitted: m=48 m=48 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_47 :
    schedulerCoefficient 48 0 = 48 + 1 ∧
      ∀ d, 0 < d → d ≤ 48 → schedulerCoefficient 48 d = 2 * (48 + 1 - d) :=
  ⟨schedulerCoefficient_zero 48, fun d hd0 hdm => schedulerCoefficient_positive 48 d hd0 hdm⟩

/-- Mechanically emitted: m=49 m=49 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_48 :
    schedulerCoefficient 49 0 = 49 + 1 ∧
      ∀ d, 0 < d → d ≤ 49 → schedulerCoefficient 49 d = 2 * (49 + 1 - d) :=
  ⟨schedulerCoefficient_zero 49, fun d hd0 hdm => schedulerCoefficient_positive 49 d hd0 hdm⟩

/-- Mechanically emitted: m=50 m=50 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_49 :
    schedulerCoefficient 50 0 = 50 + 1 ∧
      ∀ d, 0 < d → d ≤ 50 → schedulerCoefficient 50 d = 2 * (50 + 1 - d) :=
  ⟨schedulerCoefficient_zero 50, fun d hd0 hdm => schedulerCoefficient_positive 50 d hd0 hdm⟩

/-- Mechanically emitted: m=51 m=51 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_50 :
    schedulerCoefficient 51 0 = 51 + 1 ∧
      ∀ d, 0 < d → d ≤ 51 → schedulerCoefficient 51 d = 2 * (51 + 1 - d) :=
  ⟨schedulerCoefficient_zero 51, fun d hd0 hdm => schedulerCoefficient_positive 51 d hd0 hdm⟩

/-- Mechanically emitted: m=52 m=52 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_51 :
    schedulerCoefficient 52 0 = 52 + 1 ∧
      ∀ d, 0 < d → d ≤ 52 → schedulerCoefficient 52 d = 2 * (52 + 1 - d) :=
  ⟨schedulerCoefficient_zero 52, fun d hd0 hdm => schedulerCoefficient_positive 52 d hd0 hdm⟩

/-- Mechanically emitted: m=53 m=53 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_52 :
    schedulerCoefficient 53 0 = 53 + 1 ∧
      ∀ d, 0 < d → d ≤ 53 → schedulerCoefficient 53 d = 2 * (53 + 1 - d) :=
  ⟨schedulerCoefficient_zero 53, fun d hd0 hdm => schedulerCoefficient_positive 53 d hd0 hdm⟩

/-- Mechanically emitted: m=54 m=54 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_53 :
    schedulerCoefficient 54 0 = 54 + 1 ∧
      ∀ d, 0 < d → d ≤ 54 → schedulerCoefficient 54 d = 2 * (54 + 1 - d) :=
  ⟨schedulerCoefficient_zero 54, fun d hd0 hdm => schedulerCoefficient_positive 54 d hd0 hdm⟩

/-- Mechanically emitted: m=55 m=55 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_54 :
    schedulerCoefficient 55 0 = 55 + 1 ∧
      ∀ d, 0 < d → d ≤ 55 → schedulerCoefficient 55 d = 2 * (55 + 1 - d) :=
  ⟨schedulerCoefficient_zero 55, fun d hd0 hdm => schedulerCoefficient_positive 55 d hd0 hdm⟩

/-- Mechanically emitted: m=56 m=56 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_55 :
    schedulerCoefficient 56 0 = 56 + 1 ∧
      ∀ d, 0 < d → d ≤ 56 → schedulerCoefficient 56 d = 2 * (56 + 1 - d) :=
  ⟨schedulerCoefficient_zero 56, fun d hd0 hdm => schedulerCoefficient_positive 56 d hd0 hdm⟩

/-- Mechanically emitted: m=57 m=57 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_56 :
    schedulerCoefficient 57 0 = 57 + 1 ∧
      ∀ d, 0 < d → d ≤ 57 → schedulerCoefficient 57 d = 2 * (57 + 1 - d) :=
  ⟨schedulerCoefficient_zero 57, fun d hd0 hdm => schedulerCoefficient_positive 57 d hd0 hdm⟩

/-- Mechanically emitted: m=58 m=58 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_57 :
    schedulerCoefficient 58 0 = 58 + 1 ∧
      ∀ d, 0 < d → d ≤ 58 → schedulerCoefficient 58 d = 2 * (58 + 1 - d) :=
  ⟨schedulerCoefficient_zero 58, fun d hd0 hdm => schedulerCoefficient_positive 58 d hd0 hdm⟩

/-- Mechanically emitted: m=59 m=59 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_58 :
    schedulerCoefficient 59 0 = 59 + 1 ∧
      ∀ d, 0 < d → d ≤ 59 → schedulerCoefficient 59 d = 2 * (59 + 1 - d) :=
  ⟨schedulerCoefficient_zero 59, fun d hd0 hdm => schedulerCoefficient_positive 59 d hd0 hdm⟩

/-- Mechanically emitted: m=60 m=60 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_59 :
    schedulerCoefficient 60 0 = 60 + 1 ∧
      ∀ d, 0 < d → d ≤ 60 → schedulerCoefficient 60 d = 2 * (60 + 1 - d) :=
  ⟨schedulerCoefficient_zero 60, fun d hd0 hdm => schedulerCoefficient_positive 60 d hd0 hdm⟩

/-- Mechanically emitted: m=61 m=61 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_60 :
    schedulerCoefficient 61 0 = 61 + 1 ∧
      ∀ d, 0 < d → d ≤ 61 → schedulerCoefficient 61 d = 2 * (61 + 1 - d) :=
  ⟨schedulerCoefficient_zero 61, fun d hd0 hdm => schedulerCoefficient_positive 61 d hd0 hdm⟩

/-- Mechanically emitted: m=62 m=62 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_61 :
    schedulerCoefficient 62 0 = 62 + 1 ∧
      ∀ d, 0 < d → d ≤ 62 → schedulerCoefficient 62 d = 2 * (62 + 1 - d) :=
  ⟨schedulerCoefficient_zero 62, fun d hd0 hdm => schedulerCoefficient_positive 62 d hd0 hdm⟩

/-- Mechanically emitted: m=63 m=63 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_62 :
    schedulerCoefficient 63 0 = 63 + 1 ∧
      ∀ d, 0 < d → d ≤ 63 → schedulerCoefficient 63 d = 2 * (63 + 1 - d) :=
  ⟨schedulerCoefficient_zero 63, fun d hd0 hdm => schedulerCoefficient_positive 63 d hd0 hdm⟩

/-- Mechanically emitted: m=64 m=64 symbolic scheduler closed-form check -- ravel::proof::stage_generalized_multinacci_general_m
    independently reconfirmed the closed-form scheduler coefficients. -/
theorem generalized_multinacci_general_m_instance_63 :
    schedulerCoefficient 64 0 = 64 + 1 ∧
      ∀ d, 0 < d → d ≤ 64 → schedulerCoefficient 64 d = 2 * (64 + 1 - d) :=
  ⟨schedulerCoefficient_zero 64, fun d hd0 hdm => schedulerCoefficient_positive 64 d hd0 hdm⟩

/- Semantic proof graph for: generalized_multinacci_general_m_batch
  [0] lean.generalized_multinacci_general_m_certificate :: m=1 m=1 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [1] lean.generalized_multinacci_general_m_certificate :: m=2 m=2 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [2] lean.generalized_multinacci_general_m_certificate :: m=3 m=3 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [3] lean.generalized_multinacci_general_m_certificate :: m=4 m=4 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [4] lean.generalized_multinacci_general_m_certificate :: m=5 m=5 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [5] lean.generalized_multinacci_general_m_certificate :: m=6 m=6 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [6] lean.generalized_multinacci_general_m_certificate :: m=7 m=7 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [7] lean.generalized_multinacci_general_m_certificate :: m=8 m=8 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [8] lean.generalized_multinacci_general_m_certificate :: m=9 m=9 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [9] lean.generalized_multinacci_general_m_certificate :: m=10 m=10 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [10] lean.generalized_multinacci_general_m_certificate :: m=11 m=11 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [11] lean.generalized_multinacci_general_m_certificate :: m=12 m=12 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [12] lean.generalized_multinacci_general_m_certificate :: m=13 m=13 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [13] lean.generalized_multinacci_general_m_certificate :: m=14 m=14 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [14] lean.generalized_multinacci_general_m_certificate :: m=15 m=15 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [15] lean.generalized_multinacci_general_m_certificate :: m=16 m=16 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [16] lean.generalized_multinacci_general_m_certificate :: m=17 m=17 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [17] lean.generalized_multinacci_general_m_certificate :: m=18 m=18 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [18] lean.generalized_multinacci_general_m_certificate :: m=19 m=19 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [19] lean.generalized_multinacci_general_m_certificate :: m=20 m=20 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [20] lean.generalized_multinacci_general_m_certificate :: m=21 m=21 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [21] lean.generalized_multinacci_general_m_certificate :: m=22 m=22 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [22] lean.generalized_multinacci_general_m_certificate :: m=23 m=23 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [23] lean.generalized_multinacci_general_m_certificate :: m=24 m=24 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [24] lean.generalized_multinacci_general_m_certificate :: m=25 m=25 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [25] lean.generalized_multinacci_general_m_certificate :: m=26 m=26 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [26] lean.generalized_multinacci_general_m_certificate :: m=27 m=27 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [27] lean.generalized_multinacci_general_m_certificate :: m=28 m=28 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [28] lean.generalized_multinacci_general_m_certificate :: m=29 m=29 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [29] lean.generalized_multinacci_general_m_certificate :: m=30 m=30 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [30] lean.generalized_multinacci_general_m_certificate :: m=31 m=31 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [31] lean.generalized_multinacci_general_m_certificate :: m=32 m=32 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [32] lean.generalized_multinacci_general_m_certificate :: m=33 m=33 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [33] lean.generalized_multinacci_general_m_certificate :: m=34 m=34 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [34] lean.generalized_multinacci_general_m_certificate :: m=35 m=35 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [35] lean.generalized_multinacci_general_m_certificate :: m=36 m=36 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [36] lean.generalized_multinacci_general_m_certificate :: m=37 m=37 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [37] lean.generalized_multinacci_general_m_certificate :: m=38 m=38 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [38] lean.generalized_multinacci_general_m_certificate :: m=39 m=39 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [39] lean.generalized_multinacci_general_m_certificate :: m=40 m=40 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [40] lean.generalized_multinacci_general_m_certificate :: m=41 m=41 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [41] lean.generalized_multinacci_general_m_certificate :: m=42 m=42 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [42] lean.generalized_multinacci_general_m_certificate :: m=43 m=43 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [43] lean.generalized_multinacci_general_m_certificate :: m=44 m=44 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [44] lean.generalized_multinacci_general_m_certificate :: m=45 m=45 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [45] lean.generalized_multinacci_general_m_certificate :: m=46 m=46 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [46] lean.generalized_multinacci_general_m_certificate :: m=47 m=47 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [47] lean.generalized_multinacci_general_m_certificate :: m=48 m=48 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [48] lean.generalized_multinacci_general_m_certificate :: m=49 m=49 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [49] lean.generalized_multinacci_general_m_certificate :: m=50 m=50 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [50] lean.generalized_multinacci_general_m_certificate :: m=51 m=51 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [51] lean.generalized_multinacci_general_m_certificate :: m=52 m=52 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [52] lean.generalized_multinacci_general_m_certificate :: m=53 m=53 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [53] lean.generalized_multinacci_general_m_certificate :: m=54 m=54 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [54] lean.generalized_multinacci_general_m_certificate :: m=55 m=55 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [55] lean.generalized_multinacci_general_m_certificate :: m=56 m=56 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [56] lean.generalized_multinacci_general_m_certificate :: m=57 m=57 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [57] lean.generalized_multinacci_general_m_certificate :: m=58 m=58 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [58] lean.generalized_multinacci_general_m_certificate :: m=59 m=59 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [59] lean.generalized_multinacci_general_m_certificate :: m=60 m=60 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [60] lean.generalized_multinacci_general_m_certificate :: m=61 m=61 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [61] lean.generalized_multinacci_general_m_certificate :: m=62 m=62 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [62] lean.generalized_multinacci_general_m_certificate :: m=63 m=63 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
  [63] lean.generalized_multinacci_general_m_certificate :: m=64 m=64 symbolic scheduler closed-form check -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive
-/

def reflectedNodeCount : Nat := 64

end RavelGenerated
