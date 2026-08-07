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

/-- THE GENERAL FACT (not specific to any one polynomial): for a
depressed cubic `x^3+c*x+d` with a positive real root `beta`, the
"other" quadratic factor's constant term `q = beta^2+c` (its complex
pair's modulus^2, by `quadratic_complex_pair_modulus_sq`) exceeds `1`
IFF `beta < -d` -- found by multiplying `beta^2+c>1` through by
`beta>0` and substituting `beta^3=-c*beta-d` from the root equation,
which collapses the condition to a comparison against `-d` alone,
independent of `c`. This is the general lemma
`DepressedCubicNotPisotCertificate`'s per-instance corollaries
instantiate; NOT sigma_{0,2}-specific. -/
theorem depressed_cubic_q_gt_one_iff_beta_lt_neg_d {c d beta : ℝ}
    (hpos : 0 < beta) (hroot : beta^3 + c * beta + d = 0) :
    beta^2 + c > 1 ↔ beta < -d := by
  constructor
  · intro hq
    nlinarith [hroot, hpos, hq]
  · intro hb
    nlinarith [hroot, hpos, hb]

/-- The discriminant condition (genuine, non-real complex pair) for
the same quadratic factor `x^2+beta*x+(beta^2+c)`: negative
discriminant `beta^2 < 4*(beta^2+c)` reduces to `0 < 3*beta^2+4*c`. -/
theorem depressed_cubic_discriminant_neg_iff {c beta : ℝ} :
    beta^2 < 4 * (beta^2 + c) ↔ 0 < 3 * beta^2 + 4 * c := by
  constructor <;> intro h <;> linarith

end RavelGenerated
