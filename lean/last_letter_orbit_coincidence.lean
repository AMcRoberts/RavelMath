-- Author: Ravel
-- Date: 2026-08-07
--
-- The exact dual of first_letter_orbit_coincidence.lean: tracks only
-- the LAST letter of each letter's iterated substitution image,
-- closing the SUFFIX-side general case the same way the prefix side
-- was closed -- covering mixed in-run/out-of-run and cross-run pairs
-- via the SUFFIX half of `pair_has_coincidence`, exactly dual to
-- Finding 38's own relationship to Finding 17.

import Mathlib

namespace RavelGenerated

def applyOnce {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) : List (Fin d) :=
  w.flatMap sigma

def applyN {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) : List (Fin d) :=
  (applyOnce sigma)^[k] w

theorem applyN_succ {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) :
    applyN sigma (k + 1) w = applyOnce sigma (applyN sigma k w) := by
  simp [applyN, Function.iterate_succ_apply']

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

theorem constant_last_letter_forces_suffix_coincidence
    {d : ℕ} (c : Fin d) (w1' w2' : List (Fin d)) :
    hasCoincidenceSuffix (w1' ++ [c]) (w2' ++ [c]) :=
  ⟨(c, fun _ => 0), mem_suffixPairs_append_singleton w1' c, mem_suffixPairs_append_singleton w2' c⟩

variable {d : ℕ} [Inhabited (Fin d)]

/-- The LAST letter of a word's image, well-defined via `List.getLastI`
(`Inhabited` junk default, never hit for genuine nonempty images). -/
def lastLetterMap (sigma : Fin d → List (Fin d)) : Fin d → Fin d :=
  fun a => (sigma a).getLastI

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

/-- The LAST letter of `applyOnce sigma w` is `lastLetterMap sigma`
applied to the LAST letter of `w`: writing `w = l ++ [a]` (via
`List.reverseRecOn`), `applyOnce sigma w = applyOnce sigma l ++ sigma a`,
and since `sigma a` is nonempty, the last letter of the whole
concatenation is the last letter of `sigma a`. -/
theorem applyOnce_getLastI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (w : List (Fin d)) (hw : w ≠ []) :
    (applyOnce sigma w).getLastI = lastLetterMap sigma w.getLastI := by
  induction w using List.reverseRecOn with
  | nil => exact absurd rfl hw
  | append_singleton l a _ =>
      have hlast : (l ++ [a]).getLastI = a := by
        rw [List.getLastI_eq_getLast?_getD, List.getLast?_append_of_ne_nil _ (by simp)]
        simp
      have hstep : (applyOnce sigma l ++ sigma a).getLastI = (sigma a).getLastI := by
        rw [List.getLastI_eq_getLast?_getD, List.getLast?_append_of_ne_nil _ (hne a),
            ← List.getLastI_eq_getLast?_getD]
      simp only [applyOnce, List.flatMap_append, List.flatMap_cons, List.flatMap_nil,
        List.append_nil] at hstep ⊢
      rw [hstep, hlast]
      rfl

theorem applyN_getLastI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (a : Fin d) (k : ℕ) :
    (applyN sigma k [a]).getLastI = (lastLetterMap sigma)^[k] a := by
  induction k with
  | zero => rfl
  | succ k ih =>
      rw [applyN_succ, applyOnce_getLastI sigma hne _ (applyN_ne_nil sigma hne k [a] (by simp)), ih,
        Function.iterate_succ_apply']

/-- Finding 39/41's suffix-side general case, dual to
`first_letter_orbit_collision_forces_coincidence`: if two letters'
`lastLetterMap` orbits collide at depth `K`, their depth-`K`
substitution images share a LAST letter, hence exhibit suffix
coincidence. -/
theorem last_letter_orbit_collision_forces_coincidence
    (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (i j : Fin d) (k : ℕ) (hcollide : (lastLetterMap sigma)^[k] i = (lastLetterMap sigma)^[k] j) :
    hasCoincidenceSuffix (applyN sigma k [i]) (applyN sigma k [j]) := by
  have hi : (applyN sigma k [i]).getLastI = (lastLetterMap sigma)^[k] i := applyN_getLastI sigma hne i k
  have hj : (applyN sigma k [j]).getLastI = (lastLetterMap sigma)^[k] j := applyN_getLastI sigma hne j k
  have hine : applyN sigma k [i] ≠ [] := applyN_ne_nil sigma hne k [i] (by simp)
  have hjne : applyN sigma k [j] ≠ [] := applyN_ne_nil sigma hne k [j] (by simp)
  induction hw1 : applyN sigma k [i] using List.reverseRecOn with
  | nil => exact absurd hw1 hine
  | append_singleton w1' a _ =>
      induction hw2 : applyN sigma k [j] using List.reverseRecOn with
      | nil => exact absurd hw2 hjne
      | append_singleton w2' b _ =>
          have ha : a = (lastLetterMap sigma)^[k] i := by
            rw [← hi, hw1]; simp [List.getLastI_eq_getLast?_getD]
          have hb : b = (lastLetterMap sigma)^[k] j := by
            rw [← hj, hw2]; simp [List.getLastI_eq_getLast?_getD]
          have hab : a = b := by rw [ha, hb, hcollide]
          rw [hab]
          exact constant_last_letter_forces_suffix_coincidence b w1' w2'

end RavelGenerated
