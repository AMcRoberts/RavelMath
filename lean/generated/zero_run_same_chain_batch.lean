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

theorem pass_through_chain_synchronizes {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)
    (hchain : ∀ s, sigma s = [next s]) (s1 s2 : Fin d) (k1 k2 : ℕ)
    (hmeet : next^[k1] s1 = next^[k2] s2) :
    applyN sigma k1 [s1] = applyN sigma k2 [s2] := by
  rw [applyN_singleton_chain sigma next hchain s1 k1,
      applyN_singleton_chain sigma next hchain s2 k2, hmeet]

theorem identical_words_have_prefix_coincidence {d : ℕ} (c : Fin d) (w' : List (Fin d)) :
    hasCoincidencePrefix (c :: w') (c :: w') :=
  constant_first_letter_forces_prefix_coincidence c w' w'

/-- Finding 39/41's "same-chain" special case: two letters inside the same
    pass-through zero-run, converging onto the same terminal letter at possibly
    different depths, exhibit prefix coincidence. Reproduced from the
    independently kernel-checked `lean/zero_run_same_chain_coincidence.lean`
    (not re-derived here). -/
theorem same_chain_forces_coincidence {d : ℕ} (sigma : Fin d → List (Fin d)) (next : Fin d → Fin d)
    (hchain : ∀ s, sigma s = [next s]) (s1 s2 : Fin d) (k1 k2 : ℕ)
    (hmeet : next^[k1] s1 = next^[k2] s2) :
    hasCoincidencePrefix (applyN sigma k1 [s1]) (applyN sigma k2 [s2]) := by
  rw [pass_through_chain_synchronizes sigma next hchain s1 s2 k1 k2 hmeet,
      applyN_singleton_chain sigma next hchain s2 k2]
  exact identical_words_have_prefix_coincidence (next^[k2] s2) []

/-- Mechanically emitted: instantiates the general lemma above for
    zero-run starting at digit index 1, length 3. -/
theorem zero_run_same_chain_instance_0 :
    hasCoincidencePrefix
      (applyN (fun s => [(![1, 2, 3, 3] : Fin 4 → Fin 4) s]) 3 [(0 : Fin 4)])
      (applyN (fun s => [(![1, 2, 3, 3] : Fin 4 → Fin 4) s]) 1 [(2 : Fin 4)]) :=
  same_chain_forces_coincidence (fun s => [(![1, 2, 3, 3] : Fin 4 → Fin 4) s]) (![1, 2, 3, 3] : Fin 4 → Fin 4) (fun _ => rfl)
    (0 : Fin 4) (2 : Fin 4) 3 1 (by decide)

/- Semantic proof graph for: zero_run_same_chain_batch
  [0] lean.zero_run_same_chain_certificate :: R=3 offsets(0,2) zero-run starting at digit index 1, length 3 -- instantiates same_chain_forces_coincidence
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
