import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem no_strict_rank_relation_closed
    {α : Type} [Fintype α] [Nonempty α]
    (R : α → α → Prop) (rank : α → ℤ)
    (hout : ∀ x, ∃ y, R x y)
    (hstrict : ∀ ⦃x y⦄, R x y → rank x < rank y) :
    False := by
  let m : ℤ := (Finset.univ.image rank).max' (by simp)
  have hmemb : m ∈ Finset.univ.image rank :=
    Finset.max'_mem (Finset.univ.image rank) (by simp)
  obtain ⟨x, _, hxrank⟩ := Finset.mem_image.mp hmemb
  obtain ⟨y, hxy⟩ := hout x
  have hymax : rank y ≤ m := by
    exact Finset.le_max' (Finset.univ.image rank) (rank y)
      (Finset.mem_image.mpr ⟨y, Finset.mem_univ y, rfl⟩)
  have hxy' : rank x < rank y := hstrict hxy
  omega

theorem shell_empty_of_strict_first_return_rank
    {State : Type} [Fintype State]
    (Shell : Set State)
    (FirstReturn : State → State → Prop)
    (rank : State → ℤ)
    (hout :
      ∀ x, x ∈ Shell →
        ∃ y, y ∈ Shell ∧ FirstReturn x y)
    (hstrict :
      ∀ ⦃x y⦄,
        x ∈ Shell → y ∈ Shell →
        FirstReturn x y →
        rank x < rank y) :
    Shell = ∅ := by
  by_contra hne
  have hnonempty : Shell.Nonempty := Set.nonempty_iff_ne_empty.mpr hne
  letI : Nonempty Shell := hnonempty.to_subtype
  letI : Fintype Shell := (Set.toFinite Shell).fintype
  let R : Shell → Shell → Prop :=
    fun x y => FirstReturn x.1 y.1
  let shellRank : Shell → ℤ := fun x => rank x.1
  have hout' : ∀ x : Shell, ∃ y : Shell, R x y := by
    intro x
    obtain ⟨y, hyShell, hxy⟩ := hout x.1 x.2
    exact ⟨⟨y, hyShell⟩, hxy⟩
  have hstrict' :
      ∀ ⦃x y : Shell⦄, R x y → shellRank x < shellRank y := by
    intro x y hxy
    exact hstrict x.2 y.2 hxy
  exact no_strict_rank_relation_closed R shellRank hout' hstrict'

theorem no_nonempty_shell_with_strict_first_return_rank
    {State : Type} [Fintype State]
    (Shell : Set State)
    (FirstReturn : State → State → Prop)
    (rank : State → ℤ)
    (hShell : Shell.Nonempty)
    (hout :
      ∀ x, x ∈ Shell →
        ∃ y, y ∈ Shell ∧ FirstReturn x y)
    (hstrict :
      ∀ ⦃x y⦄,
        x ∈ Shell → y ∈ Shell →
        FirstReturn x y →
        rank x < rank y) :
    False := by
  have hempty :=
    shell_empty_of_strict_first_return_rank
      Shell FirstReturn rank hout hstrict
  exact Set.nonempty_iff_ne_empty.mp hShell hempty

theorem strict_rank_of_phase_offset
    {State Phase : Type}
    (level : State → ℤ)
    (phase : State → Phase)
    (offset : Phase → ℤ)
    (FirstReturn : State → State → Prop)
    (hconstraint :
      ∀ ⦃x y⦄, FirstReturn x y →
        offset (phase y) ≥
          offset (phase x) + level x - level y + 1) :
    ∀ ⦃x y⦄, FirstReturn x y →
      level x + offset (phase x) <
        level y + offset (phase y) := by
  intro x y hxy
  have h := hconstraint hxy
  omega

theorem shell_empty_of_phase_rank_transport
    {State Phase : Type}
    [Fintype State]
    (Shell : Set State)
    (FirstReturn : State → State → Prop)
    (level : State → ℤ)
    (phase : State → Phase)
    (offset : Phase → ℤ)
    (hout :
      ∀ x, x ∈ Shell →
        ∃ y, y ∈ Shell ∧ FirstReturn x y)
    (hconstraint :
      ∀ ⦃x y⦄,
        x ∈ Shell → y ∈ Shell → FirstReturn x y →
        offset (phase y) ≥
          offset (phase x) + level x - level y + 1) :
    Shell = ∅ := by
  classical
  by_contra hne
  have hnonempty : Shell.Nonempty := Set.nonempty_iff_ne_empty.mpr hne
  let rank : State → ℤ := fun x => level x + offset (phase x)
  obtain ⟨x, hxShell, hxMax⟩ :=
    Shell.exists_max_image rank (Set.toFinite Shell) hnonempty
  obtain ⟨y, hyShell, hxy⟩ := hout x hxShell
  have hstrict : rank x < rank y := by
    dsimp [rank]
    have h := hconstraint hxShell hyShell hxy
    omega
  have hle : rank y ≤ rank x := hxMax y hyShell
  omega

/-- Mechanically emitted: 3-cycle closed relation -- ravel::proof::universal_dominance::validate_shell_return_certificate
    independently verified (from raw edge data) that every one of this 3-state relation's
    states has an outgoing edge (`certificate_closed`, exactly `hout`). -/
def udRelation0 : Fin 3 → Fin 3 → Prop
  | ⟨0, _⟩, ⟨1, _⟩ => True
  | ⟨1, _⟩, ⟨2, _⟩ => True
  | ⟨2, _⟩, ⟨0, _⟩ => True
  | _, _ => False

theorem universal_dominance_no_strict_rank_instance_0 :
    ∀ rank : Fin 3 → ℤ,
      (∀ x, ∃ y, udRelation0 x y) →
      (∀ ⦃x y⦄, udRelation0 x y → rank x < rank y) →
      False :=
  fun rank hout hstrict => no_strict_rank_relation_closed udRelation0 rank hout hstrict

theorem universal_dominance_shell_empty_instance_0 :
    ∀ rank : Fin 3 → ℤ,
      (∀ x, x ∈ (Set.univ : Set (Fin 3)) → ∃ y, y ∈ (Set.univ : Set (Fin 3)) ∧ udRelation0 x y) →
      (∀ ⦃x y⦄, x ∈ (Set.univ : Set (Fin 3)) → y ∈ (Set.univ : Set (Fin 3)) → udRelation0 x y → rank x < rank y) →
      (Set.univ : Set (Fin 3)) = ∅ :=
  fun rank hout hstrict =>
    shell_empty_of_strict_first_return_rank Set.univ udRelation0 rank hout hstrict

theorem universal_dominance_no_nonempty_instance_0 :
    ∀ rank : Fin 3 → ℤ,
      (∀ x, x ∈ (Set.univ : Set (Fin 3)) → ∃ y, y ∈ (Set.univ : Set (Fin 3)) ∧ udRelation0 x y) →
      (∀ ⦃x y⦄, x ∈ (Set.univ : Set (Fin 3)) → y ∈ (Set.univ : Set (Fin 3)) → udRelation0 x y → rank x < rank y) →
      False :=
  fun rank hout hstrict =>
    no_nonempty_shell_with_strict_first_return_rank Set.univ udRelation0 rank Set.univ_nonempty hout hstrict

theorem universal_dominance_phase_rank_instance_0 :
    ∀ level : Fin 3 → ℤ,
      (∀ x, x ∈ (Set.univ : Set (Fin 3)) → ∃ y, y ∈ (Set.univ : Set (Fin 3)) ∧ udRelation0 x y) →
      (∀ ⦃x y⦄, x ∈ (Set.univ : Set (Fin 3)) → y ∈ (Set.univ : Set (Fin 3)) → udRelation0 x y →
        (0:ℤ) ≥ 0 + level x - level y + 1) →
      (Set.univ : Set (Fin 3)) = ∅ :=
  fun level hout hconstraint =>
    shell_empty_of_phase_rank_transport Set.univ udRelation0 level (fun _ => ()) (fun _ => (0:ℤ)) hout hconstraint

/- Semantic proof graph for: universal_dominance_closed_relation_batch
  [0] lean.universal_dominance_closed_relation_certificate :: states=3 edges=3 3-cycle closed relation -- instantiates no_strict_rank_relation_closed
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
