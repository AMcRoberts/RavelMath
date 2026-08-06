import Mathlib.Tactic

namespace RavelGenerated

/-- A cyclic controller run is a controller path over a digit word whose final
controller state equals its initial state. -/
structure CyclicControllerRun
    {Controller Input : Type}
    (Step : Controller → Input → Controller → Prop)
    (word : List Input) where
  stateAt : Fin (word.length + 1) → Controller
  step_ok :
    ∀ k : Fin word.length,
      Step
        (stateAt ⟨k, Nat.lt_succ_of_lt k.isLt⟩)
        (word.get k)
        (stateAt ⟨k + 1, Nat.succ_lt_succ k.isLt⟩)
  closes :
    stateAt ⟨0, Nat.zero_lt_succ _⟩ =
      stateAt ⟨word.length, Nat.lt_succ_self _⟩

/-- Abstract cyclic affine transport.

If a base path and a controller translation path obey compatible affine
recurrences, and both paths close, then the transported path closes. -/
theorem cyclic_affine_transport_closes
    {State Input : Type}
    [AddCommGroup State]
    (A : State →+ State)
    (forcing : Input → State)
    (word : List Input)
    (base translation transported :
      Fin (word.length + 1) → State)
    (hbase :
      ∀ k : Fin word.length,
        base ⟨k + 1, Nat.succ_lt_succ k.isLt⟩ =
          A (base ⟨k, Nat.lt_succ_of_lt k.isLt⟩) +
            forcing (word.get k))
    (htranslation :
      ∀ k : Fin word.length,
        translation ⟨k + 1, Nat.succ_lt_succ k.isLt⟩ =
          A (translation ⟨k, Nat.lt_succ_of_lt k.isLt⟩))
    (htransport :
      ∀ k : Fin (word.length + 1),
        transported k = base k + translation k)
    (hbaseClose :
      base ⟨word.length, Nat.lt_succ_self _⟩ =
        base ⟨0, Nat.zero_lt_succ _⟩)
    (htranslationClose :
      translation ⟨word.length, Nat.lt_succ_self _⟩ =
        translation ⟨0, Nat.zero_lt_succ _⟩) :
    transported ⟨word.length, Nat.lt_succ_self _⟩ =
      transported ⟨0, Nat.zero_lt_succ _⟩ := by
  rw [htransport, htransport, hbaseClose, htranslationClose]

/-- The exact local theorem required by the outer shell-pumping proof.

The concrete n-bonacci campaign must instantiate this proposition by producing
a cyclic continuation-controller run whose transported cycle lies on a
strictly larger shell. -/
def CyclicStrictShellPump
    {Cycle : Type}
    (RecurrentCycle : Cycle → Prop)
    (radius : Cycle → ℕ) : Prop :=
  ∀ cycle, RecurrentCycle cycle → 2 ≤ radius cycle →
    ∃ lifted,
      RecurrentCycle lifted ∧ radius cycle < radius lifted

end RavelGenerated

namespace RavelGenerated

/-- Concrete proof payload exported by the strict-shell-pump engine operation.
The payload contains the semantic facts checked by replay; it does not expose
or trust generated proof text. -/
structure StrictShellPumpWitness
    {Cycle : Type}
    (RecurrentCycle : Cycle → Prop)
    (radius : Cycle → ℕ)
    (source : Cycle) where
  lifted : Cycle
  source_recurrent : RecurrentCycle source
  lifted_recurrent : RecurrentCycle lifted
  strict_outward : radius source < radius lifted

/-- A witness-producing operation for every outer recurrent cycle discharges
the abstract cyclic strict-shell-pump obligation. -/
theorem cyclicStrictShellPump_of_witness
    {Cycle : Type}
    (RecurrentCycle : Cycle → Prop)
    (radius : Cycle → ℕ)
    (produce : ∀ source, RecurrentCycle source → 2 ≤ radius source →
      StrictShellPumpWitness RecurrentCycle radius source) :
    CyclicStrictShellPump RecurrentCycle radius := by
  intro source hrec houter
  let witness := produce source hrec houter
  exact ⟨witness.lifted, witness.lifted_recurrent,
    witness.strict_outward⟩

end RavelGenerated

namespace RavelGenerated

/-- A complete enumeration of realized recurrent cycles, paired with a strict
pump witness for every outer member, discharges cyclic strict-shell pumping.
The completeness premise is intentionally explicit: finite or bounded word
coverage alone is not enough. -/
theorem cyclicStrictShellPump_of_complete_family
    {Cycle : Type}
    (RecurrentCycle : Cycle → Prop)
    (radius : Cycle → ℕ)
    (family : List Cycle)
    (complete : ∀ source, RecurrentCycle source → source ∈ family)
    (pump : ∀ source, source ∈ family → RecurrentCycle source →
      2 ≤ radius source →
      StrictShellPumpWitness RecurrentCycle radius source) :
    CyclicStrictShellPump RecurrentCycle radius := by
  apply cyclicStrictShellPump_of_witness
  intro source hrec houter
  exact pump source (complete source hrec) hrec houter

end RavelGenerated
