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

/-! ### The exact `n+1` carry block

The matrix identity used by the carry automaton is first formalized here in
its scalar ring form.  The companion inverse `A` satisfies
`A + A^2 + ... + A^n = 1`; multiplying by `A` and comparing the shifted sum
gives `A^(n+1) = 2*A - 1`.  The explicit matrix instantiation is a separate
seam, so this theorem does not hide any matrix calculation in a tactic.
-/

/-! ### The scalar projection of the carry machine

The implementation stores a state as a nonempty list of consecutive
coefficients.  Its last coordinate is the inverse-incidence `tail` plus the
signed digit.  The following elementary identity is the one-dimensional
interface used by the exterior-escape automation.
-/

def carryUpdate (x : List ℤ) (d : ℤ) : List ℤ :=
  match x with
  | [] => []
  | x₀ :: xs => xs ++ [x₀ - xs.sum + d]

theorem carryUpdate_sum {x : List ℤ} (hx : x ≠ []) (d : ℤ) :
    (carryUpdate x d).sum = x.head hx + d := by
  cases x with
  | nil => exact (hx rfl).elim
  | cons x₀ xs =>
      simp [carryUpdate, List.sum_append]
      omega

theorem carryUpdate_sum_digit_bound {x : List ℤ} (hx : x ≠ [])
    {d : ℤ} (hd : -1 ≤ d ∧ d ≤ 1) :
    (carryUpdate x d).sum - x.head hx ∈ Set.Icc (-1 : ℤ) 1 := by
  rw [carryUpdate_sum (x := x) hx d]
  constructor <;> omega

theorem carryUpdate_take_prefix (x : List ℤ) (d : ℤ) :
    (carryUpdate x d).take (x.length - 1) = x.tail := by
  cases x with
  | nil => rfl
  | cons x xs => simp [carryUpdate]

/-! The scalar elimination used by the block-map shell argument.  The first
window sum is the previous head plus its digit; substituting that sum into the
next corrected-tail formula produces the exact `(n+1)`-block defect. -/
theorem blockDefect_of_sum_update
    {a0 a1 next sum1 d0 d1 : ℤ}
    (hnext : next = 2 * a1 - sum1 + d1)
    (hsum : sum1 = a0 + d0) :
    next - 2 * a1 + a0 = d1 - d0 := by
  omega

def nbonacciGeomSum {R : Type} [Ring R] (a : R) : ℕ → R
  | 0 => 0
  | k + 1 => nbonacciGeomSum a k + a ^ (k + 1)

theorem nbonacciGeomSum_succ {R : Type} [Ring R] (a : R) (n : ℕ) :
    nbonacciGeomSum a (n + 1) = nbonacciGeomSum a n + a ^ (n + 1) := by
  rfl

theorem nbonacci_mul_geomSum {R : Type} [Ring R] (a : R) (n : ℕ) :
    a * nbonacciGeomSum a n = nbonacciGeomSum a (n + 1) - a := by
  induction n with
  | zero => simp [nbonacciGeomSum]
  | succ n ih =>
      calc
        a * nbonacciGeomSum a (n + 1) =
            a * (nbonacciGeomSum a n + a ^ (n + 1)) := by
              rw [nbonacciGeomSum_succ]
        _ = (nbonacciGeomSum a (n + 1) - a) + a * a ^ (n + 1) := by
              rw [mul_add, ih]
        _ = nbonacciGeomSum a (n + 1 + 1) - a := by
              rw [nbonacciGeomSum_succ a (n + 1)]
              rw [← pow_succ' a (n + 1)]
              noncomm_ring
        _ = nbonacciGeomSum a (n + 2) - a := by
              congr 2

theorem nbonacci_block_identity_scalar {R : Type} [Ring R]
    (a : R) (n : ℕ) (h : nbonacciGeomSum a n = 1) :
    a ^ (n + 1) = 2 * a - 1 := by
  have hshift : nbonacciGeomSum a (n + 1) = 2 * a := by
    calc
      nbonacciGeomSum a (n + 1) =
          a * nbonacciGeomSum a n + a := by
            rw [nbonacci_mul_geomSum]
            noncomm_ring
      _ = a * 1 + a := by rw [h]
      _ = 2 * a := by noncomm_ring
  calc
    a ^ (n + 1) = nbonacciGeomSum a (n + 1) - nbonacciGeomSum a n := by
      rw [nbonacciGeomSum_succ]
      noncomm_ring
    _ = 2 * a - 1 := by rw [hshift, h]

theorem nbonacci_block_identity_matrix {ι R : Type} [Fintype ι]
    [DecidableEq ι] [Ring R] (A : Matrix ι ι R) (n : ℕ)
    (h : nbonacciGeomSum A n = 1) :
    A ^ (n + 1) = 2 * A - 1 := by
  exact nbonacci_block_identity_scalar A n h

noncomputable def nbonacciCharpoly (n : ℕ) : Polynomial ℤ :=
  (Finset.sum (Finset.range n) (fun k => Polynomial.X ^ (k + 1))) -
    Polynomial.C 1

theorem nbonacci_geomSum_of_charpoly {ι : Type} [Fintype ι]
    [DecidableEq ι] (A : Matrix ι ι ℤ) (n : ℕ)
    (hchar : A.charpoly = nbonacciCharpoly n) :
    nbonacciGeomSum A n = 1 := by
  have hc := Matrix.aeval_self_charpoly A
  rw [hchar] at hc
  rw [Polynomial.aeval_def] at hc
  simp [nbonacciCharpoly] at hc
  change (Polynomial.eval₂AddMonoidHom
      (algebraMap ℤ (Matrix ι ι ℤ)) A)
      (Finset.sum (Finset.range n)
        (fun k => Polynomial.X ^ (k + 1))) - 1 = 0 at hc
  rw [map_sum] at hc
  simp [Polynomial.eval₂AddMonoidHom_apply] at hc
  clear hchar
  have hsum_all : ∀ q : ℕ,
      nbonacciGeomSum A q =
        Finset.sum (Finset.range q) (fun k => A ^ (k + 1)) := by
    intro q
    induction q with
    | zero => simp [nbonacciGeomSum]
    | succ q ih =>
        rw [nbonacciGeomSum, Finset.sum_range_succ, ih]
  have hsum := hsum_all n
  rw [← hsum] at hc
  exact sub_eq_zero.mp hc

theorem nbonacci_block_identity_of_charpoly {ι : Type} [Fintype ι]
    [DecidableEq ι] (A : Matrix ι ι ℤ) (n : ℕ)
    (hchar : A.charpoly = nbonacciCharpoly n) :
    A ^ (n + 1) = 2 * A - 1 := by
  exact nbonacci_block_identity_matrix A n
    (nbonacci_geomSum_of_charpoly A n hchar)

/-! A concrete, executable bridge for the carry map.  The finite instances
below are intentionally small: they are kernel-checked witnesses that the
matrix hypothesis is the actual inverse-incidence matrix, rather than an
unrelated abstract ring element.  The dimension-parametric proof still has
one remaining index-arithmetic seam. -/

def inverseCarryMatrix (n : ℕ) : Matrix (Fin n) (Fin n) ℤ := fun i j =>
  if i.val + 1 = j.val then 1
  else if i.val = n - 1 then (if j.val = 0 then 1 else -1)
  else 0

example : nbonacciGeomSum (inverseCarryMatrix 2) 2 = 1 := by native_decide
example : nbonacciGeomSum (inverseCarryMatrix 3) 3 = 1 := by native_decide
example : nbonacciGeomSum (inverseCarryMatrix 4) 4 = 1 := by native_decide
example : nbonacciGeomSum (inverseCarryMatrix 5) 5 = 1 := by native_decide
example : nbonacciGeomSum (inverseCarryMatrix 6) 6 = 1 := by native_decide
example : nbonacciGeomSum (inverseCarryMatrix 7) 7 = 1 := by native_decide
example : nbonacciGeomSum (inverseCarryMatrix 8) 8 = 1 := by native_decide


/-! ### A coordinate bound for block forcing

The forcing formula has only two-term differences in interior coordinates and
one three-term endpoint. This gives a uniform coordinate bound before any
spectral or chamber argument is invoked. -/

def nbonacciBlockDigit (n : ℕ) (digits : Fin (n + 1) → ℤ) (i : Fin n) : ℤ :=
  digits (Fin.castLE (Nat.le_succ n) i)

def nbonacciBlockForcing (n : ℕ) (hn : 0 < n)
    (digits : Fin (n + 1) → ℤ) (i : Fin n) : ℤ :=
  if hzero : i.val = 0 then
    -digits ⟨0, by omega⟩ + digits ⟨1, by omega⟩
  else if hinterior : i.val + 1 < n then
    nbonacciBlockDigit n digits i -
      nbonacciBlockDigit n digits ⟨i.val + 1, hinterior⟩
  else
    2 * digits ⟨0, by omega⟩ - nbonacciBlockDigit n digits i +
      digits ⟨n, Nat.lt_succ_self n⟩

theorem nbonacciBlockForcing_coord_bound (n : ℕ) (hn : 0 < n)
    (digits : Fin (n + 1) → ℤ)
    (hdigits : ∀ k, -1 ≤ digits k ∧ digits k ≤ 1) (i : Fin n) :
    -4 ≤ nbonacciBlockForcing n hn digits i ∧
      nbonacciBlockForcing n hn digits i ≤ 4 := by
  by_cases hzero : i.val = 0
  · simp only [nbonacciBlockForcing, dif_pos hzero]
    have hzero := hdigits (⟨0, by omega⟩ : Fin (n + 1))
    have hone := hdigits (⟨1, by omega⟩ : Fin (n + 1))
    omega
  · by_cases hinterior : i.val + 1 < n
    · simp only [nbonacciBlockForcing, dif_neg hzero, dif_pos hinterior,
        nbonacciBlockDigit]
      have hleft := hdigits (Fin.castLE (Nat.le_succ n) i)
      have hright := hdigits (Fin.castLE (Nat.le_succ n)
        ⟨i.val + 1, by omega⟩)
      omega
    · simp only [nbonacciBlockForcing, dif_neg hzero, dif_neg hinterior,
        nbonacciBlockDigit]
      have hzero := hdigits (⟨0, by omega⟩ : Fin (n + 1))
      have hlast := hdigits (Fin.castLE (Nat.le_succ n) i)
      have hend := hdigits (⟨n, Nat.lt_succ_self n⟩ : Fin (n + 1))
      omega


theorem nbonacciBlockForcing_l1_bound (n : ℕ) (hn : 0 < n)
    (digits : Fin (n + 1) → ℤ)
    (hdigits : ∀ k, -1 ≤ digits k ∧ digits k ≤ 1) :
    (∑ i : Fin n, |nbonacciBlockForcing n hn digits i|) ≤ 4 * n := by
  calc
    (∑ i : Fin n, |nbonacciBlockForcing n hn digits i|) ≤
        ∑ _i : Fin n, (4 : ℤ) := by
      apply Finset.sum_le_sum
      intro i hi
      exact (abs_le).2 (nbonacciBlockForcing_coord_bound n hn digits
        hdigits i)
    _ = 4 * n := by
      simp
      ring


/-! A periodic contradiction needs no spectral analysis once an integer rank
increases strictly on every exterior step. -/

theorem no_strict_rank_finite {α : Type} [Fintype α] [Nonempty α]
    (step : α → α) (rank : α → ℤ)
    (hstep : ∀ x, rank x < rank (step x)) : False := by
  let m : ℤ := Finset.univ.image rank |>.max' (by simp)
  have hmemb : m ∈ Finset.univ.image rank :=
    Finset.max'_mem (Finset.univ.image rank) (by simp)
  obtain ⟨x, hxuniv, hxrank⟩ := Finset.mem_image.mp hmemb
  have hmax : rank x ≤ m := by
    exact Finset.le_max' (Finset.univ.image rank) (rank x)
      (Finset.mem_image.mpr ⟨x, Finset.mem_univ x, rfl⟩)
  have hnext : rank (step x) ≤ m := by
    exact Finset.le_max' (Finset.univ.image rank) (rank (step x))
      (Finset.mem_image.mpr ⟨step x, Finset.mem_univ _, rfl⟩)
  have hinc := hstep x
  omega

/-! A shell-return graph is handled by the same maximum argument, without
    choosing a deterministic successor.  This is the kernel interface for
    replayed first-return DAG certificates: a nonempty finite closed relation
    cannot carry a strictly increasing integer rank on every edge. -/
theorem no_strict_rank_relation_closed {α : Type} [Fintype α] [Nonempty α]
    (R : α → α → Prop) (rank : α → ℤ)
    (hout : ∀ x, ∃ y, R x y)
    (hstrict : ∀ ⦃x y⦄, R x y → rank x < rank y) : False := by
  let m : ℤ := Finset.univ.image rank |>.max' (by simp)
  have hmemb : m ∈ Finset.univ.image rank :=
    Finset.max'_mem (Finset.univ.image rank) (by simp)
  obtain ⟨x, hxuniv, hxrank⟩ := Finset.mem_image.mp hmemb
  obtain ⟨y, hxy⟩ := hout x
  have hymax : rank y ≤ m := by
    exact Finset.le_max' (Finset.univ.image rank) (rank y)
      (Finset.mem_image.mpr ⟨y, Finset.mem_univ y, rfl⟩)
  have hxy' : rank x < rank y := hstrict hxy
  have hxmax : rank x = m := hxrank
  omega

/-! A rank certificate only needs to cover the closed exterior subset. -/

theorem no_strict_rank_closed_subset {α : Type} [Fintype α]
    (T : Set α) (step : α → α) (rank : α → ℤ)
    (hclosed : ∀ x, x ∈ T → step x ∈ T)
    (hstep : ∀ x, x ∈ T → rank x < rank (step x)) :
  T = ∅ := by
  by_contra hnonempty
  have hT : T.Nonempty := Set.nonempty_iff_ne_empty.mpr hnonempty
  letI : Nonempty T := hT.to_subtype
  letI : Fintype T := (Set.toFinite T).fintype
  let stepT : T → T := fun x => ⟨step x, hclosed x.1 x.2⟩
  let rankT : T → ℤ := fun x => rank x.1
  have hstrict : ∀ x : T, rankT x < rankT (stepT x) := by
    intro x
    exact hstep x.1 x.2
  have hfalse : False := no_strict_rank_finite stepT rankT hstrict
  exact hfalse
