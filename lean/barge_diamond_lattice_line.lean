-- Author: Ravel
-- Date: 2026-08-06

import Mathlib

namespace RavelGenerated

/-- THE CORE ALGEBRAIC FACT closing Barge & Diamond's contradiction
(Bull. Soc. Math. France 130, 2002, proof of Theorem 1, final
paragraph): if an integer matrix `M` has an eigenvector `w` with
INTEGER entries (not all zero) for eigenvalue `β`, then `β` is
rational. Contrapositive: if `β` is irrational (in particular, the
irreducible-degree->=2 Pisot root every substitution in this project's
Barge-Diamond certificate starts from), the eigenline for `β` contains
no nonzero integer lattice point -- exactly the fact their proof uses
to force a contradiction out of an assumed non-coincident configuration
(see `ravel/proof/barge_diamond_certificate.hpp`'s header comment for
the full logical chain this closes).

Proved via `Irrational.ne_rational` (Mathlib): an irrational real is
never equal to `a / b` for integers `a, b`. Since `(M.mulVec w) i` and
`w i` are both integers (`ℤ`-valued dot products of integer vectors),
the eigenvector equation `β * w i = (M.mulVec w) i` gives exactly
`β = (M.mulVec w) i / w i`, an integer ratio -- contradicting
irrationality directly. -/
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

end RavelGenerated
