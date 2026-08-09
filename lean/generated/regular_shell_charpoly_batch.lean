import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem neighbor0_shell_below_pred (a t lambda : ℝ)
    (_ha : 3 ≤ a) (ht : t ≤ a - 3) (hlambda : 0 < lambda)
    (heq : lambda + 1 / lambda = t + 2) : lambda < a - 1 := by
  have hinv : 0 < 1 / lambda := one_div_pos.mpr hlambda
  nlinarith

theorem neighbor1_shell_below_pred (a t lambda : ℝ)
    (ha : 3 ≤ a) (ht0 : 0 ≤ t) (ht : t ≤ a - 2) (_hlambda : 0 ≤ lambda)
    (heq : lambda ^ 2 = t * (t + 2)) : lambda < a - 1 := by
  nlinarith [sq_nonneg (a - 1 - lambda), mul_nonneg ht0 (by nlinarith : (0:ℝ) ≤ t + 2)]

theorem neighbor2_shell_below_self (a k lambda : ℝ)
    (_ha : 3 ≤ a) (hk : k ≤ a - 1) (heq : lambda = k) : lambda < a := by
  nlinarith

/-- Mechanically emitted: neighbor 0 regular shell, a=8, t=3 -- the C++ certificate independently verified (exact integer
    Faddeev-LeVerrier, self-checked via Cayley-Hamilton) that this neighbor's
    concrete regular-shell compressed matrix at a=8, t=3 has characteristic
    polynomial matching docs/FAMILY_OF_FAMILIES.md's displayed closed form,
    which is where `lambda` below comes from; that matrix-charpoly identity
    itself is C++-verified data threaded into this corollary, not re-derived
    here (the same trust boundary PisotRootOrderingCertificate already has). -/
theorem regular_shell_charpoly_instance_0 :
    ∃ lambda : ℝ, 0 < lambda ∧ lambda + 1 / lambda = (3:ℝ) + 2 ∧
      lambda < 8 - 1 := by
  have hs : (0:ℝ) ≤ ((3:ℝ) + 2) ^ 2 - 4 := by norm_num
  have hsq : Real.sqrt (((3:ℝ) + 2) ^ 2 - 4) ^ 2 = ((3:ℝ) + 2) ^ 2 - 4 := Real.sq_sqrt hs
  refine ⟨((3:ℝ) + 2 + Real.sqrt (((3:ℝ) + 2) ^ 2 - 4)) / 2, by positivity, ?_, ?_⟩
  · have hpos : (0:ℝ) < ((3:ℝ) + 2 + Real.sqrt (((3:ℝ) + 2) ^ 2 - 4)) / 2 := by positivity
    have hne := ne_of_gt hpos
    field_simp
    nlinarith [hsq]
  · exact neighbor0_shell_below_pred 8 3 _ (by norm_num) (by norm_num)
      (by positivity) (by
        have hpos : (0:ℝ) < ((3:ℝ) + 2 + Real.sqrt (((3:ℝ) + 2) ^ 2 - 4)) / 2 := by positivity
        have hne := ne_of_gt hpos
        field_simp
        nlinarith [hsq])

/-- Mechanically emitted: neighbor 1 regular shell, a=8, t=4 -- the C++ certificate independently verified (exact integer
    Faddeev-LeVerrier, self-checked via Cayley-Hamilton) that this neighbor's
    concrete regular-shell compressed matrix at a=8, t=4 has characteristic
    polynomial matching docs/FAMILY_OF_FAMILIES.md's displayed closed form,
    which is where `lambda` below comes from; that matrix-charpoly identity
    itself is C++-verified data threaded into this corollary, not re-derived
    here (the same trust boundary PisotRootOrderingCertificate already has). -/
theorem regular_shell_charpoly_instance_1 :
    ∃ lambda : ℝ, 0 ≤ lambda ∧ lambda ^ 2 = (4:ℝ) * (4 + 2) ∧ lambda < 8 - 1 := by
  have hs : (0:ℝ) ≤ (4:ℝ) * (4 + 2) := by norm_num
  refine ⟨Real.sqrt ((4:ℝ) * (4 + 2)), Real.sqrt_nonneg _, Real.sq_sqrt hs, ?_⟩
  exact neighbor1_shell_below_pred 8 4 _ (by norm_num) (by norm_num) (by norm_num) (Real.sqrt_nonneg _) (Real.sq_sqrt hs)

/-- Mechanically emitted: neighbor 2 regular shell, a=8, k=5 -- the C++ certificate independently verified (exact integer
    Faddeev-LeVerrier, self-checked via Cayley-Hamilton) that this neighbor's
    concrete regular-shell compressed matrix at a=8, t=5 has characteristic
    polynomial matching docs/FAMILY_OF_FAMILIES.md's displayed closed form,
    which is where `lambda` below comes from; that matrix-charpoly identity
    itself is C++-verified data threaded into this corollary, not re-derived
    here (the same trust boundary PisotRootOrderingCertificate already has). -/
theorem regular_shell_charpoly_instance_2 :
    ∃ lambda : ℝ, lambda = (5:ℝ) ∧ lambda < 8 := by
  refine ⟨5, rfl, ?_⟩
  exact neighbor2_shell_below_self 8 5 5 (by norm_num) (by norm_num) rfl

/- Semantic proof graph for: regular_shell_charpoly_batch
  [0] lean.regular_shell_charpoly_certificate :: neighbor=0 a=8 t=3 neighbor 0 regular shell, a=8, t=3 -- instantiates neighbor_shell_below_*
  [1] lean.regular_shell_charpoly_certificate :: neighbor=1 a=8 t=4 neighbor 1 regular shell, a=8, t=4 -- instantiates neighbor_shell_below_*
  [2] lean.regular_shell_charpoly_certificate :: neighbor=2 a=8 t=5 neighbor 2 regular shell, a=8, k=5 -- instantiates neighbor_shell_below_*
-/

def reflectedNodeCount : Nat := 3

end RavelGenerated
