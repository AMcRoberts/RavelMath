import Mathlib

namespace RavelGenerated

/-- Evaluation of a word over an arbitrary generator alphabet. -/
def evalGeneratorWord {Γ α : Type*} [Monoid α]
    (M : Γ → α) : List Γ → α
  | [] => 1
  | g :: w => M g * evalGeneratorWord M w

/-- A simultaneous intertwining inequality for every generator propagates
through every word in the free monoid on the generator alphabet. -/
theorem generator_word_intertwiner
    {Γ α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (MC MK : Γ → α) (P : α)
    (hgen : ∀ g, MC g * P ≤ P * MK g) :
    ∀ w, evalGeneratorWord MC w * P ≤ P * evalGeneratorWord MK w := by
  intro w
  induction w with
  | nil => simp [evalGeneratorWord]
  | cons g w ih =>
      calc
        evalGeneratorWord MC (g :: w) * P
            = MC g * (evalGeneratorWord MC w * P) := by
                simp [evalGeneratorWord, mul_assoc]
        _ ≤ MC g * (P * evalGeneratorWord MK w) :=
              mul_left_mono (MC g) ih
        _ = (MC g * P) * evalGeneratorWord MK w := by
              simp [mul_assoc]
        _ ≤ (P * MK g) * evalGeneratorWord MK w :=
              mul_right_mono (evalGeneratorWord MK w) (hgen g)
        _ = P * evalGeneratorWord MK (g :: w) := by
              simp [evalGeneratorWord, mul_assoc]

/-- Repeated words encode natural-number coefficients in the free positive
noncommutative semiring. -/
def evalGeneratorPolynomial {Γ α : Type*} [Semiring α]
    (M : Γ → α) : List (List Γ) → α
  | [] => 0
  | w :: ws => evalGeneratorWord M w + evalGeneratorPolynomial M ws

/-- Generatorwise positive intertwining is closed under every finite positive
word polynomial, for an arbitrary generator alphabet. -/
theorem generator_polynomial_intertwiner
    {Γ α : Type*} [Preorder α] [Semiring α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (add_mono : ∀ {a b c d : α}, a ≤ b → c ≤ d → a + c ≤ b + d)
    (MC MK : Γ → α) (P : α)
    (hgen : ∀ g, MC g * P ≤ P * MK g) :
    ∀ words,
      evalGeneratorPolynomial MC words * P ≤
        P * evalGeneratorPolynomial MK words := by
  intro words
  induction words with
  | nil => simp [evalGeneratorPolynomial]
  | cons w ws ih =>
      have hw := generator_word_intertwiner
        mul_left_mono mul_right_mono MC MK P hgen w
      calc
        evalGeneratorPolynomial MC (w :: ws) * P
            = evalGeneratorWord MC w * P +
                evalGeneratorPolynomial MC ws * P := by
                  simp [evalGeneratorPolynomial, add_mul]
        _ ≤ P * evalGeneratorWord MK w +
              P * evalGeneratorPolynomial MK ws := add_mono hw ih
        _ = P * evalGeneratorPolynomial MK (w :: ws) := by
              simp [evalGeneratorPolynomial, mul_add]

/-- Scalar evaluation of a word using one nonnegative weight per generator. -/
def evalScalarGeneratorWord {Γ : Type*}
    (a : Γ → ℝ) : List Γ → ℝ
  | [] => 1
  | g :: w => a g * evalScalarGeneratorWord a w

/-- Exact abstract norm-weighted lift from any finite or infinite generator
alphabet; finiteness is needed by certificate generation, not by this induction. -/
theorem norm_generator_word_majorant
    {Γ α : Type*} [NormedRing α]
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

/-- A two-letter Q/R alphabet is merely one specialization of the generic
finite-positive-grammar theorem. -/
inductive QR
  | q
  | r
  deriving DecidableEq, Repr

example {α : Type*} [NormedRing α]
    (Q R : α) (q r : ℝ)
    (hq0 : 0 ≤ q) (hr0 : 0 ≤ r)
    (hQ : ‖Q‖ ≤ q) (hR : ‖R‖ ≤ r) :
    ∀ w : List QR,
      ‖evalGeneratorWord (fun g => match g with | QR.q => Q | QR.r => R) w‖ ≤
      evalScalarGeneratorWord (fun g => match g with | QR.q => q | QR.r => r) w := by
  apply norm_generator_word_majorant
  · intro g
    cases g <;> assumption
  · intro g
    cases g <;> assumption

end RavelGenerated
