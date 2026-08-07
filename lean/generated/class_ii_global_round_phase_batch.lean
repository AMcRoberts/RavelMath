import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

inductive ClassIIGlobalRoundPhaseG
  | base | stable | penultimate | terminal | repeated
  deriving DecidableEq

def classIIGlobalRoundPhaseG (a r : Int) : ClassIIGlobalRoundPhaseG :=
  if r ≤ 4 then .base
  else if r ≤ a - 2 then .stable
  else if r = a - 1 then .penultimate
  else if r = a then .terminal
  else .repeated

/-- Reproduced from the independently kernel-checked
    `lean/class_ii_global_round_partition.lean` (not re-derived here). -/
theorem classIIGlobalRoundPhaseG_spec (a r : Int) (ha : 7 ≤ a)
    (hr0 : 1 ≤ r) (hr1 : r ≤ a + 1) :
    (classIIGlobalRoundPhaseG a r = .base ↔ r ≤ 4) ∧
    (classIIGlobalRoundPhaseG a r = .stable ↔ 5 ≤ r ∧ r ≤ a - 2) ∧
    (classIIGlobalRoundPhaseG a r = .penultimate ↔ r = a - 1) ∧
    (classIIGlobalRoundPhaseG a r = .terminal ↔ r = a) ∧
    (classIIGlobalRoundPhaseG a r = .repeated ↔ r = a + 1) := by
  unfold classIIGlobalRoundPhaseG
  split_ifs <;> simp_all <;> omega

/-- Mechanically emitted: the concrete phase C++ actually computed at
    a=10, round=3 equals classIIGlobalRoundPhaseG at that (a,round). -/
theorem class_ii_global_round_phase_instance_0 :
    classIIGlobalRoundPhaseG (10 : Int) (3 : Int) = ClassIIGlobalRoundPhaseG.base := by
  decide

/-- Mechanically emitted: the concrete phase C++ actually computed at
    a=10, round=6 equals classIIGlobalRoundPhaseG at that (a,round). -/
theorem class_ii_global_round_phase_instance_1 :
    classIIGlobalRoundPhaseG (10 : Int) (6 : Int) = ClassIIGlobalRoundPhaseG.stable := by
  decide

/-- Mechanically emitted: the concrete phase C++ actually computed at
    a=10, round=9 equals classIIGlobalRoundPhaseG at that (a,round). -/
theorem class_ii_global_round_phase_instance_2 :
    classIIGlobalRoundPhaseG (10 : Int) (9 : Int) = ClassIIGlobalRoundPhaseG.penultimate := by
  decide

/-- Mechanically emitted: the concrete phase C++ actually computed at
    a=10, round=10 equals classIIGlobalRoundPhaseG at that (a,round). -/
theorem class_ii_global_round_phase_instance_3 :
    classIIGlobalRoundPhaseG (10 : Int) (10 : Int) = ClassIIGlobalRoundPhaseG.terminal := by
  decide

/-- Mechanically emitted: the concrete phase C++ actually computed at
    a=10, round=11 equals classIIGlobalRoundPhaseG at that (a,round). -/
theorem class_ii_global_round_phase_instance_4 :
    classIIGlobalRoundPhaseG (10 : Int) (11 : Int) = ClassIIGlobalRoundPhaseG.repeated := by
  decide

/- Semantic proof graph for: class_ii_global_round_phase_batch
  [0] lean.class_ii_global_round_phase_certificate :: a=10 round=3 phase=0 -- instantiates classIIGlobalRoundPhase
  [1] lean.class_ii_global_round_phase_certificate :: a=10 round=6 phase=1 -- instantiates classIIGlobalRoundPhase
  [2] lean.class_ii_global_round_phase_certificate :: a=10 round=9 phase=2 -- instantiates classIIGlobalRoundPhase
  [3] lean.class_ii_global_round_phase_certificate :: a=10 round=10 phase=3 -- instantiates classIIGlobalRoundPhase
  [4] lean.class_ii_global_round_phase_certificate :: a=10 round=11 phase=4 -- instantiates classIIGlobalRoundPhase
-/

def reflectedNodeCount : Nat := 5

end RavelGenerated
