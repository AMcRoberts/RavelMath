import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem word_fold_intertwiner
    {Γ α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (MC MK : Γ → α) (P : α)
    (h : ∀ g, MC g * P ≤ P * MK g) :
    ∀ w : List Γ,
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

inductive PlasticGenerator
  | neutral
  | positive
  | negative
  deriving DecidableEq, Repr

def evalPlasticWord {α : Type*} [Monoid α]
    (G : PlasticGenerator → α) : List PlasticGenerator → α :=
  List.foldr (fun g z => G g * z) 1

/-- The abstract kernel interface used by the concrete nine-role plastic certificate: three simultaneous generator inequalities propagate through every word -- a one-line corollary of `word_fold_intertwiner`. -/
theorem plastic_word_intertwiner
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (GB GU : PlasticGenerator → α) (P : α)
    (h0 : GB PlasticGenerator.neutral * P ≤ P * GU PlasticGenerator.neutral)
    (h1 : GB PlasticGenerator.positive * P ≤ P * GU PlasticGenerator.positive)
    (h2 : GB PlasticGenerator.negative * P ≤ P * GU PlasticGenerator.negative)
    :
    ∀ w, evalPlasticWord GB w * P ≤ P * evalPlasticWord GU w := by
  have h : ∀ g, GB g * P ≤ P * GU g := by
    intro g
    cases g with
    | neutral => exact h0
    | positive => exact h1
    | negative => exact h2
  exact word_fold_intertwiner mul_left_mono mul_right_mono GB GU P h

-- Mechanically emitted record 0 (plastic): plastic number's own 9-role boundary/universal Q/R/S intertwiner
-- ravel::proof::stage_plastic_three_generator_intertwiner found boundary_states=101, boundary_edges=125, universal_edges=16,
-- independently rechecking all three generator inequalities against
-- `plastic_word_intertwiner` (no separate theorem declaration
-- is needed: the abstract lemma above already covers this instantiation).

/- Semantic proof graph for: plastic_three_generator_intertwiner_batch
  [0] lean.three_generator_intertwiner_family_certificate :: plastic boundary_states=101 boundary_edges=125 plastic number's own 9-role boundary/universal Q/R/S intertwiner -- instantiates plastic_word_intertwiner
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
