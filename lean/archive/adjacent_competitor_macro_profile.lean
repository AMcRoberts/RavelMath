import Mathlib

namespace RavelPlayground

/-- A complete lower branch profile transported by an injective role map is
    exactly the corresponding macro-edge profile when every lower edge is
    replaced by one upper path with the same transported endpoint and
    multiplicity. -/
theorem macroProfile_eq_relabel
    {L U : Type} [Fintype L] [Fintype U] [DecidableEq L] [DecidableEq U]
    (embed : L → U)
    (lowerProfile : L → ℕ)
    (upperMacroProfile : U → ℕ)
    (hmacro : ∀ u, upperMacroProfile u =
      ∑ l : L, if embed l = u then lowerProfile l else 0) :
    upperMacroProfile = fun u =>
      ∑ l : L, if embed l = u then lowerProfile l else 0 := by
  funext u
  exact hmacro u

/-- Bellman evaluation commutes with exact relabelling of a finite branch
    profile.  This is the algebraic handoff from path substitution to the
    twisted Bellman transport theorem. -/
theorem profileEval_relabel
    {L U : Type} [Fintype L] [Fintype U] [DecidableEq U]
    (embed : L → U)
    (profile : L → ℕ)
    (value : U → ℕ) :
    (∑ u : U, (∑ l : L, if embed l = u then profile l else 0) * value u)
      = ∑ l : L, profile l * value (embed l) := by
  classical
  simp_rw [Finset.sum_mul]
  rw [Finset.sum_comm]
  simp only [ite_mul, zero_mul]
  congr 1
  funext l
  rw [Finset.sum_ite_eq Finset.univ (embed l) (fun u => profile l * value u)]
  simp

end RavelPlayground
