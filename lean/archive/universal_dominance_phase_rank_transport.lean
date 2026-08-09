import Mathlib.Tactic

namespace RavelGenerated

/-- A phase-offset certificate converts a strict local inequality into a strict
rank increase. This is the reusable kernel endpoint for the merged shell
transport machinery. -/
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

/-- If every shell state has a first return and the merged phase certificate
strictly increases the rank, the finite shell is empty. -/
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

end RavelGenerated
