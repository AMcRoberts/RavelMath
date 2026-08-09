import Mathlib.Tactic

namespace RavelGenerated

/-- If a strongly connected core has no exiting edge whose destination can
return to the anchor, then the core is exactly the anchor's SCC.  Outgoing
transient branches are allowed. -/
theorem exact_scc_of_stronglyConnected_noReturnAfterExit
    {Vertex : Type}
    (Edge : Vertex → Vertex → Prop)
    (Core : Set Vertex)
    (anchor : Vertex)
    (hanchor : anchor ∈ Core)
    (hstrong :
      ∀ ⦃u v⦄, u ∈ Core → v ∈ Core →
        Relation.ReflTransGen Edge u v)
    (hnoReturn :
      ∀ ⦃u v⦄,
        u ∈ Core → Edge u v → v ∉ Core →
        ¬ Relation.ReflTransGen Edge v anchor) :
    ∀ v,
      (Relation.ReflTransGen Edge anchor v ∧
       Relation.ReflTransGen Edge v anchor) ↔
      v ∈ Core := by
  intro v
  constructor
  · rintro ⟨hav, hva⟩
    by_contra hv
    have hexit :
        ∃ u w,
          u ∈ Core ∧ Edge u w ∧ w ∉ Core ∧
          Relation.ReflTransGen Edge w v := by
      induction hav with
      | refl => exact (hv hanchor).elim
      | @tail a b c hab hbc ih =>
          by_cases hb : b ∈ Core
          · by_cases hc : c ∈ Core
            · exact (hv hc).elim
            · exact ⟨b, c, hb, hbc, hc, .refl⟩
          · obtain ⟨u, w, hu, huw, hw, hwv⟩ := ih hb
            exact ⟨u, w, hu, huw, hw, hwv.tail hbc⟩
    obtain ⟨u, w, hu, huw, hw, hwv⟩ := hexit
    exact hnoReturn hu huw hw (hwv.trans hva)
  · intro hv
    exact ⟨hstrong hanchor hv, hstrong hv hanchor⟩

/-- Finite-certificate interface: a backend may report outgoing edges, but must
prove that none of their destinations belongs to the reverse basin of the core
anchor. -/
structure PredictedCoreNoReturnCertificate
    (Vertex : Type)
    (Edge : Vertex → Vertex → Prop)
    (Core : Set Vertex) where
  anchor : Vertex
  anchor_mem : anchor ∈ Core
  internally_strong :
    ∀ ⦃u v⦄, u ∈ Core → v ∈ Core →
      Relation.ReflTransGen Edge u v
  no_return_after_exit :
    ∀ ⦃u v⦄,
      u ∈ Core → Edge u v → v ∉ Core →
      ¬ Relation.ReflTransGen Edge v anchor

end RavelGenerated
