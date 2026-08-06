import Mathlib

namespace RavelGenerated

inductive NormQRGenerator
  | q
  | r
  deriving DecidableEq, Repr

def evalNormQRWord {α : Type} [Monoid α]
    (Q R : α) : List NormQRGenerator → α
  | [] => 1
  | NormQRGenerator.q :: w => Q * evalNormQRWord Q R w
  | NormQRGenerator.r :: w => R * evalNormQRWord Q R w

def evalScalarQRWord
    (q r : ℝ) : List NormQRGenerator → ℝ
  | [] => 1
  | NormQRGenerator.q :: w => q * evalScalarQRWord q r w
  | NormQRGenerator.r :: w => r * evalScalarQRWord q r w

/-- Generator norm bounds propagate through every noncommutative Q/R word.
This is the abstract operator-norm layer of the norm-weighted majorant theorem. -/
theorem norm_qr_word_majorant
    {α : Type} [NormedRing α]
    (Q R : α) (q r : ℝ)
    (hq0 : 0 ≤ q) (hr0 : 0 ≤ r)
    (hQ : ‖Q‖ ≤ q) (hR : ‖R‖ ≤ r) :
    ∀ w,
      ‖evalNormQRWord Q R w‖ ≤ evalScalarQRWord q r w := by
  intro w
  induction w with
  | nil => simp [evalNormQRWord, evalScalarQRWord]
  | cons g w ih =>
      cases g with
      | q =>
          calc
            ‖evalNormQRWord Q R (NormQRGenerator.q :: w)‖
                = ‖Q * evalNormQRWord Q R w‖ := by
                    simp [evalNormQRWord]
            _ ≤ ‖Q‖ * ‖evalNormQRWord Q R w‖ := norm_mul_le _ _
            _ ≤ q * evalScalarQRWord q r w :=
                  mul_le_mul hQ ih (norm_nonneg _) hq0
            _ = evalScalarQRWord q r (NormQRGenerator.q :: w) := by
                  simp [evalScalarQRWord]
      | r =>
          calc
            ‖evalNormQRWord Q R (NormQRGenerator.r :: w)‖
                = ‖R * evalNormQRWord Q R w‖ := by
                    simp [evalNormQRWord]
            _ ≤ ‖R‖ * ‖evalNormQRWord Q R w‖ := norm_mul_le _ _
            _ ≤ r * evalScalarQRWord q r w :=
                  mul_le_mul hR ih (norm_nonneg _) hr0
            _ = evalScalarQRWord q r (NormQRGenerator.r :: w) := by
                  simp [evalScalarQRWord]

def evalNormQRPolynomial {α : Type} [Semiring α]
    (Q R : α) : List (List NormQRGenerator) → α
  | [] => 0
  | w :: ws => evalNormQRWord Q R w + evalNormQRPolynomial Q R ws

def evalScalarQRPolynomial
    (q r : ℝ) : List (List NormQRGenerator) → ℝ
  | [] => 0
  | w :: ws => evalScalarQRWord q r w + evalScalarQRPolynomial q r ws

/-- Repeated words encode nonnegative integer coefficients, so the word
majorant is closed under every finite nonnegative Q/R boundary polynomial. -/
theorem norm_qr_polynomial_majorant
    {α : Type} [NormedRing α]
    (Q R : α) (q r : ℝ)
    (hq0 : 0 ≤ q) (hr0 : 0 ≤ r)
    (hQ : ‖Q‖ ≤ q) (hR : ‖R‖ ≤ r) :
    ∀ words,
      ‖evalNormQRPolynomial Q R words‖ ≤
        evalScalarQRPolynomial q r words := by
  intro words
  induction words with
  | nil => simp [evalNormQRPolynomial, evalScalarQRPolynomial]
  | cons w ws ih =>
      have hw := norm_qr_word_majorant Q R q r hq0 hr0 hQ hR w
      calc
        ‖evalNormQRPolynomial Q R (w :: ws)‖
            = ‖evalNormQRWord Q R w + evalNormQRPolynomial Q R ws‖ := by
                simp [evalNormQRPolynomial]
        _ ≤ ‖evalNormQRWord Q R w‖ + ‖evalNormQRPolynomial Q R ws‖ :=
              norm_add_le _ _
        _ ≤ evalScalarQRWord q r w + evalScalarQRPolynomial q r ws :=
              add_le_add hw ih
        _ = evalScalarQRPolynomial q r (w :: ws) := by
              simp [evalScalarQRPolynomial]

/-- Contractive generators recover the ordinary unweighted word bound. -/
theorem contractive_qr_word_majorant
    {α : Type} [NormedRing α]
    (Q R : α)
    (hQ : ‖Q‖ ≤ 1) (hR : ‖R‖ ≤ 1) :
    ∀ w, ‖evalNormQRWord Q R w‖ ≤ 1 := by
  intro w
  have h := norm_qr_word_majorant Q R 1 1 (by positivity) (by positivity) hQ hR w
  simpa [evalScalarQRWord] using h

/-- The scalar loop of norm two is an exact counterexample to an unweighted
unit majorant, while the norm-weighted majorant is sharp. -/
theorem scalar_two_loop_counterexample :
    ¬ ‖(2 : ℝ)‖ ≤ 1 ∧ ‖(2 : ℝ)‖ ≤ 2 := by
  constructor <;> norm_num

end RavelGenerated
