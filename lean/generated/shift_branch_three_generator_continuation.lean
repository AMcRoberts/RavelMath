import Mathlib

namespace RavelGenerated

inductive SignedDefect
  | neutral | positive | negative
  deriving DecidableEq, Repr

/-- The symbolic prefix catalogue of the shift-branch substitution consists of
    the empty prefix and one distinguished one-letter prefix.  Hence every
    ordered prefix difference has one of three signed classes. -/
theorem two_prefixes_give_three_signed_defects
    (p q : Bool) :
    (p = q) ∨ (p = false ∧ q = true) ∨ (p = true ∧ q = false) := by
  cases p <;> cases q <;> simp

/-- Generatorwise intertwining propagates to all words over the three-letter
    plastic continuation alphabet; this is a specialization of the generic
    finite-positive-grammar theorem. -/
theorem three_generator_word_induction
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a*b ≤ a*c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a*c ≤ b*c)
    (MC MK : SignedDefect → α) (P : α)
    (h : ∀ g, MC g * P ≤ P * MK g) :
    ∀ w : List SignedDefect,
      w.foldr (fun g z => MC g * z) 1 * P ≤
      P * w.foldr (fun g z => MK g * z) 1 := by
  intro w
  induction w with
  | nil => simp
  | cons g w ih =>
      simp only [List.foldr]
      calc
        (MC g * w.foldr (fun g z => MC g * z) 1) * P
            = MC g * (w.foldr (fun g z => MC g * z) 1 * P) := by simp [mul_assoc]
        _ ≤ MC g * (P * w.foldr (fun g z => MK g * z) 1) := mul_left_mono _ ih
        _ = (MC g * P) * w.foldr (fun g z => MK g * z) 1 := by simp [mul_assoc]
        _ ≤ (P * MK g) * w.foldr (fun g z => MK g * z) 1 :=
              mul_right_mono _ (h g)
        _ = P * (MK g * w.foldr (fun g z => MK g * z) 1) := by simp [mul_assoc]

end RavelGenerated
