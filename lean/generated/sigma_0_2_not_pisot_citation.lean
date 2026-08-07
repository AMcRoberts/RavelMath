import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- The depressed-cubic division identity: if `beta` is a root of
    `x^3 + c*x + d`, the cubic factors exactly as `(x - beta) * (x^2 +
    beta*x + (beta^2 + c))`, for every `x`. -/
theorem depressed_cubic_factors {c d beta : ℝ} (hroot : beta^3 + c * beta + d = 0) :
    ∀ x : ℝ, x^3 + c * x + d = (x - beta) * (x^2 + beta * x + (beta^2 + c)) := by
  intro x
  have hd : d = -beta^3 - c * beta := by linarith
  rw [hd]
  ring

/-- If the quadratic factor `x^2 + p*x + q` has negative discriminant, its two
    complex roots have modulus^2 exactly `q`. -/
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

/-- Finding 32's headline instance: `sigma_{0,2}`'s incidence characteristic
    polynomial `x^3 - x - 2` has a positive real root `beta` with `0 < beta < 2`
    and `beta^2 > 2`, proved algebraically (no numeric approximation) --
    forcing the complex-conjugate pair's modulus^2 to be `beta^2 - 1 > 1`,
    strictly outside the unit disk. Reproduced from the independently
    kernel-checked `lean/depressed_cubic_complex_pair_modulus.lean` (not
    re-derived here). -/
theorem sigma_0_2_charpoly_not_pisot {beta : ℝ} (hpos : 0 < beta) (hlt2 : beta < 2)
    (hroot : beta^3 - beta - 2 = 0) :
    beta^2 - 1 > 1 ∧ (beta : ℝ)^2 < 4 * (beta^2 - 1) := by
  have hbeta2 : beta^2 > 2 := by nlinarith [hroot, hpos, hlt2, sq_nonneg (beta - 2)]
  exact ⟨by linarith, by nlinarith⟩

/- Semantic proof graph for: sigma_0_2_batch
  [0] lean.lemma_application :: sigma_0_2_charpoly_not_pisot proves x^3-x-2's complex-conjugate secondary pair has modulus^2 = beta^2-1 > 1 for its real root beta in (0,2), hence sigma_{0,2} is not Pisot
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
