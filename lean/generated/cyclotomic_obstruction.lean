import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem cyclotomic_two : Polynomial.cyclotomic 2 ℤ = Polynomial.X + 1 :=
  Polynomial.cyclotomic_two ℤ

theorem x_add_one_dvd_iff_eval_neg_one_zero (p : Polynomial ℤ) :
    Polynomial.X + 1 ∣ p ↔ p.eval (-1) = 0 := by
  have h : (Polynomial.X + 1 : Polynomial ℤ) = Polynomial.X - Polynomial.C (-1) := by
    simp
  rw [h, Polynomial.dvd_iff_isRoot]
  rfl

-- A Horner-form embedding of an explicit integer coefficient list (ascending,
-- cs[0] = constant term) into `Polynomial ℤ`, plus the fact that `.eval`
-- reduces to plain Horner arithmetic on that same list -- lets concrete
-- polynomial data checked in C++ be reflected as an actual `Polynomial ℤ`
-- term (not just an arithmetic expression in a free variable), so
-- `x_add_one_dvd_iff_eval_neg_one_zero` can be instantiated at it directly.
noncomputable def polyOfCoeffs : List ℤ → Polynomial ℤ
  | [] => 0
  | c :: cs => Polynomial.C c + Polynomial.X * polyOfCoeffs cs

theorem eval_polyOfCoeffs (cs : List ℤ) (x : ℤ) :
    (polyOfCoeffs cs).eval x = cs.foldr (fun c acc => c + x * acc) 0 := by
  induction cs with
  | nil => simp [polyOfCoeffs]
  | cons c cs ih => simp [polyOfCoeffs, ih]

/-- Mechanically emitted: deg=4 Phi2*Phi3*(x-2) composite factorization witness -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_0_eval :
    (polyOfCoeffs [(-2 : ℤ), (-3 : ℤ), (-2 : ℤ), (0 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_0 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-2 : ℤ), (-3 : ℤ), (-2 : ℤ), (0 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-2 : ℤ), (-3 : ℤ), (-2 : ℤ), (0 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_0_eval

/-- Mechanically emitted: deg=3 D=3 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_1_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_1 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_1_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=4 D=4 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_2_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_2 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_2_eval

/-- Mechanically emitted: deg=5 D=5 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_3_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_3 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_3_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=6 D=6 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_4_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_4 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_4_eval

/-- Mechanically emitted: deg=7 D=7 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_5_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_5 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_5_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=8 D=8 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_6_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_6 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_6_eval

/-- Mechanically emitted: deg=9 D=9 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_7_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_7 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_7_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=10 D=10 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_8_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_8 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_8_eval

/-- Mechanically emitted: deg=11 D=11 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_9_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_9 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_9_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=12 D=12 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_10_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_10 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_10_eval

/-- Mechanically emitted: deg=13 D=13 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_11_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_11 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_11_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=14 D=14 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_12_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_12 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_12_eval

/-- Mechanically emitted: deg=15 D=15 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_13_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_13 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_13_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=16 D=16 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_14_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_14 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_14_eval

/-- Mechanically emitted: deg=17 D=17 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_15_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_15 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_15_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=18 D=18 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_16_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_16 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_16_eval

/-- Mechanically emitted: deg=19 D=19 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_17_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_17 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_17_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=20 D=20 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_18_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_18 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_18_eval

/-- Mechanically emitted: deg=21 D=21 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_19_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_19 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_19_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=22 D=22 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_20_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_20 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_20_eval

/-- Mechanically emitted: deg=23 D=23 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_21_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_21 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_21_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=24 D=24 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_22_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_22 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_22_eval

/-- Mechanically emitted: deg=25 D=25 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_23_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_23 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_23_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=26 D=26 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_24_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_24 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_24_eval

/-- Mechanically emitted: deg=27 D=27 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_25_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_25 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_25_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=28 D=28 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_26_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_26 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_26_eval

/-- Mechanically emitted: deg=29 D=29 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_27_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_27 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_27_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=30 D=30 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_28_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_28 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_28_eval

/-- Mechanically emitted: deg=31 D=31 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_29_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_29 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_29_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=32 D=32 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_30_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_30 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_30_eval

/-- Mechanically emitted: deg=33 D=33 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_31_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_31 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_31_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=34 D=34 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_32_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_32 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_32_eval

/-- Mechanically emitted: deg=35 D=35 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_33_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_33 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_33_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=36 D=36 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_34_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_34 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_34_eval

/-- Mechanically emitted: deg=37 D=37 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_35_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_35 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_35_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=38 D=38 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_36_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_36 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_36_eval

/-- Mechanically emitted: deg=39 D=39 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_37_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_37 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_37_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=40 D=40 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_38_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_38 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_38_eval

/-- Mechanically emitted: deg=41 D=41 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_39_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_39 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_39_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=42 D=42 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_40_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_40 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_40_eval

/-- Mechanically emitted: deg=43 D=43 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_41_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_41 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_41_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=44 D=44 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_42_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_42 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_42_eval

/-- Mechanically emitted: deg=45 D=45 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_43_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_43 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_43_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=46 D=46 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_44_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_44 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_44_eval

/-- Mechanically emitted: deg=47 D=47 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_45_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_45 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_45_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=48 D=48 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_46_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_46 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_46_eval

/-- Mechanically emitted: deg=49 D=49 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_47_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_47 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_47_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=50 D=50 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_48_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_48 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_48_eval

/-- Mechanically emitted: deg=51 D=51 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_49_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_49 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_49_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=52 D=52 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_50_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_50 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_50_eval

/-- Mechanically emitted: deg=53 D=53 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_51_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_51 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_51_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=54 D=54 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_52_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_52 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_52_eval

/-- Mechanically emitted: deg=55 D=55 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_53_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_53 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_53_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=56 D=56 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_54_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_54 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_54_eval

/-- Mechanically emitted: deg=57 D=57 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_55_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_55 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_55_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=58 D=58 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_56_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_56 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_56_eval

/-- Mechanically emitted: deg=59 D=59 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_57_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_57 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_57_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=60 D=60 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_58_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_58 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_58_eval

/-- Mechanically emitted: deg=61 D=61 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_59_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_59 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_59_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=62 D=62 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_60_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_60 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_60_eval

/-- Mechanically emitted: deg=63 D=63 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=-3 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_61_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) ≠ 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_61 :
    ¬ Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  fun h => cyclotomic_obstruction_instance_61_eval ((x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mp h)

/-- Mechanically emitted: deg=64 D=64 nearest-left profile factorization instance -- ravel::proof::stage_cyclotomic_obstruction
    independently recomputed eval(-1)=0 via Horner arithmetic. -/
theorem cyclotomic_obstruction_instance_62_eval :
    (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)]).eval (-1) = 0 := by
  rw [eval_polyOfCoeffs]; native_decide

theorem cyclotomic_obstruction_instance_62 :
    Polynomial.X + 1 ∣ polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)] :=
  (x_add_one_dvd_iff_eval_neg_one_zero (polyOfCoeffs [(-1 : ℤ), (0 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (-1 : ℤ), (1 : ℤ)])).mpr cyclotomic_obstruction_instance_62_eval

/- Semantic proof graph for: cyclotomic_obstruction_batch
  [0] lean.cyclotomic_obstruction_certificate :: deg=4 has_order_two=true Phi2*Phi3*(x-2) composite factorization witness -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [1] lean.cyclotomic_obstruction_certificate :: deg=3 has_order_two=false D=3 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [2] lean.cyclotomic_obstruction_certificate :: deg=4 has_order_two=true D=4 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [3] lean.cyclotomic_obstruction_certificate :: deg=5 has_order_two=false D=5 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [4] lean.cyclotomic_obstruction_certificate :: deg=6 has_order_two=true D=6 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [5] lean.cyclotomic_obstruction_certificate :: deg=7 has_order_two=false D=7 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [6] lean.cyclotomic_obstruction_certificate :: deg=8 has_order_two=true D=8 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [7] lean.cyclotomic_obstruction_certificate :: deg=9 has_order_two=false D=9 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [8] lean.cyclotomic_obstruction_certificate :: deg=10 has_order_two=true D=10 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [9] lean.cyclotomic_obstruction_certificate :: deg=11 has_order_two=false D=11 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [10] lean.cyclotomic_obstruction_certificate :: deg=12 has_order_two=true D=12 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [11] lean.cyclotomic_obstruction_certificate :: deg=13 has_order_two=false D=13 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [12] lean.cyclotomic_obstruction_certificate :: deg=14 has_order_two=true D=14 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [13] lean.cyclotomic_obstruction_certificate :: deg=15 has_order_two=false D=15 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [14] lean.cyclotomic_obstruction_certificate :: deg=16 has_order_two=true D=16 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [15] lean.cyclotomic_obstruction_certificate :: deg=17 has_order_two=false D=17 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [16] lean.cyclotomic_obstruction_certificate :: deg=18 has_order_two=true D=18 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [17] lean.cyclotomic_obstruction_certificate :: deg=19 has_order_two=false D=19 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [18] lean.cyclotomic_obstruction_certificate :: deg=20 has_order_two=true D=20 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [19] lean.cyclotomic_obstruction_certificate :: deg=21 has_order_two=false D=21 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [20] lean.cyclotomic_obstruction_certificate :: deg=22 has_order_two=true D=22 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [21] lean.cyclotomic_obstruction_certificate :: deg=23 has_order_two=false D=23 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [22] lean.cyclotomic_obstruction_certificate :: deg=24 has_order_two=true D=24 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [23] lean.cyclotomic_obstruction_certificate :: deg=25 has_order_two=false D=25 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [24] lean.cyclotomic_obstruction_certificate :: deg=26 has_order_two=true D=26 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [25] lean.cyclotomic_obstruction_certificate :: deg=27 has_order_two=false D=27 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [26] lean.cyclotomic_obstruction_certificate :: deg=28 has_order_two=true D=28 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [27] lean.cyclotomic_obstruction_certificate :: deg=29 has_order_two=false D=29 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [28] lean.cyclotomic_obstruction_certificate :: deg=30 has_order_two=true D=30 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [29] lean.cyclotomic_obstruction_certificate :: deg=31 has_order_two=false D=31 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [30] lean.cyclotomic_obstruction_certificate :: deg=32 has_order_two=true D=32 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [31] lean.cyclotomic_obstruction_certificate :: deg=33 has_order_two=false D=33 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [32] lean.cyclotomic_obstruction_certificate :: deg=34 has_order_two=true D=34 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [33] lean.cyclotomic_obstruction_certificate :: deg=35 has_order_two=false D=35 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [34] lean.cyclotomic_obstruction_certificate :: deg=36 has_order_two=true D=36 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [35] lean.cyclotomic_obstruction_certificate :: deg=37 has_order_two=false D=37 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [36] lean.cyclotomic_obstruction_certificate :: deg=38 has_order_two=true D=38 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [37] lean.cyclotomic_obstruction_certificate :: deg=39 has_order_two=false D=39 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [38] lean.cyclotomic_obstruction_certificate :: deg=40 has_order_two=true D=40 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [39] lean.cyclotomic_obstruction_certificate :: deg=41 has_order_two=false D=41 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [40] lean.cyclotomic_obstruction_certificate :: deg=42 has_order_two=true D=42 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [41] lean.cyclotomic_obstruction_certificate :: deg=43 has_order_two=false D=43 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [42] lean.cyclotomic_obstruction_certificate :: deg=44 has_order_two=true D=44 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [43] lean.cyclotomic_obstruction_certificate :: deg=45 has_order_two=false D=45 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [44] lean.cyclotomic_obstruction_certificate :: deg=46 has_order_two=true D=46 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [45] lean.cyclotomic_obstruction_certificate :: deg=47 has_order_two=false D=47 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [46] lean.cyclotomic_obstruction_certificate :: deg=48 has_order_two=true D=48 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [47] lean.cyclotomic_obstruction_certificate :: deg=49 has_order_two=false D=49 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [48] lean.cyclotomic_obstruction_certificate :: deg=50 has_order_two=true D=50 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [49] lean.cyclotomic_obstruction_certificate :: deg=51 has_order_two=false D=51 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [50] lean.cyclotomic_obstruction_certificate :: deg=52 has_order_two=true D=52 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [51] lean.cyclotomic_obstruction_certificate :: deg=53 has_order_two=false D=53 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [52] lean.cyclotomic_obstruction_certificate :: deg=54 has_order_two=true D=54 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [53] lean.cyclotomic_obstruction_certificate :: deg=55 has_order_two=false D=55 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [54] lean.cyclotomic_obstruction_certificate :: deg=56 has_order_two=true D=56 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [55] lean.cyclotomic_obstruction_certificate :: deg=57 has_order_two=false D=57 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [56] lean.cyclotomic_obstruction_certificate :: deg=58 has_order_two=true D=58 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [57] lean.cyclotomic_obstruction_certificate :: deg=59 has_order_two=false D=59 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [58] lean.cyclotomic_obstruction_certificate :: deg=60 has_order_two=true D=60 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [59] lean.cyclotomic_obstruction_certificate :: deg=61 has_order_two=false D=61 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [60] lean.cyclotomic_obstruction_certificate :: deg=62 has_order_two=true D=62 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [61] lean.cyclotomic_obstruction_certificate :: deg=63 has_order_two=false D=63 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
  [62] lean.cyclotomic_obstruction_certificate :: deg=64 has_order_two=true D=64 nearest-left profile factorization instance -- instantiates x_add_one_dvd_iff_eval_neg_one_zero
-/

def reflectedNodeCount : Nat := 63

end RavelGenerated
