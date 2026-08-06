import Mathlib

namespace RavelGenerated

inductive SupergoldenGenerator
  | neutral
  | positive
  | negative
  deriving DecidableEq, Repr

def evalSupergoldenWord {α : Type*} [Monoid α]
    (G : SupergoldenGenerator → α) : List SupergoldenGenerator → α
  | [] => 1
  | g :: w => G g * evalSupergoldenWord G w

/-- The abstract kernel interface used by the concrete nine-role supergolden
certificate (`ravel::proof::derive_supergolden_three_generator_intertwiner`):
three simultaneous generator inequalities propagate through every word. Same
statement and proof shape as `plastic_word_intertwiner`, retargeted at the
supergolden number's own concrete Q/R/S generators and boundary/universal
matrices -- the underlying combinatorial fact (three base inequalities in an
ordered monoid extend to every word by induction) does not depend on which
Pisot substitution produced the base inequalities. -/
theorem supergolden_word_intertwiner
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (GB GU : SupergoldenGenerator → α) (P : α)
    (h0 : GB SupergoldenGenerator.neutral * P ≤ P * GU SupergoldenGenerator.neutral)
    (hp : GB SupergoldenGenerator.positive * P ≤ P * GU SupergoldenGenerator.positive)
    (hm : GB SupergoldenGenerator.negative * P ≤ P * GU SupergoldenGenerator.negative) :
    ∀ w, evalSupergoldenWord GB w * P ≤ P * evalSupergoldenWord GU w := by
  intro w
  induction w with
  | nil => simp [evalSupergoldenWord]
  | cons g w ih =>
      have hg : GB g * P ≤ P * GU g := by
        cases g with
        | neutral => exact h0
        | positive => exact hp
        | negative => exact hm
      calc
        evalSupergoldenWord GB (g :: w) * P
            = GB g * (evalSupergoldenWord GB w * P) := by
                simp [evalSupergoldenWord, mul_assoc]
        _ ≤ GB g * (P * evalSupergoldenWord GU w) := mul_left_mono _ ih
        _ = (GB g * P) * evalSupergoldenWord GU w := by simp [mul_assoc]
        _ ≤ (P * GU g) * evalSupergoldenWord GU w :=
              mul_right_mono _ hg
        _ = P * evalSupergoldenWord GU (g :: w) := by
              simp [evalSupergoldenWord, mul_assoc]

end RavelGenerated
