import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- THE CORE ALGEBRAIC FACT closing Barge & Diamond's contradiction
    (Bull. Soc. Math. France 130, 2002, proof of Theorem 1): if an integer
    matrix `M` has a nonzero-at-`i` integer-valued eigenvector `w` for a real
    eigenvalue `β`, then `β` is rational. Reproduced from the independently
    kernel-checked `lean/barge_diamond_lattice_line.lean` (not re-derived
    here). -/
theorem irrational_eigenvalue_has_no_integer_eigenvector
    {n : ℕ} (M : Matrix (Fin n) (Fin n) ℤ) (w : Fin n → ℤ) (β : ℝ)
    (hβ : Irrational β) (i : Fin n) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = ((M.mulVec w) i : ℝ)) :
    False := by
  have hwi' : (w i : ℝ) ≠ 0 := Int.cast_ne_zero.mpr hwi
  have hrat : β = ((M.mulVec w) i : ℝ) / (w i : ℝ) := by
    field_simp
    linarith [heig]
  exact hβ.ne_rational ((M.mulVec w) i) (w i) hrat

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^3 + -x^2 + -2*x + -1). -/
theorem barge_diamond_instance_0 (β : ℝ) (hβ : Irrational β)
    (w : Fin 3 → ℤ) (i : Fin 3) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 1, 1; 1, 0, 0; 1, 1, 0]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 1, 1; 1, 0, 0; 1, 1, 0] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^3 + -x^2 + 0*x + -1). -/
theorem barge_diamond_instance_1 (β : ℝ) (hβ : Irrational β)
    (w : Fin 3 → ℤ) (i : Fin 3) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 0, 1; 1, 0, 0; 0, 1, 0]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 0, 1; 1, 0, 0; 0, 1, 0] w β hβ i hwi heig

/- Semantic proof graph for: barge_diamond_wide_batch
  [0] lean.integer_eigenvector_no_witness :: n=3 M=charpoly=x^3 + -x^2 + -2*x + -1 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [1] lean.integer_eigenvector_no_witness :: n=3 M=charpoly=x^3 + -x^2 + 0*x + -1 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
