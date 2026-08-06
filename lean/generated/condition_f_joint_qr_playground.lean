import Mathlib

namespace RavelGenerated

/-- The two canonical Condition-F generators.  `q` is balanced prefix
transport and `r` is one-sided prefix-defect transport. -/
inductive QRGenerator
  | q
  | r
  deriving DecidableEq, Repr

/-- Evaluate a noncommutative Q/R word. -/
def evalQRWord {α : Type} [Monoid α]
    (Q R : α) : List QRGenerator → α
  | [] => 1
  | QRGenerator.q :: w => Q * evalQRWord Q R w
  | QRGenerator.r :: w => R * evalQRWord Q R w

/-- A simultaneous Q/R intertwining inequality propagates through every
noncommutative word.  This is the kernel interface used by the terminal-parent
boundary substitution. -/
theorem qr_word_intertwiner
    {α : Type} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (QC RC QK RK P : α)
    (hQ : QC * P ≤ P * QK)
    (hR : RC * P ≤ P * RK) :
    ∀ w,
      evalQRWord QC RC w * P ≤
        P * evalQRWord QK RK w := by
  intro w
  induction w with
  | nil => simp [evalQRWord]
  | cons g w ih =>
      cases g with
      | q =>
          calc
            evalQRWord QC RC (QRGenerator.q :: w) * P
                = QC * (evalQRWord QC RC w * P) := by
                    simp [evalQRWord, mul_assoc]
            _ ≤ QC * (P * evalQRWord QK RK w) :=
                  mul_left_mono QC ih
            _ = (QC * P) * evalQRWord QK RK w := by
                  simp [mul_assoc]
            _ ≤ (P * QK) * evalQRWord QK RK w :=
                  mul_right_mono (evalQRWord QK RK w) hQ
            _ = P * evalQRWord QK RK (QRGenerator.q :: w) := by
                  simp [evalQRWord, mul_assoc]
      | r =>
          calc
            evalQRWord QC RC (QRGenerator.r :: w) * P
                = RC * (evalQRWord QC RC w * P) := by
                    simp [evalQRWord, mul_assoc]
            _ ≤ RC * (P * evalQRWord QK RK w) :=
                  mul_left_mono RC ih
            _ = (RC * P) * evalQRWord QK RK w := by
                  simp [mul_assoc]
            _ ≤ (P * RK) * evalQRWord QK RK w :=
                  mul_right_mono (evalQRWord QK RK w) hR
            _ = P * evalQRWord QK RK (QRGenerator.r :: w) := by
                  simp [evalQRWord, mul_assoc]

/-- A boundary substitution is represented as a list of Q/R words; repeated
words record multiplicity. -/
def evalQRPolynomial {α : Type} [Semiring α]
    (Q R : α) : List (List QRGenerator) → α
  | [] => 0
  | w :: ws => evalQRWord Q R w + evalQRPolynomial Q R ws

/-- Wordwise Q/R intertwining is closed under every finite nonnegative
boundary polynomial. -/
theorem qr_polynomial_intertwiner
    {α : Type} [Preorder α] [Semiring α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (add_mono : ∀ {a b c d : α}, a ≤ b → c ≤ d → a + c ≤ b + d)
    (QC RC QK RK P : α)
    (hQ : QC * P ≤ P * QK)
    (hR : RC * P ≤ P * RK) :
    ∀ words,
      evalQRPolynomial QC RC words * P ≤
        P * evalQRPolynomial QK RK words := by
  intro words
  induction words with
  | nil => simp [evalQRPolynomial]
  | cons w ws ih =>
      have hw := qr_word_intertwiner
        mul_left_mono mul_right_mono QC RC QK RK P hQ hR w
      calc
        evalQRPolynomial QC RC (w :: ws) * P
            = evalQRWord QC RC w * P +
                evalQRPolynomial QC RC ws * P := by
                  simp [evalQRPolynomial, add_mul]
        _ ≤ P * evalQRWord QK RK w +
              P * evalQRPolynomial QK RK ws := add_mono hw ih
        _ = P * evalQRPolynomial QK RK (w :: ws) := by
              simp [evalQRPolynomial, mul_add]

/-- The ordinary natural-number induction interface used after the concrete
terminal-parent word polynomial has been generated and replayed. -/
theorem jointQR_dimension_induction
    (Good : ℕ → Prop)
    (hbase : Good 2)
    (hstep : ∀ D, 2 ≤ D → Good D → Good (D + 1)) :
    ∀ D, 2 ≤ D → Good D := by
  intro D hD
  induction D, hD using Nat.le_induction with
  | base => exact hbase
  | succ D hD ih => exact hstep D hD ih

/-- Final order-theoretic sandwich used after the Condition-F quotient and
terminal-fibre reductions identify upper and lower bounds by the core. -/
theorem universal_dominance_sandwich
    {α : Type} [PartialOrder α]
    (literal quotient core : α)
    (hlq : literal ≤ quotient)
    (hqc : quotient ≤ core)
    (hcl : core ≤ literal) :
    literal = core := by
  apply le_antisymm
  · exact le_trans hlq hqc
  · exact hcl

end RavelGenerated
