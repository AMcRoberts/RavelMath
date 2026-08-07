-- Author: Ravel
-- Date: 2026-08-07
--
-- General, reusable infrastructure for iterated substitution
-- application, built as the foundation stone for retrofitting Finding
-- 39/41 (the zero-run coincidence-depth bound) -- NOT that theorem
-- itself. `check_strong_coincidence`'s own depth-K search
-- (`include/adelic/coincidence_and_property_f.hpp`) works by applying
-- the substitution to each letter-word K times and checking
-- `pair_has_coincidence` at each depth; formalizing Finding 39/41
-- honestly requires exactly this iteration to be modeled first,
-- before any zero-run-specific argument can even be stated.
--
-- Scope, stated honestly: this file proves the substitution-application
-- primitive is well-behaved (distributes over append/concatenation,
-- and singleton "pass-through" letters chain predictably under
-- iteration) -- both genuinely reusable facts, independent of the
-- zero-run structure. It does NOT yet state or prove Finding 39/41's
-- own exact-depth claim, which additionally needs the
-- constant-first-letter machinery
-- (`lean/constant_first_letter_forces_prefix_coincidence.lean`)
-- composed with the chaining lemma below at the specific depth where
-- a zero-run ends -- left for the next session/pass.

import Mathlib

namespace RavelGenerated

/-- One step of substitution application: replace every letter of `w`
by its image under `sigma`, concatenated in order -- exactly
`List.bind`, spelled out under its own name so future proofs about
this project's substitutions read naturally. -/
def applyOnce {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) : List (Fin d) :=
  w.flatMap sigma

/-- `applyOnce` distributes over concatenation -- the basic fact that
lets a word's substitution image be computed letter-by-letter and
reassembled, needed for any argument (like Finding 39/41's) that
splits a word into pieces and tracks each piece's image separately. -/
theorem applyOnce_append {d : ℕ} (sigma : Fin d → List (Fin d)) (w1 w2 : List (Fin d)) :
    applyOnce sigma (w1 ++ w2) = applyOnce sigma w1 ++ applyOnce sigma w2 := by
  simp [applyOnce, List.flatMap_append]

/-- `K` iterations of substitution application. -/
def applyN {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) : List (Fin d) :=
  (applyOnce sigma)^[k] w

theorem applyN_zero {d : ℕ} (sigma : Fin d → List (Fin d)) (w : List (Fin d)) :
    applyN sigma 0 w = w := rfl

theorem applyN_succ {d : ℕ} (sigma : Fin d → List (Fin d)) (k : ℕ) (w : List (Fin d)) :
    applyN sigma (k + 1) w = applyOnce sigma (applyN sigma k w) := by
  simp [applyN, Function.iterate_succ_apply']

/-- THE PASS-THROUGH CHAINING LEMMA -- the mechanism Finding 39/41's
own informal proof names explicitly ("a chain of k such non-branching
letters delays reaching a genuine leading-zero image by exactly k
steps"): if a letter `s` has a SINGLETON image `sigma s = [s']`
(exactly the zero-digit case, `sigma(s) = 0^0 (s+1) = [s+1]`), then
applying `sigma` once to the singleton word `[s]` gives `[s']` again,
so a chain of such letters propagates a singleton word forward,
one step at a time, indefinitely. -/
theorem applyOnce_singleton_of_passthrough {d : ℕ} (sigma : Fin d → List (Fin d))
    (s s' : Fin d) (h : sigma s = [s']) :
    applyOnce sigma [s] = [s'] := by
  simp [applyOnce, h]

/-- Iterated version: given a CHAIN function `next : Fin d → Fin d`
such that every letter along the chain passes through
(`sigma s = [next s]` for `s` in the chain), the singleton word `[s0]`
becomes `[next^[k] s0]` after `k` applications -- proved by induction
on `k`, composing `applyOnce_singleton_of_passthrough` at each step. -/
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

end RavelGenerated
