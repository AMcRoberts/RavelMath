import Mathlib

namespace RavelGenerated

/-- The second cyclotomic polynomial. -/
theorem cyclotomic_two : Polynomial.cyclotomic 2 ℤ = Polynomial.X + 1 := by
  native_decide

/-- Exact order-two obstruction criterion: `x + 1` divides `p` iff `p(-1)=0`. -/
theorem x_add_one_dvd_iff_eval_neg_one_zero (p : Polynomial ℤ) :
    Polynomial.X + 1 ∣ p ↔ p.eval (-1) = 0 := by
  rw [Polynomial.X_add_C_dvd_iff]
  norm_num


end RavelGenerated
