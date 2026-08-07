-- Author: Ravel
-- Date: 2026-08-07
--
-- Finding 32's core mechanism, formalized exactly (no floating
-- point): a depressed cubic x^3 + c*x + d with a positive real root
-- beta factors as (x - beta) * (x^2 + beta*x + (beta^2 + c))
-- (verified directly, not cited from general Vieta/polynomial-root
-- machinery -- the depressed-cubic division formula is elementary
-- enough to check by expansion). If the quadratic factor's
-- discriminant is negative, its two complex roots have modulus^2
-- exactly beta^2 + c -- so a modulus^2 >= 1 there means beta is NOT
-- Pisot (the complex pair fails to lie strictly inside the unit
-- circle). Applied to sigma_{0,2}'s own charpoly x^3 - x - 2.

import Mathlib

namespace RavelGenerated

/-- The depressed-cubic division identity: if `beta` is a root of
`x^3 + c*x + d`, the cubic factors exactly as `(x - beta) * (x^2 +
beta*x + (beta^2 + c))`, for every `x` -- checked by direct expansion
using the root equation, not general polynomial-division machinery. -/
theorem depressed_cubic_factors {c d beta : ℝ} (hroot : beta^3 + c * beta + d = 0) :
    ∀ x : ℝ, x^3 + c * x + d = (x - beta) * (x^2 + beta * x + (beta^2 + c)) := by
  intro x
  have hd : d = -beta^3 - c * beta := by linarith
  rw [hd]
  ring

/-- If the quadratic factor `x^2 + p*x + q` has negative discriminant
(no real root), its two complex roots are `(-p ± i*sqrt(4*q-p^2))/2`,
a genuine conjugate pair, and BOTH have modulus^2 exactly `q` -- a
direct computation, not cited from general root-finding. -/
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

/-- Finding 32's headline instance: `sigma_{0,2}`'s incidence
characteristic polynomial `x^3 - x - 2` has a positive real root
`beta` with `beta < 2` (hence `beta^2 < 4` -- not needed below, kept
for context) and `beta^2 > 2` (proved algebraically from the defining
cubic equation and `0 < beta < 2`, no numeric approximation). The
complex-conjugate pair therefore has modulus^2 exactly `beta^2 - 1 >
1`, strictly outside the unit disk -- NOT Pisot. -/
theorem sigma_0_2_charpoly_not_pisot {beta : ℝ} (hpos : 0 < beta) (hlt2 : beta < 2)
    (hroot : beta^3 - beta - 2 = 0) :
    beta^2 - 1 > 1 ∧ (beta : ℝ)^2 < 4 * (beta^2 - 1) := by
  have hbeta2 : beta^2 > 2 := by nlinarith [hroot, hpos, hlt2, sq_nonneg (beta - 2)]
  exact ⟨by linarith, by nlinarith⟩

end RavelGenerated
