import Mathlib

namespace RavelGenerated

/-- The alternating sum of the nearest left-outside coefficient profile
`(1,...,1,0,1)` vanishes at `-1` in even dimension.  This is the exact
cyclotomic `x + 1` obstruction produced by deleting one renewal phase. -/
theorem nearest_left_profile_even_has_minus_one_root
    (k : ℕ) :
    (-1 : ℤ)^(2*k+4) -
      (∑ j in Finset.Icc 2 (2*k+3), (-1 : ℤ)^j) - 1 = 0 := by
  norm_num [Finset.sum_Icc_succ_top]

end RavelGenerated
