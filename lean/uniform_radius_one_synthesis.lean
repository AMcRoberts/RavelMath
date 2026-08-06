import Mathlib.Tactic

namespace RavelGenerated

/-- The n-bonacci state update with an arbitrary integer forcing digit. -/
def forcedStep {n : ℕ} (x : Fin (n + 1) → ℤ) (digit : ℤ) :
    Fin (n + 1) → ℤ := fun i =>
  if h : (i : ℕ) + 1 < n + 1 then
    x ⟨(i : ℕ) + 1, h⟩
  else
    x 0 - (∑ j : Fin n, x j.succ) + digit

/-- A defect in the forcing coordinate is absorbed exactly by changing the
input digit by the same amount. This is the algebraic core of the uniform
radius-one controller. -/
theorem forcedStep_add_translation
    {n : ℕ}
    (x t : Fin (n + 1) → ℤ)
    (digit defect : ℤ) :
    forcedStep (fun i => x i + t i) (digit + defect) =
      fun i => forcedStep x digit i + forcedStep t defect i := by
  funext i
  simp only [forcedStep]
  split_ifs with h
  · rfl
  · simp only [Finset.sum_add_distrib]
    ring

/-- Radius-one tube geometry is independent of the shell radius. If the base
path is strictly inside the M-box at an intermediate time, adding any
radius-one translation remains strictly inside the (M+1)-box. -/
theorem interior_transport_radius_one
    {n : ℕ}
    (M : ℤ)
    (x t : Fin n → ℤ)
    (hx : ∀ i, |x i| ≤ M - 1)
    (ht : ∀ i, |t i| ≤ 1) :
    ∀ i, |x i + t i| ≤ M := by
  intro i
  calc
    |x i + t i| ≤ |x i| + |t i| := abs_add _ _
    _ ≤ (M - 1) + 1 := add_le_add (hx i) (ht i)
    _ = M := by ring

/-- Alignment with one outer face promotes a boundary point of the M-box to a
boundary point of the (M+1)-box. -/
theorem boundary_alignment_radius_one
    {n : ℕ}
    (M : ℤ)
    (x t : Fin n → ℤ)
    (i : Fin n)
    (sign : ℤ)
    (hsign : sign = 1 ∨ sign = -1)
    (hx : x i = sign * M)
    (ht : t i = sign) :
    |x i + t i| = M + 1 := by
  rcases hsign with rfl | rfl <;> simp [hx, ht]

/-- Generic correctness interface for a synthesized controller. Once a finite
state search supplies radius-one translation windows, admissible adjusted
inputs, and source/target face alignment, the resulting path is an exact
outer-shell first-return transport for every shell radius M. -/
theorem uniform_radius_one_transport_correct
    {State : Type}
    [AddCommGroup State]
    (A : State →+ State)
    (base translation transported : ℕ → State)
    (digit adjusted defect forcing : ℕ → State)
    (hbase : ∀ k, base (k + 1) = A (base k) + digit k)
    (htranslation : ∀ k, translation (k + 1) = A (translation k) + defect k)
    (hadjusted : ∀ k, adjusted k = digit k + defect k)
    (htransported0 : transported 0 = base 0 + translation 0)
    (htransported : ∀ k, transported (k + 1) = A (transported k) + adjusted k) :
    ∀ k, transported k = base k + translation k := by
  intro k
  induction k with
  | zero => exact htransported0
  | succ k ih =>
      rw [htransported k, ih, map_add, hbase k, htranslation k, hadjusted k]
      abel

end RavelGenerated
