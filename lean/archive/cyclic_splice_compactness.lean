import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem recurrent_radius_le_one_of_pump
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (bound : ℕ)
    (hbounded : ∀ x, ReturnCapable x → radius x ≤ bound)
    (hpump : ∀ source, ReturnCapable source → 2 ≤ radius source →
      ∃ lifted, ReturnCapable lifted ∧ radius source < radius lifted) :
    ∀ x, ReturnCapable x → radius x ≤ 1 := by
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

universe u v

structure ClosedOrbit {Controller : Type u}
    (lap : Controller → Controller → Prop) where
  states : List Controller
  nonempty : states ≠ []
  positive : 1 < states.length
  closes : states.head? = states.getLast?
  steps : ∀ i : Fin (states.length - 1),
    lap (states.get ⟨i, by omega⟩)
        (states.get ⟨i + 1, by omega⟩)

structure CyclicLapSystem
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (source : State) where
  Controller : Type u
  lap : Controller → Controller → Prop
  orbit : ClosedOrbit lap
  repeatedOrbitPumps :
    ClosedOrbit lap →
      ∃ lifted : State,
        ReturnCapable lifted ∧ radius source < radius lifted

theorem strict_shell_pump_of_cyclic_lap
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (system : ∀ source, ReturnCapable source → 2 ≤ radius source →
      CyclicLapSystem ReturnCapable radius source) :
    ∀ source, ReturnCapable source → 2 ≤ radius source →
      ∃ lifted, ReturnCapable lifted ∧ radius source < radius lifted := by
  intro source hrec houter
  let S := system source hrec houter
  exact S.repeatedOrbitPumps S.orbit

theorem recurrent_radius_le_one_of_cyclic_lap
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (bound : ℕ)
    (hbounded : ∀ x, ReturnCapable x → radius x ≤ bound)
    (system : ∀ source, ReturnCapable source → 2 ≤ radius source →
      CyclicLapSystem ReturnCapable radius source) :
    ∀ x, ReturnCapable x → radius x ≤ 1 :=
  recurrent_radius_le_one_of_pump ReturnCapable radius bound hbounded
    (strict_shell_pump_of_cyclic_lap ReturnCapable radius system)

/-- Mechanically emitted: state_count=5 orbit_length=3 5-state relation with a length-2 cycle at {3,4} -- ravel::proof::stage_cyclic_splice_compactness
    independently replayed every consecutive orbit step. -/
def cyclicSpliceRel0Succ : Fin 5 → List (Fin 5)
  | 0 => [1]
  | 1 => [2]
  | 2 => []
  | 3 => [4]
  | 4 => [3]

def cyclicSpliceRel0Lap (a b : Fin 5) : Prop := b ∈ cyclicSpliceRel0Succ a

instance : DecidableRel cyclicSpliceRel0Lap := fun a b => by unfold cyclicSpliceRel0Lap; infer_instance

theorem cyclic_splice_compactness_instance_0 :
    ∃ orbit : ClosedOrbit cyclicSpliceRel0Lap, orbit.states = [3, 4, 3] := by
  refine ⟨⟨[3, 4, 3], by decide, by decide, by decide, ?_⟩, rfl⟩
  intro i
  fin_cases i <;> decide

/- Semantic proof graph for: cyclic_splice_compactness_batch
  [0] lean.cyclic_splice_compactness_certificate :: state_count=5 orbit_length=3 5-state relation with a length-2 cycle at {3,4} -- instantiates ClosedOrbit
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
