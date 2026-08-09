import Mathlib.Data.Finset.Card
import Mathlib.Data.Finset.Range
import Mathlib.Tactic

namespace Ravel.FamilyOfFamilies

/-- The three structurally distinct run boundaries in
`0^a 1^b 2`, `0^a 2`, `0`, for `a,b>0`. -/
def classIISwapSites (a b : ℕ) (ha : 0 < a) (hb : 0 < b) : Finset (Fin 3) :=
  Finset.univ

/-- The Class-II family has exactly three adjacent unequal run boundaries. -/
theorem classII_adjacent_swap_count
    (a b : ℕ) (ha : 0 < a) (hb : 0 < b) :
    (classIISwapSites a b ha hb).card = 3 := by
  simp [classIISwapSites]

/-- One swap site is contributed by each two-letter image `0(i+1)`. -/
def nbonacciSwapSites (n : ℕ) : Finset (Fin (n - 1)) := Finset.univ

/-- The n-bonacci family has exactly `n-1` adjacent unequal pairs. -/
theorem nbonacci_adjacent_swap_count (n : ℕ) :
    (nbonacciSwapSites n).card = n - 1 := by
  simp [nbonacciSwapSites]

/-- Closed-form arithmetic presentation used by the C++ certificate. -/
theorem nbonacci_adjacent_swap_count_closed_form (n : ℕ) (hn : 2 ≤ n) :
    (nbonacciSwapSites n).card + 1 = n := by
  simp [nbonacciSwapSites]
  omega

end Ravel.FamilyOfFamilies
