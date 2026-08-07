import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

/-- Mechanically emitted: sigma_{0,1}'s incidence matrix (Finding 23's Cayley-Hamilton relation). -/
theorem cayley_hamilton_cubic_instance_0 :
    (!![0,0,1;1,0,0;1,1,0] : Matrix (Fin 3) (Fin 3) Int) ^ 3 =
    (!![0,0,1;1,0,0;1,1,0] : Matrix (Fin 3) (Fin 3) Int) + 1 := by
  decide

/- Semantic proof graph for: cayley_hamilton_cubic_batch
  [0] lean.cayley_hamilton_cubic_certificate :: sigma_{0,1}'s incidence matrix (Finding 23's Cayley-Hamilton relation) -- M^3 = M + I, verified by exact integer arithmetic
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
