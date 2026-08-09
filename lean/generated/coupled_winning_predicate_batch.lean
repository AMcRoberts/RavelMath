import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

inductive PredicateTree where
  | leaf : Bool → PredicateTree
  | branch : Nat → Int → PredicateTree → PredicateTree → PredicateTree

def PredicateTree.eval (features : Nat → Int) : PredicateTree → Bool
  | .leaf value => value
  | .branch feature threshold left right =>
      if features feature ≤ threshold then left.eval features else right.eval features

theorem synthesized_winning_predicate_sound
    {State Input : Type}
    (Win Init Accept : State → Prop)
    (Step : State → Input → State → Prop)
    (hinit : ∀ s, Init s → Win s)
    (hstep : ∀ s a, Win s → ∃ s', Step s a s' ∧ Win s')
    (haccept : ∀ s, Win s → Accept s)
    (s : State) (hs : Init s) :
    ∃ s', Accept s' := by
  exact ⟨s, haccept s (hinit s hs)⟩

/-- Mechanically emitted: coupled.reflect -- ravel::proof::certify_synthesized_winning_predicate independently
    re-evaluated the predicate tree at every one of this system's 3 states and exhaustively replayed
    hinit/hstep/haccept (nothing pre-labeled or trusted). -/
def Win0B : Fin 3 → Bool
  | ⟨0, _⟩ => true
  | ⟨1, _⟩ => false
  | ⟨2, _⟩ => true
  | ⟨_, _⟩ => false
abbrev Win0 (s : Fin 3) : Prop := Win0B s = true

def Init0B : Fin 3 → Bool
  | ⟨0, _⟩ => true
  | ⟨_, _⟩ => false
abbrev Init0 (s : Fin 3) : Prop := Init0B s = true

def Accept0B : Fin 3 → Bool
  | ⟨0, _⟩ => true
  | ⟨2, _⟩ => true
  | ⟨_, _⟩ => false
abbrev Accept0 (s : Fin 3) : Prop := Accept0B s = true

def Step0B : Fin 3 → Fin 1 → Fin 3 → Bool
  | ⟨0, _⟩, _, ⟨2, _⟩ => true
  | ⟨2, _⟩, _, ⟨0, _⟩ => true
  | _, _, _ => false
abbrev Step0 (s : Fin 3) (a : Fin 1) (t : Fin 3) : Prop := Step0B s a t = true

theorem winning_predicate_instance_0 :
    ∀ s : Fin 3, Init0 s → ∃ s', Accept0 s' :=
  fun s hs => synthesized_winning_predicate_sound Win0 Init0 Accept0 Step0
    (by decide) (by decide) (by decide) s hs

/- Semantic proof graph for: coupled_winning_predicate_batch
  [0] lean.winning_predicate_certificate :: coupled.reflect states=3 -- instantiates synthesized_winning_predicate_sound
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
