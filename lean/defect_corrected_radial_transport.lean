import Mathlib.Tactic

namespace RavelGenerated

/-- Abstract one-step n-bonacci shift assumptions.  The first `n` coordinates
shift left, while the final coordinate is the signed aggregate. -/
structure NBonacciLinearStepData (n : ℕ) where
  source : Fin (n + 1) → ℤ
  stepped : Fin (n + 1) → ℤ
  shift : ∀ i : Fin n, stepped i.castSucc = source i.succ
  terminal : stepped (Fin.last n) =
    source 0 - ∑ i : Fin n, source i.succ

/-- The defect in every nonterminal coordinate is the adjacent boundary flux. -/
theorem nbonacci_radial_defect_adjacent
    {n : ℕ} (p : ℤ) (s : Fin (n + 1) → ℤ)
    (D : NBonacciLinearStepData n)
    (hsource : D.source = fun i => p * s i)
    (i : Fin n) :
    D.stepped i.castSucc - D.source i.castSucc =
      p * (s i.succ - s i.castSucc) := by
  rw [D.shift i, hsource]
  ring

/-- The final defect is controlled by one global signed imbalance. -/
theorem nbonacci_radial_defect_terminal
    {n : ℕ} (p : ℤ) (s : Fin (n + 1) → ℤ)
    (D : NBonacciLinearStepData n)
    (hsource : D.source = fun i => p * s i) :
    D.stepped (Fin.last n) - D.source (Fin.last n) =
      p * (s 0 - (∑ i : Fin n, s i.succ) - s (Fin.last n)) := by
  rw [D.terminal, hsource]
  simp only [Fin.isValue]
  rw [Finset.mul_sum]
  ring

/-- Consequently the entire radial translation defect is determined by
adjacent sign fluxes and the single terminal signed-imbalance expression. -/
theorem nbonacci_radial_defect_profile_complete
    {n : ℕ} (p : ℤ) (s : Fin (n + 1) → ℤ)
    (D : NBonacciLinearStepData n)
    (hsource : D.source = fun i => p * s i) :
    (∀ i : Fin n,
      D.stepped i.castSucc - D.source i.castSucc =
        p * (s i.succ - s i.castSucc)) ∧
    D.stepped (Fin.last n) - D.source (Fin.last n) =
      p * (s 0 - (∑ i : Fin n, s i.succ) - s (Fin.last n)) := by
  constructor
  · intro i
    exact nbonacci_radial_defect_adjacent p s D hsource i
  · exact nbonacci_radial_defect_terminal p s D hsource

end RavelGenerated
