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

inductive SupergoldenGenerator
  | neutral
  | positive
  | negative
  deriving DecidableEq, Repr

def evalSupergoldenWord {α : Type*} [Monoid α]
    (G : SupergoldenGenerator → α) : List SupergoldenGenerator → α :=
  List.foldr (fun g z => G g * z) 1

/-- The abstract kernel interface used by the concrete nine-role supergolden certificate (`ravel::proof::derive_supergolden_three_generator_intertwiner`): three simultaneous generator inequalities propagate through every word -- a one-line corollary of `word_fold_intertwiner`, same as `plastic_word_intertwiner`, retargeted at the supergolden number's own concrete Q/R/S generators and boundary/universal matrices. -/
theorem supergolden_word_intertwiner
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (GB GU : SupergoldenGenerator → α) (P : α)
    (h0 : GB SupergoldenGenerator.neutral * P ≤ P * GU SupergoldenGenerator.neutral)
    (h1 : GB SupergoldenGenerator.positive * P ≤ P * GU SupergoldenGenerator.positive)
    (h2 : GB SupergoldenGenerator.negative * P ≤ P * GU SupergoldenGenerator.negative)
    :
    ∀ w, evalSupergoldenWord GB w * P ≤ P * evalSupergoldenWord GU w := by
  have h : ∀ g, GB g * P ≤ P * GU g := by
    intro g
    cases g with
    | neutral => exact h0
    | positive => exact h1
    | negative => exact h2
  exact word_fold_intertwiner mul_left_mono mul_right_mono GB GU P h

-- Mechanically emitted record 0 (supergolden): supergolden number's own 9-role boundary/universal Q/R/S intertwiner
-- ravel::proof::stage_supergolden_three_generator_intertwiner found boundary_states=20, boundary_edges=25, universal_edges=16,
-- independently rechecking all three generator inequalities against
-- `supergolden_word_intertwiner` (no separate theorem declaration
-- is needed: the abstract lemma above already covers this instantiation).

/- Semantic proof graph for: supergolden_three_generator_intertwiner_batch
  [0] lean.three_generator_intertwiner_family_certificate :: supergolden boundary_states=20 boundary_edges=25 supergolden number's own 9-role boundary/universal Q/R/S intertwiner -- instantiates supergolden_word_intertwiner
-/

def reflectedNodeCount : Nat := 1

end RavelGenerated
