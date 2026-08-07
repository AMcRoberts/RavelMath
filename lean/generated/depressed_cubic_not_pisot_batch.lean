import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem depressed_cubic_factors {c d beta : ℝ} (hroot : beta^3 + c * beta + d = 0) :
    ∀ x : ℝ, x^3 + c * x + d = (x - beta) * (x^2 + beta * x + (beta^2 + c)) := by
  intro x
  have hd : d = -beta^3 - c * beta := by linarith
  rw [hd]
  ring

theorem quadratic_complex_pair_modulus_sq {p q : ℝ} (hdisc : p^2 < 4 * q) :
    ∃ z : ℂ, z^2 + (p:ℂ) * z + (q:ℂ) = 0 ∧ Complex.normSq z = q := by
  have hs : (Real.sqrt (4 * q - p^2) : ℝ) ^ 2 = 4 * q - p^2 :=
    Real.sq_sqrt (by linarith)
  set s : ℝ := Real.sqrt (4 * q - p^2) with hsdef
  refine ⟨Complex.mk (-p / 2) (s / 2), ?_, ?_⟩
  · apply Complex.ext
    · simp [Complex.mul_re, Complex.add_re, pow_two]
      nlinarith [hs]
    · simp [Complex.mul_im, Complex.add_im, pow_two]
      ring
  · rw [Complex.normSq_mk]
    nlinarith [hs]

/-- THE GENERAL FACT: for a depressed cubic `x^3+c*x+d` with positive real
    root `beta`, the complex pair's modulus^2 `beta^2+c` exceeds `1` IFF
    `beta < -d` -- independent of `c`. Reproduced from the independently
    kernel-checked `lean/depressed_cubic_complex_pair_modulus.lean` (not
    re-derived here). -/
theorem depressed_cubic_q_gt_one_iff_beta_lt_neg_d {c d beta : ℝ}
    (hpos : 0 < beta) (hroot : beta^3 + c * beta + d = 0) :
    beta^2 + c > 1 ↔ beta < -d := by
  constructor
  · intro hq; nlinarith [hroot, hpos, hq]
  · intro hb; nlinarith [hroot, hpos, hb]

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-1x-2, bracket (0,2). -/
theorem depressed_cubic_instance_0 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (0 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (-1 : ℝ) * beta + (-2 : ℝ) = 0) :
    beta^2 + (-1 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-2 : ℝ) := by norm_num
  have hb : beta < -(-2 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-3x-6, bracket (2,3). -/
theorem depressed_cubic_instance_1 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (2 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-3 : ℝ) * beta + (-6 : ℝ) = 0) :
    beta^2 + (-3 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-6 : ℝ) := by norm_num
  have hb : beta < -(-6 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-3x-5, bracket (2,3). -/
theorem depressed_cubic_instance_2 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (2 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-3 : ℝ) * beta + (-5 : ℝ) = 0) :
    beta^2 + (-3 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-5 : ℝ) := by norm_num
  have hb : beta < -(-5 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-3x-4, bracket (2,3). -/
theorem depressed_cubic_instance_3 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (2 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-3 : ℝ) * beta + (-4 : ℝ) = 0) :
    beta^2 + (-3 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-4 : ℝ) := by norm_num
  have hb : beta < -(-4 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-3x-3, bracket (2,3). -/
theorem depressed_cubic_instance_4 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (2 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-3 : ℝ) * beta + (-3 : ℝ) = 0) :
    beta^2 + (-3 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-3 : ℝ) := by norm_num
  have hb : beta < -(-3 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-2x-6, bracket (2,3). -/
theorem depressed_cubic_instance_5 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (2 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-2 : ℝ) * beta + (-6 : ℝ) = 0) :
    beta^2 + (-2 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-6 : ℝ) := by norm_num
  have hb : beta < -(-6 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-2x-5, bracket (2,3). -/
theorem depressed_cubic_instance_6 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (2 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-2 : ℝ) * beta + (-5 : ℝ) = 0) :
    beta^2 + (-2 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-5 : ℝ) := by norm_num
  have hb : beta < -(-5 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-2x-4, bracket (1,3). -/
theorem depressed_cubic_instance_7 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-2 : ℝ) * beta + (-4 : ℝ) = 0) :
    beta^2 + (-2 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-4 : ℝ) := by norm_num
  have hb : beta < -(-4 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-2x-3, bracket (1,2). -/
theorem depressed_cubic_instance_8 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (-2 : ℝ) * beta + (-3 : ℝ) = 0) :
    beta^2 + (-2 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-3 : ℝ) := by norm_num
  have hb : beta < -(-3 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-2x-2, bracket (1,2). -/
theorem depressed_cubic_instance_9 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (-2 : ℝ) * beta + (-2 : ℝ) = 0) :
    beta^2 + (-2 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-2 : ℝ) := by norm_num
  have hb : beta < -(-2 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-1x-6, bracket (1,3). -/
theorem depressed_cubic_instance_10 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (3 : ℝ))
    (hroot : beta^3 + (-1 : ℝ) * beta + (-6 : ℝ) = 0) :
    beta^2 + (-1 : ℝ) > 1 := by
  have hbound : (3 : ℝ) ≤ -(-6 : ℝ) := by norm_num
  have hb : beta < -(-6 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-1x-5, bracket (1,2). -/
theorem depressed_cubic_instance_11 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (-1 : ℝ) * beta + (-5 : ℝ) = 0) :
    beta^2 + (-1 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-5 : ℝ) := by norm_num
  have hb : beta < -(-5 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-1x-4, bracket (1,2). -/
theorem depressed_cubic_instance_12 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (-1 : ℝ) * beta + (-4 : ℝ) = 0) :
    beta^2 + (-1 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-4 : ℝ) := by norm_num
  have hb : beta < -(-4 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-1x-3, bracket (1,2). -/
theorem depressed_cubic_instance_13 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (-1 : ℝ) * beta + (-3 : ℝ) = 0) :
    beta^2 + (-1 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-3 : ℝ) := by norm_num
  have hb : beta < -(-3 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3-1x-2, bracket (1,2). -/
theorem depressed_cubic_instance_14 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (-1 : ℝ) * beta + (-2 : ℝ) = 0) :
    beta^2 + (-1 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-2 : ℝ) := by norm_num
  have hb : beta < -(-2 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3+0x-6, bracket (1,2). -/
theorem depressed_cubic_instance_15 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (0 : ℝ) * beta + (-6 : ℝ) = 0) :
    beta^2 + (0 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-6 : ℝ) := by norm_num
  have hb : beta < -(-6 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3+0x-5, bracket (1,2). -/
theorem depressed_cubic_instance_16 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (0 : ℝ) * beta + (-5 : ℝ) = 0) :
    beta^2 + (0 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-5 : ℝ) := by norm_num
  have hb : beta < -(-5 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3+0x-4, bracket (1,2). -/
theorem depressed_cubic_instance_17 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (0 : ℝ) * beta + (-4 : ℝ) = 0) :
    beta^2 + (0 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-4 : ℝ) := by norm_num
  have hb : beta < -(-4 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3+0x-3, bracket (1,2). -/
theorem depressed_cubic_instance_18 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (0 : ℝ) * beta + (-3 : ℝ) = 0) :
    beta^2 + (0 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-3 : ℝ) := by norm_num
  have hb : beta < -(-3 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/-- Mechanically emitted: instantiates the general lemma above for
    x^3+0x-2, bracket (1,2). -/
theorem depressed_cubic_instance_19 {beta : ℝ} (hpos : 0 < beta)
    (hlo : (1 : ℝ) < beta) (hhi : beta < (2 : ℝ))
    (hroot : beta^3 + (0 : ℝ) * beta + (-2 : ℝ) = 0) :
    beta^2 + (0 : ℝ) > 1 := by
  have hbound : (2 : ℝ) ≤ -(-2 : ℝ) := by norm_num
  have hb : beta < -(-2 : ℝ) := by linarith
  exact (depressed_cubic_q_gt_one_iff_beta_lt_neg_d hpos hroot).mpr hb

/- Semantic proof graph for: depressed_cubic_batch
  [0] lean.depressed_cubic_not_pisot_certificate :: c=-1 d=-2 bracket=(0,2) x^3-1x-2, bracket (0,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [1] lean.depressed_cubic_not_pisot_certificate :: c=-3 d=-6 bracket=(2,3) x^3-3x-6, bracket (2,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [2] lean.depressed_cubic_not_pisot_certificate :: c=-3 d=-5 bracket=(2,3) x^3-3x-5, bracket (2,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [3] lean.depressed_cubic_not_pisot_certificate :: c=-3 d=-4 bracket=(2,3) x^3-3x-4, bracket (2,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [4] lean.depressed_cubic_not_pisot_certificate :: c=-3 d=-3 bracket=(2,3) x^3-3x-3, bracket (2,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [5] lean.depressed_cubic_not_pisot_certificate :: c=-2 d=-6 bracket=(2,3) x^3-2x-6, bracket (2,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [6] lean.depressed_cubic_not_pisot_certificate :: c=-2 d=-5 bracket=(2,3) x^3-2x-5, bracket (2,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [7] lean.depressed_cubic_not_pisot_certificate :: c=-2 d=-4 bracket=(1,3) x^3-2x-4, bracket (1,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [8] lean.depressed_cubic_not_pisot_certificate :: c=-2 d=-3 bracket=(1,2) x^3-2x-3, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [9] lean.depressed_cubic_not_pisot_certificate :: c=-2 d=-2 bracket=(1,2) x^3-2x-2, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [10] lean.depressed_cubic_not_pisot_certificate :: c=-1 d=-6 bracket=(1,3) x^3-1x-6, bracket (1,3) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [11] lean.depressed_cubic_not_pisot_certificate :: c=-1 d=-5 bracket=(1,2) x^3-1x-5, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [12] lean.depressed_cubic_not_pisot_certificate :: c=-1 d=-4 bracket=(1,2) x^3-1x-4, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [13] lean.depressed_cubic_not_pisot_certificate :: c=-1 d=-3 bracket=(1,2) x^3-1x-3, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [14] lean.depressed_cubic_not_pisot_certificate :: c=-1 d=-2 bracket=(1,2) x^3-1x-2, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [15] lean.depressed_cubic_not_pisot_certificate :: c=0 d=-6 bracket=(1,2) x^3+0x-6, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [16] lean.depressed_cubic_not_pisot_certificate :: c=0 d=-5 bracket=(1,2) x^3+0x-5, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [17] lean.depressed_cubic_not_pisot_certificate :: c=0 d=-4 bracket=(1,2) x^3+0x-4, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [18] lean.depressed_cubic_not_pisot_certificate :: c=0 d=-3 bracket=(1,2) x^3+0x-3, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
  [19] lean.depressed_cubic_not_pisot_certificate :: c=0 d=-2 bracket=(1,2) x^3+0x-2, bracket (1,2) -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d
-/

def reflectedNodeCount : Nat := 20

end RavelGenerated
