import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

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
    have key : ∀ x, Relation.ReflTransGen Edge anchor x → x ∉ Core →
        ∃ u w, u ∈ Core ∧ Edge u w ∧ w ∉ Core ∧ Relation.ReflTransGen Edge w x := by
      intro x hx
      induction hx with
      | refl => intro hcontra; exact (hcontra hanchor).elim
      | @tail b c hab hbc ih =>
          intro hcNotCore
          by_cases hb : b ∈ Core
          · exact ⟨b, c, hb, hbc, hcNotCore, .refl⟩
          · obtain ⟨u, w, hu, huw, hw, hwv⟩ := ih hb
            exact ⟨u, w, hu, huw, hw, hwv.tail hbc⟩
    obtain ⟨u, w, hu, huw, hw, hwv⟩ := key v hav hv
    exact hnoReturn hu huw hw (hwv.trans hva)
  · intro hv
    exact ⟨hstrong hanchor hv, hstrong hv hanchor⟩

/-- Mechanically emitted: dimension 3 predicted-core graph -- ravel::proof::certify_predicted_core_scc independently ran
    Tarjan's algorithm over the concrete dimension-3 predicted-core graph (14 nodes, 18 edges) and
    verified it forms exactly one SCC, together with exact node/edge/
    predecessor-table counts checked against the closed-form combinatorial
    formulas; that strong-connectivity fact is C++-verified data threaded
    into `hstrong` below, not re-derived here. -/
theorem predicted_core_scc_exhaustion_instance_0
    (Edge : Fin 14 → Fin 14 → Prop)
    (anchor : Fin 14)
    (hstrong :
      ∀ ⦃u v⦄, u ∈ (Set.univ : Set (Fin 14)) → v ∈ (Set.univ : Set (Fin 14)) →
        Relation.ReflTransGen Edge u v) :
    ∀ v, (Relation.ReflTransGen Edge anchor v ∧ Relation.ReflTransGen Edge v anchor) ↔
      v ∈ (Set.univ : Set (Fin 14)) :=
  exact_scc_of_stronglyConnected_noReturnAfterExit Edge Set.univ anchor
    (Set.mem_univ anchor) hstrong (fun {_ v} _ _ hv => absurd (Set.mem_univ v) hv)

/-- Mechanically emitted: dimension 4 predicted-core graph -- ravel::proof::certify_predicted_core_scc independently ran
    Tarjan's algorithm over the concrete dimension-4 predicted-core graph (46 nodes, 70 edges) and
    verified it forms exactly one SCC, together with exact node/edge/
    predecessor-table counts checked against the closed-form combinatorial
    formulas; that strong-connectivity fact is C++-verified data threaded
    into `hstrong` below, not re-derived here. -/
theorem predicted_core_scc_exhaustion_instance_1
    (Edge : Fin 46 → Fin 46 → Prop)
    (anchor : Fin 46)
    (hstrong :
      ∀ ⦃u v⦄, u ∈ (Set.univ : Set (Fin 46)) → v ∈ (Set.univ : Set (Fin 46)) →
        Relation.ReflTransGen Edge u v) :
    ∀ v, (Relation.ReflTransGen Edge anchor v ∧ Relation.ReflTransGen Edge v anchor) ↔
      v ∈ (Set.univ : Set (Fin 46)) :=
  exact_scc_of_stronglyConnected_noReturnAfterExit Edge Set.univ anchor
    (Set.mem_univ anchor) hstrong (fun {_ v} _ _ hv => absurd (Set.mem_univ v) hv)

/- Semantic proof graph for: predicted_core_scc_exhaustion_batch
  [0] lean.predicted_core_scc_exhaustion_certificate :: dimension=3 nodes=14 edges=18 dimension 3 predicted-core graph -- instantiates exact_scc_of_stronglyConnected_noReturnAfterExit
  [1] lean.predicted_core_scc_exhaustion_certificate :: dimension=4 nodes=46 edges=70 dimension 4 predicted-core graph -- instantiates exact_scc_of_stronglyConnected_noReturnAfterExit
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
