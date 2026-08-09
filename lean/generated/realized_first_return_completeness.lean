import Mathlib

namespace RavelFirstReturnPlayground

universe u v

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

theorem eval_deriveWord_iff_path {State : Type u}
    (step : State → Int → State → Prop)
    (word : List Int) (terminal : ResidualFormula State)
    (source : State) :
    (deriveWord word terminal).Eval step source ↔
      ∃ target, Follows step source word target ∧ terminal.Eval step target := by
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
        obtain ⟨target, hpath, hterminal⟩ := (ih middle).mp hrest
        exact ⟨target, Follows.cons hstep hpath, hterminal⟩
      · rintro ⟨target, path, hterminal⟩
        cases path with
        | cons hstep htail =>
            exact ⟨_, hstep, (ih _).mpr ⟨target, htail, hterminal⟩⟩

structure StrictShellPumpWitness
    {Cycle : Type v}
    (Recurrent : Cycle → Prop)
    (radius : Cycle → ℕ)
    (source : Cycle) where
  lifted : Cycle
  lifted_recurrent : Recurrent lifted
  strict_outward : radius source < radius lifted

def StrictShellPump
    {Cycle : Type v}
    (Recurrent : Cycle → Prop)
    (radius : Cycle → ℕ) : Prop :=
  ∀ source, Recurrent source → 2 ≤ radius source →
    ∃ lifted, Recurrent lifted ∧ radius source < radius lifted

/-- The concrete first-return implementation must prove this premise: every
outer recurrent cycle is accepted by a residual formula derived from its
realized digit word and outward endpoint monitor. -/
def RealizedLanguageComplete
    {Cycle : Type v} {Controller : Type u}
    (Recurrent : Cycle → Prop) (radius : Cycle → ℕ)
    (step : Controller → Int → Controller → Prop) : Prop :=
  ∀ sourceCycle, Recurrent sourceCycle → 2 ≤ radius sourceCycle →
    ∃ (word : List Int) (sourceController : Controller)
      (terminal : Controller → Prop),
      (deriveWord word (.terminal terminal)).Eval step sourceController

/-- The concrete affine-transport implementation must prove this premise:
any accepted realized controller path constructs a replayable strict pump.
Stated as a genuine `Prop` (an existential exhibiting the lift), rather than
returning the `Type`-sorted `StrictShellPumpWitness` structure directly --
the latter does not elaborate at sort `Prop`. -/
def AcceptedPathPumps
    {Cycle : Type v} {Controller : Type u}
    (Recurrent : Cycle → Prop) (radius : Cycle → ℕ)
    (step : Controller → Int → Controller → Prop) : Prop :=
  ∀ (sourceCycle : Cycle), Recurrent sourceCycle → 2 ≤ radius sourceCycle →
    ∀ (word : List Int) (sourceController targetController : Controller)
      (terminal : Controller → Prop),
      Follows step sourceController word targetController →
      terminal targetController →
      ∃ lifted, Recurrent lifted ∧ radius sourceCycle < radius lifted

/-- This closes the realized-language seam.  It contains no finite-family or
quotient assumption: residual acceptance is converted to a concrete path, and
sound affine transport converts that path to a strict recurrent lift. -/
theorem strictShellPump_of_realized_language
    {Cycle : Type v} {Controller : Type u}
    (Recurrent : Cycle → Prop) (radius : Cycle → ℕ)
    (step : Controller → Int → Controller → Prop)
    (hcomplete : RealizedLanguageComplete Recurrent radius step)
    (hsound : AcceptedPathPumps Recurrent radius step) :
    StrictShellPump Recurrent radius := by
  intro sourceCycle hrec houter
  obtain ⟨word, sourceController, terminal, haccept⟩ :=
    hcomplete sourceCycle hrec houter
  obtain ⟨targetController, hpath, hterminal⟩ :=
    (eval_deriveWord_iff_path step word (.terminal terminal) sourceController).mp haccept
  exact hsound sourceCycle hrec houter word sourceController targetController
    terminal hpath hterminal

theorem iterate_strict_shell_lift
    {State : Type v}
    (Recurrent : State → Prop) (radius : State → ℕ)
    (hlift : StrictShellPump Recurrent radius) :
    ∀ k x, Recurrent x → 2 ≤ radius x →
      ∃ y, Recurrent y ∧ radius x + k ≤ radius y := by
  intro k
  induction k with
  | zero =>
      intro x hx _
      exact ⟨x, hx, by simp⟩
  | succ k ih =>
      intro x hx hxOuter
      obtain ⟨y, hy, hxy⟩ := ih x hx hxOuter
      have hyOuter : 2 ≤ radius y := by omega
      obtain ⟨z, hz, hyz⟩ := hlift y hy hyOuter
      exact ⟨z, hz, by omega⟩

/-- Kernel-checkable composition through the universal carry bound. -/
theorem recurrent_radius_le_one_of_realized_language
    {Cycle : Type v} {Controller : Type u}
    (Recurrent : Cycle → Prop) (radius : Cycle → ℕ)
    (step : Controller → Int → Controller → Prop)
    (bound : ℕ)
    (hbounded : ∀ x, Recurrent x → radius x ≤ bound)
    (hcomplete : RealizedLanguageComplete Recurrent radius step)
    (hsound : AcceptedPathPumps Recurrent radius step) :
    ∀ x, Recurrent x → radius x ≤ 1 := by
  have hpump : StrictShellPump Recurrent radius :=
    strictShellPump_of_realized_language Recurrent radius step hcomplete hsound
  intro x hx
  by_contra hnot
  have hxOuter : 2 ≤ radius x := by omega
  obtain ⟨y, hy, hlarge⟩ :=
    iterate_strict_shell_lift Recurrent radius hpump (bound + 1) x hx hxOuter
  have hyBound := hbounded y hy
  omega

end RavelFirstReturnPlayground
