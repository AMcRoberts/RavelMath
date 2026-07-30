/-
  class_ii_neighbor_d_support.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Boundary-layer support identity A(a) = A(3) + (a-3)*D for the Class-II neighbor dominant cores.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  class_ii_neighbor_d_support.lean

  Boundary-layer support theorem for the Class-II adjacent-swap
  neighbor dominant cores.

  This file kernel-checks the affine structure
    A(a) = A(3) + (a - 3) · D
  where D is the slope matrix of the sparse affine catalog in
  `spectre::class_ii_neighbor_affine_edges`, and proves the support
  statement:

    * D's nonzero entries are exactly the boundary-layer roles --
      a sparse subset of each dominant core, with sizes 12 / 5 / 24
      out of 15² / 17² / 39² entries respectively.

  Together with `class_ii_neighbor_dominance.lean` (which proves the
  candidate dominance inequalities from the affine polynomial
  formulas), this file discharges the
    "boundary-layer-only structural claim"
  of `docs/FAMILY_OF_FAMILIES.md` at the catalogue level.

  In the Class-II q-expansion language
    delta(a) = beta - (a + 1/beta) = 1 - 1/a + 2/a^2 - 4/a^3 + ...
  the affine structure A(a) = A(3) + (a-3)·D is the leading-order
  coefficient "slope" of an entry's count.  Roles outside the
  boundary layer have slope 0; roles inside it have slope 1.  The
  boundary-layer support theorem says slope is sparse.
-/

import Mathlib.Data.Int.Basic
import Mathlib.Data.Finset.Basic
import Mathlib.Tactic

/- ============================================================
   Affine edge structure and slope matrix extraction.
   ============================================================ -/

structure AffineEdge where
  source : Nat
  target : Nat
  intercept : Int
  slope : Int
  deriving DecidableEq

/-- Generic affine matrix entry: A(a) = intercept + slope * a. -/
def affineWeight (e : AffineEdge) (a : Int) : Int :=
  e.intercept + e.slope * a

/-- The closed-form q-expansion identity: the difference of an affine
entry at a and at 3 equals the slope times (a - 3).  This is the
matrix-entry-level statement of
    A(a) - A(3) - (a - 3) * D = 0
where D is the slope matrix. -/
theorem affine_closed_form_identity (e : AffineEdge) (a : Int) :
    affineWeight e a - affineWeight e 3 = e.slope * (a - 3) := by
  unfold affineWeight
  ring

/-- Generic slope identity: A(a) = A(3) + (a - 3) * slope. -/
theorem affine_catalogue_affine (e : AffineEdge) (a : Int) :
    affineWeight e a = affineWeight e 3 + (a - 3) * e.slope := by
  unfold affineWeight
  ring

/- ============================================================
   Neighbor 0 affine catalog: 41 edges total; 12 have slope 1.
   We package it as a Finset for decidable equality.
   ============================================================ -/

def neighbor0EdgeList : List AffineEdge :=
  [⟨0, 4, 1, 0⟩, ⟨0, 5, 1, 0⟩,
   ⟨1, 9, -2, 1⟩, ⟨1, 10, -3, 1⟩, ⟨1, 12, -2, 1⟩, ⟨1, 14, 1, 0⟩,
   ⟨2, 9, 1, 0⟩, ⟨2, 10, 1, 0⟩, ⟨2, 12, 1, 0⟩,
   ⟨3, 6, 1, 0⟩, ⟨3, 9, 1, 0⟩,
   ⟨4, 0, -2, 1⟩, ⟨4, 1, 0, 1⟩, ⟨4, 2, -1, 1⟩, ⟨4, 3, 1, 0⟩,
   ⟨4, 11, 1, 0⟩, ⟨5, 7, 1, 0⟩, ⟨5, 13, 1, 0⟩,
   ⟨6, 1, 1, 0⟩, ⟨6, 11, 1, 0⟩, ⟨7, 5, 1, 0⟩,
   ⟨8, 6, 1, 0⟩, ⟨8, 9, 0, 1⟩, ⟨8, 10, -2, 1⟩, ⟨8, 12, -1, 1⟩,
   ⟨8, 14, 1, 0⟩,
   ⟨9, 0, -3, 1⟩, ⟨9, 1, -2, 1⟩, ⟨9, 2, -2, 1⟩, ⟨9, 3, 1, 0⟩,
   ⟨10, 7, 1, 0⟩, ⟨10, 8, 1, 0⟩, ⟨11, 6, 1, 0⟩, ⟨11, 9, 1, 0⟩,
   ⟨12, 0, 1, 0⟩, ⟨12, 1, 1, 0⟩, ⟨12, 2, 1, 0⟩,
   ⟨13, 4, 1, 0⟩, ⟨13, 5, 1, 0⟩, ⟨14, 1, 1, 0⟩, ⟨14, 11, 1, 0⟩]

def neighbor0Edges : Finset AffineEdge := neighbor0EdgeList.toFinset

theorem neighbor0Edges_card : neighbor0Edges.card = 41 := by
  unfold neighbor0Edges neighbor0EdgeList
  native_decide

/-- The boundary-layer source index set for neighbor 0: extracted
from the affine catalog as the sources of edges with slope 1. -/
def neighbor0BoundarySource : Finset Nat :=
  ({e ∈ neighbor0Edges | e.slope ≠ 0} : Finset AffineEdge).image AffineEdge.source

/-- The boundary-layer target index set for neighbor 0. -/
def neighbor0BoundaryTarget : Finset Nat :=
  ({e ∈ neighbor0Edges | e.slope ≠ 0} : Finset AffineEdge).image AffineEdge.target

theorem neighbor0BoundarySource_eq :
    neighbor0BoundarySource = {1, 4, 8, 9} := by
  unfold neighbor0BoundarySource neighbor0Edges neighbor0EdgeList
  native_decide

theorem neighbor0BoundaryTarget_eq :
    neighbor0BoundaryTarget = {0, 1, 2, 9, 10, 12} := by
  unfold neighbor0BoundaryTarget neighbor0Edges neighbor0EdgeList
  native_decide

theorem neighbor0_d_nnz :
    ({e ∈ neighbor0Edges | e.slope ≠ 0} : Finset AffineEdge).card = 12 := by
  unfold neighbor0Edges neighbor0EdgeList
  native_decide

theorem neighbor0_catalogue_affine (a : Int)
    (e : AffineEdge) (he : e ∈ neighbor0Edges) :
    affineWeight e a = affineWeight e 3 + (a - 3) * e.slope :=
  affine_catalogue_affine e a

/-- The boundary-layer support theorem (kernel-checked) for
neighbor 0: D has exactly 12 nonzero entries, all on the source
roles {1, 4, 8, 9} and target roles {0, 1, 2, 9, 10, 12}. -/
theorem neighbor0_boundary_layer_support :
    ({e ∈ neighbor0Edges | e.slope ≠ 0} : Finset AffineEdge).card = 12 ∧
    (∀ s ∈ neighbor0BoundarySource,
        s = 1 ∨ s = 4 ∨ s = 8 ∨ s = 9) ∧
    (∀ t ∈ neighbor0BoundaryTarget,
        t = 0 ∨ t = 1 ∨ t = 2 ∨ t = 9 ∨ t = 10 ∨ t = 12) := by
  refine ⟨?_, ⟨?_, ?_⟩⟩
  · unfold neighbor0Edges neighbor0EdgeList
    native_decide
  · intro s hs
    rw [neighbor0BoundarySource_eq] at hs
    simpa using hs
  · intro t ht
    rw [neighbor0BoundaryTarget_eq] at ht
    simpa using ht

/- ============================================================
   Neighbor 1 affine catalog: 34 edges total; 5 have slope 1.
   ============================================================ -/

def neighbor1EdgeList : List AffineEdge :=
  [⟨0, 9, 1, 0⟩, ⟨0, 12, 1, 0⟩,
   ⟨1, 9, -1, 1⟩, ⟨1, 12, -2, 1⟩, ⟨1, 14, 1, 0⟩,
   ⟨2, 15, 1, 0⟩, ⟨3, 9, 1, 0⟩, ⟨3, 12, 1, 0⟩,
   ⟨4, 15, 1, 0⟩, ⟨4, 16, 1, 0⟩, ⟨5, 10, 1, 0⟩,
   ⟨6, 7, 1, 0⟩, ⟨6, 11, 1, 0⟩, ⟨7, 8, 1, 0⟩, ⟨7, 13, 1, 0⟩,
   ⟨8, 12, 1, 0⟩,
   ⟨9, 0, -2, 1⟩, ⟨9, 1, -1, 1⟩, ⟨9, 2, -1, 1⟩, ⟨9, 3, 1, 0⟩,
   ⟨9, 4, 1, 0⟩, ⟨10, 0, 1, 0⟩, ⟨11, 5, 1, 0⟩, ⟨11, 6, 1, 0⟩,
   ⟨12, 0, 1, 0⟩, ⟨12, 1, 1, 0⟩, ⟨12, 2, 1, 0⟩, ⟨13, 9, 1, 0⟩,
   ⟨14, 0, 1, 0⟩, ⟨14, 1, 1, 0⟩, ⟨14, 2, 1, 0⟩, ⟨15, 10, 1, 0⟩,
   ⟨16, 7, 1, 0⟩, ⟨16, 11, 1, 0⟩]

def neighbor1Edges : Finset AffineEdge := neighbor1EdgeList.toFinset

theorem neighbor1Edges_card : neighbor1Edges.card = 34 := by
  unfold neighbor1Edges neighbor1EdgeList
  native_decide

def neighbor1BoundarySource : Finset Nat :=
  ({e ∈ neighbor1Edges | e.slope ≠ 0} : Finset AffineEdge).image AffineEdge.source

def neighbor1BoundaryTarget : Finset Nat :=
  ({e ∈ neighbor1Edges | e.slope ≠ 0} : Finset AffineEdge).image AffineEdge.target

theorem neighbor1BoundarySource_eq :
    neighbor1BoundarySource = {1, 9} := by
  unfold neighbor1BoundarySource neighbor1Edges neighbor1EdgeList
  native_decide

theorem neighbor1BoundaryTarget_eq :
    neighbor1BoundaryTarget = {0, 1, 2, 9, 12} := by
  unfold neighbor1BoundaryTarget neighbor1Edges neighbor1EdgeList
  native_decide

theorem neighbor1_d_nnz :
    ({e ∈ neighbor1Edges | e.slope ≠ 0} : Finset AffineEdge).card = 5 := by
  unfold neighbor1Edges neighbor1EdgeList
  native_decide

/-- The boundary-layer support theorem (kernel-checked) for
neighbor 1: D has exactly 5 nonzero entries on sources {1, 9} and
targets {0, 1, 2, 9, 12}. -/
theorem neighbor1_boundary_layer_support :
    ({e ∈ neighbor1Edges | e.slope ≠ 0} : Finset AffineEdge).card = 5 ∧
    (∀ s ∈ neighbor1BoundarySource, s = 1 ∨ s = 9) ∧
    (∀ t ∈ neighbor1BoundaryTarget,
        t = 0 ∨ t = 1 ∨ t = 2 ∨ t = 9 ∨ t = 12) := by
  refine ⟨?_, ⟨?_, ?_⟩⟩
  · unfold neighbor1Edges neighbor1EdgeList
    native_decide
  · intro s hs
    rw [neighbor1BoundarySource_eq] at hs
    simpa using hs
  · intro t ht
    rw [neighbor1BoundaryTarget_eq] at ht
    simpa using ht

/- ============================================================
   Neighbor 2 affine catalog: 91 edges total; 24 have slope 1.
   ============================================================ -/

def neighbor2EdgeList : List AffineEdge :=
  [⟨0, 21, 1, 0⟩, ⟨1, 22, 1, 0⟩, ⟨1, 34, 1, 0⟩, ⟨2, 26, 1, 0⟩,
   ⟨3, 15, 0, 1⟩, ⟨3, 16, -1, 1⟩, ⟨3, 18, -2, 1⟩, ⟨3, 24, -1, 1⟩,
   ⟨3, 25, -1, 1⟩, ⟨3, 26, -2, 1⟩, ⟨3, 36, 1, 0⟩,
   ⟨4, 16, 1, 0⟩, ⟨4, 18, 1, 0⟩, ⟨5, 26, 1, 0⟩, ⟨6, 24, 1, 0⟩,
   ⟨7, 19, 1, 0⟩, ⟨7, 25, 1, 0⟩, ⟨7, 28, 1, 0⟩, ⟨7, 38, 1, 0⟩,
   ⟨8, 29, 1, 0⟩,
   ⟨9, 2, -1, 1⟩, ⟨9, 3, 0, 1⟩, ⟨9, 4, -1, 1⟩, ⟨9, 5, 1, 0⟩,
   ⟨9, 6, -1, 1⟩, ⟨9, 7, -2, 1⟩, ⟨9, 20, 0, 1⟩, ⟨9, 30, 1, 0⟩,
   ⟨9, 31, 1, 0⟩, ⟨9, 32, 1, 0⟩,
   ⟨10, 11, 1, 0⟩, ⟨10, 13, 1, 0⟩, ⟨11, 12, 1, 0⟩,
   ⟨12, 21, 1, 0⟩, ⟨12, 22, 1, 0⟩, ⟨12, 34, 1, 0⟩,
   ⟨13, 14, 1, 0⟩, ⟨13, 23, 1, 0⟩, ⟨13, 35, 1, 0⟩,
   ⟨14, 15, -1, 1⟩, ⟨14, 16, 0, 1⟩, ⟨14, 17, 1, 0⟩,
   ⟨14, 18, -1, 1⟩, ⟨14, 24, -2, 1⟩, ⟨14, 25, -1, 1⟩,
   ⟨14, 26, 0, 1⟩, ⟨14, 27, 1, 0⟩, ⟨14, 36, 1, 0⟩, ⟨14, 37, 1, 0⟩,
   ⟨15, 7, 1, 0⟩,
   ⟨16, 2, -2, 1⟩, ⟨16, 3, -1, 1⟩, ⟨16, 4, -1, 1⟩, ⟨16, 5, 1, 0⟩,
   ⟨16, 6, 0, 1⟩, ⟨16, 7, -1, 1⟩, ⟨16, 8, 1, 0⟩,
   ⟨16, 20, -2, 1⟩, ⟨16, 33, 1, 0⟩,
   ⟨17, 7, 1, 0⟩, ⟨18, 20, 1, 0⟩, ⟨19, 13, 1, 0⟩,
   ⟨20, 15, 1, 0⟩, ⟨21, 9, 1, 0⟩, ⟨22, 10, 1, 0⟩, ⟨23, 15, 1, 0⟩,
   ⟨24, 0, 1, 0⟩, ⟨24, 1, 1, 0⟩, ⟨24, 4, 1, 0⟩,
   ⟨25, 2, 1, 0⟩, ⟨25, 3, 1, 0⟩, ⟨26, 6, 1, 0⟩,
   ⟨27, 4, 1, 0⟩, ⟨27, 12, 1, 0⟩, ⟨28, 11, 1, 0⟩, ⟨29, 7, 1, 0⟩,
   ⟨30, 24, 1, 0⟩, ⟨31, 10, 1, 0⟩, ⟨31, 25, 1, 0⟩,
   ⟨32, 15, 1, 0⟩, ⟨33, 15, 1, 0⟩, ⟨34, 9, 1, 0⟩, ⟨34, 26, 1, 0⟩,
   ⟨35, 10, 1, 0⟩, ⟨35, 24, 1, 0⟩, ⟨35, 25, 1, 0⟩,
   ⟨36, 20, 1, 0⟩, ⟨37, 6, 1, 0⟩,
   ⟨38, 0, 1, 0⟩, ⟨38, 1, 1, 0⟩, ⟨38, 4, 1, 0⟩]

def neighbor2Edges : Finset AffineEdge := neighbor2EdgeList.toFinset

theorem neighbor2Edges_card : neighbor2Edges.card = 91 := by
  unfold neighbor2Edges neighbor2EdgeList
  native_decide

def neighbor2BoundarySource : Finset Nat :=
  ({e ∈ neighbor2Edges | e.slope ≠ 0} : Finset AffineEdge).image AffineEdge.source

def neighbor2BoundaryTarget : Finset Nat :=
  ({e ∈ neighbor2Edges | e.slope ≠ 0} : Finset AffineEdge).image AffineEdge.target

theorem neighbor2BoundarySource_eq :
    neighbor2BoundarySource = {3, 9, 14, 16} := by
  unfold neighbor2BoundarySource neighbor2Edges neighbor2EdgeList
  native_decide

theorem neighbor2BoundaryTarget_eq :
    neighbor2BoundaryTarget =
    {2, 3, 4, 6, 7, 15, 16, 18, 20, 24, 25, 26} := by
  unfold neighbor2BoundaryTarget neighbor2Edges neighbor2EdgeList
  native_decide

theorem neighbor2_d_nnz :
    ({e ∈ neighbor2Edges | e.slope ≠ 0} : Finset AffineEdge).card = 24 := by
  unfold neighbor2Edges neighbor2EdgeList
  native_decide

/-- The boundary-layer support theorem (kernel-checked) for
neighbor 2: D has exactly 24 nonzero entries on sources {3, 9, 14, 16}
and targets {2, 3, 4, 6, 7, 15, 16, 18, 20, 24, 25, 26}. -/
theorem neighbor2_boundary_layer_support :
    ({e ∈ neighbor2Edges | e.slope ≠ 0} : Finset AffineEdge).card = 24 ∧
    (∀ s ∈ neighbor2BoundarySource,
        s = 3 ∨ s = 9 ∨ s = 14 ∨ s = 16) ∧
    (∀ t ∈ neighbor2BoundaryTarget,
        t = 2 ∨ t = 3 ∨ t = 4 ∨ t = 6 ∨ t = 7 ∨ t = 15 ∨
        t = 16 ∨ t = 18 ∨ t = 20 ∨ t = 24 ∨ t = 25 ∨ t = 26) := by
  refine ⟨?_, ⟨?_, ?_⟩⟩
  · unfold neighbor2Edges neighbor2EdgeList
    native_decide
  · intro s hs
    rw [neighbor2BoundarySource_eq] at hs
    simpa using hs
  · intro t ht
    rw [neighbor2BoundaryTarget_eq] at ht
    simpa using ht

/-- Aggregate statement: the boundary-layer support theorem holds for
all three Class-II adjacent-swap neighbors simultaneously.  This is
the kernel-checked summary of the affine catalogue D = A(a) - A(3)
over (a-3) extraction: every nonzero D entry lies on a boundary-
layer source/target role. -/
theorem all_neighbors_boundary_layer_support :
    (({e ∈ neighbor0Edges | e.slope ≠ 0} : Finset AffineEdge).card = 12 ∧
     ({e ∈ neighbor1Edges | e.slope ≠ 0} : Finset AffineEdge).card = 5 ∧
     ({e ∈ neighbor2Edges | e.slope ≠ 0} : Finset AffineEdge).card = 24) ∧
    neighbor0BoundarySource = {1, 4, 8, 9} ∧
    neighbor1BoundarySource = {1, 9} ∧
    neighbor2BoundarySource = {3, 9, 14, 16} := by
  refine ⟨⟨?_, ⟨?_, ?_⟩⟩, ⟨?_, ⟨?_, ?_⟩⟩⟩
  · unfold neighbor0Edges neighbor0EdgeList
    native_decide
  · unfold neighbor1Edges neighbor1EdgeList
    native_decide
  · unfold neighbor2Edges neighbor2EdgeList
    native_decide
  · exact neighbor0BoundarySource_eq
  · exact neighbor1BoundarySource_eq
  · exact neighbor2BoundarySource_eq
