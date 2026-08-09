import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

def evalGeneratorWord {Γ α : Type*} [Monoid α]
    (M : Γ → α) : List Γ → α
  | [] => 1
  | g :: w => M g * evalGeneratorWord M w

def evalScalarGeneratorWord {Γ : Type*}
    (a : Γ → ℝ) : List Γ → ℝ
  | [] => 1
  | g :: w => a g * evalScalarGeneratorWord a w

theorem norm_generator_word_majorant
    {Γ α : Type*} [NormedRing α] [NormOneClass α]
    (M : Γ → α) (a : Γ → ℝ)
    (ha0 : ∀ g, 0 ≤ a g)
    (hM : ∀ g, ‖M g‖ ≤ a g) :
    ∀ w, ‖evalGeneratorWord M w‖ ≤ evalScalarGeneratorWord a w := by
  intro w
  induction w with
  | nil => simp [evalGeneratorWord, evalScalarGeneratorWord]
  | cons g w ih =>
      calc
        ‖evalGeneratorWord M (g :: w)‖
            = ‖M g * evalGeneratorWord M w‖ := by
                simp [evalGeneratorWord]
        _ ≤ ‖M g‖ * ‖evalGeneratorWord M w‖ := norm_mul_le _ _
        _ ≤ a g * evalScalarGeneratorWord a w :=
              mul_le_mul (hM g) ih (norm_nonneg _) (ha0 g)
        _ = evalScalarGeneratorWord a (g :: w) := by
              simp [evalScalarGeneratorWord]

/-- Mechanically emitted: 3-generator balanced/left-defect/right-defect grammar -- ravel::proof::stage_finite_positive_grammar_majorant
    independently re-summed the per-generator count/norm-weighted
    matrices over all 3 generators (nothing pre-trusted). -/
def grammarM0 : Fin 3 → ℚ
  | ⟨0, _⟩ => (2 : ℚ) / (1 : ℚ)
  | ⟨1, _⟩ => (1 : ℚ) / (1 : ℚ)
  | ⟨2, _⟩ => (3 : ℚ) / (4 : ℚ)
  | ⟨_, _⟩ => 0

noncomputable def grammarA0 : Fin 3 → ℝ
  | ⟨0, _⟩ => (2 : ℝ) / (1 : ℝ)
  | ⟨1, _⟩ => (2 : ℝ) / (1 : ℝ)
  | ⟨2, _⟩ => (1 : ℝ) / (1 : ℝ)
  | ⟨_, _⟩ => 0

theorem grammar_majorant_instance_0_nonneg :
    ∀ g, (0:ℝ) ≤ grammarA0 g := by
  intro g
  fin_cases g <;> (simp only [grammarA0]; norm_num)

theorem grammar_majorant_instance_0_bound :
    ∀ g, ‖grammarM0 g‖ ≤ grammarA0 g := by
  intro g
  fin_cases g <;> simp only [grammarM0, grammarA0] <;>
    rw [← Rat.norm_cast_real] <;> norm_num

theorem grammar_majorant_instance_0 :
    ∀ w : List (Fin 3),
      ‖evalGeneratorWord grammarM0 w‖ ≤ evalScalarGeneratorWord grammarA0 w :=
  norm_generator_word_majorant grammarM0 grammarA0 grammar_majorant_instance_0_nonneg grammar_majorant_instance_0_bound

/- Semantic proof graph for: finite_positive_grammar_majorant_batch
  [0] lean.finite_positive_grammar_majorant_certificate :: generators=3 base_vertices=2 3-generator balanced/left-defect/right-defect grammar -- instantiates norm_generator_word_majorant
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
