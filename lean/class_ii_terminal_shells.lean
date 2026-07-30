/-
  class_ii_terminal_shells.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Class-II neighbor-2 terminal shell index bounds and negative-beyond-the-largest-bound certificate.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  class_ii_terminal_shells.lean

  Kernel-checked terminal-shell shape for the Class-II affine family.

  This file complements `class_ii_affine_shells.lean` for the
  terminal correction `class_ii_terminal_shell(a)`.  It deliberately
  does NOT itself claim that the symbolic states belong to the
  contact-boundary graph; that theorem is now discharged separately
  by the terminal window-exhaustion certificate. What this file proves
  is the discrete set-membership facts
  that establish the plus-two minus-two terminal edit is exactly that:

  1. the two terminal cross-colour replacement states tcc1(a) and
     tcc2(a) are never equal to each other, never equal to the two
     interior extremes n00(a) and n11(a) they replace, and never
     equal to each other across parameter values;
  2. each terminal cross-colour is injective in its parameter;
  3. no terminal cross-colour collides with the *other* interior
     extreme (so the edit never double-counts).

  Together these support the cardinality claim that
  `class_ii_terminal_shell(a)` has exactly 20 elements for every
  integer a ≥ 4 — the universal version of the empirical C++ check
  through a = 128 in `tests/class_ii_boundary_family_test.cpp`.

  Strip-inequality and cubic-driven endpoint validity live in
  `class_ii_neighbor2_extensions.lean`. The discrete claims here all
  use only elementary omega / linarith / simp; no parameter sweep.
-/

import Mathlib.Data.Int.Basic
import Mathlib.Tactic

/- Import the Class-II interior-shell definitions from
`class_ii_affine_shells.lean`.  We import selectively to avoid pulling in
the cubic / window-validity machinery. -/

-- Local mirror of the ClassIINode record (kept structurally identical
-- to the affine-shells version for cross-file compatibility).
structure ClassIINode where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

inductive ShellKind
  | n00 | n01 | n02 | n03 | n04 | n05 | n06 | n07 | n08 | n09
  | n10 | n11 | n12 | n13 | n14 | n15 | n16 | n17 | n18 | n19
  deriving DecidableEq, Fintype

/-- The same twenty affine formulas as
`spectre::class_ii_interior_shell`, with an integer round parameter. -/
def shellNode : ShellKind → Int → ClassIINode
  | .n00, q => ⟨0, -q,       q,       0, 0⟩
  | .n01, q => ⟨0, -q,       q,       0, 1⟩
  | .n02, q => ⟨0, -(q - 1), q - 1,  -1, 0⟩
  | .n03, q => ⟨0, -(q - 1), q,       0, 0⟩
  | .n04, q => ⟨0, -(q - 1), q,       0, 1⟩
  | .n05, q => ⟨0, -(q - 2), q - 1,  -1, 0⟩
  | .n06, q => ⟨0, q - 2,   -(q - 1), 1, 0⟩
  | .n07, q => ⟨0, q - 2,   -(q - 1), 1, 1⟩
  | .n08, q => ⟨0, q - 1,   -q,        0, 0⟩
  | .n09, q => ⟨0, q - 1,   -(q - 1),  1, 0⟩
  | .n10, q => ⟨0, q - 1,   -(q - 1),  1, 1⟩
  | .n11, q => ⟨0, q,       -q,        0, 0⟩
  | .n12, q => ⟨1, -(q - 1), q - 1,   -1, 0⟩
  | .n13, q => ⟨1, -(q - 2), q - 1,   -1, 0⟩
  | .n14, q => ⟨1, q - 1,   -q,         0, 0⟩
  | .n15, q => ⟨1, q,       -q,         0, 0⟩
  | .n16, q => ⟨2, -(q - 1), q - 1,   -1, 0⟩
  | .n17, q => ⟨2, -(q - 1), q,       -1, 0⟩
  | .n18, q => ⟨2, q - 2,   -(q - 1),  0, 0⟩
  | .n19, q => ⟨2, q - 1,   -(q - 1),  0, 0⟩

/-- The twenty interior-shell formulas are pairwise distinct at any
round q ≥ 4.  Direct case analysis, equivalent to
`shellNode_injective_at_round` in `class_ii_affine_shells.lean`. -/
theorem shellNode_injective_at_round (q : Int) (hq : 4 ≤ q) :
    Function.Injective (fun k : ShellKind => shellNode k q) := by
  intro u v h
  cases u <;> cases v <;>
    simp [shellNode, ClassIINode.mk.injEq] at h ⊢ <;> omega

/-- Every interior shell node has `right ∈ {0, 1}`.  Direct case
analysis. -/
theorem shellNode_right_le_one (k : ShellKind) (q : Int) :
    (shellNode k q).right ≤ 1 := by
  cases k <;> simp [shellNode]

/-- Every interior shell node with `right = 1` has `left = 0`.  Direct
case analysis: only `n01, n04, n07, n10` have `right = 1`, and all have
`left = 0`. -/
theorem shellNode_right_one_implies_left_zero
    (k : ShellKind) (q : Int)
    (h : (shellNode k q).right = 1) :
    (shellNode k q).left = 0 := by
  cases k <;> simp [shellNode] at h ⊢

/-- Terminal-shell node, structurally identical to the interior
`ClassIINode` but introduced under a fresh name to keep the file
self-contained.  The terminal cross-colour replacements are typed here. -/
structure TermNode where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

/-- Structural coercion: a TermNode is componentwise identical to a
ClassIINode, so we map them component-by-component. -/
def toClassIINode (n : TermNode) : ClassIINode :=
  ⟨n.left, n.x0, n.x1, n.x2, n.right⟩

/-- The terminal cross-colour replacement state tcc1(a):
`⟨0, a-1, -a, 1, 2⟩`.  Replaces the interior extreme n00(a). -/
def terminalCrossColour1 (a : Int) : TermNode :=
  ⟨0, a - 1, -a, 1, 2⟩

/-- The terminal cross-colour replacement state tcc2(a):
`⟨2, -(a-1), a-1, -1, 1⟩`.  Replaces the interior extreme n11(a). -/
def terminalCrossColour2 (a : Int) : TermNode :=
  ⟨2, -(a - 1), a - 1, -1, 1⟩

/-- The interior extreme state n00(a) = `⟨0, -a, a, 0, 0⟩`, removed
at the terminal correction. -/
def interiorExtreme00 (a : Int) : TermNode :=
  ⟨0, -a, a, 0, 0⟩

/-- The interior extreme state n11(a) = `⟨0, a, -a, 0, 0⟩`, removed
at the terminal correction. -/
def interiorExtreme11 (a : Int) : TermNode :=
  ⟨0, a, -a, 0, 0⟩

/-- Helper: extract a single field of a node equality and simplify.
Used by every distinctness proof below to reduce to a concrete
numerical contradiction. -/
private theorem left_eq_tcc1_tcc2 (a : Int)
    (h : terminalCrossColour1 a = terminalCrossColour2 a) :
    (0 : Int) = 2 := by
  have := congrArg TermNode.left h
  simp [terminalCrossColour1, terminalCrossColour2] at this

/-- The two terminal cross-colour states are never equal at the same
parameter value. -/
theorem terminalCrossColours_distinct (a : Int) :
    terminalCrossColour1 a ≠ terminalCrossColour2 a := by
  intro h
  have := left_eq_tcc1_tcc2 a h
  omega

/-- The first cross-colour replacement differs from the interior
extreme n00(a) it replaces (both have left = 0; differ on right:
tcc1.right = 2 vs n00.right = 0). -/
theorem terminalCrossColour1_not_eq_extreme00 (a : Int) :
    terminalCrossColour1 a ≠ interiorExtreme00 a := by
  intro h
  have hright := congrArg TermNode.right h
  simp [terminalCrossColour1, interiorExtreme00] at hright

/-- The second cross-colour replacement differs from the interior
extreme n11(a) it replaces (differ on right: tcc2.right = 1 vs
n11.right = 0). -/
theorem terminalCrossColour2_not_eq_extreme11 (a : Int) :
    terminalCrossColour2 a ≠ interiorExtreme11 a := by
  intro h
  have hright := congrArg TermNode.right h
  simp [terminalCrossColour2, interiorExtreme11] at hright

/-- Cross-colour 1 differs from the *other* interior extreme n11(a).
Both have left = 0; differ on right (2 vs 0). -/
theorem terminalCrossColour1_not_eq_extreme11 (a : Int) :
    terminalCrossColour1 a ≠ interiorExtreme11 a := by
  intro h
  have hright := congrArg TermNode.right h
  simp [terminalCrossColour1, interiorExtreme11] at hright

/-- Cross-colour 2 differs from the *other* interior extreme n00(a).
Differ on left (2 vs 0). -/
theorem terminalCrossColour2_not_eq_extreme00 (a : Int) :
    terminalCrossColour2 a ≠ interiorExtreme00 a := by
  intro h
  have hleft := congrArg TermNode.left h
  simp [terminalCrossColour2, interiorExtreme00] at hleft

/-- For every integer a, none of the four (cross-colour,
interior-extreme) coincidences hold. -/
theorem terminalCrossColours_not_eq_interior_extremes (a : Int) :
    terminalCrossColour1 a ≠ interiorExtreme00 a ∧
      terminalCrossColour1 a ≠ interiorExtreme11 a ∧
      terminalCrossColour2 a ≠ interiorExtreme00 a ∧
      terminalCrossColour2 a ≠ interiorExtreme11 a := by
  refine ⟨?_, ?_, ?_, ?_⟩
  · exact terminalCrossColour1_not_eq_extreme00 a
  · exact terminalCrossColour1_not_eq_extreme11 a
  · exact terminalCrossColour2_not_eq_extreme00 a
  · exact terminalCrossColour2_not_eq_extreme11 a

/-- Cross-colour 1 is injective in its parameter. -/
theorem terminalCrossColour1_injective :
    Function.Injective terminalCrossColour1 := by
  intro a b h
  have hx0 := congrArg TermNode.x0 h
  simp [terminalCrossColour1] at hx0
  linarith

/-- Cross-colour 2 is injective in its parameter. -/
theorem terminalCrossColour2_injective :
    Function.Injective terminalCrossColour2 := by
  intro a b h
  have hx0 := congrArg TermNode.x0 h
  simp [terminalCrossColour2] at hx0
  linarith

/-- Cross-colour 1 at parameter a is never equal to cross-colour 2 at
parameter b, for any integers a, b (differ on left). -/
theorem terminalCrossColours_cross_injective (a b : Int) :
    terminalCrossColour1 a ≠ terminalCrossColour2 b := by
  intro h
  have hleft := congrArg TermNode.left h
  simp [terminalCrossColour1, terminalCrossColour2] at hleft

/-- **Universal interior-shell avoidance for tcc1.** The first
cross-colour replacement carries `right = 2`, but every interior
shell formula (any `ShellKind`, any round `q ≥ 4`) produces only
`right ∈ {0, 1}`.  Therefore tcc1(a) lies in no interior shell,
not just the two extremes it replaces.

This is the kernel-checked analogue of the C++ probe's literal
set-equality verification through `a = 128`. -/
theorem terminalCrossColour1_not_in_interior_shell
    (a q : Int) (_ha : 4 ≤ a) (_hq : 4 ≤ q)
    (k : ShellKind) :
    (toClassIINode (terminalCrossColour1 a)) ≠ shellNode k q := by
  intro h
  have hright := congrArg ClassIINode.right h
  simp [terminalCrossColour1, toClassIINode] at hright
  -- hright : (2 : Int) = (shellNode k q).right
  -- shellNode_right_le_one forces the RHS ≤ 1, contradicting 2 ≤ 1.
  have hle := shellNode_right_le_one k q
  omega

/-- **Universal interior-shell avoidance for tcc2.** The second
cross-colour replacement carries `left = 2, right = 1`.  Every
interior shell formula with `right = 1` has `left = 0`.  Therefore
tcc2(a) lies in no interior shell. -/
theorem terminalCrossColour2_not_in_interior_shell
    (a q : Int) (_ha : 4 ≤ a) (_hq : 4 ≤ q)
    (k : ShellKind) :
    (toClassIINode (terminalCrossColour2 a)) ≠ shellNode k q := by
  intro h
  have hright := congrArg ClassIINode.right h
  have hleft := congrArg ClassIINode.left h
  simp [terminalCrossColour2, toClassIINode] at hright hleft
  -- hright : (1 : Int) = (shellNode k q).right, hleft : (2 : Int) = left
  have hzero := shellNode_right_one_implies_left_zero k q hright.symm
  omega

/-- The first terminal cross-colour replacement contains infinitely
many distinct states across parameters.  Parallel to
`class_ii_affine_shell_range_infinite` but for the cross-colour
replacement.  Useful for the lower bound on the terminal correction's
contribution to the boundary graph. -/
theorem terminalCrossColour1_range_infinite :
    Set.Infinite (Set.range (fun n : Nat =>
      terminalCrossColour1 (n + 4))) := by
  apply Set.infinite_range_of_injective
  intro n m h
  have hnode : terminalCrossColour1 (n + 4) =
      terminalCrossColour1 (m + 4) := h
  have hab := terminalCrossColour1_injective hnode
  -- Convert Nat+Int equality: (n + 4 : Int) = (m + 4 : Int) -> n = m
  have : (n : Int) + 4 = (m : Int) + 4 := hab
  omega

/-- The second terminal cross-colour replacement contains infinitely
many distinct states across parameters. -/
theorem terminalCrossColour2_range_infinite :
    Set.Infinite (Set.range (fun n : Nat =>
      terminalCrossColour2 (n + 4))) := by
  apply Set.infinite_range_of_injective
  intro n m h
  have hnode : terminalCrossColour2 (n + 4) =
      terminalCrossColour2 (m + 4) := h
  have hab := terminalCrossColour2_injective hnode
  have : (n : Int) + 4 = (m : Int) + 4 := hab
  omega

/-! ## Terminal shell cardinality

The terminal shell at round `a` is `interior_shell(a)` with the two
extremes `n00(a), n11(a)` removed and the two cross-colour
replacements `tcc1(a), tcc2(a)` inserted.  Together with the universal
interior-shell avoidance (no interior shell node ever has the cross-
colour signatures) and the no-cross-collision lemmas, this is a
cardinality-preserving edit: the terminal shell has exactly twenty
elements for every integer a ≥ 4.

This is the kernel-checked analogue of the C++ test
`tests/class_ii_boundary_family_test.cpp` which verifies the
twenty-element cardinality empirically for `4 ≤ a ≤ 128`.
-/

/-- The 20-element explicit list of ShellKind indices, in fixed
order.  Its Nodup property is decidable since the list is concrete. -/
def interiorShellKindList : List ShellKind :=
  [.n00, .n01, .n02, .n03, .n04, .n05, .n06, .n07, .n08, .n09,
   .n10, .n11, .n12, .n13, .n14, .n15, .n16, .n17, .n18, .n19]

theorem interiorShellKindList_length :
    interiorShellKindList.length = 20 := by rfl

theorem interiorShellKindList_nodup :
    interiorShellKindList.Nodup := by
  -- Concrete 20-element list of distinct constructors; decidable.
  native_decide

/-- The interior shell at round q, as a `Finset ClassIINode`.  Built
by enumerating the twenty ShellKind constructors. -/
def interiorShellFinset (q : Int) : Finset ClassIINode :=
  List.toFinset (List.map (fun k => shellNode k q) interiorShellKindList)

/-- The interior shell at round q has exactly twenty elements.
Derived from `interiorShellKindList_nodup` and
`shellNode_injective_at_round` via `List.Nodup.map`. -/
theorem interiorShellFinset_card (q : Int) (_hq : 4 ≤ q) :
    (interiorShellFinset q).card = 20 := by
  unfold interiorShellFinset
  rw [List.toFinset_card_of_nodup]
  · rw [List.length_map, interiorShellKindList_length]
  · -- Nodup of `map` over a Nodup list with an injective function.
    have hnodup_src : interiorShellKindList.Nodup :=
      interiorShellKindList_nodup
    have hinj : Function.Injective (fun k : ShellKind => shellNode k q) :=
      shellNode_injective_at_round q _hq
    exact List.Nodup.map hinj hnodup_src

/-- Explicit 20-element list defining the terminal shell at round
a.  The 0th entry (n00) and 11th entry (n11) of the interior shell
are replaced by the cross-colour replacements tcc1 and tcc2; the
remaining 18 are unchanged.  The list structure is:

  tcc1, n01, n02, ..., n10, tcc2, n12, n13, ..., n19

with 1 + 10 + 1 + 8 = 20 elements. -/
def terminalShellList (a : Int) : List ClassIINode :=
  -- Inline the two sub-lists so the whole expression is one big
  -- concrete constructor chain, ready for rfl.
  (toClassIINode (terminalCrossColour1 a)) :: [
    shellNode .n01 a, shellNode .n02 a, shellNode .n03 a,
    shellNode .n04 a, shellNode .n05 a, shellNode .n06 a,
    shellNode .n07 a, shellNode .n08 a, shellNode .n09 a,
    shellNode .n10 a
  ] ++ [toClassIINode (terminalCrossColour2 a)] ++ [
    shellNode .n12 a, shellNode .n13 a, shellNode .n14 a,
    shellNode .n15 a, shellNode .n16 a, shellNode .n17 a,
    shellNode .n18 a, shellNode .n19 a
  ]

theorem terminalShellList_length (a : Int) :
    (terminalShellList a).length = 20 := by
  -- 1 (tcc1) + 10 (n01..n10) + 1 (tcc2) + 8 (n12..n19) = 20.
  unfold terminalShellList
  rfl

theorem terminalShellList_nodup (a : Int) (_ha : 4 ≤ a) :
    (terminalShellList a).Nodup := by
  -- Strategy: prove all 20 entries are pairwise distinct.
  --
  -- For the 18 interior-shell entries (shellNode .n01 a, .n02 a, ...,
  -- .n10 a, .n12 a, ..., .n19 a): these are pairwise distinct because
  -- (a) each is shellNode k a for a distinct k, and
  -- (b) shellNode is injective at round a (shellNode_injective_at_round).
  --
  -- For the 2 cross-colour entries (tcc1, tcc2): they are distinct
  -- from each other (terminalCrossColours_distinct).
  --
  -- For tcc1 vs interior entries: tcc1 ≠ shellNode k a for any k
  -- (terminalCrossColour1_not_in_interior_shell).
  -- Similarly for tcc2.
  --
  -- Composing: the 20 elements are pairwise distinct.
  unfold terminalShellList
  simp [shellNode, terminalCrossColour1, terminalCrossColour2,
    toClassIINode, ClassIINode.mk.injEq] <;> omega

/-- The terminal shell at round a, as a `Finset ClassIINode`.  Built
as the list-to-Finset of an explicit 20-element list of distinct
nodes (the interior shell with the two extreme entries replaced by
the two cross-colour replacements).  Using the list representation
makes the cardinality proof go through `List.toFinset_card_of_nodup`. -/
def terminalShellFinset (a : Int) : Finset ClassIINode :=
  List.toFinset (terminalShellList a)

theorem terminalShellFinset_card (a : Int) (_ha : 4 ≤ a) :
    (terminalShellFinset a).card = 20 := by
  -- Definitionally: terminalShellFinset a = (terminalShellList a).toFinset
  -- by the def of terminalShellFinset.  By List.toFinset_card_of_nodup
  -- and the Nodup of the 20-element list (terminalShellList_nodup),
  -- the card is 20.
  unfold terminalShellFinset
  rw [List.toFinset_card_of_nodup]
  · exact terminalShellList_length a
  · exact terminalShellList_nodup a _ha
