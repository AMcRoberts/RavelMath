import Mathlib

namespace RavelGenerated

inductive QR
  | q
  | r
  deriving DecidableEq, Repr

def evalWord {α : Type*} [Monoid α] (Q R : α) : List QR → α
  | [] => 1
  | QR.q :: w => Q * evalWord Q R w
  | QR.r :: w => R * evalWord Q R w

def roofWord (d : ℕ) : List QR :=
  if d = 0 then [QR.q] else List.replicate d QR.r

/-- A simultaneous Q/R intertwiner propagates through every generalized-
multinacci roof word. -/
theorem roof_word_intertwiner
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (Qc Rc Qk Rk P : α)
    (hQ : Qc * P ≤ P * Qk)
    (hR : Rc * P ≤ P * Rk) :
    ∀ d,
      evalWord Qc Rc (roofWord d) * P ≤
      P * evalWord Qk Rk (roofWord d) := by
  intro d
  unfold roofWord
  split
  · simpa [evalWord] using hQ
  · induction d with
    | zero => contradiction
    | succ d ih =>
      simp only [List.replicate_succ, evalWord]
      calc
        Rc * evalWord Qc Rc (List.replicate d QR.r) * P
            = Rc * (evalWord Qc Rc (List.replicate d QR.r) * P) := by
                simp [mul_assoc]
        _ ≤ Rc * (P * evalWord Qk Rk (List.replicate d QR.r)) :=
              mul_left_mono Rc ih
        _ = (Rc * P) * evalWord Qk Rk (List.replicate d QR.r) := by
              simp [mul_assoc]
        _ ≤ (P * Rk) * evalWord Qk Rk (List.replicate d QR.r) :=
              mul_right_mono _ hR
        _ = P * (Rk * evalWord Qk Rk (List.replicate d QR.r)) := by
              simp [mul_assoc]

/-- The symbolic cut catalogue for a prefix 0^m consists of exactly
(m+1)^2 ordered cut pairs. -/
theorem cut_pair_count (m : ℕ) :
    (Fin (m + 1) × Fin (m + 1)) ≃ Fin ((m + 1) * (m + 1)) :=
  Equiv.prodFinEquiv

end RavelGenerated
