-- Author: Ravel
-- Date: 2026-08-07
--
-- Closes a real, honestly-scoped SPECIAL CASE of Finding 39/41 (the
-- zero-run coincidence-depth bound): two letters inside the SAME
-- maximal zero-run of a canonical terminating substitution
-- synchronize onto the IDENTICAL single-letter word after enough
-- applications (not merely a shared first letter), which trivially
-- gives coincidence -- exactly the "pairs straddling that run achieve
-- the bound exactly" case the original informal proof names.
--
-- SCOPE, stated honestly: this covers pairs of letters BOTH inside
-- the same pass-through chain. It does NOT yet cover the harder case
-- (a letter inside a run paired with a letter outside it, or two
-- letters in DIFFERENT runs), which needs composing this file's
-- `applyN_singleton_chain` with `constant_first_letter_forces_prefix_
-- coincidence` at the moment a nonzero digit is finally reached (the
-- image is no longer a singleton) -- left for the next pass, exactly
-- as `substitution_iteration_infrastructure.lean`'s own scope note
-- says.

import Mathlib

namespace RavelGenerated

-- Reproduces `applyOnce`/`applyN`/`applyN_singleton_chain`
-- (`lean/substitution_iteration_infrastructure.lean`, independently
-- kernel-checked) and `prefixPairs`/`hasCoincidencePrefix`
-- (`lean/constant_first_letter_forces_prefix_coincidence.lean`,
-- independently kernel-checked) verbatim, so this file is
-- self-contained -- matching every other file in this project's
-- reflection tier.

def applyOnce {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) : List (Fin d) :=
  w.flatMap sigma

def applyN {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) : List (Fin d) :=
  (applyOnce sigma)^[k] w

theorem applyN_succ {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) :
    applyN sigma (k + 1) w = applyOnce sigma (applyN sigma k w) := by
  simp [applyN, Function.iterate_succ_apply']

theorem applyOnce_singleton_of_passthrough {d : ℕ} (sigma : Fin d → List (Fin d))
    (s s' : Fin d) (h : sigma s = [s']) :
    applyOnce sigma [s] = [s'] := by
  simp [applyOnce, h]

theorem applyN_singleton_chain {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)
    (hchain : ∀ s, sigma s = [next s]) (s0 : Fin d) (k : ℕ) :
    applyN sigma k [s0] = [next^[k] s0] := by
  induction k with
  | zero => rfl
  | succ k ih =>
      have hs : sigma (next^[k] s0) = [next^[k + 1] s0] := by
        rw [hchain (next^[k] s0)]
        congr 1
        exact (Function.iterate_succ_apply' next k s0).symm
      rw [applyN_succ, ih, applyOnce_singleton_of_passthrough sigma (next^[k] s0) (next^[k + 1] s0) hs]

def runningSeq {d : ℕ} : List (Fin d) → List (Fin d → ℤ)
  | [] => []
  | (a :: rest) =>
      (fun _ => (0 : ℤ)) :: (runningSeq rest).map (fun v => fun j => v j + if j = a then 1 else 0)

def prefixPairs {d : ℕ} (w : List (Fin d)) : List (Fin d × (Fin d → ℤ)) :=
  w.zip (runningSeq w)

def hasCoincidencePrefix {d : ℕ} (w1 w2 : List (Fin d)) : Prop :=
  ∃ p, p ∈ prefixPairs w1 ∧ p ∈ prefixPairs w2

theorem constant_first_letter_forces_prefix_coincidence
    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :
    hasCoincidencePrefix (c :: w1') (c :: w2') := by
  refine ⟨(c, fun _ => (0 : ℤ)), ?_, ?_⟩ <;>
    simp [prefixPairs, runningSeq]

-- The new content: chain synchronization + the coincidence consequence.

/-- Two letters BOTH inside the same pass-through chain, converging to
the SAME terminal letter `c` (possibly at DIFFERENT depths `k1`, `k2`
-- exactly the "different distances into the same run" situation), have
IDENTICAL images under substitution at their respective depths. -/
theorem pass_through_chain_synchronizes {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)
    (hchain : ∀ s, sigma s = [next s]) (s1 s2 : Fin d) (k1 k2 : ℕ)
    (hmeet : next^[k1] s1 = next^[k2] s2) :
    applyN sigma k1 [s1] = applyN sigma k2 [s2] := by
  rw [applyN_singleton_chain sigma next hchain s1 k1,
      applyN_singleton_chain sigma next hchain s2 k2, hmeet]

/-- Identical nonempty words trivially exhibit prefix coincidence --
the degenerate instance of `constant_first_letter_forces_prefix_
coincidence` with `w1' = w2'` and `c` the shared head. -/
theorem identical_words_have_prefix_coincidence {d : ℕ} (c : Fin d) (w' : List (Fin d)) :
    hasCoincidencePrefix (c :: w') (c :: w') :=
  constant_first_letter_forces_prefix_coincidence c w' w'

/-- THE MAIN RESULT of this file: two letters inside the same
pass-through chain, converging onto the same terminal letter (at
possibly different depths), exhibit prefix coincidence at their
respective depths. This is the "achieved exactly" case of Finding
39/41 -- two letters straddling a zero-run, at distances `k1 != k2`
from the run's end, coincide once both reach the run's terminal
letter, exactly matching the informal proof's own account of why the
bound is tight. -/
theorem same_chain_forces_coincidence {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)
    (hchain : ∀ s, sigma s = [next s]) (s1 s2 : Fin d) (k1 k2 : ℕ)
    (hmeet : next^[k1] s1 = next^[k2] s2) :
    hasCoincidencePrefix (applyN sigma k1 [s1]) (applyN sigma k2 [s2]) := by
  rw [pass_through_chain_synchronizes sigma next hchain s1 s2 k1 k2 hmeet,
      applyN_singleton_chain sigma next hchain s2 k2]
  exact identical_words_have_prefix_coincidence (next^[k2] s2) []

end RavelGenerated
