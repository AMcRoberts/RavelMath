import Mathlib

namespace RavelCyclicSpliceCompletion

universe u v

/-- A chosen orbit in a serial finite relation. -/
noncomputable def chooseNext {α : Type u}
    (R : α → α → Prop) (hserial : ∀ x, ∃ y, R x y) (x : α) : α :=
  Classical.choose (hserial x)

lemma chooseNext_spec {α : Type u}
    (R : α → α → Prop) (hserial : ∀ x, ∃ y, R x y) (x : α) :
    R x (chooseNext R hserial x) :=
  Classical.choose_spec (hserial x)

noncomputable def orbit {α : Type u}
    (R : α → α → Prop) (hserial : ∀ x, ∃ y, R x y)
    (start : α) (n : ℕ) : α :=
  (chooseNext R hserial)^[n] start

lemma orbit_step {α : Type u}
    (R : α → α → Prop) (hserial : ∀ x, ∃ y, R x y)
    (start : α) (n : ℕ) :
    R (orbit R hserial start n) (orbit R hserial start (n + 1)) := by
  simpa [orbit, Function.iterate_succ_apply'] using
    chooseNext_spec R hserial (orbit R hserial start n)

/-- Finiteness converts a serial one-lap controller relation into a periodic
controller orbit.  A one-lap fixed point is not required: the base cycle may be
repeated a positive number of times. -/
theorem finite_serial_relation_has_repeated_orbit
    {α : Type u} [Finite α] [Nonempty α]
    (R : α → α → Prop) (hserial : ∀ x, ∃ y, R x y) :
    ∃ start : α, ∃ m n : ℕ,
      m < n ∧ orbit R hserial start m = orbit R hserial start n := by
  classical
  let start : α := Classical.choice inferInstance
  obtain ⟨m, n, hne, heq⟩ :=
    Finite.exists_ne_map_eq_of_infinite
      (fun k : ℕ => orbit R hserial start k)
  rcases lt_or_gt_of_ne hne with hlt | hgt
  · exact ⟨start, m, n, hlt, heq⟩
  · exact ⟨start, n, m, hgt, heq.symm⟩

/-- Logical data supplied by the concrete first-return/controller system for
one outer recurrent source.  `lap` is one complete traversal of the recurrent
base cycle on the invariant face-aligned controller fiber. -/
structure SerialLapSystem
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (source : State) where
  Controller : Type u
  finiteController : Finite Controller
  nonemptyController : Nonempty Controller
  lap : Controller → Controller → Prop
  serial : ∀ c, ∃ d, lap c d
  /-- Soundness of repeating the base cycle between equal orbit states. -/
  repeatedLapPumps :
    (∃ start : Controller, ∃ m n : ℕ,
      m < n ∧ orbit lap serial start m = orbit lap serial start n) →
    ∃ lifted : State, ReturnCapable lifted ∧ radius source < radius lifted

attribute [instance] SerialLapSystem.finiteController
attribute [instance] SerialLapSystem.nonemptyController

/-- The former `CyclicSpliceComplete` obligation follows from a serial finite
one-lap fiber plus the already replayed affine soundness of a repeated lap. -/
theorem strict_shell_pump_of_serial_lap
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (system : ∀ source, ReturnCapable source → 2 ≤ radius source →
      SerialLapSystem ReturnCapable radius source) :
    ∀ source, ReturnCapable source → 2 ≤ radius source →
      ∃ lifted, ReturnCapable lifted ∧ radius source < radius lifted := by
  intro source hrec houter
  let S := system source hrec houter
  apply S.repeatedLapPumps
  exact finite_serial_relation_has_repeated_orbit S.lap S.serial

/-- Final boundedness consequence after cyclic splice has been reduced to the
serial-lap construction. -/
theorem recurrent_radius_le_one_of_serial_lap
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (bound : ℕ)
    (hbounded : ∀ x, ReturnCapable x → radius x ≤ bound)
    (system : ∀ source, ReturnCapable source → 2 ≤ radius source →
      SerialLapSystem ReturnCapable radius source) :
    ∀ x, ReturnCapable x → radius x ≤ 1 := by
  have hpump := strict_shell_pump_of_serial_lap ReturnCapable radius system
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

end RavelCyclicSpliceCompletion
