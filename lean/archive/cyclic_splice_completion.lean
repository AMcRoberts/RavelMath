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
  repeatedLapPumps :
    (∃ start : Controller, ∃ m n : ℕ,
      m < n ∧ orbit lap serial start m = orbit lap serial start n) →
    ∃ lifted : State, ReturnCapable lifted ∧ radius source < radius lifted

attribute [instance] SerialLapSystem.finiteController
attribute [instance] SerialLapSystem.nonemptyController

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

theorem recurrent_radius_le_one_of_serial_lap
    {State : Type v}
    (ReturnCapable : State → Prop)
    (radius : State → ℕ)
    (bound : ℕ)
    (hbounded : ∀ x, ReturnCapable x → radius x ≤ bound)
    (system : ∀ source, ReturnCapable source → 2 ≤ radius source →
      SerialLapSystem ReturnCapable radius source) :
    ∀ x, ReturnCapable x → radius x ≤ 1 :=
  recurrent_radius_le_one_of_pump ReturnCapable radius bound hbounded
    (strict_shell_pump_of_serial_lap ReturnCapable radius system)

/-- Mechanically emitted: state_count=3 transient=1 period=2 3-state relation, transient 1 into a period-2 cycle -- ravel::proof::stage_cyclic_splice_completion
    independently reconfirmed the repeat under the deterministic
    "pick the first successor" transition function. -/
def cyclicSpliceCompletion0Next : Fin 3 → Fin 3
  | 0 => 1
  | 1 => 2
  | 2 => 1

theorem cyclic_splice_completion_instance_0 :
    cyclicSpliceCompletion0Next^[1] (0 : Fin 3) = cyclicSpliceCompletion0Next^[3] (0 : Fin 3) := by decide

/- Semantic proof graph for: cyclic_splice_completion_batch
  [0] lean.cyclic_splice_completion_certificate :: state_count=3 transient=1 period=2 3-state relation, transient 1 into a period-2 cycle -- instantiates finite_serial_relation_has_repeated_orbit
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
