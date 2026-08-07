import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

def abelianize {d : ℕ} : List (Fin d) → (Fin d → ℤ)
  | [] => fun _ => 0
  | (a :: rest) => fun j => abelianize rest j + if j = a then 1 else 0

def sufSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)
  | [] => []
  | (_ :: rest) => abelianize rest :: sufSeq rest

def suffixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=
  w.zip (sufSeq w)

def hasCoincidenceSuffix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=
  ∃ p, p ∈ suffixPairs w1 ∧ p ∈ suffixPairs w2

theorem mem_suffixPairs_append_singleton {d : ℕ} (w : List (Fin d)) (c : Fin d) :
    (c, fun _ => (0 : ℤ)) ∈ suffixPairs (w ++ [c]) := by
  induction w with
  | nil => simp [suffixPairs, sufSeq, abelianize]
  | cons a w' ih =>
      simp only [List.cons_append, suffixPairs, sufSeq, List.zip_cons_cons, List.mem_cons]
      exact Or.inr ih

/-- Finding 38: if `w1` and `w2` both END with the same letter `c`, the pair
    `(c, zeroVector)` is recorded in BOTH lists' `suffixPairs`. Reproduced from
    the independently kernel-checked
    `lean/constant_last_letter_forces_suffix_coincidence.lean` (not re-derived
    here). -/
theorem constant_last_letter_forces_suffix_coincidence
    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :
    hasCoincidenceSuffix (w1' ++ [c]) (w2' ++ [c]) :=
  ⟨(c, fun _ => 0), mem_suffixPairs_append_singleton w1' c, mem_suffixPairs_append_singleton w2' c⟩

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 1 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_0 :
    @hasCoincidenceSuffix 4 [2, 3, 1, 0] [1, 0, 3, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [2, 3, 1] [1, 0, 3]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 2 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_1 :
    @hasCoincidenceSuffix 4 [2, 3, 1, 0] [3, 3, 0, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [2, 3, 1] [3, 3, 0]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 3 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_2 :
    @hasCoincidenceSuffix 4 [2, 3, 1, 0] [0, 3, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [2, 3, 1] [0, 3]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 1 and 2 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_3 :
    @hasCoincidenceSuffix 4 [1, 0, 3, 0] [3, 3, 0, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [1, 0, 3] [3, 3, 0]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 1 and 3 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_4 :
    @hasCoincidenceSuffix 4 [1, 0, 3, 0] [0, 3, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [1, 0, 3] [0, 3]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 2 and 3 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_5 :
    @hasCoincidenceSuffix 4 [3, 3, 0, 0] [0, 3, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [3, 3, 0] [0, 3]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 1 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_6 :
    @hasCoincidenceSuffix 4 [3, 1, 0, 0] [1, 2, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [3, 1, 0] [1, 2]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 2 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_7 :
    @hasCoincidenceSuffix 4 [3, 1, 0, 0] [0, 2, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [3, 1, 0] [0, 2]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 0 and 3 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_8 :
    @hasCoincidenceSuffix 4 [3, 1, 0, 0] [2, 0, 2, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [3, 1, 0] [2, 0, 2]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 1 and 2 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_9 :
    @hasCoincidenceSuffix 4 [1, 2, 0] [0, 2, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [1, 2] [0, 2]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 1 and 3 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_10 :
    @hasCoincidenceSuffix 4 [1, 2, 0] [2, 0, 2, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [1, 2] [2, 0, 2]

/-- Mechanically emitted: instantiates the general lemma above for
    this substitution's own images 2 and 3 (d=4 letters, constant last letter 0). -/
theorem constant_last_letter_instance_11 :
    @hasCoincidenceSuffix 4 [0, 2, 0] [2, 0, 2, 0] :=
  constant_last_letter_forces_suffix_coincidence (0 : Fin 4) [0, 2] [2, 0, 2]

/- Semantic proof graph for: constant_last_letter_batch
  [0] lean.constant_last_letter_certificate :: d=4 c=0 d=4 letters, constant last letter 0 -- instantiates constant_last_letter_forces_suffix_coincidence
  [1] lean.constant_last_letter_certificate :: d=4 c=0 d=4 letters, constant last letter 0 -- instantiates constant_last_letter_forces_suffix_coincidence
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
