import Mathlib.Tactic
import Mathlib.RingTheory.AdjoinRoot
import Mathlib.LinearAlgebra.Matrix.Charpoly.Basic
import Mathlib.LinearAlgebra.Matrix.Charpoly.Minpoly
import Mathlib.LinearAlgebra.Matrix.Reindex

namespace RavelGenerated

open Matrix Polynomial

noncomputable section

/-- The polynomial forced by a directed cycle of length `n`. -/
def cyclePolynomial (K : Type*) [Field K] (n : ℕ) : Polynomial K :=
  Polynomial.X ^ n - 1

/-- The cycle polynomial is monic in every positive dimension. -/
theorem cyclePolynomial_monic_succ
    (K : Type*) [Field K] (n : ℕ) :
    (cyclePolynomial K (n + 1)).Monic := by
  simpa [cyclePolynomial] using
    (Polynomial.monic_X_pow_sub_C (1 : K) (Nat.succ_ne_zero n))

theorem cyclePolynomial_ne_zero
    (K : Type*) [Field K] (n : ℕ) :
    cyclePolynomial K (n + 1) ≠ 0 := by
  exact (cyclePolynomial_monic_succ K n).ne_zero

/-- The canonical cycle-core matrix is the matrix of left multiplication by
an adjoined root of `X^(n+1)-1`, in its power basis. -/
noncomputable def canonicalCycleCore
    (K : Type*) [Field K] (n : ℕ) :=
  let hf : cyclePolynomial K (n + 1) ≠ 0 := cyclePolynomial_ne_zero K n
  let pb := AdjoinRoot.powerBasis hf
  (Algebra.leftMulMatrix pb.basis) pb.gen

/-- The canonical cycle core has characteristic polynomial `X^(n+1)-1`. -/
theorem canonicalCycleCore_charpoly
    (K : Type*) [Field K] (n : ℕ) :
    (canonicalCycleCore K n).charpoly = cyclePolynomial K (n + 1) := by
  let f := cyclePolynomial K (n + 1)
  have hfMonic : f.Monic := cyclePolynomial_monic_succ K n
  have hf : f ≠ 0 := hfMonic.ne_zero
  let pb := AdjoinRoot.powerBasis hf
  change ((Algebra.leftMulMatrix pb.basis) pb.gen).charpoly = f
  rw [charpoly_leftMulMatrix pb]
  exact AdjoinRoot.minpoly_powerBasis_gen_of_monic hfMonic

/-- The exact companion-matrix formula exposed by the power-basis machinery.
This is the structural entry theorem consumed by the graph certificate bridge.

The matrix remains indexed by `Fin pb.dim`, its native power-basis dimension.
A separate dimension-transport theorem may later identify `pb.dim` with `n+1`;
that identification is intentionally not smuggled in definitionally here. -/
theorem canonicalCycleCore_companion_entries
    (K : Type*) [Field K] (n : ℕ) :
    let f := cyclePolynomial K (n + 1)
    let hf : f ≠ 0 := cyclePolynomial_ne_zero K n
    let pb := AdjoinRoot.powerBasis hf
    Algebra.leftMulMatrix pb.basis pb.gen =
      @Matrix.of (Fin pb.dim) (Fin pb.dim) K fun i j =>
        if (j : ℕ) + 1 = pb.dim then
          -pb.minpolyGen.coeff (i : ℕ)
        else if (i : ℕ) = (j : ℕ) + 1 then 1 else 0 := by
  dsimp
  exact (AdjoinRoot.powerBasis (cyclePolynomial_ne_zero K n)).leftMulMatrix

/-- The dimension-transport theorem `canonicalCycleCore_companion_entries`'s own
docstring flagged as future work: `pb.dim` for the `AdjoinRoot.powerBasis` of
`X^(n+1)-1` is literally `n+1`, by that polynomial's `natDegree`. -/
theorem cyclePolynomial_natDegree_succ (K : Type*) [Field K] (n : ℕ) :
    (cyclePolynomial K (n + 1)).natDegree = n + 1 := by
  simpa [cyclePolynomial] using
    Polynomial.natDegree_X_pow_sub_C (R := K) (r := (1 : K))

theorem canonicalCycleCore_dim (K : Type*) [Field K] (n : ℕ) :
    (AdjoinRoot.powerBasis (cyclePolynomial_ne_zero K n)).dim = n + 1 :=
  cyclePolynomial_natDegree_succ K n

/-- The concrete cyclic-shift 0/1 matrix on `Fin (n+1)`: edge `i -> i+1 mod (n+1)`,
exactly the "installed graph attractor model"'s directed-cycle adjacency matrix
(`docs/GRAPH_CYCLE_CANONICALIZATION_AND_FACTOR_SEAM.md`), not an abstract stand-in. -/
def concreteCycleMatrix (K : Type*) [Field K] (n : ℕ) : Matrix (Fin (n + 1)) (Fin (n + 1)) K :=
  Matrix.of (fun i j =>
    if (i : ℕ) = (j : ℕ) + 1 then 1
    else if (j : ℕ) + 1 = n + 1 ∧ (i : ℕ) = 0 then 1 else 0)

/-- Completes `canonicalCycleCore_charpoly` down to the CONCRETE cycle-adjacency
matrix (the dimension-transport step the campaign doc named as the remaining
gap): reindexing `canonicalCycleCore` by `canonicalCycleCore_dim` and unfolding
the companion-matrix entries against `X^(n+1)-1`'s own coefficients shows it
equals `concreteCycleMatrix` on the nose, so its characteristic polynomial
transports across for free. -/
theorem concreteCycleMatrix_charpoly (K : Type*) [Field K] (n : ℕ) :
    (concreteCycleMatrix K n).charpoly = cyclePolynomial K (n + 1) := by
  let f := cyclePolynomial K (n + 1)
  let hf : f ≠ 0 := cyclePolynomial_ne_zero K n
  let pb := AdjoinRoot.powerBasis hf
  have hdim : pb.dim = n + 1 := canonicalCycleCore_dim K n
  have hentries := pb.leftMulMatrix
  have hreindex :
      Matrix.reindex (finCongr hdim) (finCongr hdim) (canonicalCycleCore K n) =
        concreteCycleMatrix K n := by
    ext i j
    simp only [canonicalCycleCore, Matrix.reindex_apply, Matrix.submatrix_apply,
      finCongr_symm, finCongr_apply, concreteCycleMatrix, Matrix.of_apply]
    show (Algebra.leftMulMatrix pb.basis) pb.gen
        ((Fin.cast hdim.symm) i) ((Fin.cast hdim.symm) j) = _
    rw [hentries]
    simp only [Matrix.of_apply]
    have hminpolyGen : pb.minpolyGen = f := by
      rw [pb.minpolyGen_eq]
      exact AdjoinRoot.minpoly_powerBasis_gen_of_monic (cyclePolynomial_monic_succ K n)
    have hi0 : (Fin.cast hdim.symm i : ℕ) = (i : ℕ) := by simp
    have hj0 : (Fin.cast hdim.symm j : ℕ) = (j : ℕ) := by simp
    rw [hi0, hj0, hdim]
    rw [hminpolyGen]
    have hcoeff : ∀ k : ℕ, k < n + 1 → f.coeff k = if k = 0 then (-1 : K) else 0 := by
      intro k hk
      by_cases h0 : k = 0
      · simp [f, cyclePolynomial, h0]
      · simp only [f, cyclePolynomial, Polynomial.coeff_sub, Polynomial.coeff_X_pow,
          Polynomial.coeff_one, h0, if_false]
        rw [if_neg (by omega : ¬ (k = n + 1))]
        ring
    have hi_lt : (i : ℕ) < n + 1 := i.isLt
    have hcoeff_i := hcoeff (i : ℕ) hi_lt
    split_ifs with h1 h2 h3 h3 <;>
      first
        | rfl
        | (exfalso; omega)
        | (rw [hcoeff_i]; split_ifs at hcoeff_i ⊢ <;> first | rfl | (exfalso; omega) | ring)
  rw [← hreindex, Matrix.charpoly_reindex]
  exact canonicalCycleCore_charpoly K n

end

end RavelGenerated
