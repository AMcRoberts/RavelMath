import Mathlib

namespace RavelGenerated

universe u

inductive ResidualFormula (State : Type u) where
  | terminal (accept : State → Prop)
  | pre (digit : Int) (child : ResidualFormula State)

def ResidualFormula.Eval {State : Type u}
    (step : State → Int → State → Prop) :
    ResidualFormula State → State → Prop
  | .terminal accept, state => accept state
  | .pre digit child, state =>
      ∃ target, step state digit target ∧ child.Eval step target

def deriveWord {State : Type u} :
    List Int → ResidualFormula State → ResidualFormula State
  | [], terminal => terminal
  | digit :: rest, terminal => .pre digit (deriveWord rest terminal)

inductive Follows {State : Type u}
    (step : State → Int → State → Prop) :
    State → List Int → State → Prop where
  | nil (state) : Follows step state [] state
  | cons {source middle target digit rest}
      (head : step source digit middle)
      (tail : Follows step middle rest target) :
      Follows step source (digit :: rest) target

/-- Predecessor is existential inverse image for every state type,
transition relation, digit, formula, and state. -/
theorem eval_pre_iff {State : Type u}
    (step : State → Int → State → Prop)
    (digit : Int) (child : ResidualFormula State) (state : State) :
    (ResidualFormula.pre digit child).Eval step state ↔
      ∃ target, step state digit target ∧ child.Eval step target := by
  rfl

/-- A residual derived along a word accepts exactly the starts of
legal paths carrying that word to the terminal formula. -/
theorem eval_deriveWord_iff_path {State : Type u}
    (step : State → Int → State → Prop)
    (word : List Int) (terminal : ResidualFormula State)
    (source : State) :
    (deriveWord word terminal).Eval step source ↔
      ∃ target, Follows step source word target ∧
        terminal.Eval step target := by
  induction word generalizing source with
  | nil =>
      constructor
      · intro h
        exact ⟨source, Follows.nil source, h⟩
      · rintro ⟨target, path, hterminal⟩
        cases path
        exact hterminal
  | cons digit rest ih =>
      constructor
      · rintro ⟨middle, hstep, hrest⟩
        obtain ⟨target, hpath, hterminal⟩ :=
          (ih middle).mp hrest
        exact ⟨target, Follows.cons hstep hpath, hterminal⟩
      · rintro ⟨target, path, hterminal⟩
        cases path with
        | cons hstep htail =>
            exact ⟨_, hstep, (ih _).mpr ⟨target, htail, hterminal⟩⟩

end RavelGenerated
