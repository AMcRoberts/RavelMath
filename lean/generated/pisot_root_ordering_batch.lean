import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem pisot_root_strictly_between {hi3 lo4 hi4 beta3 beta4 target : ℝ}
    (h3hi : beta3 ≤ hi3) (h4lo : lo4 ≤ beta4) (h4hi : beta4 ≤ hi4)
    (hgap : hi3 < lo4) (hbound : hi4 < target) :
    beta3 < beta4 ∧ beta4 < target := by
  constructor <;> linarith

/-- Mechanically emitted: instantiates the general lemma above for the
    a-bonacci family at a=1 (Finding 29). -/
theorem pisot_root_ordering_instance_0 {beta3 beta4 : ℝ}
    (h3hi : beta3 ≤ (4044634348345 : ℝ) / (2199023255552 : ℝ))
    (h4lo : (1059688402651 : ℝ) / (549755813888 : ℝ) ≤ beta4)
    (h4hi : beta4 ≤ (4238753610605 : ℝ) / (2199023255552 : ℝ)) :
    beta3 < beta4 ∧ beta4 < (2 : ℝ) := by
  have hgap : (4044634348345 : ℝ) / (2199023255552 : ℝ) < (1059688402651 : ℝ) / (549755813888 : ℝ) := by norm_num
  have hbound : (4238753610605 : ℝ) / (2199023255552 : ℝ) < (2 : ℝ) := by norm_num
  exact pisot_root_strictly_between h3hi h4lo h4hi hgap hbound

/-- Mechanically emitted: instantiates the general lemma above for the
    a-bonacci family at a=2 (Finding 29). -/
theorem pisot_root_ordering_instance_1 {beta3 beta4 : ℝ}
    (h3hi : beta3 ≤ (6420355303111 : ℝ) / (2199023255552 : ℝ))
    (h4lo : (6540883061229 : ℝ) / (2199023255552 : ℝ) ≤ beta4)
    (h4hi : beta4 ≤ (3270441530615 : ℝ) / (1099511627776 : ℝ)) :
    beta3 < beta4 ∧ beta4 < (3 : ℝ) := by
  have hgap : (6420355303111 : ℝ) / (2199023255552 : ℝ) < (6540883061229 : ℝ) / (2199023255552 : ℝ) := by norm_num
  have hbound : (3270441530615 : ℝ) / (1099511627776 : ℝ) < (3 : ℝ) := by norm_num
  exact pisot_root_strictly_between h3hi h4lo h4hi hgap hbound

/-- Mechanically emitted: instantiates the general lemma above for the
    a-bonacci family at a=3 (Finding 29). -/
theorem pisot_root_ordering_instance_2 {beta3 beta4 : ℝ}
    (h3hi : beta3 ≤ (2172290299157 : ℝ) / (549755813888 : ℝ))
    (h4lo : (274062979441 : ℝ) / (68719476736 : ℝ) ≤ beta4)
    (h4hi : beta4 ≤ (17540030684227 : ℝ) / (4398046511104 : ℝ)) :
    beta3 < beta4 ∧ beta4 < (4 : ℝ) := by
  have hgap : (2172290299157 : ℝ) / (549755813888 : ℝ) < (274062979441 : ℝ) / (68719476736 : ℝ) := by norm_num
  have hbound : (17540030684227 : ℝ) / (4398046511104 : ℝ) < (4 : ℝ) := by norm_num
  exact pisot_root_strictly_between h3hi h4lo h4hi hgap hbound

/-- Mechanically emitted: instantiates the general lemma above for the
    a-bonacci family at a=4 (Finding 29). -/
theorem pisot_root_ordering_instance_3 {beta3 beta4 : ℝ}
    (h3hi : beta3 ≤ (10923351464749 : ℝ) / (2199023255552 : ℝ))
    (h4lo : (5490484932861 : ℝ) / (1099511627776 : ℝ) ≤ beta4)
    (h4hi : beta4 ≤ (10980969865723 : ℝ) / (2199023255552 : ℝ)) :
    beta3 < beta4 ∧ beta4 < (5 : ℝ) := by
  have hgap : (10923351464749 : ℝ) / (2199023255552 : ℝ) < (5490484932861 : ℝ) / (1099511627776 : ℝ) := by norm_num
  have hbound : (10980969865723 : ℝ) / (2199023255552 : ℝ) < (5 : ℝ) := by norm_num
  exact pisot_root_strictly_between h3hi h4lo h4hi hgap hbound

/-- Mechanically emitted: instantiates the general lemma above for the
    a-bonacci family at a=5 (Finding 29). -/
theorem pisot_root_ordering_instance_4 {beta3 beta4 : ℝ}
    (h3hi : beta3 ≤ (13142635420417 : ℝ) / (2199023255552 : ℝ))
    (h4lo : (13185633734937 : ℝ) / (2199023255552 : ℝ) ≤ beta4)
    (h4hi : beta4 ≤ (52742534939753 : ℝ) / (8796093022208 : ℝ)) :
    beta3 < beta4 ∧ beta4 < (6 : ℝ) := by
  have hgap : (13142635420417 : ℝ) / (2199023255552 : ℝ) < (13185633734937 : ℝ) / (2199023255552 : ℝ) := by norm_num
  have hbound : (52742534939753 : ℝ) / (8796093022208 : ℝ) < (6 : ℝ) := by norm_num
  exact pisot_root_strictly_between h3hi h4lo h4hi hgap hbound

/- Semantic proof graph for: pisot_root_ordering_batch
  [0] lean.pisot_root_ordering_certificate :: a=1 -- instantiates pisot_root_strictly_between
  [1] lean.pisot_root_ordering_certificate :: a=2 -- instantiates pisot_root_strictly_between
  [2] lean.pisot_root_ordering_certificate :: a=3 -- instantiates pisot_root_strictly_between
  [3] lean.pisot_root_ordering_certificate :: a=4 -- instantiates pisot_root_strictly_between
  [4] lean.pisot_root_ordering_certificate :: a=5 -- instantiates pisot_root_strictly_between
-/

def reflectedNodeCount : Nat := 5

end RavelGenerated
