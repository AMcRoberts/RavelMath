import Mathlib

namespace RavelCyclicSpliceCompactness

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
    ∀ x, ReturnCapable x → radius x ≤ 1 := by
  have hpump := strict_shell_pump_of_cyclic_lap ReturnCapable radius system
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

end RavelCyclicSpliceCompactness
