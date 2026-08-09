import Mathlib.Tactic

namespace RavelGenerated

/-- A splice defect in the forcing coordinate is absorbed by adding it to the
base digit. This is the one-step algebraic rule used by synthesized covering
translation tubes. -/
theorem defect_splice_step
    {State : Type*} [AddCommGroup State]
    (A : State →+ State)
    (forcing : ℤ →+ State)
    (x t : State)
    (digit defect : ℤ) :
    A (x + t) + forcing (digit + defect) =
      (A x + forcing digit) + (A t + forcing defect) := by
  rw [map_add, map_add]
  abel

/-- A source-conditioned sequence of admissible forcing-coordinate splice
 defects transports an affine path exactly. The adjusted digit at step `k` is
 `digit k + defect k`.

 This theorem is independent of n-bonacci coordinates; a compartment supplies
 its additive state map and digit-forcing homomorphism. -/
theorem defect_spliced_affine_path_transport
    {State : Type*} [AddCommGroup State]
    (A : State →+ State)
    (forcing : ℤ →+ State)
    (base translation transported : ℕ → State)
    (digit defect : ℕ → ℤ)
    (hzero : transported 0 = base 0 + translation 0)
    (hbase :
      ∀ k, base (k + 1) = A (base k) + forcing (digit k))
    (htranslation :
      ∀ k, translation (k + 1) =
        A (translation k) + forcing (defect k))
    (htransported :
      ∀ k, transported (k + 1) =
        A (transported k) + forcing (digit k + defect k)) :
    ∀ k, transported k = base k + translation k := by
  intro k
  induction k with
  | zero => exact hzero
  | succ k ih =>
      rw [htransported k, ih, defect_splice_step,
        hbase k, htranslation k]

end RavelGenerated
