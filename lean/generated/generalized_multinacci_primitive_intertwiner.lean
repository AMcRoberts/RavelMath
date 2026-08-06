import Mathlib

namespace RavelGenerated

/-- A generator-labelled edge map into a larger graph transports every
primitive edge when source and target maps commute with its label. -/
theorem labelled_subgraph_intertwiner
    {Γ S T : Type*}
    [DecidableEq S] [DecidableEq T]
    (A : Γ → S → S → ℕ)
    (B : Γ → T → T → ℕ)
    (f : S → T)
    (hsub : ∀ g s t, A g s t ≤ B g (f s) (f t)) :
    ∀ g s u,
      (∑ t, A g s t * if f t = u then 1 else 0) ≤
      (∑ v, (if f s = v then 1 else 0) * B g v u) := by
  intro g s u
  classical
  by_cases h : f s = u
  · subst u
    simp only [if_pos, one_mul]
    calc
      (∑ t, A g s t * if f t = f s then 1 else 0)
          ≤ ∑ t, B g (f s) (f t) * if f t = f s then 1 else 0 := by
              gcongr with t
              exact Nat.mul_le_mul_right _ (hsub g s t)
      _ ≤ ∑ v, B g (f s) v := by
              exact Finset.sum_le_sum_of_subset_of_nonneg
                (by intro x hx; simp)
                (by intro i hi hni; positivity)
  · simp [h]

end RavelGenerated
