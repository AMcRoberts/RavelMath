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

/-- THE SHARED GRAPH FACT underlying both Finding 35 (above) and
Finding 26 (`coincidence_gcd_obstruction_theorem.hpp`'s own `d +
dist(terminal) = 0 (mod g)` invariant): if a directed graph `E` on
`Fin n` carries an integer coloring `c` that steps by exactly `1`
modulo `p` along every edge, then walking `L` edges from any start
vertex changes the coloring by exactly `L` modulo `p` -- the general
mechanism this project's own C++ certificate
(`ravel/proof/period_rotation_certificate.hpp`) already exploits (via
BFS-distance colorings) to justify `period_coloring_rotates_eigenvalue`
above, extracted here as its own reusable fact rather than re-derived
inline for each finding that needs it. A walk is represented directly
as `w : ℕ → Fin n` together with a length `L` and a step hypothesis,
avoiding any List-API dependency. -/
theorem colored_walk_congruence
    {n : ℕ} (E : Fin n → Fin n → Prop) (c : Fin n → ℤ) (p : ℕ) (k : Fin n → Fin n → ℤ)
    (hColor : ∀ i j, E i j → c j = c i + 1 + k i j * (p : ℤ))
    (w : ℕ → Fin n) (L : ℕ) (hwalk : ∀ i, i < L → E (w i) (w (i + 1))) :
    ∃ m : ℤ, c (w L) = c (w 0) + (L : ℤ) + m * (p : ℤ) := by
  induction L with
  | zero => exact ⟨0, by simp⟩
  | succ L ih =>
    obtain ⟨m, hm⟩ := ih (fun i hi => hwalk i (Nat.lt_succ_of_lt hi))
    have hstep := hColor (w L) (w (L + 1)) (hwalk L (Nat.lt_succ_self L))
    refine ⟨m + k (w L) (w (L + 1)), ?_⟩
    rw [hstep, hm]
    simp only [Nat.cast_add, Nat.cast_one]
    ring

/-- Finding 26's own conclusion, in general graph form: TWO walks
between the SAME pair of vertices (same start, same end) must have
lengths differing by a multiple of `p` -- exactly "the gap between two
run-in lengths reaching the same coincidence-closure state is
divisible by g", stated and proved purely in terms of the coloring's
existence, with no reference to the coincidence closure's own
bookkeeping. -/
theorem colored_walk_lengths_agree_mod
    {n : ℕ} (E : Fin n → Fin n → Prop) (c : Fin n → ℤ) (p : ℕ) (k : Fin n → Fin n → ℤ)
    (hColor : ∀ i j, E i j → c j = c i + 1 + k i j * (p : ℤ))
    (w1 w2 : ℕ → Fin n) (L1 L2 : ℕ)
    (hwalk1 : ∀ i, i < L1 → E (w1 i) (w1 (i + 1)))
    (hwalk2 : ∀ i, i < L2 → E (w2 i) (w2 (i + 1)))
    (hstart : w1 0 = w2 0) (hend : w1 L1 = w2 L2) :
    ∃ m : ℤ, (L1 : ℤ) - (L2 : ℤ) = m * (p : ℤ) := by
  obtain ⟨m1, hm1⟩ := colored_walk_congruence E c p k hColor w1 L1 hwalk1
  obtain ⟨m2, hm2⟩ := colored_walk_congruence E c p k hColor w2 L2 hwalk2
  rw [hstart, hend] at hm1
  refine ⟨m2 - m1, ?_⟩
  linear_combination hm2 - hm1

end RavelGenerated
