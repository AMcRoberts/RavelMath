import Mathlib

namespace RavelFirstReturnObligations

universe u v

inductive ResidualFormula (Controller : Type u) where
  | terminal (accept : Controller → Prop)
  | pre (digit : Int) (child : ResidualFormula Controller)

def ResidualFormula.Eval {Controller : Type u}
    (step : Controller → Int → Controller → Prop) :
    ResidualFormula Controller → Controller → Prop
  | .terminal accept, state => accept state
  | .pre digit child, state =>
      ∃ target, step state digit target ∧ child.Eval step target

def deriveWord {Controller : Type u} :
    List Int → ResidualFormula Controller → ResidualFormula Controller
  | [], terminal => terminal
  | digit :: rest, terminal => .pre digit (deriveWord rest terminal)

inductive Follows {Controller : Type u}
    (step : Controller → Int → Controller → Prop) :
    Controller → List Int → Controller → Prop where
  | nil (state) : Follows step state [] state
  | cons {source middle target digit rest}
      (head : step source digit middle)
      (tail : Follows step middle rest target) :
      Follows step source (digit :: rest) target

theorem eval_deriveWord_iff_path {Controller : Type u}
    (step : Controller → Int → Controller → Prop)
    (word : List Int) (terminal : Controller → Prop)
    (source : Controller) :
    (deriveWord word (.terminal terminal)).Eval step source ↔
      ∃ target, Follows step source word target ∧ terminal target := by
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

/-- Exact logical payload of one replayed first-return segment.  The C++
operation now verifies all fields represented here for every stored n=3,4
witness: controller replay, endpoint monitor acceptance, affine recurrence,
admissible adjusted digits, and strict outward first-return transport. -/
structure ReplayedFirstReturnSegment
    {State : Type v} {Controller : Type u}
    (controllerStep : Controller → Int → Controller → Prop)
    (firstReturn : State → State → Prop)
    (radius : State → ℕ)
    (source : State) where
  target : State
  word : List Int
  sourceController : Controller
  targetController : Controller
  terminal : Controller → Prop
  controllerPath :
    Follows controllerStep sourceController word targetController
  terminalAccepted : terminal targetController
  liftedSource : State
  liftedTarget : State
  transportedFirstReturn : firstReturn liftedSource liftedTarget
  strictOutward : radius source < radius liftedSource

/-- Residual acceptance follows from the replayed controller path.  This is the
concrete logical form of the former `RealizedLanguageComplete` obligation. -/
theorem realized_language_complete_of_replayed_segment
    {State : Type v} {Controller : Type u}
    (controllerStep : Controller → Int → Controller → Prop)
    (firstReturn : State → State → Prop)
    (radius : State → ℕ)
    {source : State}
    (segment : ReplayedFirstReturnSegment controllerStep firstReturn radius source) :
    (deriveWord segment.word (.terminal segment.terminal)).Eval
      controllerStep segment.sourceController := by
  exact (eval_deriveWord_iff_path
    controllerStep segment.word segment.terminal
    segment.sourceController).2
      ⟨segment.targetController, segment.controllerPath,
        segment.terminalAccepted⟩

/-- An accepted path represented by a replayed segment already carries a
sound strict outward affine first-return transport.  This discharges the local
replay/transport content of the former `AcceptedPathPumps` obligation. -/
theorem accepted_path_affine_transport_of_replayed_segment
    {State : Type v} {Controller : Type u}
    (controllerStep : Controller → Int → Controller → Prop)
    (firstReturn : State → State → Prop)
    (radius : State → ℕ)
    {source : State}
    (segment : ReplayedFirstReturnSegment controllerStep firstReturn radius source) :
    ∃ liftedTarget,
      firstReturn segment.liftedSource liftedTarget ∧
      radius source < radius segment.liftedSource := by
  exact ⟨segment.liftedTarget, segment.transportedFirstReturn,
    segment.strictOutward⟩

/-- The single remaining system theorem: transported first-return segments
must splice around every recurrent first-return cycle so that the transported
source is itself return-capable.  This is strictly narrower than either of the
old opaque obligations. -/
def CyclicSpliceComplete
    {State : Type v} {Controller : Type u}
    (ReturnCapable : State → Prop)
    (controllerStep : Controller → Int → Controller → Prop)
    (firstReturn : State → State → Prop)
    (radius : State → ℕ) : Prop :=
  ∀ source, ReturnCapable source → 2 ≤ radius source →
    ∃ segment : ReplayedFirstReturnSegment
        controllerStep firstReturn radius source,
      ReturnCapable segment.liftedSource

/-- Once cyclic splice completeness is supplied, the two replay obligations
compose immediately to the strict shell pump required by the universal carry
bound. -/
theorem strict_shell_pump_of_cyclic_splice
    {State : Type v} {Controller : Type u}
    (ReturnCapable : State → Prop)
    (controllerStep : Controller → Int → Controller → Prop)
    (firstReturn : State → State → Prop)
    (radius : State → ℕ)
    (hsplice : CyclicSpliceComplete ReturnCapable controllerStep firstReturn radius) :
    ∀ source, ReturnCapable source → 2 ≤ radius source →
      ∃ lifted, ReturnCapable lifted ∧ radius source < radius lifted := by
  intro source hrec houter
  obtain ⟨segment, hlifted⟩ := hsplice source hrec houter
  exact ⟨segment.liftedSource, hlifted, segment.strictOutward⟩

/-- Final boundedness composition, ready for an external kernel pass. -/
theorem recurrent_radius_le_one_of_cyclic_splice
    {State : Type v} {Controller : Type u}
    (ReturnCapable : State → Prop)
    (controllerStep : Controller → Int → Controller → Prop)
    (firstReturn : State → State → Prop)
    (radius : State → ℕ)
    (bound : ℕ)
    (hbounded : ∀ x, ReturnCapable x → radius x ≤ bound)
    (hsplice : CyclicSpliceComplete ReturnCapable controllerStep firstReturn radius) :
    ∀ x, ReturnCapable x → radius x ≤ 1 := by
  have hpump := strict_shell_pump_of_cyclic_splice
    ReturnCapable controllerStep firstReturn radius hsplice
  intro x hx
  by_contra hnot
  have hxOuter : 2 ≤ radius x := by omega
  have iterate : ∀ k source, ReturnCapable source → 2 ≤ radius source →
      ∃ y, ReturnCapable y ∧ radius source + k ≤ radius y := by
    intro k
    induction k with
    | zero =>
        intro source hrec _
        exact ⟨source, hrec, by simp⟩
    | succ k ih =>
        intro source hrec houter
        obtain ⟨middle, hmiddle, hgrowth⟩ := ih source hrec houter
        have hmiddleOuter : 2 ≤ radius middle := by omega
        obtain ⟨target, htarget, hstrict⟩ := hpump middle hmiddle hmiddleOuter
        exact ⟨target, htarget, by omega⟩
  obtain ⟨y, hy, hlarge⟩ := iterate (bound + 1) x hx hxOuter
  have hyBound := hbounded y hy
  omega

end RavelFirstReturnObligations
