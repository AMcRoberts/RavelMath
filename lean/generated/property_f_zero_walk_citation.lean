import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- The walk underlying the property-(F) zero-expansion automaton:
    `γ_0 = 0`, `γ_{k+1} = (γ_k + δ_k) / β`. -/
noncomputable def zeroWalk (β : ℝ) (δ : ℕ → ℝ) : ℕ → ℝ
  | 0 => 0
  | (k + 1) => (zeroWalk β δ k + δ k) / β

theorem zeroWalk_nonneg (β : ℝ) (hβ : 1 < β) (δ : ℕ → ℝ) (hδ : ∀ k, 0 ≤ δ k) :
    ∀ n, 0 ≤ zeroWalk β δ n := by
  intro n
  induction n with
  | zero => simp [zeroWalk]
  | succ k ih =>
      have hβ0 : (0 : ℝ) < β := lt_trans one_pos hβ
      have hsum : 0 ≤ zeroWalk β δ k + δ k := add_nonneg ih (hδ k)
      simpa [zeroWalk] using div_nonneg hsum hβ0.le

/-- THE CORE LEMMA behind property (F) (Findings 18-21): the walk returns to
    exactly `0` at step `n` iff EVERY increment before step `n` was itself
    exactly `0` -- so no nonzero-`γ` node can ever have an edge into a
    zero-`γ` node, i.e. a mixed zero/nonzero SCC can never occur. Reproduced
    from the independently kernel-checked
    `lean/generated/property_f_zero_walk.lean` (not re-derived here). -/
theorem zeroWalk_eq_zero_iff (β : ℝ) (hβ : 1 < β) (δ : ℕ → ℝ) (hδ : ∀ k, 0 ≤ δ k) :
    ∀ n, zeroWalk β δ n = 0 ↔ ∀ k < n, δ k = 0 := by
  intro n
  induction n with
  | zero => simp [zeroWalk]
  | succ k ih =>
      have hβ0 : (0 : ℝ) < β := lt_trans one_pos hβ
      have hnonneg := zeroWalk_nonneg β hβ δ hδ k
      constructor
      · intro hz
        have hsum : zeroWalk β δ k + δ k = 0 := by
          have hz' : (zeroWalk β δ k + δ k) / β = 0 := by
            simpa [zeroWalk] using hz
          have := (div_eq_zero_iff).mp hz'
          rcases this with h | h
          · exact h
          · exact absurd h (ne_of_gt hβ0)
        have h1 : zeroWalk β δ k = 0 := by linarith [hδ k]
        have h2 : δ k = 0 := by linarith
        intro j hj
        rcases lt_or_eq_of_le (Nat.lt_succ_iff.mp hj) with hjk | hjk
        · exact (ih.mp h1) j hjk
        · rw [hjk]; exact h2
      · intro hall
        have h1 : ∀ j < k, δ j = 0 := fun j hj => hall j (Nat.lt_succ_of_lt hj)
        have hzk : zeroWalk β δ k = 0 := ih.mpr h1
        have hdk : δ k = 0 := hall k (Nat.lt_succ_self k)
        simp [zeroWalk, hzk, hdk]

/- Semantic proof graph for: property_f_fibonacci
  [0] lean.lemma_application :: zeroWalk_eq_zero_iff proves a mixed zero/nonzero SCC can never occur in the property-(F) zero-expansion automaton, so checking scc_has_nonzero alone correctly implements the paper's 'not entirely zero' violation condition
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
