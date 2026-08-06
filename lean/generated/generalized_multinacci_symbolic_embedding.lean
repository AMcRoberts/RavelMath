import Mathlib

namespace RavelGenerated

/-- Number of parent occurrences of the letter `0` in the generalized
multinacci substitution of dimension `D+2` and multiplicity `m+1`. -/
def zeroParentCount (D m : ℕ) : ℕ := (D + 1) * (m + 1) + 1

/-- Total parent occurrences over all inner letters. -/
def totalParentCount (D m : ℕ) : ℕ := (D + 1) * (m + 2) + 1

/-- The literal parent catalogue has `(D-1)m+1` zero parents and one parent
for each of the other `D-1` letters (written with shifted natural parameters). -/
theorem total_parent_count_identity (D m : ℕ) :
    zeroParentCount D m + (D + 1) = totalParentCount D m := by
  simp [zeroParentCount, totalParentCount]
  omega

/-- Two prefix cuts in `[0,m]` have a renewal roof bounded by `m`. -/
theorem prefix_roof_le {m p q : ℕ} (hp : p ≤ m) (hq : q ≤ m) :
    q - p ≤ m ∧ p - q ≤ m := by
  omega

/-- Equal cuts emit the balanced generator; unequal cuts emit a positive
residual power with exponent bounded by the prefix multiplicity. -/
theorem cut_classification {m p q : ℕ} (hp : p ≤ m) (hq : q ≤ m) :
    p = q ∨ (0 < q - p ∧ q - p ≤ m) ∨ (0 < p - q ∧ p - q ≤ m) := by
  omega

/-- Abstract deletion-only closure: retaining any subset of a collection of
nonnegative channels preserves an entrywise upper majorant. -/
theorem deletion_only_subsum
    {ι α : Type*} [Fintype ι] [OrderedAddCommMonoid α]
    (channel majorant : ι → α)
    (keep : ι → Bool)
    (h : ∀ i, channel i ≤ majorant i) :
    ∑ i, if keep i then channel i else 0 ≤
      ∑ i, if keep i then majorant i else 0 := by
  exact Finset.sum_le_sum fun i _ => by
    by_cases hi : keep i
    · simp [hi, h i]
    · simp [hi]

end RavelGenerated
