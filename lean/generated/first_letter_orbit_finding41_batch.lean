import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

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

def firstLetterMap (sigma : Fin d → List (Fin d)) : Fin d → Fin d :=
  fun a => (sigma a).headI

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

theorem applyOnce_headI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (w : List (Fin d)) (hw : w ≠ []) :
    (applyOnce sigma w).headI = firstLetterMap sigma w.headI := by
  cases w with
  | nil => exact absurd rfl hw
  | cons a rest =>
      obtain ⟨x, xs, hxs⟩ := List.exists_cons_of_ne_nil (hne a)
      simp [applyOnce, hxs, firstLetterMap]

theorem applyN_headI (sigma : Fin d → List (Fin d)) (hne : ∀ a, sigma a ≠ [])
    (a : Fin d) (k : ℕ) :
    (applyN sigma k [a]).headI = (firstLetterMap sigma)^[k] a := by
  induction k with
  | zero => rfl
  | succ k ih =>
      rw [applyN_succ, applyOnce_headI sigma hne _ (applyN_ne_nil sigma hne k [a] (by simp)), ih,
        Function.iterate_succ_apply']

/-- Finding 39/41's general case: if two letters' `firstLetterMap` orbits
    collide at depth `K`, their depth-`K` substitution images share a first
    letter, hence exhibit prefix coincidence. Reproduced from the
    independently kernel-checked `lean/first_letter_orbit_coincidence.lean`
    (not re-derived here). -/
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

/-- Mechanically emitted: instantiates the general lemma above for
    letters 0 and 3 on a 4-letter alphabet (3 collision steps). -/
theorem first_letter_orbit_instance_0 :
    hasCoincidencePrefix
      (applyN (![[0, 1], [2], [0, 3], [1]] : Fin 4 → List (Fin 4)) 3 [(0 : Fin 4)])
      (applyN (![[0, 1], [2], [0, 3], [1]] : Fin 4 → List (Fin 4)) 3 [(3 : Fin 4)]) :=
  first_letter_orbit_collision_forces_coincidence (![[0, 1], [2], [0, 3], [1]] : Fin 4 → List (Fin 4)) (by decide)
    (0 : Fin 4) (3 : Fin 4) 3 (by decide)

/-- Mechanically emitted: instantiates the general lemma above for
    letters 1 and 3 on a 4-letter alphabet (3 collision steps). -/
theorem first_letter_orbit_instance_1 :
    hasCoincidencePrefix
      (applyN (![[0, 1], [2], [0, 3], [1]] : Fin 4 → List (Fin 4)) 3 [(1 : Fin 4)])
      (applyN (![[0, 1], [2], [0, 3], [1]] : Fin 4 → List (Fin 4)) 3 [(3 : Fin 4)]) :=
  first_letter_orbit_collision_forces_coincidence (![[0, 1], [2], [0, 3], [1]] : Fin 4 → List (Fin 4)) (by decide)
    (1 : Fin 4) (3 : Fin 4) 3 (by decide)

/- Semantic proof graph for: first_letter_orbit_finding41_batch
  [0] lean.first_letter_orbit_certificate :: d=4 i=0 j=3 k=3 letters 0 and 3 on a 4-letter alphabet -- instantiates first_letter_orbit_collision_forces_coincidence
  [1] lean.first_letter_orbit_certificate :: d=4 i=1 j=3 k=3 letters 1 and 3 on a 4-letter alphabet -- instantiates first_letter_orbit_collision_forces_coincidence
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
