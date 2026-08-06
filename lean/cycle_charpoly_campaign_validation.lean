import Mathlib.Tactic
import Mathlib.RingTheory.AdjoinRoot
import Mathlib.LinearAlgebra.Matrix.Charpoly.Minpoly

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

end

end RavelGenerated
