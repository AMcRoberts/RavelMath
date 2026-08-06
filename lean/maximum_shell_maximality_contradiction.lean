import Mathlib.Tactic

namespace RavelGenerated

/-- A replayed strict recurrent shell lift contradicts selection of its source
as a recurrent state of maximum radius. -/
theorem maximum_shell_contradiction_of_strict_lift
    {State : Type}
    (Recurrent : State → Prop)
    (radius : State → ℕ)
    (source lifted : State)
    (maximumRadius : ℕ)
    (hsource : Recurrent source)
    (hsourceMax : radius source = maximumRadius)
    (hlifted : Recurrent lifted)
    (hstrict : radius source < radius lifted)
    (hmax : ∀ x, Recurrent x → radius x ≤ maximumRadius) :
    False := by
  have hbound := hmax lifted hlifted
  omega

/-- Finite cyclic-kernel branch packaged at the theorem boundary. -/
theorem maximum_shell_not_recurrent_of_pump
    {State : Type}
    (Recurrent : State → Prop)
    (radius : State → ℕ)
    (maximumRadius : ℕ)
    (hmax : ∀ x, Recurrent x → radius x ≤ maximumRadius)
    (source : State)
    (hsource : Recurrent source)
    (hsourceMax : radius source = maximumRadius)
    (hpump : ∃ lifted, Recurrent lifted ∧ radius source < radius lifted) :
    False := by
  obtain ⟨lifted, hlifted, hstrict⟩ := hpump
  exact maximum_shell_contradiction_of_strict_lift
    Recurrent radius source lifted maximumRadius
    hsource hsourceMax hlifted hstrict hmax

end RavelGenerated
