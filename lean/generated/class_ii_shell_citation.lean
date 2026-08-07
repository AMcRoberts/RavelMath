import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

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
    `ravel::class_ii_interior_shell`, with an integer round parameter. -/
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

/-- Seven distinct signed-hop values occur in this twenty-entry table. -/
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

/-- Universal affine corona-candidate propagation: the displayed state in
    round `q` is exactly its predecessor in round `q-1` plus its fixed contact
    hop -- no finite parameter sweep. Reproduced from the independently
    kernel-checked `lean/class_ii_affine_shells.lean` (not re-derived here). -/
theorem shellNode_propagates (kind : ShellKind) (q : Int) :
    composeHop (shellNode (shellSourceKind kind) (q - 1))
      (shellHop kind) = shellNode kind q := by
  cases kind <;>
    simp [composeHop, shellNode, shellSourceKind, shellHop,
      ClassIINode.mk.injEq] <;> omega

/-- No two entries in one interior shell coincide once `q >= 4`. -/
theorem shellNode_injective_at_round {q : Int} (hq : 4 ≤ q) :
    Function.Injective (fun kind => shellNode kind q) := by
  intro u v h
  cases u <;> cases v <;>
    simp [shellNode, ClassIINode.mk.injEq] at h ⊢ <;> omega

/- Semantic proof graph for: class_ii_shell_batch
  [0] lean.lemma_application :: shellNode_propagates proves the interior-shell 20-state table at round 7 is exactly RavelGenerated.shellNode, pairwise distinct within the round (shellNode_injective_at_round) and equal to the previous round's state plus a fixed contact hop (shellNode_propagates), for any round -- not a per-round coincidence
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
