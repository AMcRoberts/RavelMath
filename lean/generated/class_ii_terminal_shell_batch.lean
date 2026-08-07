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

/-- Mechanically emitted: instantiates the general lemma above at the
    concrete round interior-shell round q=7. -/
theorem class_ii_shell_round_instance_0 :
    ∀ kind : ShellKind, composeHop (shellNode (shellSourceKind kind) (6 : Int)) (shellHop kind) = shellNode kind (7 : Int) := by
  decide

/-- Mechanically emitted: injectivity at the same concrete round. -/
theorem class_ii_shell_round_injective_instance_1 :
    Function.Injective (fun kind : ShellKind => shellNode kind (7 : Int)) := by
  decide

/-- Mechanically emitted: instantiates the general lemma above at the
    concrete round interior-shell round q=12. -/
theorem class_ii_shell_round_instance_2 :
    ∀ kind : ShellKind, composeHop (shellNode (shellSourceKind kind) (11 : Int)) (shellHop kind) = shellNode kind (12 : Int) := by
  decide

/-- Mechanically emitted: injectivity at the same concrete round. -/
theorem class_ii_shell_round_injective_instance_3 :
    Function.Injective (fun kind : ShellKind => shellNode kind (12 : Int)) := by
  decide

structure TermNodeG where
  left : Int
  x0 : Int
  x1 : Int
  x2 : Int
  right : Int
  deriving DecidableEq

def terminalCrossColour1G (a : Int) : TermNodeG := ⟨0, a - 1, -a, 1, 2⟩
def terminalCrossColour2G (a : Int) : TermNodeG := ⟨2, -(a - 1), a - 1, -1, 1⟩
def interiorExtreme00G (a : Int) : TermNodeG := ⟨0, -a, a, 0, 0⟩
def interiorExtreme11G (a : Int) : TermNodeG := ⟨0, a, -a, 0, 0⟩

/-- For every integer a, none of the four (cross-colour, interior-extreme)
    coincidences hold -- the terminal edit never double-counts. Reproduced
    from the independently kernel-checked
    `lean/class_ii_terminal_shells.lean` (not re-derived here). -/
theorem terminalCrossColours_not_eq_interior_extremesG (a : Int) :
    terminalCrossColour1G a ≠ interiorExtreme00G a ∧
      terminalCrossColour1G a ≠ interiorExtreme11G a ∧
      terminalCrossColour2G a ≠ interiorExtreme00G a ∧
      terminalCrossColour2G a ≠ interiorExtreme11G a := by
  refine ⟨?_, ?_, ?_, ?_⟩ <;> intro h <;>
    simp [terminalCrossColour1G, terminalCrossColour2G, interiorExtreme00G,
      interiorExtreme11G, TermNodeG.mk.injEq] at h

/-- Mechanically emitted: instantiates the general lemma above at a=7. -/
theorem class_ii_terminal_shell_instance_0 :
    terminalCrossColour1G (7 : Int) ≠ interiorExtreme00G (7 : Int) ∧
      terminalCrossColour1G (7 : Int) ≠ interiorExtreme11G (7 : Int) ∧
      terminalCrossColour2G (7 : Int) ≠ interiorExtreme00G (7 : Int) ∧
      terminalCrossColour2G (7 : Int) ≠ interiorExtreme11G (7 : Int) :=
  terminalCrossColours_not_eq_interior_extremesG (7 : Int)

/-- Mechanically emitted: instantiates the general lemma above at a=12. -/
theorem class_ii_terminal_shell_instance_1 :
    terminalCrossColour1G (12 : Int) ≠ interiorExtreme00G (12 : Int) ∧
      terminalCrossColour1G (12 : Int) ≠ interiorExtreme11G (12 : Int) ∧
      terminalCrossColour2G (12 : Int) ≠ interiorExtreme00G (12 : Int) ∧
      terminalCrossColour2G (12 : Int) ≠ interiorExtreme11G (12 : Int) :=
  terminalCrossColours_not_eq_interior_extremesG (12 : Int)

/- Semantic proof graph for: class_ii_terminal_shell_batch
  [0] lean.class_ii_shell_round_certificate :: q=7 interior-shell round q=7 -- instantiates shellNode_propagates/shellNode_injective_at_round
  [1] lean.class_ii_terminal_shell_certificate :: a=7 -- instantiates terminalCrossColours_not_eq_interior_extremes
  [2] lean.class_ii_shell_round_certificate :: q=12 interior-shell round q=12 -- instantiates shellNode_propagates/shellNode_injective_at_round
  [3] lean.class_ii_terminal_shell_certificate :: a=12 -- instantiates terminalCrossColours_not_eq_interior_extremes
-/

def reflectedNodeCount : Nat := 4

end RavelGenerated
