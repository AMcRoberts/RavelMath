import Mathlib

/-!
Kernel-checkable interface for a sign-flux-generated rank.

The C++ synthesizer supplies an integer-valued potential.  Once exact replay
has established positive gain on every first-return edge, this theorem turns
that evidence into the strict-rank statement used by the shell argument.
-/

namespace SignFluxRank

variable {State : Type} (edge : State → State → Prop)

structure GeneratedPotential where
  value : State → Int

def StrictOn (R : GeneratedPotential State) : Prop :=
  ∀ ⦃x y⦄, edge x y → R.value x < R.value y

theorem strict_of_positive_gain
    (R : GeneratedPotential State)
    (hgain : ∀ ⦃x y⦄, edge x y → 0 < R.value y - R.value x) :
    StrictOn edge R := by
  intro x y hxy
  have h := hgain hxy
  omega

end SignFluxRank
