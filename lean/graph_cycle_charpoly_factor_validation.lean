import Mathlib.Tactic
import Mathlib.LinearAlgebra.Matrix.Charpoly.Basic

namespace RavelGenerated

open Matrix Polynomial

/-- A feeder vertex placed before a recurrent core gives a block upper-triangular
adjacency matrix. The transient zero block contributes exactly one factor `X`. -/
theorem feeder_cycle_charpoly_factor
    {R : Type*} [CommRing R]
    {ι : Type*} [Fintype ι] [DecidableEq ι]
    (feederToCore : Matrix (Fin 1) ι R)
    (core : Matrix ι ι R) :
    (Matrix.fromBlocks
        (0 : Matrix (Fin 1) (Fin 1) R)
        feederToCore
        (0 : Matrix ι (Fin 1) R)
        core).charpoly
      = Polynomial.X * core.charpoly := by
  rw [Matrix.charpoly_fromBlocks_zero₂₁]
  simp

/-- Once the canonical cycle core has characteristic polynomial `X^n - 1`,
the whole feeder-plus-cycle graph has characteristic polynomial
`X * (X^n - 1)`. -/
theorem feeder_cycle_charpoly_closed
    {R : Type*} [CommRing R]
    {ι : Type*} [Fintype ι] [DecidableEq ι]
    (n : ℕ)
    (feederToCore : Matrix (Fin 1) ι R)
    (core : Matrix ι ι R)
    (hcore : core.charpoly = Polynomial.X ^ n - 1) :
    (Matrix.fromBlocks
        (0 : Matrix (Fin 1) (Fin 1) R)
        feederToCore
        (0 : Matrix ι (Fin 1) R)
        core).charpoly
      = Polynomial.X * (Polynomial.X ^ n - 1) := by
  rw [feeder_cycle_charpoly_factor, hcore]

end RavelGenerated
