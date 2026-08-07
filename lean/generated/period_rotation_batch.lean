import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- THE CORE ALGEBRAIC FACT behind Finding 35 (`g>1` forces the incidence
    matrix's Perron-Frobenius period to be `>=2`, contradicting Pisot's
    unique-dominant-eigenvalue requirement): a period-`p` integer coloring of
    `M`'s support graph rotates any eigenvalue `lam` to `lam * zeta⁻¹` for
    every `p`-th root of unity `zeta`. Reproduced from the independently
    kernel-checked `lean/period_rotation_forces_equal_modulus.lean` (not
    re-derived here). -/
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

/-- A period-`p` rotated eigenvalue (`p >= 2`, `zeta ≠ 1`) is DISTINCT from
    the original but has the SAME modulus -- rules out a unique dominant
    (Pisot) eigenvalue whenever such a rotation exists. Reproduced from
    `lean/period_rotation_forces_equal_modulus.lean`. -/
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

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix and coloring (junction jump-gcd g=2 on 3 letters). -/
theorem period_rotation_instance_0 (v : Fin 3 → ℂ) (lam : ℂ)
    (hEig : (fun i => ∑ j, ((!![0, 1, 1; 1, 0, 0; 1, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) i j : ℂ) * v j) = fun i => lam * v i)
    (zeta : ℂ) (hzne : zeta ≠ 0) (hzeta : zeta ^ 2 = 1) :
    (fun i => ∑ j, ((!![0, 1, 1; 1, 0, 0; 1, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) i j : ℂ) * (zeta ^ ((![0, 1, 1] : Fin 3 → ℤ) j) * v j))
      = fun i => (lam * zeta⁻¹) * (zeta ^ ((![0, 1, 1] : Fin 3 → ℤ) i) * v i) :=
  period_coloring_rotates_eigenvalue (!![0, 1, 1; 1, 0, 0; 1, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) (![0, 1, 1] : Fin 3 → ℤ) 2 (!![0, -1, -1; 0, 0, 0; 0, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) v lam hEig
    (by decide) zeta hzne hzeta

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix and coloring (junction jump-gcd g=2 on 8 letters). -/
theorem period_rotation_instance_1 (v : Fin 8 → ℂ) (lam : ℂ)
    (hEig : (fun i => ∑ j, ((!![0, 0, 0, 1, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 8) (Fin 8) ℤ) i j : ℂ) * v j) = fun i => lam * v i)
    (zeta : ℂ) (hzne : zeta ≠ 0) (hzeta : zeta ^ 2 = 1) :
    (fun i => ∑ j, ((!![0, 0, 0, 1, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 8) (Fin 8) ℤ) i j : ℂ) * (zeta ^ ((![0, 1, 2, 1, 3, 3, 4, 5] : Fin 8 → ℤ) j) * v j))
      = fun i => (lam * zeta⁻¹) * (zeta ^ ((![0, 1, 2, 1, 3, 3, 4, 5] : Fin 8 → ℤ) i) * v i) :=
  period_coloring_rotates_eigenvalue (!![0, 0, 0, 1, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 8) (Fin 8) ℤ) (![0, 1, 2, 1, 3, 3, 4, 5] : Fin 8 → ℤ) 2 (!![0, 0, 0, -1, -2, 0, 0, -3; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0] : Matrix (Fin 8) (Fin 8) ℤ) v lam hEig
    (by decide) zeta hzne hzeta

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix and coloring (junction jump-gcd g=4 on 7 letters). -/
theorem period_rotation_instance_2 (v : Fin 7 → ℂ) (lam : ℂ)
    (hEig : (fun i => ∑ j, ((!![0, 0, 0, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 7) (Fin 7) ℤ) i j : ℂ) * v j) = fun i => lam * v i)
    (zeta : ℂ) (hzne : zeta ≠ 0) (hzeta : zeta ^ 4 = 1) :
    (fun i => ∑ j, ((!![0, 0, 0, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 7) (Fin 7) ℤ) i j : ℂ) * (zeta ^ ((![0, 1, 2, 3, 1, 2, 3] : Fin 7 → ℤ) j) * v j))
      = fun i => (lam * zeta⁻¹) * (zeta ^ ((![0, 1, 2, 3, 1, 2, 3] : Fin 7 → ℤ) i) * v i) :=
  period_coloring_rotates_eigenvalue (!![0, 0, 0, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 7) (Fin 7) ℤ) (![0, 1, 2, 3, 1, 2, 3] : Fin 7 → ℤ) 4 (!![0, 0, 0, -1, 0, 0, -1; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0] : Matrix (Fin 7) (Fin 7) ℤ) v lam hEig
    (by decide) zeta hzne hzeta

/-- THE SHARED GRAPH FACT underlying both Finding 35 and Finding 26: if a
    directed graph `E` on `Fin n` carries an integer coloring `c` that steps by
    exactly `1` modulo `p` along every edge, then walking `L` edges from any
    start vertex changes the coloring by exactly `L` modulo `p`. Reproduced from
    the independently kernel-checked
    `lean/period_rotation_forces_equal_modulus.lean` (not re-derived here). -/
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

/-- Finding 26's own conclusion, in general graph form: TWO walks between the
    SAME pair of vertices must have lengths differing by a multiple of `p`.
    Reproduced from `lean/period_rotation_forces_equal_modulus.lean`. -/
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

/-- Mechanically emitted: Finding 26's own conclusion for this
    substitution's full-alphabet graph and coloring (junction jump-gcd g=2 on 3 letters). -/
theorem walk_congruence_instance_0 (w1 w2 : ℕ → Fin 3) (L1 L2 : ℕ)
    (hwalk1 : ∀ i, i < L1 → (fun i j => (!![0, 1, 1; 1, 0, 0; 1, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) j i ≠ 0) (w1 i) (w1 (i + 1)))
    (hwalk2 : ∀ i, i < L2 → (fun i j => (!![0, 1, 1; 1, 0, 0; 1, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) j i ≠ 0) (w2 i) (w2 (i + 1)))
    (hstart : w1 0 = w2 0) (hend : w1 L1 = w2 L2) :
    ∃ m : ℤ, (L1 : ℤ) - (L2 : ℤ) = m * 2 :=
  colored_walk_lengths_agree_mod (fun i j => (!![0, 1, 1; 1, 0, 0; 1, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) j i ≠ 0) (![0, 1, 1] : Fin 3 → ℤ) 2 (fun i j => (!![0, -1, -1; 0, 0, 0; 0, 0, 0] : Matrix (Fin 3) (Fin 3) ℤ) j i)
    (by decide) w1 w2 L1 L2 hwalk1 hwalk2 hstart hend

/-- Mechanically emitted: Finding 26's own conclusion for this
    substitution's full-alphabet graph and coloring (junction jump-gcd g=2 on 8 letters). -/
theorem walk_congruence_instance_1 (w1 w2 : ℕ → Fin 8) (L1 L2 : ℕ)
    (hwalk1 : ∀ i, i < L1 → (fun i j => (!![0, 0, 0, 1, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 8) (Fin 8) ℤ) j i ≠ 0) (w1 i) (w1 (i + 1)))
    (hwalk2 : ∀ i, i < L2 → (fun i j => (!![0, 0, 0, 1, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 8) (Fin 8) ℤ) j i ≠ 0) (w2 i) (w2 (i + 1)))
    (hstart : w1 0 = w2 0) (hend : w1 L1 = w2 L2) :
    ∃ m : ℤ, (L1 : ℤ) - (L2 : ℤ) = m * 2 :=
  colored_walk_lengths_agree_mod (fun i j => (!![0, 0, 0, 1, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 8) (Fin 8) ℤ) j i ≠ 0) (![0, 1, 2, 1, 3, 3, 4, 5] : Fin 8 → ℤ) 2 (fun i j => (!![0, 0, 0, -1, -2, 0, 0, -3; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0, 0] : Matrix (Fin 8) (Fin 8) ℤ) j i)
    (by decide) w1 w2 L1 L2 hwalk1 hwalk2 hstart hend

/-- Mechanically emitted: Finding 26's own conclusion for this
    substitution's full-alphabet graph and coloring (junction jump-gcd g=4 on 7 letters). -/
theorem walk_congruence_instance_2 (w1 w2 : ℕ → Fin 7) (L1 L2 : ℕ)
    (hwalk1 : ∀ i, i < L1 → (fun i j => (!![0, 0, 0, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 7) (Fin 7) ℤ) j i ≠ 0) (w1 i) (w1 (i + 1)))
    (hwalk2 : ∀ i, i < L2 → (fun i j => (!![0, 0, 0, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 7) (Fin 7) ℤ) j i ≠ 0) (w2 i) (w2 (i + 1)))
    (hstart : w1 0 = w2 0) (hend : w1 L1 = w2 L2) :
    ∃ m : ℤ, (L1 : ℤ) - (L2 : ℤ) = m * 4 :=
  colored_walk_lengths_agree_mod (fun i j => (!![0, 0, 0, 1, 0, 0, 1; 1, 0, 0, 0, 0, 0, 0; 0, 1, 0, 0, 0, 0, 0; 0, 0, 1, 0, 0, 0, 0; 1, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 1, 0, 0; 0, 0, 0, 0, 0, 1, 0] : Matrix (Fin 7) (Fin 7) ℤ) j i ≠ 0) (![0, 1, 2, 3, 1, 2, 3] : Fin 7 → ℤ) 4 (fun i j => (!![0, 0, 0, -1, 0, 0, -1; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0, 0] : Matrix (Fin 7) (Fin 7) ℤ) j i)
    (by decide) w1 w2 L1 L2 hwalk1 hwalk2 hstart hend

/- Semantic proof graph for: period_rotation_batch
  [0] lean.period_rotation_certificate :: n=3 p=2 junction jump-gcd g=2 on 3 letters -- instantiates period_coloring_rotates_eigenvalue
  [1] lean.period_rotation_certificate :: n=8 p=2 junction jump-gcd g=2 on 8 letters -- instantiates period_coloring_rotates_eigenvalue
  [2] lean.period_rotation_certificate :: n=7 p=4 junction jump-gcd g=4 on 7 letters -- instantiates period_coloring_rotates_eigenvalue
-/

def reflectedNodeCount : Nat := 3

end RavelGenerated
