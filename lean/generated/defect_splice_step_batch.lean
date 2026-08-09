import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem defect_splice_step
    {State : Type*} [AddCommGroup State]
    (A : State →+ State)
    (forcing : ℤ →+ State)
    (x t : State)
    (digit defect : ℤ) :
    A (x + t) + forcing (digit + defect) =
      (A x + forcing digit) + (A t + forcing defect) := by
  rw [map_add, map_add]
  abel

def nbonacciA (d : ℕ) : Matrix (Fin d) (Fin d) ℤ :=
  Matrix.of (fun i j =>
    if (i:ℕ) + 1 = d then
      (if (j:ℕ) = 0 then 1 else -1)
    else
      (if (j:ℕ) = (i:ℕ) + 1 then 1 else 0))

def nbonacciAHom (d : ℕ) : (Fin d → ℤ) →+ (Fin d → ℤ) :=
  AddMonoidHom.mk' (fun x => (nbonacciA d) *ᵥ x) (fun x y => by simp [Matrix.mulVec_add])

def nbonacciForcingHom (d : ℕ) (hd : 0 < d) : ℤ →+ (Fin d → ℤ) :=
  AddMonoidHom.mk' (fun n => n • (Pi.single (⟨d-1, by omega⟩ : Fin d) (1:ℤ)))
    (fun x y => by simp [add_smul])

/-- Mechanically emitted: n3 homogeneous splice step 0 -- ravel::proof::stage_defect_splice_steps independently
    re-derived this dimension-3 step's base/translation/transported n-bonacci recurrences
    from raw integer arithmetic (nothing pre-trusted). -/
def spliceX0 : Fin 3 → ℤ := ![(1 : ℤ), (-2 : ℤ), (1 : ℤ)]
def spliceT0 : Fin 3 → ℤ := ![(-1 : ℤ), (-1 : ℤ), (-1 : ℤ)]

theorem defect_splice_step_instance_0 :
    (nbonacciAHom 3) (spliceX0 + spliceT0) + (nbonacciForcingHom 3 (by norm_num)) ((-1 : ℤ) + (0 : ℤ)) =
      ((nbonacciAHom 3) spliceX0 + (nbonacciForcingHom 3 (by norm_num)) (-1 : ℤ)) + ((nbonacciAHom 3) spliceT0 + (nbonacciForcingHom 3 (by norm_num)) (0 : ℤ)) :=
  defect_splice_step (nbonacciAHom 3) (nbonacciForcingHom 3 (by norm_num)) spliceX0 spliceT0 (-1 : ℤ) (0 : ℤ)

theorem defect_splice_step_instance_0_value :
    (nbonacciAHom 3) spliceX0 + (nbonacciForcingHom 3 (by norm_num)) (-1 : ℤ) = ![(-2 : ℤ), (1 : ℤ), (1 : ℤ)] ∧
      (nbonacciAHom 3) spliceT0 + (nbonacciForcingHom 3 (by norm_num)) (0 : ℤ) = ![(-1 : ℤ), (-1 : ℤ), (1 : ℤ)] ∧
      (nbonacciAHom 3) (spliceX0 + spliceT0) + (nbonacciForcingHom 3 (by norm_num)) ((-1 : ℤ) + (0 : ℤ)) = ![(-3 : ℤ), (0 : ℤ), (2 : ℤ)] := by
  refine ⟨?_, ?_, ?_⟩ <;>
    · unfold nbonacciAHom nbonacciForcingHom nbonacciA
      ext i
      fin_cases i <;> decide

/- Semantic proof graph for: defect_splice_step_batch
  [0] lean.defect_splice_step_certificate :: dim=3 n3 homogeneous splice step 0 -- instantiates defect_splice_step
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
