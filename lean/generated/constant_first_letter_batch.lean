import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

def runningSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)
  | [] => []
  | (a :: rest) =>
      (fun _ => (0 : ℤ)) :: (runningSeq rest).map (fun v => fun j => v j + if j = a then 1 else 0)

def prefixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=
  w.zip (runningSeq w)

/-- The PREFIX half of `pair_has_coincidence` (see
    `lean/constant_first_letter_forces_prefix_coincidence.lean` for the full
    correspondence to the C++ loop). -/
def hasCoincidencePrefix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=
  ∃ p, p ∈ prefixPairs w1 ∧ p ∈ prefixPairs w2

/-- Finding 17: if `w1` and `w2` both start with the same letter `c`, the pair
    `(c, zeroVector)` is recorded in BOTH lists' `prefixPairs`, so the prefix
    coincidence fires immediately, unconditionally. Reproduced from the
    independently kernel-checked
    `lean/constant_first_letter_forces_prefix_coincidence.lean` (not re-derived
    here). -/
theorem constant_first_letter_forces_prefix_coincidence
    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :
    hasCoincidencePrefix (c :: w1') (c :: w2') := by
  refine ⟨(c, fun _ => (0 : ℤ)), ?_, ?_⟩ <;>
    simp [prefixPairs, runningSeq]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 1 (d=3 letters, constant first letter 0). -/
theorem constant_first_letter_instance_0 :
    @hasCoincidencePrefix 3 [0, 1, 2] [0, 2] :=
  constant_first_letter_forces_prefix_coincidence (0 : Fin 3) [1, 2] [2]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 2 (d=3 letters, constant first letter 0). -/
theorem constant_first_letter_instance_1 :
    @hasCoincidencePrefix 3 [0, 1, 2] [0] :=
  constant_first_letter_forces_prefix_coincidence (0 : Fin 3) [1, 2] []

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 1 and 2 (d=3 letters, constant first letter 0). -/
theorem constant_first_letter_instance_2 :
    @hasCoincidencePrefix 3 [0, 2] [0] :=
  constant_first_letter_forces_prefix_coincidence (0 : Fin 3) [2] []

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 1 (d=3 letters, constant first letter 0). -/
theorem constant_first_letter_instance_3 :
    @hasCoincidencePrefix 3 [0, 0, 1, 2] [0, 0, 2] :=
  constant_first_letter_forces_prefix_coincidence (0 : Fin 3) [0, 1, 2] [0, 2]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 2 (d=3 letters, constant first letter 0). -/
theorem constant_first_letter_instance_4 :
    @hasCoincidencePrefix 3 [0, 0, 1, 2] [0] :=
  constant_first_letter_forces_prefix_coincidence (0 : Fin 3) [0, 1, 2] []

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 1 and 2 (d=3 letters, constant first letter 0). -/
theorem constant_first_letter_instance_5 :
    @hasCoincidencePrefix 3 [0, 0, 2] [0] :=
  constant_first_letter_forces_prefix_coincidence (0 : Fin 3) [0, 2] []

/- Semantic proof graph for: constant_first_letter_batch
  [0] lean.constant_first_letter_certificate :: d=3 c=0 d=3 letters, constant first letter 0 -- instantiates constant_first_letter_forces_prefix_coincidence
  [1] lean.constant_first_letter_certificate :: d=3 c=0 d=3 letters, constant first letter 0 -- instantiates constant_first_letter_forces_prefix_coincidence
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
