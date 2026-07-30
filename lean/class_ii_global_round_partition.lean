/-
  class_ii_global_round_partition.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Total disjoint five-phase partition of legal rounds for the Class-II neighbor-2 global layer induction.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  Total round-domain partition for the Class-II neighbor-2 global
  layer induction. Local raw-corona and Red certificates are indexed
  by these five cases; this file proves that, for a >= 7, every legal
  round through the first repeated-terminal step belongs to exactly
  one case.
-/

import Mathlib.Data.Int.Basic
import Mathlib.Tactic

inductive ClassIIGlobalRoundPhase
  | base
  | stable
  | penultimate
  | terminal
  | repeated
  deriving DecidableEq

def classIIGlobalRoundPhase (a r : Int) :
    ClassIIGlobalRoundPhase :=
  if r ≤ 4 then .base
  else if r ≤ a - 2 then .stable
  else if r = a - 1 then .penultimate
  else if r = a then .terminal
  else .repeated

/-- The five local-certificate domains cover every legal round. -/
theorem class_ii_global_round_domains_exhaustive
    (a r : Int) (_ha : 7 ≤ a) (_hr0 : 1 ≤ r) (hr1 : r ≤ a + 1) :
    r ≤ 4 ∨
      (5 ≤ r ∧ r ≤ a - 2) ∨
      r = a - 1 ∨
      r = a ∨
      r = a + 1 := by
  omega

/-- The phase selector agrees with each displayed domain. -/
theorem class_ii_global_round_phase_spec
    (a r : Int) (ha : 7 ≤ a) (hr0 : 1 ≤ r) (hr1 : r ≤ a + 1) :
    (classIIGlobalRoundPhase a r = .base ↔ r ≤ 4) ∧
    (classIIGlobalRoundPhase a r = .stable ↔
      5 ≤ r ∧ r ≤ a - 2) ∧
    (classIIGlobalRoundPhase a r = .penultimate ↔ r = a - 1) ∧
    (classIIGlobalRoundPhase a r = .terminal ↔ r = a) ∧
    (classIIGlobalRoundPhase a r = .repeated ↔ r = a + 1) := by
  unfold classIIGlobalRoundPhase
  split_ifs <;> simp_all <;> omega

/-- A legal round has one uniquely determined phase. -/
theorem class_ii_global_round_phase_unique
    (a r : Int)
    (left right : ClassIIGlobalRoundPhase)
    (hleft : classIIGlobalRoundPhase a r = left)
    (hright : classIIGlobalRoundPhase a r = right) :
    left = right := by
  rw [← hleft, ← hright]

/--
Route a round-local transition proposition through the five exact
certificate domains.  This is the logical stitching step: it does not
assert any raw-corona or Red premise, but shows that supplying those
five premises proves the proposition at every legal round.
-/
theorem class_ii_global_round_stitch
    (a : Int) (P : Int → Prop) (ha : 7 ≤ a)
    (hbase : ∀ r, 1 ≤ r → r ≤ 4 → P r)
    (hstable : ∀ r, 5 ≤ r → r ≤ a - 2 → P r)
    (hpenultimate : P (a - 1))
    (hterminal : P a)
    (hrepeated : P (a + 1)) :
    ∀ r, 1 ≤ r → r ≤ a + 1 → P r := by
  intro r hr0 hr1
  rcases class_ii_global_round_domains_exhaustive a r ha hr0 hr1 with
    hbaseDomain | hstableDomain | hpenultimateDomain |
      hterminalDomain | hrepeatedDomain
  · exact hbase r hr0 hbaseDomain
  · exact hstable r hstableDomain.1 hstableDomain.2
  · simpa [hpenultimateDomain] using hpenultimate
  · simpa [hterminalDomain] using hterminal
  · simpa [hrepeatedDomain] using hrepeated
