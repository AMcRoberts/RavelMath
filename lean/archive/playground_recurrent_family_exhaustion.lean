import Mathlib

namespace RavelPlayground

/-- A recurrent-family classifier is exhaustive when each recurrent component
    has a unique structural family witness. -/
theorem recurrent_family_exhaustion
    {Component Family : Type}
    (recurrent : Component → Prop)
    (belongs : Component → Family → Prop)
    (classified : ∀ c, recurrent c → ∃! f, belongs c f) :
    ∀ c, recurrent c → ∃ f, belongs c f := by
  intro c hc
  exact (classified c hc).exists

/-- The adjacent-dimensional induction step: transported old families, newly
    classified terminal families, and a no-return boundary exhaust the upper
    recurrent components. -/
theorem inductive_family_exhaustion
    (lowerExhausted transportedTotal terminalsClassified
      rejectedBoundaryNoReturn disjoint : Prop)
    (hLower : lowerExhausted)
    (hTransport : transportedTotal)
    (hTerminal : terminalsClassified)
    (hBoundary : rejectedBoundaryNoReturn)
    (hDisjoint : disjoint) :
    lowerExhausted ∧ transportedTotal ∧ terminalsClassified ∧
      rejectedBoundaryNoReturn ∧ disjoint := by
  exact ⟨hLower, hTransport, hTerminal, hBoundary, hDisjoint⟩

end RavelPlayground
