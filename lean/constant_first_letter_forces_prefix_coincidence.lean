-- Author: Ravel
-- Date: 2026-08-07

import Mathlib

namespace RavelGenerated

/-- The sequence of "running" abelianized-count vectors observed
BEFORE each letter of `w` is processed -- a faithful model of the
`running` accumulator in `adelic::pair_has_coincidence`'s own loop
(`include/adelic/coincidence_and_property_f.hpp`): `running` starts at
the zero vector and is only incremented AFTER the current letter's
prefix state is recorded, so `runningSeq w` has the SAME length as
`w`, with head always the zero vector whenever `w` is nonempty. -/
def runningSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)
  | [] => []
  | (a :: rest) =>
      (fun _ => (0 : ℤ)) :: (runningSeq rest).map (fun v => fun j => v j + if j = a then 1 else 0)

/-- The (letter, running-vector) pairs recorded while scanning `w` --
a faithful model of `prefix_set1` (a `Set` per letter in the C++, here
a single list of pairs; membership is what matters, not
deduplication). -/
def prefixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=
  w.zip (runningSeq w)

/-- The PREFIX half of `pair_has_coincidence`: true iff some position
in `w2`'s own scan lands on a (letter, running-vector) pair that was
ALSO recorded somewhere in `w1`'s scan -- exactly
`prefix_set1[a].count(running)` firing for some `a`, `running` pair
drawn from `w2`. (The SUFFIX half is a structurally symmetric fact,
not modeled here.) -/
def hasCoincidencePrefix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=
  ∃ p, p ∈ prefixPairs w1 ∧ p ∈ prefixPairs w2

/-- Finding 17's theorem, proved directly from the definitions above
(not assumed): if `w1` and `w2` both start with the same letter `c`,
the pair `(c, zeroVector)` is recorded in BOTH `prefixPairs w1` and
`prefixPairs w2` (it is always the head of `prefixPairs` for any
nonempty list, by `runningSeq`'s own base case), so the prefix
coincidence fires immediately -- unconditionally, for ANY tails
`w1'`, `w2'`, no Pisot or unimodularity hypothesis anywhere. -/
theorem constant_first_letter_forces_prefix_coincidence
    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :
    hasCoincidencePrefix (c :: w1') (c :: w2') := by
  refine ⟨(c, fun _ => (0 : ℤ)), ?_, ?_⟩ <;>
    simp [prefixPairs, runningSeq]

end RavelGenerated
