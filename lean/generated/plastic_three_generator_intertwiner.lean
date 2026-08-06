import Mathlib

namespace RavelGenerated

inductive PlasticGenerator
  | neutral
  | positive
  | negative
  deriving DecidableEq, Repr

def evalPlasticWord {α : Type*} [Monoid α]
    (G : PlasticGenerator → α) : List PlasticGenerator → α
  | [] => 1
  | g :: w => G g * evalPlasticWord G w

/-- The abstract kernel interface used by the concrete nine-role plastic
certificate: three simultaneous generator inequalities propagate through every
word. -/
theorem plastic_word_intertwiner
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (GB GU : PlasticGenerator → α) (P : α)
    (h0 : GB PlasticGenerator.neutral * P ≤ P * GU PlasticGenerator.neutral)
    (hp : GB PlasticGenerator.positive * P ≤ P * GU PlasticGenerator.positive)
    (hm : GB PlasticGenerator.negative * P ≤ P * GU PlasticGenerator.negative) :
    ∀ w, evalPlasticWord GB w * P ≤ P * evalPlasticWord GU w := by
  intro w
  induction w with
  | nil => simp [evalPlasticWord]
  | cons g w ih =>
      have hg : GB g * P ≤ P * GU g := by
        cases g with
        | neutral => exact h0
        | positive => exact hp
        | negative => exact hm
      calc
        evalPlasticWord GB (g :: w) * P
            = GB g * (evalPlasticWord GB w * P) := by
                simp [evalPlasticWord, mul_assoc]
        _ ≤ GB g * (P * evalPlasticWord GU w) := mul_left_mono _ ih
        _ = (GB g * P) * evalPlasticWord GU w := by simp [mul_assoc]
        _ ≤ (P * GU g) * evalPlasticWord GU w :=
              mul_right_mono _ hg
        _ = P * evalPlasticWord GU (g :: w) := by
              simp [evalPlasticWord, mul_assoc]

end RavelGenerated
