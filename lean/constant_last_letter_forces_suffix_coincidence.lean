-- Author: Ravel
-- Date: 2026-08-07

import Mathlib

namespace RavelGenerated

/-- Full abelianization (total letter-count vector) of a list -- the
`total1`/`total2` accumulators in `adelic::pair_has_coincidence`. -/
def abelianize {d : ℕ} : List (Fin d) → (Fin d → ℤ)
  | [] => fun _ => 0
  | (a :: rest) => fun j => abelianize rest j + if j = a then 1 else 0

/-- The sequence of suffix vectors recorded AFTER each letter of `w`:
at position `i`, `total - running[i] - e_{w[i]}` in the C++ -- exactly
the abelianization of everything strictly AFTER position `i`. -/
def sufSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)
  | [] => []
  | (_ :: rest) => abelianize rest :: sufSeq rest

/-- The (letter, suffix-vector) pairs recorded while scanning `w` --
a faithful model of `suffix_set1`. -/
def suffixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=
  w.zip (sufSeq w)

/-- The SUFFIX half of `pair_has_coincidence`. -/
def hasCoincidenceSuffix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=
  ∃ p, p ∈ suffixPairs w1 ∧ p ∈ suffixPairs w2

/-- The last position of `w ++ [c]` always records `(c, zeroVector)` in
`suffixPairs` -- the suffix AFTER the very last letter is always
empty, hence the zero vector. Proved by induction on `w`. -/
theorem mem_suffixPairs_append_singleton {d : ℕ} (w : List (Fin d)) (c : Fin d) :
    (c, fun _ => (0 : ℤ)) ∈ suffixPairs (w ++ [c]) := by
  induction w with
  | nil => simp [suffixPairs, sufSeq, abelianize]
  | cons a w' ih =>
      simp only [List.cons_append, suffixPairs, sufSeq, List.zip_cons_cons, List.mem_cons]
      exact Or.inr ih

/-- Finding 38: if `w1` and `w2` both END with the same letter `c`,
the pair `(c, zeroVector)` is recorded in BOTH lists' `suffixPairs`
(at their respective last positions), so the suffix coincidence fires
-- unconditionally, for ANY heads `w1'`, `w2'`. Exact dual of
`constant_first_letter_forces_prefix_coincidence`
(`lean/constant_first_letter_forces_prefix_coincidence.lean`). -/
theorem constant_last_letter_forces_suffix_coincidence
    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :
    hasCoincidenceSuffix (w1' ++ [c]) (w2' ++ [c]) :=
  ⟨(c, fun _ => 0), mem_suffixPairs_append_singleton w1' c, mem_suffixPairs_append_singleton w2' c⟩

end RavelGenerated
