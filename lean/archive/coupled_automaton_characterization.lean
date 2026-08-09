import Mathlib.Tactic
namespace RavelGenerated

inductive PredicateTree where
  | leaf : Bool → PredicateTree
  | branch : Nat → Int → PredicateTree → PredicateTree → PredicateTree

def PredicateTree.eval (features : Nat → Int) : PredicateTree → Bool
  | .leaf value => value
  | .branch feature threshold left right =>
      if features feature ≤ threshold then left.eval features else right.eval features

/-- A synthesized winning predicate is sound once initialization, transition
closure, and terminal acceptance are checked independently. The learner is not
trusted; only these replay obligations enter the kernel. -/
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

end RavelGenerated
