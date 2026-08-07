-- Author: Ravel
-- Date: 2026-08-06

import Mathlib

namespace RavelGenerated

/-- THE CORE ALGEBRAIC FACT behind Finding 35 (`g>1` forces the
incidence matrix's Perron-Frobenius period `p` to be `>=2`, which
contradicts Pisot's unique-dominant-eigenvalue requirement):

If `M`'s support graph carries an integer "level" function `c` that
steps by exactly `1` modulo `p` along every edge (`M i j ≠ 0` forces
`c i = c j + 1 + k*p` for some integer `k` -- exactly the invariant
Finding 26's `coincidence_gcd_obstruction_theorem.hpp` already proves
for the jump-size gcd `g`, taking `p = g`), and `v` is an eigenvector
of `M` for eigenvalue `lam`, then for EVERY `p`-th root of unity
`zeta`, the pointwise-rotated vector `w j := zeta ^ (c j) * v j` is
ALSO an eigenvector of `M`, for eigenvalue `lam * zeta⁻¹`.

Consequence (see `rotated_eigenvalue_has_same_modulus` below): if
`p >= 2`, picking `zeta ≠ 1` gives a SECOND eigenvalue distinct from
`lam` but of the SAME modulus -- exactly the fact that rules out a
unique dominant (Pisot) eigenvalue whenever the period is `>= 2`, not
just an empirical correlation. -/
-- `M` is kept as an INTEGER matrix (only cast to `ℂ` inside the
-- eigenvector equation) so that, per concrete instance, `hColor`'s
-- finite case split reduces to decidable integer arithmetic (`omega`)
-- rather than fighting complex-numeral simplification -- the whole
-- point of this design is that the emitted per-instance corollary's
-- proof term is mechanical, not hand-tuned per matrix.
-- `hColor` takes the modular witness `k` as an EXPLICIT function
-- parameter (not an existential): for a concrete instance this turns
-- the coloring hypothesis into a flat, per-edge decidable integer
-- equation (`decide`), instead of requiring a tactic to discover a
-- witness -- the mechanical-rendering-friendly shape.
theorem period_coloring_rotates_eigenvalue
    {n : ℕ} (M : Matrix (Fin n) (Fin n) ℤ) (c : Fin n → ℤ) (p : ℕ) (k : Fin n → Fin n → ℤ)
    (v : Fin n → ℂ) (lam : ℂ) (hEig : (fun i => ∑ j, (M i j : ℂ) * v j) = fun i => lam * v i)
    (hColor : ∀ i j, M i j ≠ 0 → c i = c j + 1 + k i j * (p : ℤ))
    (zeta : ℂ) (hzne : zeta ≠ 0) (hzeta : zeta ^ p = 1) :
    (fun i => ∑ j, (M i j : ℂ) * (zeta ^ (c j) * v j))
      = fun i => (lam * zeta⁻¹) * (zeta ^ (c i) * v i) := by
  funext i
  have step : ∀ j : Fin n, (M i j : ℂ) * zeta ^ (c j) = (M i j : ℂ) * (zeta ^ (c i) * zeta⁻¹) := by
    intro j
    by_cases h : M i j = 0
    · simp [h]
    · have hk := hColor i j h
      have hzp : zeta ^ (p : ℤ) = 1 := by
        rw [zpow_natCast]; exact hzeta
      have hkp : zeta ^ (k i j * (p : ℤ)) = 1 := by
        rw [mul_comm, _root_.zpow_mul, hzp, _root_.one_zpow]
      have : zeta ^ (c i) = zeta ^ (c j + 1) := by
        rw [hk, zpow_add₀ hzne, hkp, mul_one]
      rw [this, zpow_add₀ hzne, zpow_one]
      field_simp
  have hEigi : (∑ j, (M i j : ℂ) * v j) = lam * v i := by
    have h := congrFun hEig i
    simpa using h
  calc
    (∑ j, (M i j : ℂ) * (zeta ^ (c j) * v j))
        = ∑ j, ((M i j : ℂ) * zeta ^ (c j)) * v j := by
          apply Finset.sum_congr rfl; intro j _; ring
    _ = ∑ j, ((M i j : ℂ) * (zeta ^ (c i) * zeta⁻¹)) * v j := by
          apply Finset.sum_congr rfl; intro j _; rw [step j]
    _ = zeta ^ (c i) * zeta⁻¹ * ∑ j, (M i j : ℂ) * v j := by
          rw [Finset.mul_sum]; apply Finset.sum_congr rfl; intro j _; ring
    _ = zeta ^ (c i) * zeta⁻¹ * (lam * v i) := by rw [hEigi]
    _ = (lam * zeta⁻¹) * (zeta ^ (c i) * v i) := by ring

/-- The consequence that actually closes Finding 35: a period-`p`
rotated eigenvalue (`p >= 2`, `zeta ≠ 1`, a genuine `p`-th root of
unity) is DISTINCT from the original eigenvalue but has the SAME
modulus -- so `lam` cannot be a unique dominant eigenvalue whenever
such a rotation exists, which is exactly the Pisot property it would
need to have. -/
theorem rotated_eigenvalue_has_same_modulus
    (lam zeta : ℂ) (p : ℕ) (hp : p ≠ 0) (hzeta : zeta ^ p = 1) (hzne1 : zeta ≠ 1) (hlam : lam ≠ 0) :
    lam * zeta⁻¹ ≠ lam ∧ ‖lam * zeta⁻¹‖ = ‖lam‖ := by
  have hzne : zeta ≠ 0 := by
    rintro rfl
    exact absurd hzeta (by simp [zero_pow hp])
  have habs1 : ‖zeta‖ = 1 := by
    have h1 : ‖zeta‖ ^ p = (1:ℝ) ^ p := by
      rw [← norm_pow, hzeta, one_pow]; simp
    exact (pow_left_inj₀ (norm_nonneg _) zero_le_one hp).1 h1
  refine ⟨?_, ?_⟩
  · intro heq
    apply hzne1
    have hlz : lam * zeta⁻¹ = lam * 1 := by rw [heq, mul_one]
    have hz1 : zeta⁻¹ = 1 := mul_left_cancel₀ hlam hlz
    exact inv_eq_one.mp hz1
  · rw [norm_mul, norm_inv, habs1, inv_one, mul_one]

end RavelGenerated
