import Mathlib.Tactic

namespace RavelGenerated

/-- A finite nonempty closed relation cannot carry a strictly increasing
integer rank on every edge. This is the kernel interface consumed by exact
maximum-shell first-return certificates. -/
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

/-- Certificate-conversion form: if a finite shell relation has an outgoing
return from every shell state and every certified return strictly increases
rank, then the shell is empty. -/
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

/-- This theorem packages the logical endpoint used by maximum-shell escape:
a nonempty shell cannot satisfy a complete strict first-return certificate. -/
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

end RavelGenerated
