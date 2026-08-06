import Mathlib.Tactic

namespace RavelGenerated

/-- Strong connectivity can be promoted from a strongly connected shadow when
all vertices are reachable from the shadow and can return to it. -/
theorem stronglyConnected_of_shadow
    {Vertex : Type}
    (Reach : Vertex → Vertex → Prop)
    (Shadow : Set Vertex)
    (htrans : ∀ {x y z}, Reach x y → Reach y z → Reach x z)
    (hshadow :
      ∀ {x y}, x ∈ Shadow → y ∈ Shadow → Reach x y)
    (hfrom : ∀ y, ∃ x ∈ Shadow, Reach x y)
    (hto : ∀ x, ∃ y ∈ Shadow, Reach x y) :
    ∀ x y, Reach x y := by
  intro x y
  obtain ⟨sx, hsx, hxs⟩ := hto x
  obtain ⟨sy, hsy, hsyy⟩ := hfrom y
  exact htrans hxs (htrans (hshadow hsx hsy) hsyy)

/-- Dimension induction interface for the formula-defined predicted core. -/
theorem predictedCore_stronglyConnected_induction
    (StronglyConnected : ℕ → Prop)
    (hbase : StronglyConnected 3)
    (hstep : ∀ n, 3 ≤ n → StronglyConnected n → StronglyConnected (n + 1)) :
    ∀ n, 3 ≤ n → StronglyConnected n := by
  intro n hn
  induction n, hn using Nat.le_induction with
  | base => exact hbase
  | succ n hn ih => exact hstep n hn ih

/-- A nonempty, strongly connected formula-defined core is exactly one SCC
once the literal-graph exhaustion premise excludes any outside vertex mutually
reachable with its anchor. -/
theorem core_is_exact_scc
    {Vertex : Type}
    (Reach : Vertex → Vertex → Prop)
    (Core : Vertex → Prop)
    (anchor : Vertex)
    (hanchor : Core anchor)
    (hcoreConnected :
      ∀ x y, Core x → Core y → Reach x y)
    (hexhaust :
      ∀ x, Reach anchor x → Reach x anchor → Core x) :
    ∀ x, (Reach anchor x ∧ Reach x anchor) ↔ Core x := by
  intro x
  constructor
  · rintro ⟨hax, hxa⟩
    exact hexhaust x hax hxa
  · intro hx
    exact ⟨hcoreConnected anchor x hanchor hx,
      hcoreConnected x anchor hx hanchor⟩

end RavelGenerated
