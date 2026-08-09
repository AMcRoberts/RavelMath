import Mathlib

-- Exact algebraic identity behind d_beta(1) = 1001001.
theorem third_smallest_pisot_parry_factorization (x : ℤ) :
    (x^2 + 1) * (x^5 - x^4 - x^3 + x^2 - 1)
      = x^7 - x^6 - x^3 - 1 := by
  ring
