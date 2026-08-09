import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- Mechanically emitted: d_beta(1)=1001001 simple-Parry factorization for the third-smallest Pisot number -- ravel::proof::stage_third_smallest_pisot_parry_factorization
    independently recomputed minimal_polynomial * cyclotomic_factor and
    compared against parry_polynomial (nothing pre-trusted). -/
theorem third_smallest_pisot_parry_factorization_instance_0 (x : ℤ) :
    (1 + x^2) * (-1 + x^2 - x^3 - x^4 + x^5)
      = (-1 - x^3 - x^6 + x^7) := by
  ring

/- Semantic proof graph for: third_smallest_pisot_parry_factorization_batch
  [0] lean.third_smallest_pisot_parry_factorization_certificate :: deg(minpoly)=5 d_beta(1)=1001001 simple-Parry factorization for the third-smallest Pisot number -- instantiates third_smallest_pisot_parry_factorization
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
