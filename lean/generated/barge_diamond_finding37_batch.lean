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
    this substitution's own incidence matrix (charpoly=x^4 + -3*x^3 + -3*x^2 + x + 2). -/
theorem barge_diamond_instance_0 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 2, 2, 2; 1, 1, 0, 0; 1, 0, 0, 0; 1, 1, 2, 1]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 2, 2, 2; 1, 1, 0, 0; 1, 0, 0, 0; 1, 1, 2, 1] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^4 + -4*x^3 + -x^2 + 0*x + -3). -/
theorem barge_diamond_instance_1 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![2, 2, 1, 2; 0, 0, 1, 1; 1, 1, 1, 0; 1, 2, 2, 1]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![2, 2, 1, 2; 0, 0, 1, 1; 1, 1, 1, 0; 1, 2, 2, 1] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^4 + -4*x^3 + 2*x^2 + -3*x + 2). -/
theorem barge_diamond_instance_2 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![2, 1, 2, 2; 1, 1, 0, 0; 0, 1, 1, 2; 1, 0, 0, 0]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![2, 1, 2, 2; 1, 1, 0, 0; 0, 1, 1, 2; 1, 0, 0, 0] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^4 + -4*x^3 + x^2 + -2*x + -2). -/
theorem barge_diamond_instance_3 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 1, 2, 1; 1, 1, 0, 1; 1, 2, 2, 1; 1, 0, 0, 0]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 1, 2, 1; 1, 1, 0, 1; 1, 2, 2, 1; 1, 0, 0, 0] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^4 + -2*x^3 + -4*x^2 + 0*x + 2). -/
theorem barge_diamond_instance_4 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 1, 2, 1; 1, 0, 0, 1; 1, 1, 0, 1; 0, 1, 1, 1]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 1, 2, 1; 1, 0, 0, 1; 1, 1, 0, 1; 0, 1, 1, 1] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^4 + -4*x^3 + x^2 + x + -3). -/
theorem barge_diamond_instance_5 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![2, 1, 1, 3; 1, 1, 0, 0; 0, 1, 1, 0; 1, 1, 1, 0]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![2, 1, 1, 3; 1, 1, 0, 0; 0, 1, 1, 0; 1, 1, 1, 0] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^4 + -3*x^3 + -4*x^2 + 2*x + 3). -/
theorem barge_diamond_instance_6 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![2, 1, 2, 1; 1, 0, 1, 1; 1, 2, 0, 0; 0, 1, 1, 1]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![2, 1, 2, 1; 1, 0, 1, 1; 1, 2, 0, 0; 0, 1, 1, 1] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^4 + -3*x^3 + -3*x^2 + 0*x + 2). -/
theorem barge_diamond_instance_7 (β : ℝ) (hβ : Irrational β)
    (w : Fin 4 → ℤ) (i : Fin 4) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 2, 2, 1; 1, 1, 2, 1; 1, 0, 0, 0; 1, 1, 0, 1]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 2, 2, 1; 1, 1, 2, 1; 1, 0, 0, 0; 1, 1, 0, 1] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^5 + -3*x^4 + 0*x^3 + -4*x^2 + -x + -2). -/
theorem barge_diamond_instance_8 (β : ℝ) (hβ : Irrational β)
    (w : Fin 5 → ℤ) (i : Fin 5) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 1, 2, 1, 1; 2, 0, 0, 1, 0; 0, 1, 0, 0, 0; 1, 0, 0, 1, 1; 0, 1, 2, 0, 1]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 1, 2, 1, 1; 2, 0, 0, 1, 0; 0, 1, 0, 0, 0; 1, 0, 0, 1, 1; 0, 1, 2, 0, 1] w β hβ i hwi heig

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own incidence matrix (charpoly=x^5 + -3*x^4 + -3*x^3 + 0*x^2 + 0*x + -2). -/
theorem barge_diamond_instance_9 (β : ℝ) (hβ : Irrational β)
    (w : Fin 5 → ℤ) (i : Fin 5) (hwi : w i ≠ 0)
    (heig : β * (w i : ℝ) = (((!![1, 2, 1, 1, 2; 0, 1, 0, 1, 1; 1, 0, 0, 1, 0; 1, 0, 1, 1, 0; 1, 1, 2, 0, 0]).mulVec w) i : ℝ)) : False :=
  irrational_eigenvalue_has_no_integer_eigenvector !![1, 2, 1, 1, 2; 0, 1, 0, 1, 1; 1, 0, 0, 1, 0; 1, 0, 1, 1, 0; 1, 1, 2, 0, 0] w β hβ i hwi heig

/- Semantic proof graph for: barge_diamond_finding37_batch
  [0] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -3*x^3 + -3*x^2 + x + 2 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [1] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -4*x^3 + -x^2 + 0*x + -3 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [2] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -4*x^3 + 2*x^2 + -3*x + 2 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [3] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -4*x^3 + x^2 + -2*x + -2 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [4] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -2*x^3 + -4*x^2 + 0*x + 2 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [5] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -4*x^3 + x^2 + x + -3 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [6] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -3*x^3 + -4*x^2 + 2*x + 3 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [7] lean.integer_eigenvector_no_witness :: n=4 M=charpoly=x^4 + -3*x^3 + -3*x^2 + 0*x + 2 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [8] lean.integer_eigenvector_no_witness :: n=5 M=charpoly=x^5 + -3*x^4 + 0*x^3 + -4*x^2 + -x + -2 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
  [9] lean.integer_eigenvector_no_witness :: n=5 M=charpoly=x^5 + -3*x^4 + -3*x^3 + 0*x^2 + 0*x + -2 -- instantiates irrational_eigenvalue_has_no_integer_eigenvector
-/

def reflectedNodeCount : Nat := 10

end RavelGenerated
