/-
  class_ii_affine_shells.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Class-II affine-shell injectivity, round disjointness, infinite range, and the universal predecessor-plus-fixed-hop identity.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  class_ii_affine_shells.lean

  Kernel-checked affine part of the Class-II growth argument.

  This file deliberately does not claim that the symbolic states below
  belong to the contact-boundary graph.  That is the remaining corona
  inclusion/exhaustion lemma.  What it proves is that the twenty-state
  interior-shell formula is injective both within and between rounds,
  and hence describes an infinite set of pairwise distinct states.
-/

import Mathlib.Data.Int.Basic
import Mathlib.Data.Fintype.Card
import Mathlib.Tactic

structure ClassIINode where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

def mirrorNode (node : ClassIINode) : ClassIINode :=
  ⟨node.right, -node.x0, -node.x1, -node.x2, node.left⟩

inductive ContactKind
  | c00 | c01 | c02 | c03 | c04 | c05 | c06
  | c07 | c08 | c09 | c10 | c11 | c12 | c13
  deriving DecidableEq, Fintype

/-- The signed contact set has exactly 14 elements. -/
theorem contactKind_card : Fintype.card ContactKind = 14 := by
  native_decide

/-- Closed fourteen-state candidate for the Class-II contact set. -/
def contactNode : ContactKind → ClassIINode
  | .c00 => ⟨0, -1,  1,  1, 1⟩
  | .c01 => ⟨0,  0,  0,  0, 1⟩
  | .c02 => ⟨0,  0,  0,  0, 2⟩
  | .c03 => ⟨0,  0,  0,  1, 0⟩
  | .c04 => ⟨0,  0,  0,  1, 1⟩
  | .c05 => ⟨0,  0,  1,  0, 0⟩
  | .c06 => ⟨0,  1, -1,  0, 0⟩
  | .c07 => ⟨1,  0,  0,  0, 2⟩
  | .c08 => ⟨1,  1, -1,  0, 0⟩
  | .c09 => ⟨1,  1, -1,  0, 2⟩
  | .c10 => ⟨1,  1,  0, -1, 0⟩
  | .c11 => ⟨2,  0,  1, -1, 0⟩
  | .c12 => ⟨2,  0,  1,  0, 0⟩
  | .c13 => ⟨2,  1,  0, -1, 0⟩

inductive PreContactKind
  | p00 | p01 | p02 | p03 | p04 | p05 | p06 | p07
  | p08 | p09 | p10 | p11 | p12 | p13 | p14 | p15
  deriving DecidableEq, Fintype

theorem preContactKind_card : Fintype.card PreContactKind = 16 := by
  native_decide

/-- Closed sixteen-state candidate for the restricted pre-contact
graph, before the contact-set Red pass. -/
def preContactNode : PreContactKind → ClassIINode
  | .p00 => ⟨0, -1,  1,  1, 1⟩
  | .p01 => ⟨0,  0,  0,  0, 1⟩
  | .p02 => ⟨0,  0,  0,  0, 2⟩
  | .p03 => ⟨0,  0,  0,  1, 0⟩
  | .p04 => ⟨0,  0,  0,  1, 1⟩
  | .p05 => ⟨0,  0,  1,  0, 0⟩
  | .p06 => ⟨0,  1, -1,  0, 0⟩
  | .p07 => ⟨1,  0,  0,  0, 2⟩
  | .p08 => ⟨1,  0,  0,  1, 1⟩
  | .p09 => ⟨1,  1, -1,  0, 0⟩
  | .p10 => ⟨1,  1, -1,  0, 2⟩
  | .p11 => ⟨1,  1,  0, -1, 0⟩
  | .p12 => ⟨2,  0,  1, -1, 0⟩
  | .p13 => ⟨2,  0,  1, -1, 1⟩
  | .p14 => ⟨2,  0,  1,  0, 0⟩
  | .p15 => ⟨2,  1,  0, -1, 0⟩

inductive DContKind
  | d00 | d01 | d02 | d03 | d04 | d05 | d06 | d07 | d08
  deriving DecidableEq, Fintype

theorem dContKind_card : Fintype.card DContKind = 9 := by
  native_decide

def dContNode : DContKind → ClassIINode
  | .d00 => ⟨0, 0,  0,  1, 0⟩
  | .d01 => ⟨0, 0,  1,  0, 0⟩
  | .d02 => ⟨0, 0,  0,  0, 1⟩
  | .d03 => ⟨0, 0,  0,  0, 2⟩
  | .d04 => ⟨1, 1, -1,  0, 0⟩
  | .d05 => ⟨1, 0,  0,  1, 1⟩
  | .d06 => ⟨1, 0,  0,  0, 2⟩
  | .d07 => ⟨2, 1,  0, -1, 0⟩
  | .d08 => ⟨2, 0,  1, -1, 1⟩

theorem dContNode_in_preContact (kind : DContKind) :
    ∃ pre : PreContactKind,
      dContNode kind = preContactNode pre := by
  cases kind <;> native_decide

inductive FirstBackwardKind
  | b00 | b01 | b02 | b03 | b04 | b05
  deriving DecidableEq, Fintype

def firstBackwardNode : FirstBackwardKind → ClassIINode
  | .b00 => ⟨1,  1, -1,  0, 2⟩
  | .b01 => ⟨2,  0,  1,  0, 0⟩
  | .b02 => ⟨0,  0,  0,  1, 1⟩
  | .b03 => ⟨2,  0,  1, -1, 0⟩
  | .b04 => ⟨0,  1, -1,  0, 0⟩
  | .b05 => ⟨0, -1,  1,  1, 1⟩

def secondBackwardNode : ClassIINode :=
  ⟨1, 1, 0, -1, 0⟩

/-- Finite layer shape of the candidate backward closure:
9 seeds, then 6 states, then 1 state. -/
theorem preContactNode_backward_layer_partition
    (kind : PreContactKind) :
    (∃ seed : DContKind,
      preContactNode kind = dContNode seed) ∨
    (∃ first : FirstBackwardKind,
      preContactNode kind = firstBackwardNode first) ∨
    preContactNode kind = secondBackwardNode := by
  cases kind <;> native_decide

inductive PreContactBackwardWitnessKind
  | w00 | w01 | w02 | w03 | w04 | w05 | w06
  deriving DecidableEq, Fintype

def preContactBackwardPredecessor :
    PreContactBackwardWitnessKind → ClassIINode
  | .w00 => ⟨0, -1,  1,  1, 1⟩
  | .w01 => ⟨0,  0,  0,  1, 1⟩
  | .w02 => ⟨0,  1, -1,  0, 0⟩
  | .w03 => ⟨1,  1, -1,  0, 2⟩
  | .w04 => ⟨2,  0,  1, -1, 0⟩
  | .w05 => ⟨2,  0,  1,  0, 0⟩
  | .w06 => ⟨1,  1,  0, -1, 0⟩

def preContactBackwardDestination :
    PreContactBackwardWitnessKind → ClassIINode
  | .w00 => ⟨2, 1, 0, -1, 0⟩
  | .w01 => ⟨0, 0, 1,  0, 0⟩
  | .w02 => ⟨1, 1,-1,  0, 0⟩
  | .w03 => ⟨0, 0, 0,  1, 0⟩
  | .w04 => ⟨0, 0, 1,  0, 0⟩
  | .w05 => ⟨0, 0, 0,  1, 0⟩
  | .w06 => ⟨2, 0, 1, -1, 0⟩

def preContactBackwardTypeOne :
    PreContactBackwardWitnessKind → Bool
  | .w00 | .w01 | .w03 => true
  | _ => false

def preContactBackwardPrefixes :
    PreContactBackwardWitnessKind → Int →
      (Int × Int × Int) × (Int × Int × Int)
  | .w00, a => ((0, 0, 0), (a, 0, 0))
  | .w01, a => ((0, 0, 0), (a, 0, 0))
  | .w02, _ => ((1, 0, 0), (0, 0, 0))
  | .w03, a => ((a, 0, 0), (a, 1, 0))
  | .w04, a => ((a, 1, 0), (0, 0, 0))
  | .w05, a => ((a, 1, 0), (a - 1, 0, 0))
  | .w06, a => ((a, 0, 0), (0, 0, 0))

def classIIIncidenceMul
    (a : Int) (node : ClassIINode) : Int × Int × Int :=
  (a * node.x0 + a * node.x1 + node.x2,
   node.x0, node.x0 + node.x1)

def signedBackwardDisplacement
    (typeOne : Bool)
    (mx lp lq : Int × Int × Int) : Int × Int × Int :=
  let sign : Int := if typeOne then 1 else -1
  (sign * mx.1 + lp.1 - lq.1,
   sign * mx.2.1 + lp.2.1 - lq.2.1,
   sign * mx.2.2 + lp.2.2 - lq.2.2)

def nodeDisplacement (node : ClassIINode) : Int × Int × Int :=
  (node.x0, node.x1, node.x2)

/-- The seven `9+6+1` backward-occurrence witnesses satisfy the
Class-II incidence/prefix lattice equation for every integer
parameter. Prefix-cut legality and letter colors are recorded by the
C++ witness table; this is the parameter-dependent coordinate part. -/
theorem preContactBackwardWitness_lattice_identity
    (kind : PreContactBackwardWitnessKind) (a : Int) :
    signedBackwardDisplacement
        (preContactBackwardTypeOne kind)
        (classIIIncidenceMul a
          (preContactBackwardDestination kind))
        (preContactBackwardPrefixes kind a).1
        (preContactBackwardPrefixes kind a).2
      =
    nodeDisplacement (preContactBackwardPredecessor kind) := by
  cases kind <;>
    (simp [signedBackwardDisplacement, classIIIncidenceMul,
      preContactBackwardDestination, preContactBackwardPredecessor,
      preContactBackwardTypeOne, preContactBackwardPrefixes,
      nodeDisplacement] <;> ring)

inductive DContFaceCandidateKind
  | f00 | f01 | f02 | f03 | f04 | f05 | f06 | f07 | f08 | f09
  | f10 | f11 | f12 | f13 | f14 | f15 | f16 | f17 | f18 | f19
  | f20 | f21 | f22 | f23 | f24 | f25 | f26 | f27 | f28 | f29
  | f30 | f31 | f32
  deriving DecidableEq, Fintype

theorem dContFaceCandidateKind_card :
    Fintype.card DContFaceCandidateKind = 33 := by
  native_decide

/-- The parameter-independent face-intersection candidates in
`[-2,2]^3`, before applying the Perron-window inequality. -/
def dContFaceCandidateNode : DContFaceCandidateKind → ClassIINode
  | .f00 => ⟨0,  0, -1,  0, 0⟩
  | .f01 => ⟨0,  0,  0, -1, 0⟩
  | .f02 => ⟨0,  0,  0,  1, 0⟩
  | .f03 => ⟨0,  0,  1,  0, 0⟩
  | .f04 => ⟨0, -1,  0,  0, 1⟩
  | .f05 => ⟨0, -1,  1,  0, 1⟩
  | .f06 => ⟨0,  0,  0,  0, 1⟩
  | .f07 => ⟨0,  0,  1,  0, 1⟩
  | .f08 => ⟨0, -1,  0,  0, 2⟩
  | .f09 => ⟨0, -1,  0,  1, 2⟩
  | .f10 => ⟨0,  0,  0,  0, 2⟩
  | .f11 => ⟨0,  0,  0,  1, 2⟩
  | .f12 => ⟨1,  0, -1,  0, 0⟩
  | .f13 => ⟨1,  1, -1,  0, 0⟩
  | .f14 => ⟨1,  1,  0,  0, 0⟩
  | .f15 => ⟨1, -1,  0,  0, 1⟩
  | .f16 => ⟨1,  0,  0, -1, 1⟩
  | .f17 => ⟨1,  0,  0,  1, 1⟩
  | .f18 => ⟨1,  1,  0,  0, 1⟩
  | .f19 => ⟨1,  0, -1,  0, 2⟩
  | .f20 => ⟨1,  0, -1,  1, 2⟩
  | .f21 => ⟨1,  0,  0,  0, 2⟩
  | .f22 => ⟨1,  0,  0,  1, 2⟩
  | .f23 => ⟨2,  0,  0, -1, 0⟩
  | .f24 => ⟨2,  1,  0, -1, 0⟩
  | .f25 => ⟨2,  1,  0,  0, 0⟩
  | .f26 => ⟨2,  0,  0, -1, 1⟩
  | .f27 => ⟨2,  0,  1, -1, 1⟩
  | .f28 => ⟨2,  0,  1,  0, 1⟩
  | .f29 => ⟨2, -1,  0,  0, 2⟩
  | .f30 => ⟨2,  0, -1,  0, 2⟩
  | .f31 => ⟨2,  0,  1,  0, 2⟩
  | .f32 => ⟨2,  1,  0,  0, 2⟩

def dContFaceCandidateAccepted : DContFaceCandidateKind → Bool
  | .f02 | .f03 | .f06 | .f10 | .f13
  | .f17 | .f21 | .f24 | .f27 => true
  | _ => false

/-- The nine accepted face candidates are exactly the displayed
`D_cont` catalogue, as a finite shape statement. -/
theorem acceptedFaceCandidate_is_dCont
    (kind : DContFaceCandidateKind)
    (h : dContFaceCandidateAccepted kind = true) :
    ∃ seed : DContKind,
      dContFaceCandidateNode kind = dContNode seed := by
  cases kind <;>
    simp_all [dContFaceCandidateAccepted] <;>
    native_decide

inductive ContactRedExcludedKind
  | e00 | e01
  deriving DecidableEq, Fintype

def contactRedExcludedNode : ContactRedExcludedKind → ClassIINode
  | .e00 => ⟨1, 0, 0,  1, 1⟩
  | .e01 => ⟨2, 0, 1, -1, 1⟩

/-- The pre-contact catalogue is exactly the disjoint union of the
fourteen contact states and the two displayed Red exclusions. -/
theorem preContactNode_partition (kind : PreContactKind) :
    (∃ contact : ContactKind,
      preContactNode kind = contactNode contact) ∨
    (∃ excluded : ContactRedExcludedKind,
      preContactNode kind = contactRedExcludedNode excluded) := by
  cases kind <;> native_decide

theorem contactRedExcluded_disjoint
    (contact : ContactKind) (excluded : ContactRedExcludedKind) :
    contactNode contact ≠ contactRedExcludedNode excluded := by
  cases contact <;> cases excluded <;> native_decide

inductive ShellKind
  | n00 | n01 | n02 | n03 | n04 | n05 | n06 | n07 | n08 | n09
  | n10 | n11 | n12 | n13 | n14 | n15 | n16 | n17 | n18 | n19
  deriving DecidableEq, Fintype

theorem shellKind_card : Fintype.card ShellKind = 20 := by
  native_decide

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

/-- A predecessor choice for each interior-shell state. -/
def shellSourceKind : ShellKind → ShellKind
  | .n00 => .n00 | .n01 => .n00 | .n02 => .n00 | .n03 => .n00
  | .n04 => .n03 | .n05 => .n01 | .n06 => .n06 | .n07 => .n08
  | .n08 => .n08 | .n09 => .n09 | .n10 => .n11 | .n11 => .n11
  | .n12 => .n12 | .n13 => .n12 | .n14 => .n14 | .n15 => .n15
  | .n16 => .n16 | .n17 => .n17 | .n18 => .n18 | .n19 => .n19

/-- The fixed signed-contact hop paired with `shellSourceKind`.
Seven distinct signed-hop values occur in this twenty-entry table. -/
def shellHop : ShellKind → ClassIINode
  | .n00 => ⟨0, -1,  1,  0, 0⟩
  | .n01 => ⟨0, -1,  1,  0, 1⟩
  | .n02 => ⟨0,  0,  0, -1, 0⟩
  | .n03 => ⟨0,  0,  1,  0, 0⟩
  | .n04 => ⟨0, -1,  1,  0, 1⟩
  | .n05 => ⟨1,  1,  0, -1, 0⟩
  | .n06 => ⟨0,  1, -1,  0, 0⟩
  | .n07 => ⟨0,  0,  0,  1, 1⟩
  | .n08 => ⟨0,  1, -1,  0, 0⟩
  | .n09 => ⟨0,  1, -1,  0, 0⟩
  | .n10 => ⟨0,  0,  0,  1, 1⟩
  | .n11 => ⟨0,  1, -1,  0, 0⟩
  | .n12 => ⟨0, -1,  1,  0, 0⟩
  | .n13 => ⟨0,  0,  1,  0, 0⟩
  | .n14 => ⟨0,  1, -1,  0, 0⟩
  | .n15 => ⟨0,  1, -1,  0, 0⟩
  | .n16 => ⟨0, -1,  1,  0, 0⟩
  | .n17 => ⟨0, -1,  1,  0, 0⟩
  | .n18 => ⟨0,  1, -1,  0, 0⟩
  | .n19 => ⟨0,  1, -1,  0, 0⟩

def composeHop (source hop : ClassIINode) : ClassIINode :=
  ⟨source.left, source.x0 + hop.x0, source.x1 + hop.x1,
    source.x2 + hop.x2, hop.right⟩

/-- The color interfaces in the witness table really compose. -/
theorem shellHop_color_matches (kind : ShellKind) (q : Int) :
    (shellNode (shellSourceKind kind) q).right =
      (shellHop kind).left := by
  cases kind <;> rfl

/-- Every propagation hop is a member of the explicit signed contact
set: it is either one of the fourteen contact nodes or its mirror. -/
theorem shellHop_from_explicit_signed_contact (kind : ShellKind) :
    ∃ contact : ContactKind,
      shellHop kind = contactNode contact ∨
      shellHop kind = mirrorNode (contactNode contact) := by
  cases kind
  · exact ⟨.c06, Or.inr rfl⟩
  · exact ⟨.c08, Or.inr rfl⟩
  · exact ⟨.c03, Or.inr rfl⟩
  · exact ⟨.c05, Or.inl rfl⟩
  · exact ⟨.c08, Or.inr rfl⟩
  · exact ⟨.c10, Or.inl rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩
  · exact ⟨.c04, Or.inl rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩
  · exact ⟨.c04, Or.inl rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩
  · exact ⟨.c06, Or.inr rfl⟩
  · exact ⟨.c05, Or.inl rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩
  · exact ⟨.c06, Or.inr rfl⟩
  · exact ⟨.c06, Or.inr rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩
  · exact ⟨.c06, Or.inl rfl⟩

/-- Universal affine corona-candidate propagation: the displayed state
in round `q` is exactly its predecessor in round `q-1` plus its fixed
contact hop.  No finite parameter sweep occurs in this proof. -/
theorem shellNode_propagates (kind : ShellKind) (q : Int) :
    composeHop (shellNode (shellSourceKind kind) (q - 1))
      (shellHop kind) = shellNode kind q := by
  cases kind <;>
    simp [composeHop, shellNode, shellSourceKind, shellHop,
      ClassIINode.mk.injEq] <;> omega

def nodeHeight (node : ClassIINode) (b c : ℝ) : ℝ :=
  node.x0 * b + node.x1 * c + node.x2

def rightWidth (node : ClassIINode) (b c : ℝ) : ℝ :=
  if node.right = 0 then b else if node.right = 1 then c else 1

def InRestrictedH (node : ClassIINode) (b c : ℝ) : Prop :=
  0 ≤ nodeHeight node b c ∧
    nodeHeight node b c < rightWidth node b c

/-- Every state in the fourteen-state contact catalogue lies in the
restricted stepped hyperplane.  The only estimate beyond
`1 < c < b` is the Class-II gap bound `b - c < 1`, used by the
translated face `(-1,1,1)`. -/
theorem contactNode_in_restrictedH
    (kind : ContactKind) (b c : ℝ)
    (hc : 1 < c) (hcb : c < b) (hgap : b - c < 1) :
    InRestrictedH (contactNode kind) b c := by
  cases kind <;>
    simp [InRestrictedH, nodeHeight, rightWidth, contactNode] <;>
    first
    | (constructor <;> linarith)
    | linarith

/-- The two states removed by contact Red are nevertheless valid
restricted states.  Their removal is genuinely a rank-one graph
effect, not a stepped-hyperplane window exclusion. -/
theorem contactRedExcludedNode_in_restrictedH
    (kind : ContactRedExcludedKind) (b c : ℝ)
    (hc : 1 < c) :
    InRestrictedH (contactRedExcludedNode kind) b c := by
  cases kind <;>
    simp [InRestrictedH, nodeHeight, rightWidth,
      contactRedExcludedNode] <;>
    linarith

/-- A raw contact-envelope category has `x1 ∈ [-2,2]` and
`x2 ∈ [-1,1]`.  Once the Perron gap exceeds one half, restricted
window membership forces its remaining integer coordinate into the
five slices `x0 ∈ [-2,2]`. -/
theorem restrictedH_forces_bounded_x0
    (node : ClassIINode) (b c : ℝ)
    (hc : 1 < c) (hcb : c < b) (hgap : 1 / 2 < b - c)
    (hx1lo : -2 ≤ node.x1) (hx1hi : node.x1 ≤ 2)
    (hx2lo : -1 ≤ node.x2) (hx2hi : node.x2 ≤ 1)
    (hright : node.right = 0 ∨ node.right = 1 ∨ node.right = 2)
    (hwindow : InRestrictedH node b c) :
    -2 ≤ node.x0 ∧ node.x0 ≤ 2 := by
  have hb : 0 < b := by linarith
  constructor
  · by_contra hnot
    have hx0 : node.x0 ≤ -3 := by omega
    have hx0r : (node.x0 : ℝ) ≤ -3 := by exact_mod_cast hx0
    have hx1r : (node.x1 : ℝ) ≤ 2 := by exact_mod_cast hx1hi
    have hx2r : (node.x2 : ℝ) ≤ 1 := by exact_mod_cast hx2hi
    have h0 :
        (node.x0 : ℝ) * b ≤ -3 * b :=
      mul_le_mul_of_nonneg_right hx0r hb.le
    have h1 :
        (node.x1 : ℝ) * c ≤ 2 * c :=
      mul_le_mul_of_nonneg_right hx1r (by linarith)
    have hheight : nodeHeight node b c < 0 := by
      simp [nodeHeight]
      linarith
    linarith [hwindow.1]
  · by_contra hnot
    have hx0 : 3 ≤ node.x0 := by omega
    have hx0r : (3 : ℝ) ≤ node.x0 := by exact_mod_cast hx0
    have hx1r : (-2 : ℝ) ≤ node.x1 := by exact_mod_cast hx1lo
    have hx2r : (-1 : ℝ) ≤ node.x2 := by exact_mod_cast hx2lo
    have h0 :
        3 * b ≤ (node.x0 : ℝ) * b :=
      mul_le_mul_of_nonneg_right hx0r hb.le
    have h1 :
        -2 * c ≤ (node.x1 : ℝ) * c :=
      mul_le_mul_of_nonneg_right hx1r (by linarith)
    have hheight : b < nodeHeight node b c := by
      simp [nodeHeight]
      linarith
    rcases hright with hr | hr | hr <;>
      simp [InRestrictedH, rightWidth, hr] at hwindow <;>
      linarith

/-- Once `1<c<b`, the thirty-three integer face-contact candidates
have a parameter-independent window classification: exactly the nine
flagged candidates lie in the restricted stepped hyperplane. -/
theorem dContFaceCandidate_window_iff
    (kind : DContFaceCandidateKind) (b c : ℝ)
    (hc : 1 < c) (hcb : c < b) :
    InRestrictedH (dContFaceCandidateNode kind) b c ↔
      dContFaceCandidateAccepted kind = true := by
  cases kind <;>
    simp [InRestrictedH, rightWidth, nodeHeight,
      dContFaceCandidateNode, dContFaceCandidateAccepted] <;>
    first
    | linarith
    | (constructor <;> linarith)
    | (intro h; linarith)

def InOpenSignedStrip (node : ClassIINode) (b c : ℝ) : Prop :=
  -rightWidth node b c < nodeHeight node b c ∧
    nodeHeight node b c < rightWidth node b c

/-- All twenty shell endpoints satisfy the signed
stepped-hyperplane inequality once two scalar gap estimates are known.
For the Class-II Perron coordinates these estimates are the remaining
family-specific algebraic lemmas. -/
theorem shellNode_in_open_signed_strip
    (kind : ShellKind) (q : Int) (b c : ℝ)
    (hq : 4 ≤ q)
    (hbc : c < b) (hc : 1 < c)
    (hspan : (q : ℝ) * (b - c) < c)
    (hedge : ((q : ℝ) - 1) * (b - c) + 1 < c) :
    InOpenSignedStrip (shellNode kind q) b c := by
  have hqReal : (4 : ℝ) ≤ (q : ℝ) := by exact_mod_cast hq
  cases kind <;>
    simp [InOpenSignedStrip, rightWidth, nodeHeight, shellNode] <;>
    constructor <;> ring_nf at * <;> nlinarith

/-- The two scalar acceptance-window bounds follow from scaled forms
of the Class-II Perron relation.  The hypotheses `hd` and `hcscale`
are denominator-free versions of
`b-c = (a*b+1)/b^2` and `c = a+1/b`. -/
theorem class_ii_window_bounds
    (a q b c : ℝ)
    (ha : 2 ≤ a) (hq : q ≤ a - 1)
    (hb : 0 < b) (hba : a < b)
    (hd : b^2 * (b - c) = a * b + 1)
    (hcscale : b^2 * c = a * b^2 + b) :
    q * (b - c) < c ∧ (q - 1) * (b - c) + 1 < c := by
  have hb2 : 0 < b^2 := sq_pos_of_pos hb
  have hab : 0 < a * b + 1 := by positivity
  have hqmul :
      q * (a * b + 1) ≤ (a - 1) * (a * b + 1) :=
    mul_le_mul_of_nonneg_right hq hab.le
  have hfactor : 0 < (a * b + 1) * (b - a + 1) :=
    mul_pos hab (by nlinarith)
  have hbase :
      (a - 1) * (a * b + 1) < a * b^2 + b := by
    nlinarith [hfactor]
  have hscaled :
      b^2 * (q * (b - c)) < b^2 * c := by
    calc
      b^2 * (q * (b - c)) = q * (a * b + 1) := by
        rw [← hd]
        ring
      _ ≤ (a - 1) * (a * b + 1) := hqmul
      _ < a * b^2 + b := hbase
      _ = b^2 * c := hcscale.symm
  constructor
  · exact lt_of_mul_lt_mul_left
      (by simpa [mul_assoc] using hscaled) hb2.le
  · have hqedge : q - 1 ≤ a - 2 := by linarith
    have hqedgemul :
        (q - 1) * (a * b + 1) ≤ (a - 2) * (a * b + 1) :=
      mul_le_mul_of_nonneg_right hqedge hab.le
    have ha1 : 0 < a - 1 := by linarith
    have hmulba :
        (a - 1) * a < (a - 1) * b :=
      mul_lt_mul_of_pos_left hba ha1
    have hbracket :
        0 < (a - 1) * b - a * (a - 2) := by
      nlinarith
    have hpositive :
        0 < b * ((a - 1) * b - a * (a - 2)) :=
      mul_pos hb hbracket
    have hrest : 0 < b - a + 2 := by nlinarith
    have hedgebase :
        (a - 2) * (a * b + 1) + b^2 < a * b^2 + b := by
      nlinarith [hpositive, hrest]
    have hedgescaled :
        b^2 * ((q - 1) * (b - c) + 1) < b^2 * c := by
      calc
        b^2 * ((q - 1) * (b - c) + 1)
            = (q - 1) * (a * b + 1) + b^2 := by
                rw [← hd]
                ring
        _ ≤ (a - 2) * (a * b + 1) + b^2 := by
          linarith
        _ < a * b^2 + b := hedgebase
        _ = b^2 * c := hcscale.symm
    exact lt_of_mul_lt_mul_left
      (by simpa [mul_assoc] using hedgescaled) hb2.le

/-- Direct Perron-polynomial form of `class_ii_window_bounds`. -/
theorem class_ii_perron_window_bounds
    (a q beta : ℝ)
    (ha : 2 ≤ a) (hq : q ≤ a - 1)
    (hbeta : 0 < beta) (hbeta_a : a < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    q * (beta - (a + 1 / beta)) < a + 1 / beta ∧
      (q - 1) * (beta - (a + 1 / beta)) + 1 <
        a + 1 / beta := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hcscale :
      beta^2 * (a + 1 / beta) = a * beta^2 + beta := by
    field_simp [hne]
  exact class_ii_window_bounds
    a q beta (a + 1 / beta) ha hq hbeta hbeta_a hd hcscale

/-- The Class-II Perron gap between the first two covector
coordinates is strictly smaller than one. -/
theorem class_ii_perron_gap_lt_one
    (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    beta - (a + 1 / beta) < 1 := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hscaled :
      beta * (beta - a) = a + 1 + 1 / beta := by
    field_simp [hne]
    nlinarith [hcubic]
  have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
  have hunit : 1 < beta * (beta - a) := by
    rw [hscaled]
    linarith
  have hcompare : a * beta + 1 < beta^2 := by
    nlinarith
  have hb2 : 0 < beta^2 := sq_pos_of_pos hbeta
  have hmul :
      beta^2 * (beta - (a + 1 / beta)) < beta^2 * 1 := by
    rw [hd]
    simpa using hcompare
  exact lt_of_mul_lt_mul_left hmul hb2.le

/-- A coarse lower gap bound sufficient to collapse the contact
backward envelope to the five integer slices `-2 ≤ x0 ≤ 2`. -/
theorem class_ii_perron_gap_gt_half
    (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    1 / 2 < beta - (a + 1 / beta) := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  by_contra hnot
  have hgap_le :
      beta - (a + 1 / beta) ≤ 1 / 2 := le_of_not_gt hnot
  have hscaled :
      2 * (a * beta + 1) ≤ beta^2 := by
    have hmul := mul_le_mul_of_nonneg_left hgap_le (sq_nonneg beta)
    rw [hd] at hmul
    nlinarith
  have hbeta_two_a : 2 * a < beta := by
    by_contra hle
    have hbeta_le : beta ≤ 2 * a := le_of_not_gt hle
    have hproduct : 0 ≤ beta * (2 * a - beta) :=
      mul_nonneg hbeta.le (by linarith)
    nlinarith
  have heq :
      beta^2 * (beta - a) = (a + 1) * beta + 1 := by
    nlinarith [hcubic]
  have hleft : a * beta^2 < beta^2 * (beta - a) := by
    have hdelta : a < beta - a := by linarith
    simpa [mul_comm] using
      mul_lt_mul_of_pos_left hdelta (sq_pos_of_pos hbeta)
  have hfactor : 1 < a * beta - a - 1 := by
    nlinarith [mul_nonneg
      (show 0 ≤ a by linarith)
      (show 0 ≤ beta - 2 * a by linarith)]
  have hright : (a + 1) * beta + 1 < a * beta^2 := by
    have hproduct : beta < beta * (a * beta - a - 1) := by
      simpa using mul_lt_mul_of_pos_left hfactor hbeta
    nlinarith
  nlinarith

/-- The sharper gap bound used by the exact 255-case affine endpoint
classifier.  Unlike the earlier neighbor-endpoint lemma, this proof
works uniformly from the contact-base threshold `a=2`. -/
theorem class_ii_perron_gap_gt_two_thirds
    (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    2 / 3 < beta - (a + 1 / beta) := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  by_contra hnot
  have hgap_le :
      beta - (a + 1 / beta) ≤ 2 / 3 := le_of_not_gt hnot
  have hscaled :
      3 * (a * beta + 1) ≤ 2 * beta^2 := by
    have hmul := mul_le_mul_of_nonneg_left hgap_le (sq_nonneg beta)
    rw [hd] at hmul
    nlinarith
  have hbeta_three_halves : 3 * a < 2 * beta := by
    by_contra hle
    have hbeta_le : 2 * beta ≤ 3 * a := le_of_not_gt hle
    have hproduct : 0 ≤ beta * (3 * a - 2 * beta) :=
      mul_nonneg hbeta.le (by linarith)
    nlinarith
  have heq :
      beta^2 * (beta - a) = (a + 1) * beta + 1 := by
    nlinarith [hcubic]
  have hleft :
      (a / 2) * beta^2 < beta^2 * (beta - a) := by
    have hdelta : a / 2 < beta - a := by linarith
    simpa [mul_comm] using
      mul_lt_mul_of_pos_left hdelta (sq_pos_of_pos hbeta)
  have hscaled_a :
      a * (3 * (a * beta + 1)) ≤ a * (2 * beta^2) :=
    mul_le_mul_of_nonneg_left hscaled (by linarith)
  have hfactor :
      0 ≤ (3 * a + 2) * (a - 2) :=
    mul_nonneg (by linarith) (by linarith)
  have hright :
      (a + 1) * beta + 1 < (a / 2) * beta^2 := by
    nlinarith
  nlinarith

/-- Universal Class-II form of contact-catalogue validity. -/
theorem class_ii_contactNode_valid
    (kind : ContactKind) (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    InRestrictedH
      (contactNode kind) beta (a + 1 / beta) := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hprod :
      0 < beta^2 * (beta - (a + 1 / beta)) := by
    rw [hd]
    positivity
  have hbc : a + 1 / beta < beta := by
    have hgap : 0 < beta - (a + 1 / beta) :=
      pos_of_mul_pos_right hprod (sq_nonneg beta)
    linarith
  have hc : (1 : ℝ) < a + 1 / beta := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    linarith
  exact contactNode_in_restrictedH kind beta (a + 1 / beta)
    hc hbc (class_ii_perron_gap_lt_one a beta ha hbeta hcubic)

/-- Universal Class-II specialization of the five-slice reduction
for any node having the raw contact-envelope coordinate bounds. -/
theorem class_ii_rawContact_x0_bounded
    (node : ClassIINode) (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1)
    (hx1lo : -2 ≤ node.x1) (hx1hi : node.x1 ≤ 2)
    (hx2lo : -1 ≤ node.x2) (hx2hi : node.x2 ≤ 1)
    (hright : node.right = 0 ∨ node.right = 1 ∨ node.right = 2)
    (hwindow :
      InRestrictedH node beta (a + 1 / beta)) :
    -2 ≤ node.x0 ∧ node.x0 ≤ 2 := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hprod :
      0 < beta^2 * (beta - (a + 1 / beta)) := by
    rw [hd]
    positivity
  have hbc : a + 1 / beta < beta := by
    have hgap : 0 < beta - (a + 1 / beta) :=
      pos_of_mul_pos_right hprod (sq_nonneg beta)
    linarith
  have hc : (1 : ℝ) < a + 1 / beta := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    linarith
  exact restrictedH_forces_bounded_x0
    node beta (a + 1 / beta) hc hbc
    (class_ii_perron_gap_gt_half a beta ha hbeta hcubic)
    hx1lo hx1hi hx2lo hx2hi hright hwindow

/-- Universal Perron-window classification of the fixed `D_cont`
face-candidate table. The geometric face-intersection enumeration is
integer-only; this theorem closes its parameter-dependent half. -/
theorem class_ii_dCont_face_candidate_valid_iff
    (kind : DContFaceCandidateKind) (a beta : ℝ)
    (ha : 2 ≤ a) (hbeta : 0 < beta)
    (hcubic :
      beta^3 = a * beta^2 + (a + 1) * beta + 1) :
    InRestrictedH
        (dContFaceCandidateNode kind) beta (a + 1 / beta) ↔
      dContFaceCandidateAccepted kind = true := by
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - (a + 1 / beta)) = a * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hprod :
      0 < beta^2 * (beta - (a + 1 / beta)) := by
    rw [hd]
    positivity
  have hbc : a + 1 / beta < beta := by
    have hgap : 0 < beta - (a + 1 / beta) :=
      pos_of_mul_pos_right hprod (sq_nonneg beta)
    linarith
  have hc : (1 : ℝ) < a + 1 / beta := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    linarith
  exact dContFaceCandidate_window_iff
    kind beta (a + 1 / beta) hc hbc

/-- Universal interior-shell endpoint validity in the Class-II signed
stepped hyperplane, expressed in the project's integer parameters. -/
theorem class_ii_shell_endpoint_valid
    (kind : ShellKind) (a q : Int) (beta : ℝ)
    (ha : 2 ≤ a) (hq4 : 4 ≤ q) (hqa : q < a)
    (hbeta : 0 < beta) (hbeta_a : (a : ℝ) < beta)
    (hcubic :
      beta^3 = (a : ℝ) * beta^2 +
        ((a : ℝ) + 1) * beta + 1) :
    InOpenSignedStrip
      (shellNode kind q) beta ((a : ℝ) + 1 / beta) := by
  have haReal : (2 : ℝ) ≤ (a : ℝ) := by exact_mod_cast ha
  have hqReal : (q : ℝ) ≤ (a : ℝ) - 1 := by
    exact_mod_cast (show q ≤ a - 1 by omega)
  have bounds := class_ii_perron_window_bounds
    (a : ℝ) (q : ℝ) beta haReal hqReal hbeta hbeta_a hcubic
  have hne : beta ≠ 0 := hbeta.ne'
  have hd :
      beta^2 * (beta - ((a : ℝ) + 1 / beta)) =
        (a : ℝ) * beta + 1 := by
    field_simp [hne]
    nlinarith [hcubic]
  have hprod :
      0 < beta^2 * (beta - ((a : ℝ) + 1 / beta)) := by
    rw [hd]
    positivity
  have hbc : (a : ℝ) + 1 / beta < beta := by
    have hgap :
        0 < beta - ((a : ℝ) + 1 / beta) :=
      pos_of_mul_pos_right hprod (sq_nonneg beta)
    linarith
  have hc : (1 : ℝ) < (a : ℝ) + 1 / beta := by
    have hinv : 0 < 1 / beta := one_div_pos.mpr hbeta
    linarith
  exact shellNode_in_open_signed_strip
    kind q beta ((a : ℝ) + 1 / beta)
    hq4 hbc hc bounds.1 bounds.2

/-- No two entries in one interior shell coincide once `q ≥ 4`. -/
theorem shellNode_injective_at_round {q : Int} (hq : 4 ≤ q) :
    Function.Injective (fun kind => shellNode kind q) := by
  intro u v h
  cases u <;> cases v <;>
    simp [shellNode, ClassIINode.mk.injEq] at h ⊢ <;> omega

/-- Equality between states from two interior rounds forces the rounds
to agree.  Together with `shellNode_injective_at_round`, this says the
twenty-state shells are pairwise disjoint. -/
theorem shellNode_round_eq
    {q r : Int} (hq : 4 ≤ q) (hr : 4 ≤ r)
    {u v : ShellKind} (h : shellNode u q = shellNode v r) :
    q = r := by
  cases u <;> cases v <;>
    simp [shellNode, ClassIINode.mk.injEq] at h <;> omega

/-- The parameterized interior-shell formula itself contains
infinitely many distinct states.  To transfer this to `G_B(a)`, the
remaining theorem must prove shell inclusion in each finite boundary
graph for all admissible `a` and rounds. -/
theorem class_ii_affine_shell_range_infinite :
    Set.Infinite (Set.range (fun n : Nat =>
      shellNode .n00 (n + 4))) := by
  apply Set.infinite_range_of_injective
  intro n m h
  have hround : (n : Int) + 4 = (m : Int) + 4 :=
    shellNode_round_eq (by omega) (by omega) h
  omega
