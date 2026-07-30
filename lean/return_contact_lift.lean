/-
  return_contact_lift.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Graph projection and both prefix-occurrence commuting equations for the contact/return lift, independent of C++.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  Abstract correctness theorem for the labelled return/contact lift.
  This file is independent of the C++ representation and finite search.
-/

import Mathlib.Data.Option.Basic

universe uA uX uP

structure PhaseSystem (A : Type uA) (P : Type uP) where
  letter : P → A
  image : P → Nat → Option P
  substitution : A → Nat → Option A
  projects :
    ∀ phase position,
      Option.map letter (image phase position)
        = substitution (letter phase) position

structure ContactVertex (A : Type uA) (X : Type uX) where
  left : A
  displacement : X
  right : A

structure LabelledContactEdge (A : Type uA) (X : Type uX) where
  source : ContactVertex A X
  destination : ContactVertex A X
  leftPosition : Nat
  rightPosition : Nat

structure LiftedContactVertex
    (A : Type uA) (X : Type uX) (P : Type uP)
    (phases : PhaseSystem A P) where
  bare : ContactVertex A X
  leftPhase : P
  rightPhase : P
  leftCompatible : phases.letter leftPhase = bare.left
  rightCompatible : phases.letter rightPhase = bare.right

structure LiftedContactEdge
    (A : Type uA) (X : Type uX) (P : Type uP)
    (phases : PhaseSystem A P) where
  bare : LabelledContactEdge A X
  source : LiftedContactVertex A X P phases
  destination : LiftedContactVertex A X P phases
  sourceProjects : source.bare = bare.source
  destinationProjects : destination.bare = bare.destination
  leftPhaseEquation :
    phases.image destination.leftPhase bare.leftPosition
      = some source.leftPhase
  rightPhaseEquation :
    phases.image destination.rightPhase bare.rightPosition
      = some source.rightPhase

/-- Forgetting phase data maps every lifted edge to its defining bare
contact edge. This is the graph-morphism part of the construction. -/
theorem lifted_edge_projects
    {A : Type uA} {X : Type uX} {P : Type uP}
    {phases : PhaseSystem A P}
    (edge : LiftedContactEdge A X P phases) :
    edge.source.bare = edge.bare.source
      ∧ edge.destination.bare = edge.bare.destination :=
  ⟨edge.sourceProjects, edge.destinationProjects⟩

/-- The inverse phase equation at the left prefix position commutes
with the original substitution letter map. -/
theorem lifted_left_occurrence_commutes
    {A : Type uA} {X : Type uX} {P : Type uP}
    {phases : PhaseSystem A P}
    (edge : LiftedContactEdge A X P phases) :
    phases.substitution edge.bare.destination.left
        edge.bare.leftPosition
      = some edge.bare.source.left := by
  have projection :=
    phases.projects edge.destination.leftPhase edge.bare.leftPosition
  rw [edge.leftPhaseEquation] at projection
  simp only [Option.map_some] at projection
  rw [edge.destination.leftCompatible, edge.destinationProjects] at projection
  rw [edge.source.leftCompatible, edge.sourceProjects] at projection
  exact projection.symm

/-- The corresponding right-prefix occurrence equation. -/
theorem lifted_right_occurrence_commutes
    {A : Type uA} {X : Type uX} {P : Type uP}
    {phases : PhaseSystem A P}
    (edge : LiftedContactEdge A X P phases) :
    phases.substitution edge.bare.destination.right
        edge.bare.rightPosition
      = some edge.bare.source.right := by
  have projection :=
    phases.projects edge.destination.rightPhase edge.bare.rightPosition
  rw [edge.rightPhaseEquation] at projection
  simp only [Option.map_some] at projection
  rw [edge.destination.rightCompatible, edge.destinationProjects] at projection
  rw [edge.source.rightCompatible, edge.sourceProjects] at projection
  exact projection.symm
