/-
  nbonacci_margin_catalogue.lean

  Author: Ravel.
  Date:   2026-08-02.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub.

  Kernel-checked algebraic skeleton of the dimension-parametric n-bonacci
  margin catalogue.  This file deliberately does not claim occurrence in the
  corona-built contact boundary or dominance there.  It formalizes the
  mirror-symmetric stratum grammar, its count polynomial, and the bounded
  previous-alphabet-shadow distance classification used by the C++ miner.
-/

import Mathlib.Tactic

inductive NBonacciSparseKind
  | root
  | alternatingTriple
  deriving DecidableEq

structure NBonacciDescriptor where
  kind : NBonacciSparseKind
  positive : Bool
  a : ℕ
  b : ℕ
  c : ℕ
  deriving DecidableEq

def nbonacciFlipSign (d : NBonacciDescriptor) : NBonacciDescriptor :=
  { d with positive := !d.positive }

/-- Direct symbolic cyclic-core predicate, without beta or corona data. -/
def nbonacciCoreMember
    (i j : ℕ) (d : NBonacciDescriptor) : Prop :=
  if i = j then
    d.kind = .root ∧ i < d.a
  else if i < j then
    (d.kind = .root ∨ i = 0) ∧
      ((d.positive = false ∧ d.a = i ∧ j ≤ d.b) ∨
       (d.positive = true ∧ d.a = j ∧ j < d.b))
  else
    (d.kind = .root ∨ j = 0) ∧
      ((d.positive = true ∧ d.a = j ∧ i ≤ d.b) ∨
       (d.positive = false ∧ d.a = i ∧ i < d.b))

/-- The core predicate exposes the free involution `(i,d,j) ↦
    (j,flipSign d,i)` definitionally, after the index-order split. -/
theorem nbonacciCoreMember_mirror
    (i j : ℕ) (d : NBonacciDescriptor) :
    nbonacciCoreMember i j d ↔
      nbonacciCoreMember j i (nbonacciFlipSign d) := by
  rcases lt_trichotomy i j with hij | hij | hij
  · have hne : i ≠ j := Nat.ne_of_lt hij
    have hnji : ¬ j < i := Nat.not_lt.mpr (Nat.le_of_lt hij)
    simp [nbonacciCoreMember, nbonacciFlipSign, hne, Ne.symm hne, hij, hnji]
  · subst j
    simp [nbonacciCoreMember, nbonacciFlipSign]
  · have hne : i ≠ j := Nat.ne_of_gt hij
    have hnij : ¬ i < j := Nat.not_lt.mpr (Nat.le_of_lt hij)
    simp [nbonacciCoreMember, nbonacciFlipSign, hne, Ne.symm hne, hij, hnij]

/-- Per-face-pair node count recovered from the direct grammar. -/
def nbonacciPairNodeCount (n i j : ℕ) : ℕ :=
  if i = j then
    (n - i - 1) * (n - i - 2)
  else if i = 0 ∨ j = 0 then
    (n - max i j) ^ 2
  else
    2 * (n - max i j) - 1

theorem nbonacciPairNodeCount_mirror (n i j : ℕ) :
    nbonacciPairNodeCount n i j = nbonacciPairNodeCount n j i := by
  simp only [nbonacciPairNodeCount]
  rw [max_comm]
  by_cases h : i = j
  · subst j
    simp
  · have h' : j ≠ i := Ne.symm h
    simp [h, h', or_comm]

/-- Numerator identity behind the decomposition into diagonal triangular
    layers, square zero-row/column layers, and odd interior layers. -/
theorem nbonacciCoreStrata_numerator_identity (n : ℤ) :
    n * (n - 1) * (n - 2) +
        n * (n - 1) * (2 * n - 1) +
        (n - 2) * (n - 1) * (2 * n - 3)
      = (n - 1) * (5 * n ^ 2 - 10 * n + 6) := by
  ring

/-- Symbolic distance from the previous-alphabet state shadow. -/
def nbonacciDistanceFromPreviousShadow
    (n : ℕ) (d : NBonacciDescriptor) : ℕ :=
  if d.a + 1 = n - 1 then 2
  else if d.kind = .alternatingTriple ∧ d.c = d.b + 1 then
    if d.b = d.a + 1 then 3 else 2
  else 1

theorem nbonacciDistanceFromPreviousShadow_le_three
    (n : ℕ) (d : NBonacciDescriptor) :
    nbonacciDistanceFromPreviousShadow n d ≤ 3 := by
  by_cases hlast : d.a + 1 = n - 1
  · simp [nbonacciDistanceFromPreviousShadow, hlast]
  · by_cases htriple :
        d.kind = .alternatingTriple ∧ d.c = d.b + 1
    · by_cases hadjacent : d.b = d.a + 1
      · simp [nbonacciDistanceFromPreviousShadow, hlast, htriple, hadjacent]
      · simp [nbonacciDistanceFromPreviousShadow, hlast, htriple, hadjacent]
    · simp [nbonacciDistanceFromPreviousShadow, hlast, htriple]

/-- Symbolic distance back to the previous-alphabet state shadow. -/
def nbonacciDistanceToPreviousShadow
    (i j : ℕ) (d : NBonacciDescriptor) : ℕ :=
  if d.kind = .alternatingTriple then 1
  else if (i = 0) != (j = 0) then 2
  else 1

theorem nbonacciDistanceToPreviousShadow_le_two
    (i j : ℕ) (d : NBonacciDescriptor) :
    nbonacciDistanceToPreviousShadow i j d ≤ 2 := by
  by_cases htriple : d.kind = .alternatingTriple
  · simp [nbonacciDistanceToPreviousShadow, htriple]
  · by_cases hzero : (i = 0) != (j = 0)
    · simp [nbonacciDistanceToPreviousShadow, htriple, hzero]
    · simp [nbonacciDistanceToPreviousShadow, htriple, hzero]
