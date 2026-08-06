import Mathlib.Tactic
namespace RavelGenerated
 theorem cover_tube_coordinate_bound {n : ℕ} (M Δ : ℤ) (x t : Fin n → ℤ) (hx : ∀ i, |x i| ≤ M) (ht : ∀ i, |t i| ≤ Δ) : ∀ i, |x i + t i| ≤ M + Δ := by
  intro i
  calc |x i + t i| ≤ |x i| + |t i| := abs_add _ _
       _ ≤ M + Δ := add_le_add (hx i) (ht i)
 theorem affine_path_transport {State : Type} [AddCommGroup State] (A : State →+ State) (forcing : ℕ → State) (base translation transported : ℕ → State) (h0 : transported 0 = base 0 + translation 0) (hb : ∀ k, base (k+1)=A(base k)+forcing k) (ht : ∀ k, translation(k+1)=A(translation k)) (hz : ∀ k, transported(k+1)=A(transported k)+forcing k) : ∀ k, transported k=base k+translation k := by
  intro k; induction k with
  | zero => exact h0
  | succ k ih => rw [hz k, ih, map_add, hb k, ht k]; abel
end RavelGenerated
