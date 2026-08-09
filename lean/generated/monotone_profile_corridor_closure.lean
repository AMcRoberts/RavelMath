import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

def corridorDigit (k i : ℕ) : ℕ := if i < k then 2 else 1

theorem corridor_extra_occurrences (D k : ℕ) (hk : k ≤ D) :
    (∑ i ∈ Finset.range D, (if i < k then 1 else 0)) = k := by
  have heq : (∑ i ∈ Finset.range D, (if i < k then 1 else 0)) =
      ((Finset.range D).filter (fun i => i < k)).card := by
    rw [Finset.card_filter]
  rw [heq]
  have : (Finset.range D).filter (fun i => i < k) = Finset.range k := by
    ext i
    simp only [Finset.mem_filter, Finset.mem_range]
    omega
  rw [this, Finset.card_range]

/-- Mechanically emitted: D=2 k=0 D=2 k=0 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_0 :
    (∑ i ∈ Finset.range 2, (if i < 0 then 1 else 0)) = 0 :=
  corridor_extra_occurrences 2 0 (by decide)

/-- Mechanically emitted: D=2 k=1 D=2 k=1 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_1 :
    (∑ i ∈ Finset.range 2, (if i < 1 then 1 else 0)) = 1 :=
  corridor_extra_occurrences 2 1 (by decide)

/-- Mechanically emitted: D=3 k=0 D=3 k=0 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_2 :
    (∑ i ∈ Finset.range 3, (if i < 0 then 1 else 0)) = 0 :=
  corridor_extra_occurrences 3 0 (by decide)

/-- Mechanically emitted: D=3 k=1 D=3 k=1 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_3 :
    (∑ i ∈ Finset.range 3, (if i < 1 then 1 else 0)) = 1 :=
  corridor_extra_occurrences 3 1 (by decide)

/-- Mechanically emitted: D=3 k=2 D=3 k=2 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_4 :
    (∑ i ∈ Finset.range 3, (if i < 2 then 1 else 0)) = 2 :=
  corridor_extra_occurrences 3 2 (by decide)

/-- Mechanically emitted: D=4 k=0 D=4 k=0 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_5 :
    (∑ i ∈ Finset.range 4, (if i < 0 then 1 else 0)) = 0 :=
  corridor_extra_occurrences 4 0 (by decide)

/-- Mechanically emitted: D=4 k=1 D=4 k=1 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_6 :
    (∑ i ∈ Finset.range 4, (if i < 1 then 1 else 0)) = 1 :=
  corridor_extra_occurrences 4 1 (by decide)

/-- Mechanically emitted: D=4 k=2 D=4 k=2 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_7 :
    (∑ i ∈ Finset.range 4, (if i < 2 then 1 else 0)) = 2 :=
  corridor_extra_occurrences 4 2 (by decide)

/-- Mechanically emitted: D=4 k=3 D=4 k=3 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_8 :
    (∑ i ∈ Finset.range 4, (if i < 3 then 1 else 0)) = 3 :=
  corridor_extra_occurrences 4 3 (by decide)

/-- Mechanically emitted: D=5 k=0 D=5 k=0 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_9 :
    (∑ i ∈ Finset.range 5, (if i < 0 then 1 else 0)) = 0 :=
  corridor_extra_occurrences 5 0 (by decide)

/-- Mechanically emitted: D=5 k=1 D=5 k=1 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_10 :
    (∑ i ∈ Finset.range 5, (if i < 1 then 1 else 0)) = 1 :=
  corridor_extra_occurrences 5 1 (by decide)

/-- Mechanically emitted: D=5 k=2 D=5 k=2 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_11 :
    (∑ i ∈ Finset.range 5, (if i < 2 then 1 else 0)) = 2 :=
  corridor_extra_occurrences 5 2 (by decide)

/-- Mechanically emitted: D=5 k=3 D=5 k=3 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_12 :
    (∑ i ∈ Finset.range 5, (if i < 3 then 1 else 0)) = 3 :=
  corridor_extra_occurrences 5 3 (by decide)

/-- Mechanically emitted: D=5 k=4 D=5 k=4 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_13 :
    (∑ i ∈ Finset.range 5, (if i < 4 then 1 else 0)) = 4 :=
  corridor_extra_occurrences 5 4 (by decide)

/-- Mechanically emitted: D=6 k=0 D=6 k=0 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_14 :
    (∑ i ∈ Finset.range 6, (if i < 0 then 1 else 0)) = 0 :=
  corridor_extra_occurrences 6 0 (by decide)

/-- Mechanically emitted: D=6 k=1 D=6 k=1 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_15 :
    (∑ i ∈ Finset.range 6, (if i < 1 then 1 else 0)) = 1 :=
  corridor_extra_occurrences 6 1 (by decide)

/-- Mechanically emitted: D=6 k=2 D=6 k=2 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_16 :
    (∑ i ∈ Finset.range 6, (if i < 2 then 1 else 0)) = 2 :=
  corridor_extra_occurrences 6 2 (by decide)

/-- Mechanically emitted: D=6 k=3 D=6 k=3 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_17 :
    (∑ i ∈ Finset.range 6, (if i < 3 then 1 else 0)) = 3 :=
  corridor_extra_occurrences 6 3 (by decide)

/-- Mechanically emitted: D=6 k=4 D=6 k=4 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_18 :
    (∑ i ∈ Finset.range 6, (if i < 4 then 1 else 0)) = 4 :=
  corridor_extra_occurrences 6 4 (by decide)

/-- Mechanically emitted: D=6 k=5 D=6 k=5 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_19 :
    (∑ i ∈ Finset.range 6, (if i < 5 then 1 else 0)) = 5 :=
  corridor_extra_occurrences 6 5 (by decide)

/-- Mechanically emitted: D=7 k=0 D=7 k=0 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_20 :
    (∑ i ∈ Finset.range 7, (if i < 0 then 1 else 0)) = 0 :=
  corridor_extra_occurrences 7 0 (by decide)

/-- Mechanically emitted: D=7 k=1 D=7 k=1 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_21 :
    (∑ i ∈ Finset.range 7, (if i < 1 then 1 else 0)) = 1 :=
  corridor_extra_occurrences 7 1 (by decide)

/-- Mechanically emitted: D=7 k=2 D=7 k=2 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_22 :
    (∑ i ∈ Finset.range 7, (if i < 2 then 1 else 0)) = 2 :=
  corridor_extra_occurrences 7 2 (by decide)

/-- Mechanically emitted: D=7 k=3 D=7 k=3 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_23 :
    (∑ i ∈ Finset.range 7, (if i < 3 then 1 else 0)) = 3 :=
  corridor_extra_occurrences 7 3 (by decide)

/-- Mechanically emitted: D=7 k=4 D=7 k=4 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_24 :
    (∑ i ∈ Finset.range 7, (if i < 4 then 1 else 0)) = 4 :=
  corridor_extra_occurrences 7 4 (by decide)

/-- Mechanically emitted: D=7 k=5 D=7 k=5 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_25 :
    (∑ i ∈ Finset.range 7, (if i < 5 then 1 else 0)) = 5 :=
  corridor_extra_occurrences 7 5 (by decide)

/-- Mechanically emitted: D=7 k=6 D=7 k=6 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_26 :
    (∑ i ∈ Finset.range 7, (if i < 6 then 1 else 0)) = 6 :=
  corridor_extra_occurrences 7 6 (by decide)

/-- Mechanically emitted: D=8 k=0 D=8 k=0 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_27 :
    (∑ i ∈ Finset.range 8, (if i < 0 then 1 else 0)) = 0 :=
  corridor_extra_occurrences 8 0 (by decide)

/-- Mechanically emitted: D=8 k=1 D=8 k=1 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_28 :
    (∑ i ∈ Finset.range 8, (if i < 1 then 1 else 0)) = 1 :=
  corridor_extra_occurrences 8 1 (by decide)

/-- Mechanically emitted: D=8 k=2 D=8 k=2 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_29 :
    (∑ i ∈ Finset.range 8, (if i < 2 then 1 else 0)) = 2 :=
  corridor_extra_occurrences 8 2 (by decide)

/-- Mechanically emitted: D=8 k=3 D=8 k=3 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_30 :
    (∑ i ∈ Finset.range 8, (if i < 3 then 1 else 0)) = 3 :=
  corridor_extra_occurrences 8 3 (by decide)

/-- Mechanically emitted: D=8 k=4 D=8 k=4 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_31 :
    (∑ i ∈ Finset.range 8, (if i < 4 then 1 else 0)) = 4 :=
  corridor_extra_occurrences 8 4 (by decide)

/-- Mechanically emitted: D=8 k=5 D=8 k=5 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_32 :
    (∑ i ∈ Finset.range 8, (if i < 5 then 1 else 0)) = 5 :=
  corridor_extra_occurrences 8 5 (by decide)

/-- Mechanically emitted: D=8 k=6 D=8 k=6 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_33 :
    (∑ i ∈ Finset.range 8, (if i < 6 then 1 else 0)) = 6 :=
  corridor_extra_occurrences 8 6 (by decide)

/-- Mechanically emitted: D=8 k=7 D=8 k=7 corridor closure cross-check -- ravel::proof::stage_monotone_profile_corridor_closure
    independently reconfirmed 0 <= k <= D-1. -/
theorem monotone_profile_corridor_closure_instance_34 :
    (∑ i ∈ Finset.range 8, (if i < 7 then 1 else 0)) = 7 :=
  corridor_extra_occurrences 8 7 (by decide)

/- Semantic proof graph for: monotone_profile_corridor_closure_batch
  [0] lean.monotone_profile_corridor_closure_certificate :: D=2 k=0 D=2 k=0 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [1] lean.monotone_profile_corridor_closure_certificate :: D=2 k=1 D=2 k=1 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [2] lean.monotone_profile_corridor_closure_certificate :: D=3 k=0 D=3 k=0 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [3] lean.monotone_profile_corridor_closure_certificate :: D=3 k=1 D=3 k=1 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [4] lean.monotone_profile_corridor_closure_certificate :: D=3 k=2 D=3 k=2 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [5] lean.monotone_profile_corridor_closure_certificate :: D=4 k=0 D=4 k=0 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [6] lean.monotone_profile_corridor_closure_certificate :: D=4 k=1 D=4 k=1 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [7] lean.monotone_profile_corridor_closure_certificate :: D=4 k=2 D=4 k=2 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [8] lean.monotone_profile_corridor_closure_certificate :: D=4 k=3 D=4 k=3 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [9] lean.monotone_profile_corridor_closure_certificate :: D=5 k=0 D=5 k=0 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [10] lean.monotone_profile_corridor_closure_certificate :: D=5 k=1 D=5 k=1 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [11] lean.monotone_profile_corridor_closure_certificate :: D=5 k=2 D=5 k=2 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [12] lean.monotone_profile_corridor_closure_certificate :: D=5 k=3 D=5 k=3 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [13] lean.monotone_profile_corridor_closure_certificate :: D=5 k=4 D=5 k=4 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [14] lean.monotone_profile_corridor_closure_certificate :: D=6 k=0 D=6 k=0 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [15] lean.monotone_profile_corridor_closure_certificate :: D=6 k=1 D=6 k=1 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [16] lean.monotone_profile_corridor_closure_certificate :: D=6 k=2 D=6 k=2 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [17] lean.monotone_profile_corridor_closure_certificate :: D=6 k=3 D=6 k=3 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [18] lean.monotone_profile_corridor_closure_certificate :: D=6 k=4 D=6 k=4 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [19] lean.monotone_profile_corridor_closure_certificate :: D=6 k=5 D=6 k=5 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [20] lean.monotone_profile_corridor_closure_certificate :: D=7 k=0 D=7 k=0 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [21] lean.monotone_profile_corridor_closure_certificate :: D=7 k=1 D=7 k=1 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [22] lean.monotone_profile_corridor_closure_certificate :: D=7 k=2 D=7 k=2 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [23] lean.monotone_profile_corridor_closure_certificate :: D=7 k=3 D=7 k=3 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [24] lean.monotone_profile_corridor_closure_certificate :: D=7 k=4 D=7 k=4 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [25] lean.monotone_profile_corridor_closure_certificate :: D=7 k=5 D=7 k=5 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [26] lean.monotone_profile_corridor_closure_certificate :: D=7 k=6 D=7 k=6 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [27] lean.monotone_profile_corridor_closure_certificate :: D=8 k=0 D=8 k=0 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [28] lean.monotone_profile_corridor_closure_certificate :: D=8 k=1 D=8 k=1 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [29] lean.monotone_profile_corridor_closure_certificate :: D=8 k=2 D=8 k=2 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [30] lean.monotone_profile_corridor_closure_certificate :: D=8 k=3 D=8 k=3 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [31] lean.monotone_profile_corridor_closure_certificate :: D=8 k=4 D=8 k=4 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [32] lean.monotone_profile_corridor_closure_certificate :: D=8 k=5 D=8 k=5 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [33] lean.monotone_profile_corridor_closure_certificate :: D=8 k=6 D=8 k=6 corridor closure cross-check -- instantiates corridor_extra_occurrences
  [34] lean.monotone_profile_corridor_closure_certificate :: D=8 k=7 D=8 k=7 corridor closure cross-check -- instantiates corridor_extra_occurrences
-/

def reflectedNodeCount : Nat := 35

end RavelGenerated
