import Mathlib.Tactic

namespace RavelGenerated

/-- A continuation controller is the set of plant states from which the
remaining input can still reach the endpoint monitor. -/
abbrev ContinuationController (State : Type) := Set State

/-- Backward application of one plant input. -/
def controllerPre
    {State Input : Type}
    (Step : State → Input → State → Prop)
    (input : Input)
    (next : ContinuationController State) :
    ContinuationController State :=
  {state | ∃ successor, Step state input successor ∧ next successor}

theorem mem_controllerPre_iff
    {State Input : Type}
    (Step : State → Input → State → Prop)
    (input : Input)
    (next : ContinuationController State)
    (state : State) :
    state ∈ controllerPre Step input next ↔
      ∃ successor, Step state input successor ∧ successor ∈ next := by
  rfl

/-- Residual continuation controllers compose recursively along an input
word. Terminal endpoint monitors are the generators of the family. -/
def residualController
    {State Input : Type}
    (Step : State → Input → State → Prop)
    (word : List Input)
    (terminal : ContinuationController State) :
    ContinuationController State :=
  word.foldr (controllerPre Step) terminal

@[simp]
theorem residualController_nil
    {State Input : Type}
    (Step : State → Input → State → Prop)
    (terminal : ContinuationController State) :
    residualController Step [] terminal = terminal := by
  rfl

@[simp]
theorem residualController_cons
    {State Input : Type}
    (Step : State → Input → State → Prop)
    (input : Input)
    (word : List Input)
    (terminal : ContinuationController State) :
    residualController Step (input :: word) terminal =
      controllerPre Step input
        (residualController Step word terminal) := by
  rfl

/-- Membership in the residual controller yields an actual plant run ending
inside the terminal monitor. -/
theorem residualController_sound
    {State Input : Type}
    (Step : State → Input → State → Prop)
    (word : List Input)
    (terminal : ContinuationController State)
    (start : State)
    (hstart : start ∈ residualController Step word terminal) :
    ∃ finish, finish ∈ terminal := by
  induction word generalizing start with
  | nil =>
      exact ⟨start, hstart⟩
  | cons input word ih =>
      rcases hstart with ⟨successor, hstep, hsuccessor⟩
      exact ih successor hsuccessor

/-- A finite indexed family is a valid controller algebra when it contains
the terminal generators and is closed under every predecessor operation. -/
structure ControllerFamilyAlgebra
    (State Input Index : Type)
    (Step : State → Input → State → Prop) where
  member : Index → ContinuationController State
  preIndex : Input → Index → Index
  pre_closed :
    ∀ input index,
      member (preIndex input index) =
        controllerPre Step input (member index)

end RavelGenerated
