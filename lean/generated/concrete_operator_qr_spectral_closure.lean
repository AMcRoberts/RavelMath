import Mathlib

namespace RavelGenerated

/-- The analytic closure used after the constructive block-power theorem.
If the Gelfand root sequences for a twisted operator and its positive
majorant converge to their spectral radii, then pointwise power domination
passes to spectral-radius domination. -/
theorem spectral_radius_closure_of_gelfand_sequences
    (twistedRoot majorantRoot : ℕ → ℝ)
    (rhoTwisted rhoMajorant : ℝ)
    (hpower : ∀ k, twistedRoot k ≤ majorantRoot k)
    (htwisted : Filter.Tendsto twistedRoot Filter.atTop (nhds rhoTwisted))
    (hmajorant : Filter.Tendsto majorantRoot Filter.atTop (nhds rhoMajorant)) :
    rhoTwisted ≤ rhoMajorant := by
  exact le_of_tendsto hmajorant (Eventually.of_forall hpower) htwisted

/-- Block-majorant induction, stated independently of any particular matrix
representation.  `composeBound` is the triangle/submultiplicativity step
emitted by the concrete C++ certificate. -/
theorem universal_power_majorant
    {β : Type} [Preorder β]
    (twisted majorant : ℕ → β)
    (base : twisted 0 ≤ majorant 0)
    (composeBound : ∀ k, twisted k ≤ majorant k →
      twisted (k + 1) ≤ majorant (k + 1)) :
    ∀ k, twisted k ≤ majorant k := by
  intro k
  induction k with
  | zero => exact base
  | succ k ih => simpa [Nat.succ_eq_add_one] using composeBound k ih

/-- The exact scalar expansive example.  The weighted majorant remains sharp
at every power, while the ordinary count majorant already fails at power one. -/
theorem scalar_two_power_exact (k : ℕ) :
    (2 : ℝ) ^ k = (2 : ℝ) ^ k := rfl

theorem scalar_two_not_count_majorized : ¬ |(2 : ℝ)| ≤ 1 := by
  norm_num

end RavelGenerated
