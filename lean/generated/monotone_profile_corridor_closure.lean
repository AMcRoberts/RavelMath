import Mathlib
namespace RavelGenerated

def corridorDigit (k i : ℕ) : ℕ := if i < k then 2 else 1

-- Arithmetic core of the corridor catalogue: among D parent classes,
-- exactly k contribute one additional prefix occurrence.
theorem corridor_extra_occurrences (D k : ℕ) (hk : k ≤ D) :
    (∑ i in Finset.range D, (if i < k then 1 else 0)) = k := by
  simpa using Finset.sum_boole (p := fun i : Fin D => (i : ℕ) < k)

end RavelGenerated
