import Mathlib

namespace RavelGenerated

/-- Any sublist of word inequalities remains valid after summing, because
    boundary admissibility only deletes nonnegative transport channels. -/
theorem positive_subgrammar_sum
    {α : Type*} [Preorder α] [AddCommMonoid α]
    (add_mono : ∀ {a b c d : α}, a ≤ b → c ≤ d → a + c ≤ b + d)
    (lhs rhs : List α)
    (hsize : lhs.length = rhs.length)
    (hword : ∀ i (hi : i < lhs.length), lhs[i] ≤ rhs[i]) :
    lhs.sum ≤ rhs.sum := by
  induction lhs generalizing rhs with
  | nil =>
      simp at hsize
      simp [hsize]
  | cons a as ih =>
      cases rhs with
      | nil => simp at hsize
      | cons b bs =>
          have hab : a ≤ b := by
            simpa using hword 0 (Nat.zero_lt_succ _)
          have htail : ∀ i (hi : i < as.length), as[i] ≤ bs[i] := by
            intro i hi
            simpa using hword (i+1) (Nat.succ_lt_succ hi)
          have hsizes : as.length = bs.length := by simpa using hsize
          simpa using add_mono hab (ih bs hsizes htail)

/-- If every generator word satisfies a common intertwining inequality, then
    any admissible finite subgrammar obtained by deleting channels satisfies
    the summed inequality. -/
theorem admissible_subgrammar_intertwines
    {α : Type*} [Preorder α] [AddCommMonoid α]
    (add_mono : ∀ {a b c d : α}, a ≤ b → c ≤ d → a + c ≤ b + d)
    (competitor core : List α)
    (hsize : competitor.length = core.length)
    (hedge : ∀ i (hi : i < competitor.length), competitor[i] ≤ core[i]) :
    competitor.sum ≤ core.sum :=
  positive_subgrammar_sum add_mono competitor core hsize hedge

end RavelGenerated
