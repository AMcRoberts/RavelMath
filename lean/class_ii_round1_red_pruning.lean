/-
  class_ii_round1_red_pruning.lean

  Author: Ravel.
  Date:   2026-07-31.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Formalizes the arithmetic core of Round 1's Red-pruning
          symbolic closure (the first of the four exceptional base
          transitions in the Class-II global occurrence theorem):
          every raw forward-candidate's x2'-image is bounded away
          from the 27-state raw pre-Red target's x2-range, for every
          integer a >= 3.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  Source of truth: app/class_ii_neighbor2_round1_red_symbolic.cpp
  (the six affine forms below, reproduced exactly from that file's
  checked output) and app/class_ii_neighbor2_round1_red_forward_check.cpp
  (the 27-state raw target list and its coordinate bounds, reproduced
  exactly from that file's `target_states()`).

  Scope, stated precisely (matching docs/GLOBAL_CATALOGUE_OCCURRENCE_
  EXHAUSTION.md's own "Round 1: Red pruning closed symbolically for
  a>=3" section): this file formalizes the ARITHMETIC core of the
  argument -- that the six raw candidates' x2'-images can never equal
  any of the 27 targets' x2-coordinates, hence (since two states with
  different x2-coordinates cannot be equal) can never equal any of
  the 27 target states, for every integer a >= 3. It does NOT
  re-derive, inside Lean, the combinatorial fact that these six are
  exactly the raw candidates produced by `forward_edges_type1`/`type2`
  applied to the two D_cont seeds A and B (that fact rests on
  `tau_a`'s fixed word forms giving letter 1 exactly one occurrence
  and letter 2 exactly two occurrences across all three images, for
  every a in the family's domain -- established and checked in the
  C++ file cited above via `class_ii_neighbor_symbolic_prefix_families`,
  not re-derived here). This is exactly the same scope convention
  `class_ii_neighbor2_extensions.lean` already uses for its fixed-
  state catalogues (e.g. `neighbor2FixedNode`, `centerInterfaceSupportNode`):
  the catalogue itself is taken as given, checked C++ data; what Lean
  adds is a kernel-checked proof that the stated arithmetic property
  holds for it.
-/

import Mathlib.Tactic

/-- A neighbor-family state, `(i, x0, x1, x2, j)` -- the same
representation `class_ii_neighbor2_extensions.lean` calls
`NeighborNode`, redefined here so this file stays self-contained
(no local file imports another local file in this project). -/
abbrev NeighborNode := ℤ × ℤ × ℤ × ℤ × ℤ

/-- The 27-state raw pre-Red target for Round 1's neighbor
`tau_a`, reproduced exactly from
`app/class_ii_neighbor2_round1_red_forward_check.cpp`'s
`target_states()`. -/
def round1Raw27 : List NeighborNode :=
  [ (0,-1, 1, 1,0), (0,-1, 1, 1,2), (0, 1,-1, 0,2), (0, 1,-1, 1,0),
    (0, 1, 0,-1,0), (1, 0, 1, 0,0), (1, 1,-1, 0,1), (1, 2,-1,-1,0),
    (2,-1, 2, 0,1), (2, 1,-1, 0,0), (2, 1,-1, 0,2),
    (0,-1, 1, 1,1), (0, 0, 0, 0,1), (0, 0, 0, 0,2), (0, 0, 0, 1,0),
    (0, 0, 0, 1,1), (0, 0, 1, 0,0), (0, 1,-1, 0,0), (1, 0, 0, 0,2),
    (1, 1,-1, 0,0), (1, 1,-1, 0,2), (1, 1, 0,-1,0), (2, 0, 1,-1,0),
    (2, 0, 1, 0,0), (2, 1, 0,-1,0),
    (1, 0, 0, 1,1), (2, 0, 1,-1,1) ]

/-- The target list has the expected cardinality (27, matching the
"27-state raw pre-Red target" name used throughout the project's
documentation). -/
theorem round1Raw27_length : round1Raw27.length = 27 := by decide

/-- Every one of the 27 raw pre-Red targets has its x2-coordinate in
`{-1, 0, 1}` -- a tighter bound than the coarse "all three coordinates
in `[-2,2]`" bound used in the prose write-up (that coarser bound
comes from x0/x1, not x2; checked directly here for x2 specifically,
not assumed). -/
theorem round1Raw27_x2_bound :
    ∀ n ∈ round1Raw27, n.2.2.2.1 = -1 ∨ n.2.2.2.1 = 0 ∨ n.2.2.2.1 = 1 := by
  decide

/-- The six raw forward-candidate kinds enumerated by
`forward_edges_type1`/`type2` applied to the two D_cont seeds
`A = {1,{0,0,1},1}` and `B = {2,{0,1,-1},1}` (2 candidates for A,
4 for B), exactly as named in
`app/class_ii_neighbor2_round1_red_symbolic.cpp`'s printed output. -/
inductive Round1RawCandidateKind
  | aType1 | aType2 | bType1First | bType1Second | bType2First | bType2Second
  deriving DecidableEq, Fintype

theorem round1RawCandidateKind_card :
    Fintype.card Round1RawCandidateKind = 6 := by native_decide

/-- Each raw candidate's x2'-image, as an affine function of `a`.
These six closed forms are reproduced exactly from
`class_ii_neighbor2_round1_red_symbolic.cpp`'s checked output
(`-a, a, a, a+1, -a, 1-a`), derived there from the fixed incidence
matrix `M(a) = [[a,a,1],[1,0,0],[1,1,0]]` by back-substitution:
`x2' = rhs[0] - a * rhs[2]`, with `rhs[2]` itself a-independent for
every one of the six candidates (checked, not assumed, in that
file). -/
def round1RawCandidateX2 : Round1RawCandidateKind → ℤ → ℤ
  | .aType1 => fun a => -a
  | .aType2 => fun a => a
  | .bType1First => fun a => a
  | .bType1Second => fun a => a + 1
  | .bType2First => fun a => -a
  | .bType2Second => fun a => 1 - a

/-- Every raw candidate's x2'-image lies outside `{-1, 0, 1}` for
every integer `a >= 3`. The binding case is `bType2Second` (`1 - a`):
at `a = 2` this equals `-1`, exactly on the target range's boundary
(matching the prose write-up's own note that `a = 2` is exceptional
and not covered by this argument); for `a >= 3`, `1 - a <= -2`,
strictly outside. -/
theorem round1RawCandidateX2_outside_target_range
    (kind : Round1RawCandidateKind) (a : ℤ) (ha : 3 ≤ a) :
    round1RawCandidateX2 kind a ≠ -1 ∧
      round1RawCandidateX2 kind a ≠ 0 ∧
      round1RawCandidateX2 kind a ≠ 1 := by
  cases kind <;> simp only [round1RawCandidateX2] <;> omega

/-- The arithmetic core of Round 1's Red-pruning closure: for every
integer `a >= 3` and every one of the six raw candidate kinds, the
candidate's x2'-image cannot equal the x2-coordinate of any of the 27
raw pre-Red targets. Since two states with different x2-coordinates
cannot be the same state, no raw candidate (of any of the six kinds)
can equal any of the 27 targets, for any `a >= 3`. This is exactly
`induced_restricted_edges` finding zero edges from either D_cont seed
into the 27-state raw set -- `red_anode`'s rank-one Red-pruning
criterion -- matching the exact finite check in
`class_ii_neighbor2_round1_red_identity.cpp` at `a = 3..60`, now
closed here for literally every integer `a >= 3`. -/
theorem round1_red_pruning_no_match
    (kind : Round1RawCandidateKind) (a : ℤ) (ha : 3 ≤ a) :
    ∀ n ∈ round1Raw27, n.2.2.2.1 ≠ round1RawCandidateX2 kind a := by
  intro n hn hx2
  have hbound := round1Raw27_x2_bound n hn
  have hnot := round1RawCandidateX2_outside_target_range kind a ha
  rw [hx2] at hbound
  rcases hbound with h | h | h
  · exact hnot.1 h
  · exact hnot.2.1 h
  · exact hnot.2.2 h
