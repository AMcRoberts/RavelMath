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
    `first_letter_orbit_collision_forces_coincidence`. Reproduced from the
    independently kernel-checked `lean/last_letter_orbit_coincidence.lean`
    (not re-derived here). -/
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

/-- Mechanically emitted: instantiates the general lemma above for
    letters 1 and 2 on a 3-letter alphabet (1 collision steps). -/
theorem last_letter_orbit_instance_0 :
    hasCoincidenceSuffix
      (applyN (![[1, 2], [0], [0]] : Fin 3 → List (Fin 3)) 1 [(1 : Fin 3)])
      (applyN (![[1, 2], [0], [0]] : Fin 3 → List (Fin 3)) 1 [(2 : Fin 3)]) :=
  last_letter_orbit_collision_forces_coincidence (![[1, 2], [0], [0]] : Fin 3 → List (Fin 3)) (by decide)
    (1 : Fin 3) (2 : Fin 3) 1 (by decide)

/- Semantic proof graph for: last_letter_orbit_batch2
  [0] lean.last_letter_orbit_certificate :: d=3 i=1 j=2 k=1 letters 1 and 2 on a 3-letter alphabet -- instantiates last_letter_orbit_collision_forces_coincidence
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
