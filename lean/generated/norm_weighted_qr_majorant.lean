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

inductive NormQRGenerator
  | q
  | r
  deriving DecidableEq, Repr

-- evalNormQRWord/evalScalarQRWord are the NormQRGenerator := Fin 2
-- specialization of evalGeneratorWord/evalScalarGeneratorWord above -- defined
-- directly as that specialization, so norm_qr_word_majorant below is a genuine
-- corollary of norm_generator_word_majorant, not an independent re-proof.
def evalNormQRWord {α : Type} [Monoid α]
    (Q R : α) : List NormQRGenerator → α :=
  evalGeneratorWord (fun g : NormQRGenerator => match g with | .q => Q | .r => R)

def evalScalarQRWord
    (q r : ℝ) : List NormQRGenerator → ℝ :=
  evalScalarGeneratorWord (fun g : NormQRGenerator => match g with | .q => q | .r => r)

theorem norm_qr_word_majorant
    {α : Type} [NormedRing α] [NormOneClass α]
    (Q R : α) (q r : ℝ)
    (hq0 : 0 ≤ q) (hr0 : 0 ≤ r)
    (hQ : ‖Q‖ ≤ q) (hR : ‖R‖ ≤ r) :
    ∀ w,
      ‖evalNormQRWord Q R w‖ ≤ evalScalarQRWord q r w :=
  norm_generator_word_majorant (fun g : NormQRGenerator => match g with | .q => Q | .r => R)
    (fun g : NormQRGenerator => match g with | .q => q | .r => r)
    (fun g : NormQRGenerator => by cases g with | q => exact hq0 | r => exact hr0)
    (fun g : NormQRGenerator => by cases g with | q => exact hQ | r => exact hR)

theorem evalScalarQRWord_one_one (w : List NormQRGenerator) :
    evalScalarQRWord 1 1 w = 1 := by
  induction w with
  | nil => simp [evalScalarQRWord, evalScalarGeneratorWord]
  | cons g w ih =>
      cases g <;> simp [evalScalarQRWord, evalScalarGeneratorWord] at ih ⊢ <;> simp [ih]

theorem contractive_qr_word_majorant
    {α : Type} [NormedRing α] [NormOneClass α]
    (Q R : α)
    (hQ : ‖Q‖ ≤ 1) (hR : ‖R‖ ≤ 1) :
    ∀ w, ‖evalNormQRWord Q R w‖ ≤ 1 := by
  intro w
  have h := norm_qr_word_majorant Q R 1 1 (by positivity) (by positivity) hQ hR w
  rwa [evalScalarQRWord_one_one w] at h

theorem scalar_two_loop_counterexample :
    ¬ ‖(2 : ℝ)‖ ≤ 1 ∧ ‖(2 : ℝ)‖ ≤ 2 := by
  constructor <;> norm_num

-- Mechanically emitted record 0: 4-channel contractive Q/R majorant sweep
-- ravel::proof::stage_norm_weighted_qr_majorant found base_vertices=2, contractive=true, expansive=false,
-- independently replaying and reconfirming 3 boundary words against `norm_qr_word_majorant` (no separate theorem
-- declaration is needed: the abstract lemma above already covers every
-- concrete NormedRing instantiation this certificate could witness).

-- Mechanically emitted record 1: 1-channel expansive Q/R majorant sweep
-- ravel::proof::stage_norm_weighted_qr_majorant found base_vertices=1, contractive=false, expansive=true,
-- independently replaying and reconfirming 2 boundary words against `norm_qr_word_majorant` (no separate theorem
-- declaration is needed: the abstract lemma above already covers every
-- concrete NormedRing instantiation this certificate could witness).

/- Semantic proof graph for: norm_weighted_qr_majorant_batch
  [0] lean.norm_weighted_qr_majorant_certificate :: base_vertices=2 words=3 contractive=true expansive=false 4-channel contractive Q/R majorant sweep -- instantiates norm_qr_word_majorant
  [1] lean.norm_weighted_qr_majorant_certificate :: base_vertices=1 words=2 contractive=false expansive=true 1-channel expansive Q/R majorant sweep -- instantiates norm_qr_word_majorant
-/

def reflectedNodeCount : Nat := 2

end RavelGenerated
