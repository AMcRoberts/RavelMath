-- Author: Ravel
-- Date: 2026-08-07
--
-- Closes Finding 39/41's GENERAL case (not just the same-chain special
-- case in zero_run_same_chain_coincidence.lean): the first letter of
-- ANY word's iterated substitution image depends ONLY on the first
-- letter of the original word, via iterating a simple functional map
-- on the (finite) alphabet -- reducing "do these two letters
-- eventually coincide" to "do these two orbits of a function
-- Fin d -> Fin d eventually collide", independent of chain/growth
-- bookkeeping entirely.

import Mathlib

namespace RavelGenerated

def applyOnce {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) : List (Fin d) :=
  w.flatMap sigma

def applyN {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) : List (Fin d) :=
  (applyOnce sigma)^[k] w

theorem applyN_succ {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) :
    applyN sigma (k + 1) w = applyOnce sigma (applyN sigma k w) := by
  simp [applyN, Function.iterate_succ_apply']

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

variable {d : ℕ} [Inhabited (Fin d)]

/-- Every letter's image, read only at its first letter -- well-defined
via `List.headI` (uses `Inhabited (Fin d)` as a junk default, never hit
in practice since `sigma a` is always nonempty for a genuine
substitution, `hne`). -/
def firstLetterMap (sigma : Fin d → List (Fin d)) : Fin d → Fin d :=
  fun a => (sigma a).headI

/-- `applyOnce` preserves nonemptiness, given every image is nonempty. -/
theorem applyOnce_ne_nil (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (w : List (Fin d)) (hw : w ≠ []) : applyOnce sigma w ≠ [] := by
  cases w with
  | nil => exact absurd rfl hw
  | cons a rest => simp [applyOnce, hne a]

theorem applyN_ne_nil (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (k : ℕ) (w : List (Fin d)) (hw : w ≠ []) : applyN sigma k w ≠ [] := by
  induction k with
  | zero => exact hw
  | succ k ih => rw [applyN_succ]; exact applyOnce_ne_nil sigma hne _ ih

/-- The head of `applyOnce sigma w` is `firstLetterMap sigma` applied to
the head of `w` -- since `applyOnce sigma (a :: rest) = sigma a ++
applyOnce sigma rest`, and `sigma a` is nonempty, the head of the whole
concatenation is the head of `sigma a`. -/
theorem applyOnce_headI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (w : List (Fin d)) (hw : w ≠ []) :
    (applyOnce sigma w).headI = firstLetterMap sigma w.headI := by
  cases w with
  | nil => exact absurd rfl hw
  | cons a rest =>
      obtain ⟨x, xs, hxs⟩ := List.exists_cons_of_ne_nil (hne a)
      simp [applyOnce, hxs, firstLetterMap]

/-- THE KEY LEMMA: the head of the K-fold image of `[a]` is exactly the
K-th iterate of `firstLetterMap` at `a` -- reducing "do these two
letters' iterated images share a first letter at some depth" entirely
to orbit collision in the finite functional graph `firstLetterMap`. -/
theorem applyN_headI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (a : Fin d) (k : ℕ) :
    (applyN sigma k [a]).headI = (firstLetterMap sigma)^[k] a := by
  induction k with
  | zero => rfl
  | succ k ih =>
      rw [applyN_succ, applyOnce_headI sigma hne _ (applyN_ne_nil sigma hne k [a] (by simp)), ih,
        Function.iterate_succ_apply']

/-- THE MAIN THEOREM, closing Finding 39/41's general case: if two
letters' `firstLetterMap` orbits collide at depth `K` (`firstLetterMap^[K]
i = firstLetterMap^[K] j` -- ALWAYS eventually true for some K, by
pigeonhole on the finite alphabet, though this file does not prove
that existence separately), their depth-`K` substitution images share
a first letter, hence exhibit prefix coincidence. This subsumes
`same_chain_forces_coincidence` (zero_run_same_chain_coincidence.lean)
as the special case where the orbits are literal chain positions, and
additionally covers mixed in-run/out-of-run and cross-run pairs, since
it never assumes either letter is on any particular kind of chain --
only that SOME depth exists where the first-letter orbits agree. -/
theorem first_letter_orbit_collision_forces_coincidence
    (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (i j : Fin d) (k : ℕ) (hcollide : (firstLetterMap sigma)^[k] i = (firstLetterMap sigma)^[k] j) :
    hasCoincidencePrefix (applyN sigma k [i]) (applyN sigma k [j]) := by
  have hi : (applyN sigma k [i]).headI = (firstLetterMap sigma)^[k] i := applyN_headI sigma hne i k
  have hj : (applyN sigma k [j]).headI = (firstLetterMap sigma)^[k] j := applyN_headI sigma hne j k
  have hine : applyN sigma k [i] ≠ [] := applyN_ne_nil sigma hne k [i] (by simp)
  have hjne : applyN sigma k [j] ≠ [] := applyN_ne_nil sigma hne k [j] (by simp)
  obtain ⟨a, w1', hw1⟩ := List.exists_cons_of_ne_nil hine
  obtain ⟨b, w2', hw2⟩ := List.exists_cons_of_ne_nil hjne
  have ha : a = (firstLetterMap sigma)^[k] i := by rw [← hi, hw1]; simp
  have hb : b = (firstLetterMap sigma)^[k] j := by rw [← hj, hw2]; simp
  have hab : a = b := by rw [ha, hb, hcollide]
  rw [hw1, hw2, hab]
  exact constant_first_letter_forces_prefix_coincidence b w1' w2'

end RavelGenerated
