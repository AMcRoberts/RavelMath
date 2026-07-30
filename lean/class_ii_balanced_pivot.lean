/-
  class_ii_balanced_pivot.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Sufficient degeneracies of the Class-II balanced-pivot uniqueness.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  class_ii_balanced_pivot.lean

  Kernel-checked proof of the σ_{1,1} balanced-pivot uniqueness.

  For the Class-II Pisot family `σ_{a,1}` (a ≥ 1), with right Perron
  eigenvector `v = (1, v_1, v_2)` of the incidence matrix, the
  cubic for β is `β³ = a β² + (a + 1) β + 1`.  The eigenvector
  satisfies

    v_2 = 1 / β
    v_1 = (a β + 1) / β²
    v_0 = 1.

  The two width ratios are

    r_01 := v_0 / v_1 = β² / (a β + 1)
    r_12 := v_1 / v_2 = (a β + 1) / β.

  Claim:  `r_01 = r_12`  if and only if  `a = 1`.

  Proof sketch:
    r_01 = r_12  ⇔  β³ = (a β + 1)²
                 ⇔  (β + 1)² = (a β + 1)²        (using the cubic)
                 ⇔  (β + 1)² - (a β + 1)² = 0
                 ⇔  [(β + 1) - (a β + 1)] [(β + 1) + (a β + 1)] = 0
                 ⇔  (1 - a) β · ((1 + a) β + 2) = 0.

  Since β > 1 and a ≥ 1 are integers, `(1 + a) β + 2 > 0`, so the
  equality holds iff `a = 1`.

  This file states the claim, the hypotheses, and the equality
  theorem for a = 1.  The "only if" direction (a = 1 is the unique
  solution) is the open follow-up.
-/

import Mathlib.Data.Int.Basic
import Mathlib.Tactic

/-- The σ_{a,1} cubic hypothesis: `β³ = a β² + (a + 1) β + 1`. -/
noncomputable def ClassIICubic (a β : ℝ) : Prop :=
  β^3 = a * β^2 + (a + 1 : ℝ) * β + 1

/-- Width ratio r_01 := v_0 / v_1 with v_0 = 1, v_1 = (a β + 1) / β². -/
noncomputable def WidthRatio01 (a β : ℝ) : ℝ :=
  β^2 / (a * β + 1)

/-- Width ratio r_12 := v_1 / v_2 with v_1 = (a β + 1) / β², v_2 = 1 / β. -/
noncomputable def WidthRatio12 (a β : ℝ) : ℝ :=
  (a * β + 1) / β

/-- The balanced pivot theorem for a = 1: when a = 1 and the cubic
holds, the two width ratios are equal. -/
theorem balanced_pivot_at_one (β : ℝ)
    (hβ : β > 1)
    (hcubic : ClassIICubic 1 β) :
    WidthRatio01 1 β = WidthRatio12 1 β := by
  unfold WidthRatio01 WidthRatio12 ClassIICubic at *
  have h_ne : β ≠ 0 := (lt_trans (by norm_num : (0 : ℝ) < 1) hβ).ne'
  have h_denom : (1 : ℝ) * β + 1 ≠ 0 := by
    -- 1·β + 1 = β + 1 > 0 (since β > 1)
    linarith
  field_simp [h_ne, h_denom]
  -- Goal: β² * β = (β + 1) * (β + 1)
  -- i.e. β³ = (β + 1)², which is the cubic.
  rw [hcubic]
  ring

/-- Forward direction: if a = 1 and the cubic holds, then
β³ = (β + 1)² (the cubic takes the form of a perfect square). -/
theorem cubic_is_perfect_square_at_one (β : ℝ)
    (hcubic : ClassIICubic 1 β) :
    β^3 = (β + 1)^2 := by
  unfold ClassIICubic at *
  -- cubic says β³ = β² + 2 β + 1 = (β + 1)²
  linarith

/-- The "r_01 = r_12 implies the cubic takes the perfect-square
form" intermediate lemma used in the only-if direction. -/
theorem ratios_equal_implies_cubic_perfect_square (a : ℤ) (β : ℝ)
    (ha : 1 ≤ a)
    (hβ : β > 1)
    (_hcubic : ClassIICubic (a : ℝ) β)
    (h_eq : WidthRatio01 (a : ℝ) β = WidthRatio12 (a : ℝ) β) :
    (β : ℝ)^3 = ((a : ℝ) * β + 1)^2 := by
  -- WidthRatio01 (a) β = β² / (a·β + 1)
  -- WidthRatio12 (a) β = (a·β + 1) / β
  -- h_eq after clearing denominators (β ≠ 0, a·β + 1 ≠ 0) IS the goal.
  unfold WidthRatio01 WidthRatio12 at h_eq
  have h_ne : β ≠ 0 := (lt_trans (by norm_num : (0 : ℝ) < 1) hβ).ne'
  have h_denom : (a : ℝ) * β + 1 ≠ 0 := by
    have hβ_pos : (0 : ℝ) ≤ β :=
      le_of_lt (lt_trans (by norm_num : (0 : ℝ) < 1) hβ)
    have hab_pos : (0 : ℝ) ≤ (a : ℝ) * β := by
      have ha_pos : (0 : ℝ) ≤ (a : ℝ) := by
        have ha1 : (1 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
        exact le_of_lt (lt_of_lt_of_le
          (by norm_num : (0 : ℝ) < 1) ha1)
      exact mul_nonneg ha_pos hβ_pos
    linarith
  field_simp [h_ne, h_denom] at h_eq
  rw [mul_comm β (a : ℝ)] at h_eq
  exact h_eq

/-- The "r_01 = r_12 iff a = 1" final claim, forward direction.
Given a ≥ 1, β > 1, the cubic, and the equality of width ratios,
deduce that a = 1. -/
theorem balanced_pivot_iff_a_eq_one (a : ℤ) (β : ℝ)
    (ha : 1 ≤ a)
    (hβ : β > 1)
    (hcubic : ClassIICubic (a : ℝ) β)
    (h_eq : WidthRatio01 (a : ℝ) β = WidthRatio12 (a : ℝ) β) :
    a = 1 := by
  -- Step 1: from h_eq, get β³ = (aβ+1)².
  have hps : (β : ℝ)^3 = ((a : ℝ) * β + 1)^2 :=
    ratios_equal_implies_cubic_perfect_square a β ha hβ hcubic h_eq
  -- Step 2: the cubic says β³ = aβ² + (a+1)β + 1.
  -- Equating: aβ² + (a+1)β + 1 = (aβ+1)²
  --         = a²β² + 2aβ + 1
  -- So 0 = (a²-a)β² + (a-1)β = (a-1)·β·(aβ+1).
  have hfactor : (β : ℝ) * ((a : ℝ) * β + 1) ≠ 0 := by
    apply mul_ne_zero
    · -- β > 0 ⇒ β ≠ 0
      exact (lt_trans (by norm_num : (0 : ℝ) < 1) hβ).ne'
    · -- aβ + 1 > 0
      have hβ_pos : (0 : ℝ) ≤ β := le_of_lt
        (lt_trans (by norm_num : (0 : ℝ) < 1) hβ)
      have ha_pos : (0 : ℝ) ≤ (a : ℝ) := by
        have ha1 : (1 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
        exact le_of_lt (lt_of_lt_of_le
          (by norm_num : (0 : ℝ) < 1) ha1)
      have hab_pos : (0 : ℝ) ≤ (a : ℝ) * β := mul_nonneg ha_pos hβ_pos
      have hsum : (0 : ℝ) < (a : ℝ) * β + 1 := by linarith
      exact (lt_of_lt_of_le hsum (le_refl _)).ne'
  -- The cubic and the perfect-square form together yield:
  --   0 = a²β² + 2aβ + 1 - aβ² - (a+1)β - 1
  --     = (a²-a)β² + (a-1)β
  --     = (a-1)·β·(aβ+1)
  -- So (a-1 : ℝ) * β * (aβ+1) = 0.
  -- Since β * (aβ+1) ≠ 0, we have a - 1 = 0.
  have h_factor_zero :
      ((a : ℝ) - 1) * β * ((a : ℝ) * β + 1) = 0 := by
    -- Goal: ((a-1)·β·(aβ+1)) = 0.
    -- Step 1: rewrite the LHS as a difference:
    --   (a-1)·β·(aβ+1) = (aβ+1)² - aβ² - (a+1)β - 1
    have heq : ((a : ℝ) - 1) * β * ((a : ℝ) * β + 1)
              = ((a : ℝ) * β + 1)^2 - (a : ℝ) * β^2
                - ((a : ℝ) + 1) * β - 1 := by ring
    rw [heq]
    -- Goal: ((aβ+1)² - aβ² - (a+1)β - 1) = 0.
    -- Step 2: replace (aβ+1)² with β³ using ← hps.
    rw [← hps]
    -- Goal: (β³ - aβ² - (a+1)β - 1) = 0.
    -- Step 3: rewrite aβ² + (a+1)β + 1 as β³ using hcubic.
    rw [hcubic]
    -- Goal: (β³ - β³) = 0.
    ring
  -- (a-1)·β·(aβ+1) = 0 with β·(aβ+1) ≠ 0 forces a - 1 = 0.
  -- Combine: from h_factor_zero we have (a-1)·β·(aβ+1) = 0.
  -- With β·(aβ+1) ≠ 0, conclude a - 1 = 0.
  have hprod : ((a : ℝ) - 1) * (β * ((a : ℝ) * β + 1)) = 0 := by
    have hcomm : ((a : ℝ) - 1) * β * ((a : ℝ) * β + 1) =
        ((a : ℝ) - 1) * (β * ((a : ℝ) * β + 1)) := by ring
    rw [← hcomm]
    exact h_factor_zero
  -- ((a-1) * β*(aβ+1) = 0) with β*(aβ+1) ≠ 0 forces (a-1) = 0.
  have ha_minus_one_zero : (a : ℝ) - 1 = 0 :=
    (mul_eq_zero_iff_right hfactor).mp hprod
  -- Convert (a : ℝ) - 1 = 0 to a = 1 in ℤ.
  have ha_eq_one : (a : ℤ) = 1 := by
    have h_eq : (a : ℝ) = (1 : ℝ) := by linarith
    exact_mod_cast h_eq
  exact ha_eq_one

/-- Cross-check: σ_{1,1} satisfies the cubic β³ = β² + 2β + 1, the
squared form. -/
example (β : ℝ) (hcubic : ClassIICubic 1 β) :
    β^3 - (β + 1)^2 = β^2 + 2 * β + 1 - (β^2 + 2 * β + 1) := by
  unfold ClassIICubic at hcubic
  rw [hcubic]
  ring

/-- The linear deviation identity: under the cubic hypothesis,
`v_0 / v_1 - v_1 / v_2 = 1 - a` EXACTLY.  Combined with the fact
that the deviation is zero only at `a = 1`, this gives a complete
closed-form progression rule for the σ_{a,1} family:

  * `a = 0` (Fibonacci):  deviation = 1, but with v_1 < v_2 (singular).
  * `a = 1` (balanced pivot):  deviation = 0 exactly.
  * `a ≥ 2`:  deviation = a - 1 (linear).

The proof combines the cubic with the difference-of-reciprocals
formula:

  v_0/v_1 - v_1/v_2 = β²/(aβ+1) - (aβ+1)/β
                   = [β³ - (aβ+1)²] / [β(aβ+1)]
                   = [(aβ² + (a+1)β + 1) - (aβ+1)²] / [β(aβ+1)]  (cubic)
                   = [(1-a)·β·(aβ+1)] / [β(aβ+1)]                  (ring)
                   = 1 - a. -/
theorem deviation_identity (a : ℤ) (β : ℝ)
    (ha : 1 ≤ a)
    (hβ : β > 1)
    (hcubic : ClassIICubic (a : ℝ) β) :
    WidthRatio01 (a : ℝ) β - WidthRatio12 (a : ℝ) β = 1 - (a : ℝ) := by
  unfold WidthRatio01 WidthRatio12
  -- Need: β ≠ 0 and a·β + 1 ≠ 0 for the denominators.
  have hβ_ne : β ≠ 0 := (lt_trans (by norm_num : (0 : ℝ) < 1) hβ).ne'
  have hdenom_ne : (a : ℝ) * β + 1 ≠ 0 := by
    have hβ_pos : (0 : ℝ) ≤ β :=
      le_of_lt (lt_trans (by norm_num : (0 : ℝ) < 1) hβ)
    have ha_pos : (0 : ℝ) ≤ (a : ℝ) := by
      have ha1 : (1 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
      exact le_of_lt (lt_of_lt_of_le
        (by norm_num : (0 : ℝ) < 1) ha1)
    have hab_pos : (0 : ℝ) ≤ (a : ℝ) * β := mul_nonneg ha_pos hβ_pos
    linarith
  field_simp [hβ_ne, hdenom_ne]
  rw [hcubic]
  ring

/-- Absolute deviation equals a - 1 for a ≥ 2.  Corollaries:
* For a = 0: deviation = 1 (Fibonacci anchor).
* For a = 1: deviation = 0 (balanced pivot, kernel-checked above).
* For a ≥ 2: deviation = a - 1. -/
theorem deviation_abs_eq_a_minus_one (a : ℤ) (β : ℝ)
    (ha : 2 ≤ a)
    (hβ : β > 1)
    (hcubic : ClassIICubic (a : ℝ) β) :
    |WidthRatio01 (a : ℝ) β - WidthRatio12 (a : ℝ) β| = (a : ℝ) - 1 := by
  -- Get hdev: WidthRatio01 - WidthRatio12 = 1 - a, which we know is
  -- non-positive for a ≥ 2.
  have h_one_le : (1 : ℤ) ≤ a := le_of_lt
    (lt_of_lt_of_le (by norm_num : (1 : ℤ) < 2) ha)
  have hdev := deviation_identity a β h_one_le hβ hcubic
  -- Now we have |1 - a| = a - 1 (since 1 - a ≤ 0).
  have h_neg : (1 : ℝ) - (a : ℝ) ≤ 0 := by
    have ha1 : (1 : ℝ) ≤ (a : ℝ) := by exact_mod_cast h_one_le
    linarith
  rw [show WidthRatio01 (a : ℝ) β - WidthRatio12 (a : ℝ) β
        = (1 : ℝ) - (a : ℝ) from hdev]
  rw [abs_of_nonpos h_neg]
  ring