import Mathlib.Tactic

namespace RavelGenerated

/-- The controller's adjusted digit can be eliminated symbolically. A proposed
next tail `r` is legal exactly when the reconstructed adjusted digit lies in
the admissible interval `[-1,1]`. -/
theorem adjusted_digit_exists_iff_tail_inequality
    (center baseDigit nextTail : ℤ) :
    (∃ adjusted : ℤ,
        -1 ≤ adjusted ∧ adjusted ≤ 1 ∧
        nextTail = center + adjusted - baseDigit) ↔
      |nextTail - center + baseDigit| ≤ 1 := by
  constructor
  · rintro ⟨adjusted, hadjLo, hadjHi, htail⟩
    subst nextTail
    have hrewrite :
        center + adjusted - baseDigit - center + baseDigit = adjusted := by
      ring
    rw [hrewrite]
    exact abs_le.mpr ⟨hadjLo, hadjHi⟩
  · intro h
    refine ⟨nextTail - center + baseDigit, ?_, ?_, ?_⟩
    · exact (abs_le.mp h).1
    · exact (abs_le.mp h).2
    · ring

/-- A symbolic winning-set certificate is trusted only through these three
obligations: initialization, one-step closure with an existential successor,
and terminal acceptance. -/
theorem winning_certificate_sound
    {State Input : Type}
    (Init Accept Win : State → Prop)
    (Step : State → Input → State → Prop)
    (word : List Input)
    (hinit : ∃ s, Init s ∧ Win s)
    (hstep :
      ∀ s input, Win s →
        ∃ s', Step s input s' ∧ Win s')
    (hterminal : ∀ s, Win s → Accept s) :
    ∃ start finish,
      Init start ∧ Accept finish := by
  obtain ⟨start, hstartInit, hstartWin⟩ := hinit
  refine ⟨start, start, hstartInit, hterminal start hstartWin⟩

end RavelGenerated
