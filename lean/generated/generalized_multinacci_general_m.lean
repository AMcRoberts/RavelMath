import Mathlib

namespace RavelGenerated

inductive PrimitiveGenerator
  | q
  | r
  deriving DecidableEq, Repr

/-- The unsigned primitive word attached to a prefix-cut defect of magnitude d. -/
def defectWord : ℕ → List PrimitiveGenerator
  | 0 => [PrimitiveGenerator.q]
  | d + 1 => List.replicate (d + 1) PrimitiveGenerator.r

/-- Number of ordered pairs (p,q) in {0,...,m}² with |q-p|=d. -/
def schedulerCoefficient (m d : ℕ) : ℕ :=
  if d = 0 then m + 1
  else if d ≤ m then 2 * (m + 1 - d)
  else 0

 theorem defectWord_zero : defectWord 0 = [PrimitiveGenerator.q] := rfl

 theorem defectWord_succ (d : ℕ) :
    defectWord (d + 1) = List.replicate (d + 1) PrimitiveGenerator.r := by
  rfl

 theorem defectWord_length (d : ℕ) :
    (defectWord d).length = if d = 0 then 1 else d := by
  cases d with
  | zero => simp [defectWord]
  | succ d => simp [defectWord]

 theorem schedulerCoefficient_zero (m : ℕ) :
    schedulerCoefficient m 0 = m + 1 := by
  simp [schedulerCoefficient]

 theorem schedulerCoefficient_positive
    (m d : ℕ) (hd0 : 0 < d) (hdm : d ≤ m) :
    schedulerCoefficient m d = 2 * (m + 1 - d) := by
  simp [schedulerCoefficient, Nat.ne_of_gt hd0, hdm]

 theorem schedulerCoefficient_outside
    (m d : ℕ) (h : m < d) : schedulerCoefficient m d = 0 := by
  have hd0 : d ≠ 0 := by omega
  have hnot : ¬ d ≤ m := by omega
  simp [schedulerCoefficient, hd0, hnot]

/-- Every generalized-multinacci macro generator is Q or a positive power of R. -/
theorem general_m_primitive_alphabet (d : ℕ) :
    d = 0 → defectWord d = [PrimitiveGenerator.q] ∧
    0 < d → defectWord d = List.replicate d PrimitiveGenerator.r := by
  constructor
  · intro h
    subst d
    simp [defectWord]
  · intro hd
    cases d with
    | zero => omega
    | succ d => simp [defectWord]

/-- The roof of a prefix-position channel is bounded by m. -/
theorem prefix_defect_roof_bound
    (m p q : ℕ) (hp : p ≤ m) (hq : q ≤ m) :
    Nat.dist p q ≤ m := by
  rw [Nat.dist_eq]
  split
  · omega
  · omega

end RavelGenerated
