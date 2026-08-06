import Mathlib.Tactic

namespace RavelGenerated

/-- Repeated strict shell lifting produces a state whose shell radius has
increased by any prescribed finite amount. -/
theorem iterate_strict_shell_lift
    {State : Type}
    (Recurrent : State → Prop)
    (radius : State → ℕ)
    (hlift :
      ∀ x, Recurrent x → 2 ≤ radius x →
        ∃ y, Recurrent y ∧ radius x < radius y) :
    ∀ k x, Recurrent x → 2 ≤ radius x →
      ∃ y, Recurrent y ∧ radius x + k ≤ radius y := by
  intro k
  induction k with
  | zero =>
      intro x hx hxOuter
      exact ⟨x, hx, by simp⟩
  | succ k ih =>
      intro x hx hxOuter
      obtain ⟨y, hy, hxy⟩ := ih x hx hxOuter
      have hyOuter : 2 ≤ radius y := by
        omega
      obtain ⟨z, hz, hyz⟩ := hlift y hy hyOuter
      refine ⟨z, hz, ?_⟩
      omega

/-- A bounded recurrent state space cannot admit strict outward pumping from
every recurrent shell of radius at least two. -/
theorem no_outer_recurrence_of_bounded_strict_lift
    {State : Type}
    (Recurrent : State → Prop)
    (radius : State → ℕ)
    (bound : ℕ)
    (hbounded : ∀ x, Recurrent x → radius x ≤ bound)
    (hlift :
      ∀ x, Recurrent x → 2 ≤ radius x →
        ∃ y, Recurrent y ∧ radius x < radius y) :
    ∀ x, Recurrent x → radius x < 2 := by
  intro x hx
  by_contra hnot
  have hxOuter : 2 ≤ radius x := Nat.le_of_not_gt hnot
  obtain ⟨y, hy, hlarge⟩ :=
    iterate_strict_shell_lift
      Recurrent radius hlift (bound + 1) x hx hxOuter
  have hyBound := hbounded y hy
  omega

/-- High-level carry-bound theorem.

`ReturnCapable` is the recurrent/indefinitely continuable part of the exact
carry system.  The controller-family theorem supplies `hpump`: a maximum-shell
first return at radius `M ≥ 2` can be transported to another return-capable
state at a strictly larger radius.  Any independent admissibility argument
supplying a finite radius bound then rules out all non-ternary recurrence. -/
theorem universal_carry_bound_from_controller_family
    {State : Type}
    (ReturnCapable : State → Prop)
    (shellRadius : State → ℕ)
    (admissibleBound : ℕ)
    (hadmissible :
      ∀ x, ReturnCapable x →
        shellRadius x ≤ admissibleBound)
    (hpump :
      ∀ x, ReturnCapable x → 2 ≤ shellRadius x →
        ∃ y, ReturnCapable y ∧
          shellRadius x < shellRadius y) :
    ∀ x, ReturnCapable x →
      shellRadius x ≤ 1 := by
  intro x hx
  have hlt :=
    no_outer_recurrence_of_bounded_strict_lift
      ReturnCapable shellRadius admissibleBound
      hadmissible hpump x hx
  omega

/-- The precise local obligation left to the continuation-controller family:
turn an exact maximum-shell first return into a strictly outward return-capable
state.  Once this proposition is established for the concrete n-bonacci
system, `universal_carry_bound_from_controller_family` closes the high-level
universal argument. -/
def StrictShellPump
    {State : Type}
    (ReturnCapable : State → Prop)
    (shellRadius : State → ℕ) : Prop :=
  ∀ x, ReturnCapable x → 2 ≤ shellRadius x →
    ∃ y, ReturnCapable y ∧ shellRadius x < shellRadius y

end RavelGenerated
