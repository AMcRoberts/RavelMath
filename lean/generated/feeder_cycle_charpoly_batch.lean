import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

noncomputable def cyclePolynomial (K : Type*) [Field K] (n : ℕ) : Polynomial K :=
  Polynomial.X ^ n - 1

theorem cyclePolynomial_monic_succ (K : Type*) [Field K] (n : ℕ) :
    (cyclePolynomial K (n + 1)).Monic := by
  simpa [cyclePolynomial] using (Polynomial.monic_X_pow_sub_C (1 : K) (Nat.succ_ne_zero n))

theorem cyclePolynomial_ne_zero (K : Type*) [Field K] (n : ℕ) :
    cyclePolynomial K (n + 1) ≠ 0 :=
  (cyclePolynomial_monic_succ K n).ne_zero

noncomputable def canonicalCycleCore (K : Type*) [Field K] (n : ℕ) :=
  let hf : cyclePolynomial K (n + 1) ≠ 0 := cyclePolynomial_ne_zero K n
  let pb := AdjoinRoot.powerBasis hf
  (Algebra.leftMulMatrix pb.basis) pb.gen

theorem canonicalCycleCore_charpoly (K : Type*) [Field K] (n : ℕ) :
    (canonicalCycleCore K n).charpoly = cyclePolynomial K (n + 1) := by
  let f := cyclePolynomial K (n + 1)
  have hfMonic : f.Monic := cyclePolynomial_monic_succ K n
  have hf : f ≠ 0 := hfMonic.ne_zero
  let pb := AdjoinRoot.powerBasis hf
  change ((Algebra.leftMulMatrix pb.basis) pb.gen).charpoly = f
  rw [charpoly_leftMulMatrix pb]
  exact AdjoinRoot.minpoly_powerBasis_gen_of_monic hfMonic

theorem cyclePolynomial_natDegree_succ (K : Type*) [Field K] (n : ℕ) :
    (cyclePolynomial K (n + 1)).natDegree = n + 1 := by
  simpa [cyclePolynomial] using Polynomial.natDegree_X_pow_sub_C (R := K) (r := (1 : K))

theorem canonicalCycleCore_dim (K : Type*) [Field K] (n : ℕ) :
    (AdjoinRoot.powerBasis (cyclePolynomial_ne_zero K n)).dim = n + 1 :=
  cyclePolynomial_natDegree_succ K n

noncomputable def concreteCycleMatrix (K : Type*) [Field K] (n : ℕ) : Matrix (Fin (n + 1)) (Fin (n + 1)) K :=
  Matrix.of (fun i j =>
    if (i : ℕ) = (j : ℕ) + 1 then 1
    else if (j : ℕ) + 1 = n + 1 ∧ (i : ℕ) = 0 then 1 else 0)

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

theorem feeder_cycle_charpoly_factor
    {R : Type*} [CommRing R] {ι : Type*} [Fintype ι] [DecidableEq ι]
    (feederToCore : Matrix (Fin 1) ι R) (core : Matrix ι ι R) :
    (Matrix.fromBlocks (0 : Matrix (Fin 1) (Fin 1) R) feederToCore
        (0 : Matrix ι (Fin 1) R) core).charpoly = Polynomial.X * core.charpoly := by
  rw [Matrix.charpoly_fromBlocks_zero₂₁]
  simp

theorem feeder_cycle_charpoly_closed
    {R : Type*} [CommRing R] {ι : Type*} [Fintype ι] [DecidableEq ι]
    (n : ℕ) (feederToCore : Matrix (Fin 1) ι R) (core : Matrix ι ι R)
    (hcore : core.charpoly = Polynomial.X ^ n - 1) :
    (Matrix.fromBlocks (0 : Matrix (Fin 1) (Fin 1) R) feederToCore
        (0 : Matrix ι (Fin 1) R) core).charpoly = Polynomial.X * (Polynomial.X ^ n - 1) := by
  rw [feeder_cycle_charpoly_factor, hcore]

def cycleWithFeederOutdegrees (n : ℕ) : List ℕ :=
  List.replicate n 1 ++ [1]

theorem cycleWithFeeder_edge_count (n : ℕ) :
    (cycleWithFeederOutdegrees n).sum = n + 1 := by
  simp [cycleWithFeederOutdegrees]

theorem edge_count_of_cycle_core_and_feeder
    (n coreEdges feederEdges totalEdges : ℕ)
    (hcore : coreEdges = n) (hfeeder : feederEdges = 1)
    (htotal : totalEdges = coreEdges + feederEdges) :
    totalEdges = n + 1 := by
  omega

/-- Mechanically emitted: 7-cycle with feeder edge into vertex 0. -/
theorem feeder_cycle_charpoly_instance_0 (K : Type*) [Field K] :
    (Matrix.fromBlocks (0 : Matrix (Fin 1) (Fin 1) K)
        (Matrix.of (fun (_ : Fin 1) (j : Fin (6 + 1)) =>
          if (j : ℕ) = 0 then (1 : K) else 0))
        (0 : Matrix (Fin (6 + 1)) (Fin 1) K)
        (concreteCycleMatrix K 6)).charpoly =
      Polynomial.X * (Polynomial.X ^ (6 + 1) - 1) := by
  have := feeder_cycle_charpoly_closed (6 + 1)
    (Matrix.of (fun (_ : Fin 1) (j : Fin (6 + 1)) =>
      if (j : ℕ) = 0 then (1 : K) else 0))
    (concreteCycleMatrix K 6) (concreteCycleMatrix_charpoly K 6)
  simpa [cyclePolynomial] using this

/-- Mechanically emitted: edge count for 7-cycle with feeder edge into vertex 0 -- instantiates both cycleWithFeeder_edge_count directly and, via its
    coreEdges/feederEdges decomposition, edge_count_of_cycle_core_and_feeder. -/
theorem feeder_cycle_edge_count_instance_0 :
    (cycleWithFeederOutdegrees (6 + 1)).sum = 6 + 2 := by
  have h := cycleWithFeeder_edge_count (6 + 1)
  have hcomposed := edge_count_of_cycle_core_and_feeder (6 + 1) (6 + 1) 1 ((6 + 1) + 1) rfl rfl rfl
  omega

/-- Mechanically emitted: 4-cycle with feeder edge into vertex 2. -/
theorem feeder_cycle_charpoly_instance_1 (K : Type*) [Field K] :
    (Matrix.fromBlocks (0 : Matrix (Fin 1) (Fin 1) K)
        (Matrix.of (fun (_ : Fin 1) (j : Fin (3 + 1)) =>
          if (j : ℕ) = 2 then (1 : K) else 0))
        (0 : Matrix (Fin (3 + 1)) (Fin 1) K)
        (concreteCycleMatrix K 3)).charpoly =
      Polynomial.X * (Polynomial.X ^ (3 + 1) - 1) := by
  have := feeder_cycle_charpoly_closed (3 + 1)
    (Matrix.of (fun (_ : Fin 1) (j : Fin (3 + 1)) =>
      if (j : ℕ) = 2 then (1 : K) else 0))
    (concreteCycleMatrix K 3) (concreteCycleMatrix_charpoly K 3)
  simpa [cyclePolynomial] using this

/-- Mechanically emitted: edge count for 4-cycle with feeder edge into vertex 2 -- instantiates both cycleWithFeeder_edge_count directly and, via its
    coreEdges/feederEdges decomposition, edge_count_of_cycle_core_and_feeder. -/
theorem feeder_cycle_edge_count_instance_1 :
    (cycleWithFeederOutdegrees (3 + 1)).sum = 3 + 2 := by
  have h := cycleWithFeeder_edge_count (3 + 1)
  have hcomposed := edge_count_of_cycle_core_and_feeder (3 + 1) (3 + 1) 1 ((3 + 1) + 1) rfl rfl rfl
  omega

/- Semantic proof graph for: feeder_cycle_charpoly_batch
  [0] lean.feeder_cycle_charpoly_certificate :: n=6 feeder_target=0 7-cycle with feeder edge into vertex 0 -- instantiates feeder_cycle_charpoly_closed + concreteCycleMatrix_charpoly
  [1] lean.feeder_cycle_charpoly_certificate :: n=3 feeder_target=2 4-cycle with feeder edge into vertex 2 -- instantiates feeder_cycle_charpoly_closed + concreteCycleMatrix_charpoly
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
