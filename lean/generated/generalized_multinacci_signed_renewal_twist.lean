import Mathlib

namespace RavelGenerated

inductive QRGenerator
  | q
  | r
  deriving DecidableEq, Repr

def defectWord : Int → List QRGenerator
  | 0 => [QRGenerator.q]
  | d => List.replicate d.natAbs QRGenerator.r

/-- A nonzero prefix defect contributes exactly a positive residual word whose
length is the roof time; zero defect contributes the balanced generator. -/
theorem defectWord_length (d : Int) :
    (defectWord d).length = if d = 0 then 1 else d.natAbs := by
  by_cases h : d = 0
  · simp [h, defectWord]
  · simp [h, defectWord]

/-- The side voltage is independent of the positive word: negating a defect
preserves its Q/R word length and changes only its sign. -/
theorem neg_defect_same_roof (d : Int) :
    (defectWord (-d)).length = (defectWord d).length := by
  by_cases h : d = 0
  · simp [h, defectWord]
  · simp [h, defectWord, Int.natAbs_neg]

/-- For defects bounded by m, the renewal roof is bounded by max 1 m. -/
theorem defect_roof_bounded (m : Nat) (d : Int)
    (h : d.natAbs ≤ m) :
    (defectWord d).length ≤ max 1 m := by
  rw [defectWord_length]
  split
  · exact Nat.le_max_left _ _
  · exact le_trans h (Nat.le_max_right _ _)

end RavelGenerated
