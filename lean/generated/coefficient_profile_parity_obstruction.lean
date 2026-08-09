import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem nearest_left_profile_even_has_minus_one_root
    (k : ℕ) :
    (-1 : ℤ)^(2*k+4) -
      (∑ j ∈ Finset.Icc 2 (2*k+3), (-1 : ℤ)^j) - 1 = 0 := by
  induction k with
  | zero => decide
  | succ k ih =>
      have step1 : (2:ℕ)*(k+1)+3 = (2*k+3) + 1 + 1 := by ring
      rw [step1, Finset.sum_Icc_succ_top (by omega), Finset.sum_Icc_succ_top (by omega)]
      have e1 : (-1:ℤ)^(2*(k+1)+4) = (-1:ℤ)^(2*k+4) := by
        rw [show 2*(k+1)+4 = 2*k+4+2 by ring, pow_add]; ring
      rw [e1]
      ring_nf
      ring_nf at ih
      linarith [ih]

-- Companion to the even case above: for odd D the same alternating-sum
-- expression is a fixed nonzero constant (-3), not merely "not verified to be
-- zero". Algebraically: writing D=2k+3, the full alternating sum of D terms is
-- 1 when D is odd (vs. 0 when even), and deleting the profile's single interior
-- zero at position D-2 adds back exactly 1 to that sum, giving
-- (-1)^D - (fullSum + 1) - 1 = -1 - 1 - 1 = -3 uniformly, independent of k. This
-- closes the converse direction the even-case theorem alone leaves open: the
-- C++ certificate checks BOTH `D even -> value = 0` and `D odd -> value != 0`;
-- previously only the first half had a proven general theorem behind it.
theorem nearest_left_profile_odd_value
    (k : ℕ) :
    (-1 : ℤ)^(2*k+3) -
      (∑ j ∈ Finset.Icc 2 (2*k+2), (-1 : ℤ)^j) - 1 = -3 := by
  induction k with
  | zero => decide
  | succ k ih =>
      have step1 : (2:ℕ)*(k+1)+2 = (2*k+2) + 1 + 1 := by ring
      rw [step1, Finset.sum_Icc_succ_top (by omega), Finset.sum_Icc_succ_top (by omega)]
      have e1 : (-1:ℤ)^(2*(k+1)+3) = (-1:ℤ)^(2*k+3) := by
        rw [show 2*(k+1)+3 = 2*k+3+2 by ring, pow_add]; ring
      rw [e1]
      ring_nf
      ring_nf at ih
      linarith [ih]

/-- Mechanically emitted: D=3 D=3 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_0 :
    (-1 : ℤ)^(2*0+3) - (∑ j ∈ Finset.Icc 2 (2*0+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 0

/-- Mechanically emitted: D=4 D=4 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_1 :
    (-1 : ℤ)^(2*0+4) - (∑ j ∈ Finset.Icc 2 (2*0+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 0

/-- Mechanically emitted: D=5 D=5 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_2 :
    (-1 : ℤ)^(2*1+3) - (∑ j ∈ Finset.Icc 2 (2*1+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 1

/-- Mechanically emitted: D=6 D=6 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_3 :
    (-1 : ℤ)^(2*1+4) - (∑ j ∈ Finset.Icc 2 (2*1+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 1

/-- Mechanically emitted: D=7 D=7 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_4 :
    (-1 : ℤ)^(2*2+3) - (∑ j ∈ Finset.Icc 2 (2*2+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 2

/-- Mechanically emitted: D=8 D=8 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_5 :
    (-1 : ℤ)^(2*2+4) - (∑ j ∈ Finset.Icc 2 (2*2+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 2

/-- Mechanically emitted: D=9 D=9 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_6 :
    (-1 : ℤ)^(2*3+3) - (∑ j ∈ Finset.Icc 2 (2*3+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 3

/-- Mechanically emitted: D=10 D=10 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_7 :
    (-1 : ℤ)^(2*3+4) - (∑ j ∈ Finset.Icc 2 (2*3+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 3

/-- Mechanically emitted: D=11 D=11 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_8 :
    (-1 : ℤ)^(2*4+3) - (∑ j ∈ Finset.Icc 2 (2*4+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 4

/-- Mechanically emitted: D=12 D=12 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_9 :
    (-1 : ℤ)^(2*4+4) - (∑ j ∈ Finset.Icc 2 (2*4+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 4

/-- Mechanically emitted: D=13 D=13 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_10 :
    (-1 : ℤ)^(2*5+3) - (∑ j ∈ Finset.Icc 2 (2*5+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 5

/-- Mechanically emitted: D=14 D=14 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_11 :
    (-1 : ℤ)^(2*5+4) - (∑ j ∈ Finset.Icc 2 (2*5+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 5

/-- Mechanically emitted: D=15 D=15 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_12 :
    (-1 : ℤ)^(2*6+3) - (∑ j ∈ Finset.Icc 2 (2*6+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 6

/-- Mechanically emitted: D=16 D=16 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_13 :
    (-1 : ℤ)^(2*6+4) - (∑ j ∈ Finset.Icc 2 (2*6+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 6

/-- Mechanically emitted: D=17 D=17 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_14 :
    (-1 : ℤ)^(2*7+3) - (∑ j ∈ Finset.Icc 2 (2*7+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 7

/-- Mechanically emitted: D=18 D=18 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_15 :
    (-1 : ℤ)^(2*7+4) - (∑ j ∈ Finset.Icc 2 (2*7+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 7

/-- Mechanically emitted: D=19 D=19 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_16 :
    (-1 : ℤ)^(2*8+3) - (∑ j ∈ Finset.Icc 2 (2*8+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 8

/-- Mechanically emitted: D=20 D=20 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_17 :
    (-1 : ℤ)^(2*8+4) - (∑ j ∈ Finset.Icc 2 (2*8+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 8

/-- Mechanically emitted: D=21 D=21 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_18 :
    (-1 : ℤ)^(2*9+3) - (∑ j ∈ Finset.Icc 2 (2*9+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 9

/-- Mechanically emitted: D=22 D=22 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_19 :
    (-1 : ℤ)^(2*9+4) - (∑ j ∈ Finset.Icc 2 (2*9+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 9

/-- Mechanically emitted: D=23 D=23 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_20 :
    (-1 : ℤ)^(2*10+3) - (∑ j ∈ Finset.Icc 2 (2*10+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 10

/-- Mechanically emitted: D=24 D=24 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_21 :
    (-1 : ℤ)^(2*10+4) - (∑ j ∈ Finset.Icc 2 (2*10+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 10

/-- Mechanically emitted: D=25 D=25 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_22 :
    (-1 : ℤ)^(2*11+3) - (∑ j ∈ Finset.Icc 2 (2*11+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 11

/-- Mechanically emitted: D=26 D=26 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_23 :
    (-1 : ℤ)^(2*11+4) - (∑ j ∈ Finset.Icc 2 (2*11+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 11

/-- Mechanically emitted: D=27 D=27 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_24 :
    (-1 : ℤ)^(2*12+3) - (∑ j ∈ Finset.Icc 2 (2*12+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 12

/-- Mechanically emitted: D=28 D=28 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_25 :
    (-1 : ℤ)^(2*12+4) - (∑ j ∈ Finset.Icc 2 (2*12+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 12

/-- Mechanically emitted: D=29 D=29 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_26 :
    (-1 : ℤ)^(2*13+3) - (∑ j ∈ Finset.Icc 2 (2*13+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 13

/-- Mechanically emitted: D=30 D=30 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_27 :
    (-1 : ℤ)^(2*13+4) - (∑ j ∈ Finset.Icc 2 (2*13+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 13

/-- Mechanically emitted: D=31 D=31 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_28 :
    (-1 : ℤ)^(2*14+3) - (∑ j ∈ Finset.Icc 2 (2*14+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 14

/-- Mechanically emitted: D=32 D=32 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_29 :
    (-1 : ℤ)^(2*14+4) - (∑ j ∈ Finset.Icc 2 (2*14+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 14

/-- Mechanically emitted: D=33 D=33 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_30 :
    (-1 : ℤ)^(2*15+3) - (∑ j ∈ Finset.Icc 2 (2*15+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 15

/-- Mechanically emitted: D=34 D=34 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_31 :
    (-1 : ℤ)^(2*15+4) - (∑ j ∈ Finset.Icc 2 (2*15+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 15

/-- Mechanically emitted: D=35 D=35 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_32 :
    (-1 : ℤ)^(2*16+3) - (∑ j ∈ Finset.Icc 2 (2*16+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 16

/-- Mechanically emitted: D=36 D=36 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_33 :
    (-1 : ℤ)^(2*16+4) - (∑ j ∈ Finset.Icc 2 (2*16+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 16

/-- Mechanically emitted: D=37 D=37 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_34 :
    (-1 : ℤ)^(2*17+3) - (∑ j ∈ Finset.Icc 2 (2*17+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 17

/-- Mechanically emitted: D=38 D=38 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_35 :
    (-1 : ℤ)^(2*17+4) - (∑ j ∈ Finset.Icc 2 (2*17+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 17

/-- Mechanically emitted: D=39 D=39 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_36 :
    (-1 : ℤ)^(2*18+3) - (∑ j ∈ Finset.Icc 2 (2*18+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 18

/-- Mechanically emitted: D=40 D=40 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_37 :
    (-1 : ℤ)^(2*18+4) - (∑ j ∈ Finset.Icc 2 (2*18+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 18

/-- Mechanically emitted: D=41 D=41 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_38 :
    (-1 : ℤ)^(2*19+3) - (∑ j ∈ Finset.Icc 2 (2*19+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 19

/-- Mechanically emitted: D=42 D=42 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_39 :
    (-1 : ℤ)^(2*19+4) - (∑ j ∈ Finset.Icc 2 (2*19+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 19

/-- Mechanically emitted: D=43 D=43 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_40 :
    (-1 : ℤ)^(2*20+3) - (∑ j ∈ Finset.Icc 2 (2*20+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 20

/-- Mechanically emitted: D=44 D=44 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_41 :
    (-1 : ℤ)^(2*20+4) - (∑ j ∈ Finset.Icc 2 (2*20+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 20

/-- Mechanically emitted: D=45 D=45 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_42 :
    (-1 : ℤ)^(2*21+3) - (∑ j ∈ Finset.Icc 2 (2*21+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 21

/-- Mechanically emitted: D=46 D=46 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_43 :
    (-1 : ℤ)^(2*21+4) - (∑ j ∈ Finset.Icc 2 (2*21+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 21

/-- Mechanically emitted: D=47 D=47 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_44 :
    (-1 : ℤ)^(2*22+3) - (∑ j ∈ Finset.Icc 2 (2*22+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 22

/-- Mechanically emitted: D=48 D=48 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_45 :
    (-1 : ℤ)^(2*22+4) - (∑ j ∈ Finset.Icc 2 (2*22+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 22

/-- Mechanically emitted: D=49 D=49 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_46 :
    (-1 : ℤ)^(2*23+3) - (∑ j ∈ Finset.Icc 2 (2*23+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 23

/-- Mechanically emitted: D=50 D=50 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_47 :
    (-1 : ℤ)^(2*23+4) - (∑ j ∈ Finset.Icc 2 (2*23+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 23

/-- Mechanically emitted: D=51 D=51 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_48 :
    (-1 : ℤ)^(2*24+3) - (∑ j ∈ Finset.Icc 2 (2*24+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 24

/-- Mechanically emitted: D=52 D=52 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_49 :
    (-1 : ℤ)^(2*24+4) - (∑ j ∈ Finset.Icc 2 (2*24+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 24

/-- Mechanically emitted: D=53 D=53 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_50 :
    (-1 : ℤ)^(2*25+3) - (∑ j ∈ Finset.Icc 2 (2*25+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 25

/-- Mechanically emitted: D=54 D=54 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_51 :
    (-1 : ℤ)^(2*25+4) - (∑ j ∈ Finset.Icc 2 (2*25+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 25

/-- Mechanically emitted: D=55 D=55 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_52 :
    (-1 : ℤ)^(2*26+3) - (∑ j ∈ Finset.Icc 2 (2*26+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 26

/-- Mechanically emitted: D=56 D=56 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_53 :
    (-1 : ℤ)^(2*26+4) - (∑ j ∈ Finset.Icc 2 (2*26+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 26

/-- Mechanically emitted: D=57 D=57 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_54 :
    (-1 : ℤ)^(2*27+3) - (∑ j ∈ Finset.Icc 2 (2*27+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 27

/-- Mechanically emitted: D=58 D=58 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_55 :
    (-1 : ℤ)^(2*27+4) - (∑ j ∈ Finset.Icc 2 (2*27+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 27

/-- Mechanically emitted: D=59 D=59 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_56 :
    (-1 : ℤ)^(2*28+3) - (∑ j ∈ Finset.Icc 2 (2*28+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 28

/-- Mechanically emitted: D=60 D=60 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_57 :
    (-1 : ℤ)^(2*28+4) - (∑ j ∈ Finset.Icc 2 (2*28+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 28

/-- Mechanically emitted: D=61 D=61 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_58 :
    (-1 : ℤ)^(2*29+3) - (∑ j ∈ Finset.Icc 2 (2*29+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 29

/-- Mechanically emitted: D=62 D=62 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_59 :
    (-1 : ℤ)^(2*29+4) - (∑ j ∈ Finset.Icc 2 (2*29+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 29

/-- Mechanically emitted: D=63 D=63 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_60 :
    (-1 : ℤ)^(2*30+3) - (∑ j ∈ Finset.Icc 2 (2*30+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 30

/-- Mechanically emitted: D=64 D=64 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_61 :
    (-1 : ℤ)^(2*30+4) - (∑ j ∈ Finset.Icc 2 (2*30+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 30

/-- Mechanically emitted: D=65 D=65 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_62 :
    (-1 : ℤ)^(2*31+3) - (∑ j ∈ Finset.Icc 2 (2*31+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 31

/-- Mechanically emitted: D=66 D=66 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_63 :
    (-1 : ℤ)^(2*31+4) - (∑ j ∈ Finset.Icc 2 (2*31+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 31

/-- Mechanically emitted: D=67 D=67 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_64 :
    (-1 : ℤ)^(2*32+3) - (∑ j ∈ Finset.Icc 2 (2*32+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 32

/-- Mechanically emitted: D=68 D=68 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_65 :
    (-1 : ℤ)^(2*32+4) - (∑ j ∈ Finset.Icc 2 (2*32+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 32

/-- Mechanically emitted: D=69 D=69 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_66 :
    (-1 : ℤ)^(2*33+3) - (∑ j ∈ Finset.Icc 2 (2*33+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 33

/-- Mechanically emitted: D=70 D=70 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_67 :
    (-1 : ℤ)^(2*33+4) - (∑ j ∈ Finset.Icc 2 (2*33+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 33

/-- Mechanically emitted: D=71 D=71 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_68 :
    (-1 : ℤ)^(2*34+3) - (∑ j ∈ Finset.Icc 2 (2*34+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 34

/-- Mechanically emitted: D=72 D=72 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_69 :
    (-1 : ℤ)^(2*34+4) - (∑ j ∈ Finset.Icc 2 (2*34+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 34

/-- Mechanically emitted: D=73 D=73 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_70 :
    (-1 : ℤ)^(2*35+3) - (∑ j ∈ Finset.Icc 2 (2*35+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 35

/-- Mechanically emitted: D=74 D=74 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_71 :
    (-1 : ℤ)^(2*35+4) - (∑ j ∈ Finset.Icc 2 (2*35+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 35

/-- Mechanically emitted: D=75 D=75 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_72 :
    (-1 : ℤ)^(2*36+3) - (∑ j ∈ Finset.Icc 2 (2*36+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 36

/-- Mechanically emitted: D=76 D=76 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_73 :
    (-1 : ℤ)^(2*36+4) - (∑ j ∈ Finset.Icc 2 (2*36+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 36

/-- Mechanically emitted: D=77 D=77 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_74 :
    (-1 : ℤ)^(2*37+3) - (∑ j ∈ Finset.Icc 2 (2*37+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 37

/-- Mechanically emitted: D=78 D=78 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_75 :
    (-1 : ℤ)^(2*37+4) - (∑ j ∈ Finset.Icc 2 (2*37+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 37

/-- Mechanically emitted: D=79 D=79 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_76 :
    (-1 : ℤ)^(2*38+3) - (∑ j ∈ Finset.Icc 2 (2*38+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 38

/-- Mechanically emitted: D=80 D=80 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_77 :
    (-1 : ℤ)^(2*38+4) - (∑ j ∈ Finset.Icc 2 (2*38+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 38

/-- Mechanically emitted: D=81 D=81 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_78 :
    (-1 : ℤ)^(2*39+3) - (∑ j ∈ Finset.Icc 2 (2*39+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 39

/-- Mechanically emitted: D=82 D=82 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_79 :
    (-1 : ℤ)^(2*39+4) - (∑ j ∈ Finset.Icc 2 (2*39+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 39

/-- Mechanically emitted: D=83 D=83 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_80 :
    (-1 : ℤ)^(2*40+3) - (∑ j ∈ Finset.Icc 2 (2*40+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 40

/-- Mechanically emitted: D=84 D=84 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_81 :
    (-1 : ℤ)^(2*40+4) - (∑ j ∈ Finset.Icc 2 (2*40+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 40

/-- Mechanically emitted: D=85 D=85 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_82 :
    (-1 : ℤ)^(2*41+3) - (∑ j ∈ Finset.Icc 2 (2*41+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 41

/-- Mechanically emitted: D=86 D=86 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_83 :
    (-1 : ℤ)^(2*41+4) - (∑ j ∈ Finset.Icc 2 (2*41+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 41

/-- Mechanically emitted: D=87 D=87 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_84 :
    (-1 : ℤ)^(2*42+3) - (∑ j ∈ Finset.Icc 2 (2*42+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 42

/-- Mechanically emitted: D=88 D=88 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_85 :
    (-1 : ℤ)^(2*42+4) - (∑ j ∈ Finset.Icc 2 (2*42+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 42

/-- Mechanically emitted: D=89 D=89 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_86 :
    (-1 : ℤ)^(2*43+3) - (∑ j ∈ Finset.Icc 2 (2*43+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 43

/-- Mechanically emitted: D=90 D=90 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_87 :
    (-1 : ℤ)^(2*43+4) - (∑ j ∈ Finset.Icc 2 (2*43+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 43

/-- Mechanically emitted: D=91 D=91 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_88 :
    (-1 : ℤ)^(2*44+3) - (∑ j ∈ Finset.Icc 2 (2*44+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 44

/-- Mechanically emitted: D=92 D=92 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_89 :
    (-1 : ℤ)^(2*44+4) - (∑ j ∈ Finset.Icc 2 (2*44+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 44

/-- Mechanically emitted: D=93 D=93 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_90 :
    (-1 : ℤ)^(2*45+3) - (∑ j ∈ Finset.Icc 2 (2*45+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 45

/-- Mechanically emitted: D=94 D=94 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_91 :
    (-1 : ℤ)^(2*45+4) - (∑ j ∈ Finset.Icc 2 (2*45+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 45

/-- Mechanically emitted: D=95 D=95 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_92 :
    (-1 : ℤ)^(2*46+3) - (∑ j ∈ Finset.Icc 2 (2*46+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 46

/-- Mechanically emitted: D=96 D=96 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_93 :
    (-1 : ℤ)^(2*46+4) - (∑ j ∈ Finset.Icc 2 (2*46+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 46

/-- Mechanically emitted: D=97 D=97 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_94 :
    (-1 : ℤ)^(2*47+3) - (∑ j ∈ Finset.Icc 2 (2*47+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 47

/-- Mechanically emitted: D=98 D=98 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_95 :
    (-1 : ℤ)^(2*47+4) - (∑ j ∈ Finset.Icc 2 (2*47+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 47

/-- Mechanically emitted: D=99 D=99 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_96 :
    (-1 : ℤ)^(2*48+3) - (∑ j ∈ Finset.Icc 2 (2*48+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 48

/-- Mechanically emitted: D=100 D=100 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_97 :
    (-1 : ℤ)^(2*48+4) - (∑ j ∈ Finset.Icc 2 (2*48+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 48

/-- Mechanically emitted: D=101 D=101 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_98 :
    (-1 : ℤ)^(2*49+3) - (∑ j ∈ Finset.Icc 2 (2*49+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 49

/-- Mechanically emitted: D=102 D=102 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_99 :
    (-1 : ℤ)^(2*49+4) - (∑ j ∈ Finset.Icc 2 (2*49+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 49

/-- Mechanically emitted: D=103 D=103 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_100 :
    (-1 : ℤ)^(2*50+3) - (∑ j ∈ Finset.Icc 2 (2*50+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 50

/-- Mechanically emitted: D=104 D=104 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_101 :
    (-1 : ℤ)^(2*50+4) - (∑ j ∈ Finset.Icc 2 (2*50+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 50

/-- Mechanically emitted: D=105 D=105 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_102 :
    (-1 : ℤ)^(2*51+3) - (∑ j ∈ Finset.Icc 2 (2*51+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 51

/-- Mechanically emitted: D=106 D=106 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_103 :
    (-1 : ℤ)^(2*51+4) - (∑ j ∈ Finset.Icc 2 (2*51+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 51

/-- Mechanically emitted: D=107 D=107 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_104 :
    (-1 : ℤ)^(2*52+3) - (∑ j ∈ Finset.Icc 2 (2*52+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 52

/-- Mechanically emitted: D=108 D=108 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_105 :
    (-1 : ℤ)^(2*52+4) - (∑ j ∈ Finset.Icc 2 (2*52+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 52

/-- Mechanically emitted: D=109 D=109 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_106 :
    (-1 : ℤ)^(2*53+3) - (∑ j ∈ Finset.Icc 2 (2*53+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 53

/-- Mechanically emitted: D=110 D=110 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_107 :
    (-1 : ℤ)^(2*53+4) - (∑ j ∈ Finset.Icc 2 (2*53+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 53

/-- Mechanically emitted: D=111 D=111 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_108 :
    (-1 : ℤ)^(2*54+3) - (∑ j ∈ Finset.Icc 2 (2*54+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 54

/-- Mechanically emitted: D=112 D=112 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_109 :
    (-1 : ℤ)^(2*54+4) - (∑ j ∈ Finset.Icc 2 (2*54+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 54

/-- Mechanically emitted: D=113 D=113 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_110 :
    (-1 : ℤ)^(2*55+3) - (∑ j ∈ Finset.Icc 2 (2*55+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 55

/-- Mechanically emitted: D=114 D=114 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_111 :
    (-1 : ℤ)^(2*55+4) - (∑ j ∈ Finset.Icc 2 (2*55+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 55

/-- Mechanically emitted: D=115 D=115 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_112 :
    (-1 : ℤ)^(2*56+3) - (∑ j ∈ Finset.Icc 2 (2*56+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 56

/-- Mechanically emitted: D=116 D=116 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_113 :
    (-1 : ℤ)^(2*56+4) - (∑ j ∈ Finset.Icc 2 (2*56+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 56

/-- Mechanically emitted: D=117 D=117 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_114 :
    (-1 : ℤ)^(2*57+3) - (∑ j ∈ Finset.Icc 2 (2*57+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 57

/-- Mechanically emitted: D=118 D=118 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_115 :
    (-1 : ℤ)^(2*57+4) - (∑ j ∈ Finset.Icc 2 (2*57+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 57

/-- Mechanically emitted: D=119 D=119 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_116 :
    (-1 : ℤ)^(2*58+3) - (∑ j ∈ Finset.Icc 2 (2*58+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 58

/-- Mechanically emitted: D=120 D=120 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_117 :
    (-1 : ℤ)^(2*58+4) - (∑ j ∈ Finset.Icc 2 (2*58+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 58

/-- Mechanically emitted: D=121 D=121 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_118 :
    (-1 : ℤ)^(2*59+3) - (∑ j ∈ Finset.Icc 2 (2*59+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 59

/-- Mechanically emitted: D=122 D=122 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_119 :
    (-1 : ℤ)^(2*59+4) - (∑ j ∈ Finset.Icc 2 (2*59+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 59

/-- Mechanically emitted: D=123 D=123 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_120 :
    (-1 : ℤ)^(2*60+3) - (∑ j ∈ Finset.Icc 2 (2*60+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 60

/-- Mechanically emitted: D=124 D=124 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_121 :
    (-1 : ℤ)^(2*60+4) - (∑ j ∈ Finset.Icc 2 (2*60+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 60

/-- Mechanically emitted: D=125 D=125 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_122 :
    (-1 : ℤ)^(2*61+3) - (∑ j ∈ Finset.Icc 2 (2*61+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 61

/-- Mechanically emitted: D=126 D=126 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_123 :
    (-1 : ℤ)^(2*61+4) - (∑ j ∈ Finset.Icc 2 (2*61+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 61

/-- Mechanically emitted: D=127 D=127 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_124 :
    (-1 : ℤ)^(2*62+3) - (∑ j ∈ Finset.Icc 2 (2*62+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 62

/-- Mechanically emitted: D=128 D=128 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_125 :
    (-1 : ℤ)^(2*62+4) - (∑ j ∈ Finset.Icc 2 (2*62+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 62

/-- Mechanically emitted: D=129 D=129 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_126 :
    (-1 : ℤ)^(2*63+3) - (∑ j ∈ Finset.Icc 2 (2*63+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 63

/-- Mechanically emitted: D=130 D=130 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_127 :
    (-1 : ℤ)^(2*63+4) - (∑ j ∈ Finset.Icc 2 (2*63+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 63

/-- Mechanically emitted: D=131 D=131 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_128 :
    (-1 : ℤ)^(2*64+3) - (∑ j ∈ Finset.Icc 2 (2*64+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 64

/-- Mechanically emitted: D=132 D=132 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_129 :
    (-1 : ℤ)^(2*64+4) - (∑ j ∈ Finset.Icc 2 (2*64+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 64

/-- Mechanically emitted: D=133 D=133 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_130 :
    (-1 : ℤ)^(2*65+3) - (∑ j ∈ Finset.Icc 2 (2*65+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 65

/-- Mechanically emitted: D=134 D=134 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_131 :
    (-1 : ℤ)^(2*65+4) - (∑ j ∈ Finset.Icc 2 (2*65+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 65

/-- Mechanically emitted: D=135 D=135 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_132 :
    (-1 : ℤ)^(2*66+3) - (∑ j ∈ Finset.Icc 2 (2*66+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 66

/-- Mechanically emitted: D=136 D=136 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_133 :
    (-1 : ℤ)^(2*66+4) - (∑ j ∈ Finset.Icc 2 (2*66+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 66

/-- Mechanically emitted: D=137 D=137 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_134 :
    (-1 : ℤ)^(2*67+3) - (∑ j ∈ Finset.Icc 2 (2*67+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 67

/-- Mechanically emitted: D=138 D=138 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_135 :
    (-1 : ℤ)^(2*67+4) - (∑ j ∈ Finset.Icc 2 (2*67+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 67

/-- Mechanically emitted: D=139 D=139 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_136 :
    (-1 : ℤ)^(2*68+3) - (∑ j ∈ Finset.Icc 2 (2*68+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 68

/-- Mechanically emitted: D=140 D=140 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_137 :
    (-1 : ℤ)^(2*68+4) - (∑ j ∈ Finset.Icc 2 (2*68+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 68

/-- Mechanically emitted: D=141 D=141 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_138 :
    (-1 : ℤ)^(2*69+3) - (∑ j ∈ Finset.Icc 2 (2*69+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 69

/-- Mechanically emitted: D=142 D=142 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_139 :
    (-1 : ℤ)^(2*69+4) - (∑ j ∈ Finset.Icc 2 (2*69+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 69

/-- Mechanically emitted: D=143 D=143 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_140 :
    (-1 : ℤ)^(2*70+3) - (∑ j ∈ Finset.Icc 2 (2*70+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 70

/-- Mechanically emitted: D=144 D=144 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_141 :
    (-1 : ℤ)^(2*70+4) - (∑ j ∈ Finset.Icc 2 (2*70+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 70

/-- Mechanically emitted: D=145 D=145 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_142 :
    (-1 : ℤ)^(2*71+3) - (∑ j ∈ Finset.Icc 2 (2*71+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 71

/-- Mechanically emitted: D=146 D=146 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_143 :
    (-1 : ℤ)^(2*71+4) - (∑ j ∈ Finset.Icc 2 (2*71+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 71

/-- Mechanically emitted: D=147 D=147 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_144 :
    (-1 : ℤ)^(2*72+3) - (∑ j ∈ Finset.Icc 2 (2*72+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 72

/-- Mechanically emitted: D=148 D=148 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_145 :
    (-1 : ℤ)^(2*72+4) - (∑ j ∈ Finset.Icc 2 (2*72+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 72

/-- Mechanically emitted: D=149 D=149 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_146 :
    (-1 : ℤ)^(2*73+3) - (∑ j ∈ Finset.Icc 2 (2*73+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 73

/-- Mechanically emitted: D=150 D=150 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_147 :
    (-1 : ℤ)^(2*73+4) - (∑ j ∈ Finset.Icc 2 (2*73+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 73

/-- Mechanically emitted: D=151 D=151 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_148 :
    (-1 : ℤ)^(2*74+3) - (∑ j ∈ Finset.Icc 2 (2*74+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 74

/-- Mechanically emitted: D=152 D=152 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_149 :
    (-1 : ℤ)^(2*74+4) - (∑ j ∈ Finset.Icc 2 (2*74+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 74

/-- Mechanically emitted: D=153 D=153 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_150 :
    (-1 : ℤ)^(2*75+3) - (∑ j ∈ Finset.Icc 2 (2*75+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 75

/-- Mechanically emitted: D=154 D=154 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_151 :
    (-1 : ℤ)^(2*75+4) - (∑ j ∈ Finset.Icc 2 (2*75+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 75

/-- Mechanically emitted: D=155 D=155 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_152 :
    (-1 : ℤ)^(2*76+3) - (∑ j ∈ Finset.Icc 2 (2*76+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 76

/-- Mechanically emitted: D=156 D=156 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_153 :
    (-1 : ℤ)^(2*76+4) - (∑ j ∈ Finset.Icc 2 (2*76+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 76

/-- Mechanically emitted: D=157 D=157 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_154 :
    (-1 : ℤ)^(2*77+3) - (∑ j ∈ Finset.Icc 2 (2*77+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 77

/-- Mechanically emitted: D=158 D=158 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_155 :
    (-1 : ℤ)^(2*77+4) - (∑ j ∈ Finset.Icc 2 (2*77+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 77

/-- Mechanically emitted: D=159 D=159 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_156 :
    (-1 : ℤ)^(2*78+3) - (∑ j ∈ Finset.Icc 2 (2*78+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 78

/-- Mechanically emitted: D=160 D=160 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_157 :
    (-1 : ℤ)^(2*78+4) - (∑ j ∈ Finset.Icc 2 (2*78+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 78

/-- Mechanically emitted: D=161 D=161 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_158 :
    (-1 : ℤ)^(2*79+3) - (∑ j ∈ Finset.Icc 2 (2*79+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 79

/-- Mechanically emitted: D=162 D=162 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_159 :
    (-1 : ℤ)^(2*79+4) - (∑ j ∈ Finset.Icc 2 (2*79+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 79

/-- Mechanically emitted: D=163 D=163 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_160 :
    (-1 : ℤ)^(2*80+3) - (∑ j ∈ Finset.Icc 2 (2*80+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 80

/-- Mechanically emitted: D=164 D=164 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_161 :
    (-1 : ℤ)^(2*80+4) - (∑ j ∈ Finset.Icc 2 (2*80+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 80

/-- Mechanically emitted: D=165 D=165 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_162 :
    (-1 : ℤ)^(2*81+3) - (∑ j ∈ Finset.Icc 2 (2*81+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 81

/-- Mechanically emitted: D=166 D=166 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_163 :
    (-1 : ℤ)^(2*81+4) - (∑ j ∈ Finset.Icc 2 (2*81+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 81

/-- Mechanically emitted: D=167 D=167 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_164 :
    (-1 : ℤ)^(2*82+3) - (∑ j ∈ Finset.Icc 2 (2*82+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 82

/-- Mechanically emitted: D=168 D=168 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_165 :
    (-1 : ℤ)^(2*82+4) - (∑ j ∈ Finset.Icc 2 (2*82+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 82

/-- Mechanically emitted: D=169 D=169 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_166 :
    (-1 : ℤ)^(2*83+3) - (∑ j ∈ Finset.Icc 2 (2*83+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 83

/-- Mechanically emitted: D=170 D=170 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_167 :
    (-1 : ℤ)^(2*83+4) - (∑ j ∈ Finset.Icc 2 (2*83+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 83

/-- Mechanically emitted: D=171 D=171 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_168 :
    (-1 : ℤ)^(2*84+3) - (∑ j ∈ Finset.Icc 2 (2*84+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 84

/-- Mechanically emitted: D=172 D=172 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_169 :
    (-1 : ℤ)^(2*84+4) - (∑ j ∈ Finset.Icc 2 (2*84+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 84

/-- Mechanically emitted: D=173 D=173 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_170 :
    (-1 : ℤ)^(2*85+3) - (∑ j ∈ Finset.Icc 2 (2*85+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 85

/-- Mechanically emitted: D=174 D=174 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_171 :
    (-1 : ℤ)^(2*85+4) - (∑ j ∈ Finset.Icc 2 (2*85+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 85

/-- Mechanically emitted: D=175 D=175 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_172 :
    (-1 : ℤ)^(2*86+3) - (∑ j ∈ Finset.Icc 2 (2*86+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 86

/-- Mechanically emitted: D=176 D=176 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_173 :
    (-1 : ℤ)^(2*86+4) - (∑ j ∈ Finset.Icc 2 (2*86+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 86

/-- Mechanically emitted: D=177 D=177 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_174 :
    (-1 : ℤ)^(2*87+3) - (∑ j ∈ Finset.Icc 2 (2*87+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 87

/-- Mechanically emitted: D=178 D=178 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_175 :
    (-1 : ℤ)^(2*87+4) - (∑ j ∈ Finset.Icc 2 (2*87+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 87

/-- Mechanically emitted: D=179 D=179 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_176 :
    (-1 : ℤ)^(2*88+3) - (∑ j ∈ Finset.Icc 2 (2*88+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 88

/-- Mechanically emitted: D=180 D=180 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_177 :
    (-1 : ℤ)^(2*88+4) - (∑ j ∈ Finset.Icc 2 (2*88+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 88

/-- Mechanically emitted: D=181 D=181 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_178 :
    (-1 : ℤ)^(2*89+3) - (∑ j ∈ Finset.Icc 2 (2*89+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 89

/-- Mechanically emitted: D=182 D=182 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_179 :
    (-1 : ℤ)^(2*89+4) - (∑ j ∈ Finset.Icc 2 (2*89+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 89

/-- Mechanically emitted: D=183 D=183 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_180 :
    (-1 : ℤ)^(2*90+3) - (∑ j ∈ Finset.Icc 2 (2*90+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 90

/-- Mechanically emitted: D=184 D=184 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_181 :
    (-1 : ℤ)^(2*90+4) - (∑ j ∈ Finset.Icc 2 (2*90+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 90

/-- Mechanically emitted: D=185 D=185 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_182 :
    (-1 : ℤ)^(2*91+3) - (∑ j ∈ Finset.Icc 2 (2*91+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 91

/-- Mechanically emitted: D=186 D=186 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_183 :
    (-1 : ℤ)^(2*91+4) - (∑ j ∈ Finset.Icc 2 (2*91+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 91

/-- Mechanically emitted: D=187 D=187 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_184 :
    (-1 : ℤ)^(2*92+3) - (∑ j ∈ Finset.Icc 2 (2*92+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 92

/-- Mechanically emitted: D=188 D=188 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_185 :
    (-1 : ℤ)^(2*92+4) - (∑ j ∈ Finset.Icc 2 (2*92+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 92

/-- Mechanically emitted: D=189 D=189 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_186 :
    (-1 : ℤ)^(2*93+3) - (∑ j ∈ Finset.Icc 2 (2*93+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 93

/-- Mechanically emitted: D=190 D=190 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_187 :
    (-1 : ℤ)^(2*93+4) - (∑ j ∈ Finset.Icc 2 (2*93+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 93

/-- Mechanically emitted: D=191 D=191 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_188 :
    (-1 : ℤ)^(2*94+3) - (∑ j ∈ Finset.Icc 2 (2*94+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 94

/-- Mechanically emitted: D=192 D=192 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_189 :
    (-1 : ℤ)^(2*94+4) - (∑ j ∈ Finset.Icc 2 (2*94+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 94

/-- Mechanically emitted: D=193 D=193 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_190 :
    (-1 : ℤ)^(2*95+3) - (∑ j ∈ Finset.Icc 2 (2*95+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 95

/-- Mechanically emitted: D=194 D=194 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_191 :
    (-1 : ℤ)^(2*95+4) - (∑ j ∈ Finset.Icc 2 (2*95+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 95

/-- Mechanically emitted: D=195 D=195 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_192 :
    (-1 : ℤ)^(2*96+3) - (∑ j ∈ Finset.Icc 2 (2*96+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 96

/-- Mechanically emitted: D=196 D=196 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_193 :
    (-1 : ℤ)^(2*96+4) - (∑ j ∈ Finset.Icc 2 (2*96+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 96

/-- Mechanically emitted: D=197 D=197 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_194 :
    (-1 : ℤ)^(2*97+3) - (∑ j ∈ Finset.Icc 2 (2*97+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 97

/-- Mechanically emitted: D=198 D=198 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_195 :
    (-1 : ℤ)^(2*97+4) - (∑ j ∈ Finset.Icc 2 (2*97+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 97

/-- Mechanically emitted: D=199 D=199 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_196 :
    (-1 : ℤ)^(2*98+3) - (∑ j ∈ Finset.Icc 2 (2*98+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 98

/-- Mechanically emitted: D=200 D=200 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_197 :
    (-1 : ℤ)^(2*98+4) - (∑ j ∈ Finset.Icc 2 (2*98+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 98

/-- Mechanically emitted: D=201 D=201 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_198 :
    (-1 : ℤ)^(2*99+3) - (∑ j ∈ Finset.Icc 2 (2*99+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 99

/-- Mechanically emitted: D=202 D=202 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_199 :
    (-1 : ℤ)^(2*99+4) - (∑ j ∈ Finset.Icc 2 (2*99+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 99

/-- Mechanically emitted: D=203 D=203 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_200 :
    (-1 : ℤ)^(2*100+3) - (∑ j ∈ Finset.Icc 2 (2*100+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 100

/-- Mechanically emitted: D=204 D=204 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_201 :
    (-1 : ℤ)^(2*100+4) - (∑ j ∈ Finset.Icc 2 (2*100+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 100

/-- Mechanically emitted: D=205 D=205 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_202 :
    (-1 : ℤ)^(2*101+3) - (∑ j ∈ Finset.Icc 2 (2*101+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 101

/-- Mechanically emitted: D=206 D=206 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_203 :
    (-1 : ℤ)^(2*101+4) - (∑ j ∈ Finset.Icc 2 (2*101+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 101

/-- Mechanically emitted: D=207 D=207 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_204 :
    (-1 : ℤ)^(2*102+3) - (∑ j ∈ Finset.Icc 2 (2*102+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 102

/-- Mechanically emitted: D=208 D=208 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_205 :
    (-1 : ℤ)^(2*102+4) - (∑ j ∈ Finset.Icc 2 (2*102+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 102

/-- Mechanically emitted: D=209 D=209 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_206 :
    (-1 : ℤ)^(2*103+3) - (∑ j ∈ Finset.Icc 2 (2*103+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 103

/-- Mechanically emitted: D=210 D=210 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_207 :
    (-1 : ℤ)^(2*103+4) - (∑ j ∈ Finset.Icc 2 (2*103+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 103

/-- Mechanically emitted: D=211 D=211 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_208 :
    (-1 : ℤ)^(2*104+3) - (∑ j ∈ Finset.Icc 2 (2*104+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 104

/-- Mechanically emitted: D=212 D=212 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_209 :
    (-1 : ℤ)^(2*104+4) - (∑ j ∈ Finset.Icc 2 (2*104+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 104

/-- Mechanically emitted: D=213 D=213 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_210 :
    (-1 : ℤ)^(2*105+3) - (∑ j ∈ Finset.Icc 2 (2*105+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 105

/-- Mechanically emitted: D=214 D=214 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_211 :
    (-1 : ℤ)^(2*105+4) - (∑ j ∈ Finset.Icc 2 (2*105+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 105

/-- Mechanically emitted: D=215 D=215 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_212 :
    (-1 : ℤ)^(2*106+3) - (∑ j ∈ Finset.Icc 2 (2*106+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 106

/-- Mechanically emitted: D=216 D=216 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_213 :
    (-1 : ℤ)^(2*106+4) - (∑ j ∈ Finset.Icc 2 (2*106+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 106

/-- Mechanically emitted: D=217 D=217 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_214 :
    (-1 : ℤ)^(2*107+3) - (∑ j ∈ Finset.Icc 2 (2*107+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 107

/-- Mechanically emitted: D=218 D=218 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_215 :
    (-1 : ℤ)^(2*107+4) - (∑ j ∈ Finset.Icc 2 (2*107+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 107

/-- Mechanically emitted: D=219 D=219 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_216 :
    (-1 : ℤ)^(2*108+3) - (∑ j ∈ Finset.Icc 2 (2*108+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 108

/-- Mechanically emitted: D=220 D=220 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_217 :
    (-1 : ℤ)^(2*108+4) - (∑ j ∈ Finset.Icc 2 (2*108+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 108

/-- Mechanically emitted: D=221 D=221 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_218 :
    (-1 : ℤ)^(2*109+3) - (∑ j ∈ Finset.Icc 2 (2*109+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 109

/-- Mechanically emitted: D=222 D=222 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_219 :
    (-1 : ℤ)^(2*109+4) - (∑ j ∈ Finset.Icc 2 (2*109+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 109

/-- Mechanically emitted: D=223 D=223 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_220 :
    (-1 : ℤ)^(2*110+3) - (∑ j ∈ Finset.Icc 2 (2*110+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 110

/-- Mechanically emitted: D=224 D=224 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_221 :
    (-1 : ℤ)^(2*110+4) - (∑ j ∈ Finset.Icc 2 (2*110+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 110

/-- Mechanically emitted: D=225 D=225 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_222 :
    (-1 : ℤ)^(2*111+3) - (∑ j ∈ Finset.Icc 2 (2*111+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 111

/-- Mechanically emitted: D=226 D=226 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_223 :
    (-1 : ℤ)^(2*111+4) - (∑ j ∈ Finset.Icc 2 (2*111+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 111

/-- Mechanically emitted: D=227 D=227 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_224 :
    (-1 : ℤ)^(2*112+3) - (∑ j ∈ Finset.Icc 2 (2*112+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 112

/-- Mechanically emitted: D=228 D=228 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_225 :
    (-1 : ℤ)^(2*112+4) - (∑ j ∈ Finset.Icc 2 (2*112+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 112

/-- Mechanically emitted: D=229 D=229 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_226 :
    (-1 : ℤ)^(2*113+3) - (∑ j ∈ Finset.Icc 2 (2*113+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 113

/-- Mechanically emitted: D=230 D=230 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_227 :
    (-1 : ℤ)^(2*113+4) - (∑ j ∈ Finset.Icc 2 (2*113+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 113

/-- Mechanically emitted: D=231 D=231 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_228 :
    (-1 : ℤ)^(2*114+3) - (∑ j ∈ Finset.Icc 2 (2*114+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 114

/-- Mechanically emitted: D=232 D=232 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_229 :
    (-1 : ℤ)^(2*114+4) - (∑ j ∈ Finset.Icc 2 (2*114+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 114

/-- Mechanically emitted: D=233 D=233 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_230 :
    (-1 : ℤ)^(2*115+3) - (∑ j ∈ Finset.Icc 2 (2*115+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 115

/-- Mechanically emitted: D=234 D=234 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_231 :
    (-1 : ℤ)^(2*115+4) - (∑ j ∈ Finset.Icc 2 (2*115+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 115

/-- Mechanically emitted: D=235 D=235 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_232 :
    (-1 : ℤ)^(2*116+3) - (∑ j ∈ Finset.Icc 2 (2*116+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 116

/-- Mechanically emitted: D=236 D=236 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_233 :
    (-1 : ℤ)^(2*116+4) - (∑ j ∈ Finset.Icc 2 (2*116+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 116

/-- Mechanically emitted: D=237 D=237 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_234 :
    (-1 : ℤ)^(2*117+3) - (∑ j ∈ Finset.Icc 2 (2*117+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 117

/-- Mechanically emitted: D=238 D=238 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_235 :
    (-1 : ℤ)^(2*117+4) - (∑ j ∈ Finset.Icc 2 (2*117+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 117

/-- Mechanically emitted: D=239 D=239 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_236 :
    (-1 : ℤ)^(2*118+3) - (∑ j ∈ Finset.Icc 2 (2*118+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 118

/-- Mechanically emitted: D=240 D=240 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_237 :
    (-1 : ℤ)^(2*118+4) - (∑ j ∈ Finset.Icc 2 (2*118+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 118

/-- Mechanically emitted: D=241 D=241 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_238 :
    (-1 : ℤ)^(2*119+3) - (∑ j ∈ Finset.Icc 2 (2*119+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 119

/-- Mechanically emitted: D=242 D=242 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_239 :
    (-1 : ℤ)^(2*119+4) - (∑ j ∈ Finset.Icc 2 (2*119+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 119

/-- Mechanically emitted: D=243 D=243 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_240 :
    (-1 : ℤ)^(2*120+3) - (∑ j ∈ Finset.Icc 2 (2*120+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 120

/-- Mechanically emitted: D=244 D=244 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_241 :
    (-1 : ℤ)^(2*120+4) - (∑ j ∈ Finset.Icc 2 (2*120+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 120

/-- Mechanically emitted: D=245 D=245 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_242 :
    (-1 : ℤ)^(2*121+3) - (∑ j ∈ Finset.Icc 2 (2*121+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 121

/-- Mechanically emitted: D=246 D=246 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_243 :
    (-1 : ℤ)^(2*121+4) - (∑ j ∈ Finset.Icc 2 (2*121+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 121

/-- Mechanically emitted: D=247 D=247 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_244 :
    (-1 : ℤ)^(2*122+3) - (∑ j ∈ Finset.Icc 2 (2*122+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 122

/-- Mechanically emitted: D=248 D=248 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_245 :
    (-1 : ℤ)^(2*122+4) - (∑ j ∈ Finset.Icc 2 (2*122+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 122

/-- Mechanically emitted: D=249 D=249 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_246 :
    (-1 : ℤ)^(2*123+3) - (∑ j ∈ Finset.Icc 2 (2*123+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 123

/-- Mechanically emitted: D=250 D=250 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_247 :
    (-1 : ℤ)^(2*123+4) - (∑ j ∈ Finset.Icc 2 (2*123+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 123

/-- Mechanically emitted: D=251 D=251 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_248 :
    (-1 : ℤ)^(2*124+3) - (∑ j ∈ Finset.Icc 2 (2*124+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 124

/-- Mechanically emitted: D=252 D=252 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_249 :
    (-1 : ℤ)^(2*124+4) - (∑ j ∈ Finset.Icc 2 (2*124+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 124

/-- Mechanically emitted: D=253 D=253 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_250 :
    (-1 : ℤ)^(2*125+3) - (∑ j ∈ Finset.Icc 2 (2*125+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 125

/-- Mechanically emitted: D=254 D=254 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_251 :
    (-1 : ℤ)^(2*125+4) - (∑ j ∈ Finset.Icc 2 (2*125+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 125

/-- Mechanically emitted: D=255 D=255 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_252 :
    (-1 : ℤ)^(2*126+3) - (∑ j ∈ Finset.Icc 2 (2*126+2), (-1 : ℤ)^j) - 1 = -3 :=
  nearest_left_profile_odd_value 126

/-- Mechanically emitted: D=256 D=256 nearest-left profile parity instance -- ravel::proof::stage_coefficient_profile_parity_obstruction
    independently reconfirmed even_dimension against the recomputed
    alternating sum. -/
theorem coefficient_profile_parity_obstruction_instance_253 :
    (-1 : ℤ)^(2*126+4) - (∑ j ∈ Finset.Icc 2 (2*126+3), (-1 : ℤ)^j) - 1 = 0 :=
  nearest_left_profile_even_has_minus_one_root 126

/- Semantic proof graph for: coefficient_profile_parity_obstruction_batch
  [0] lean.coefficient_profile_parity_obstruction_certificate :: D=3 even=false D=3 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [1] lean.coefficient_profile_parity_obstruction_certificate :: D=4 even=true D=4 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [2] lean.coefficient_profile_parity_obstruction_certificate :: D=5 even=false D=5 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [3] lean.coefficient_profile_parity_obstruction_certificate :: D=6 even=true D=6 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [4] lean.coefficient_profile_parity_obstruction_certificate :: D=7 even=false D=7 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [5] lean.coefficient_profile_parity_obstruction_certificate :: D=8 even=true D=8 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [6] lean.coefficient_profile_parity_obstruction_certificate :: D=9 even=false D=9 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [7] lean.coefficient_profile_parity_obstruction_certificate :: D=10 even=true D=10 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [8] lean.coefficient_profile_parity_obstruction_certificate :: D=11 even=false D=11 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [9] lean.coefficient_profile_parity_obstruction_certificate :: D=12 even=true D=12 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [10] lean.coefficient_profile_parity_obstruction_certificate :: D=13 even=false D=13 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [11] lean.coefficient_profile_parity_obstruction_certificate :: D=14 even=true D=14 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [12] lean.coefficient_profile_parity_obstruction_certificate :: D=15 even=false D=15 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [13] lean.coefficient_profile_parity_obstruction_certificate :: D=16 even=true D=16 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [14] lean.coefficient_profile_parity_obstruction_certificate :: D=17 even=false D=17 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [15] lean.coefficient_profile_parity_obstruction_certificate :: D=18 even=true D=18 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [16] lean.coefficient_profile_parity_obstruction_certificate :: D=19 even=false D=19 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [17] lean.coefficient_profile_parity_obstruction_certificate :: D=20 even=true D=20 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [18] lean.coefficient_profile_parity_obstruction_certificate :: D=21 even=false D=21 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [19] lean.coefficient_profile_parity_obstruction_certificate :: D=22 even=true D=22 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [20] lean.coefficient_profile_parity_obstruction_certificate :: D=23 even=false D=23 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [21] lean.coefficient_profile_parity_obstruction_certificate :: D=24 even=true D=24 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [22] lean.coefficient_profile_parity_obstruction_certificate :: D=25 even=false D=25 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [23] lean.coefficient_profile_parity_obstruction_certificate :: D=26 even=true D=26 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [24] lean.coefficient_profile_parity_obstruction_certificate :: D=27 even=false D=27 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [25] lean.coefficient_profile_parity_obstruction_certificate :: D=28 even=true D=28 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [26] lean.coefficient_profile_parity_obstruction_certificate :: D=29 even=false D=29 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [27] lean.coefficient_profile_parity_obstruction_certificate :: D=30 even=true D=30 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [28] lean.coefficient_profile_parity_obstruction_certificate :: D=31 even=false D=31 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [29] lean.coefficient_profile_parity_obstruction_certificate :: D=32 even=true D=32 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [30] lean.coefficient_profile_parity_obstruction_certificate :: D=33 even=false D=33 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [31] lean.coefficient_profile_parity_obstruction_certificate :: D=34 even=true D=34 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [32] lean.coefficient_profile_parity_obstruction_certificate :: D=35 even=false D=35 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [33] lean.coefficient_profile_parity_obstruction_certificate :: D=36 even=true D=36 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [34] lean.coefficient_profile_parity_obstruction_certificate :: D=37 even=false D=37 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [35] lean.coefficient_profile_parity_obstruction_certificate :: D=38 even=true D=38 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [36] lean.coefficient_profile_parity_obstruction_certificate :: D=39 even=false D=39 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [37] lean.coefficient_profile_parity_obstruction_certificate :: D=40 even=true D=40 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [38] lean.coefficient_profile_parity_obstruction_certificate :: D=41 even=false D=41 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [39] lean.coefficient_profile_parity_obstruction_certificate :: D=42 even=true D=42 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [40] lean.coefficient_profile_parity_obstruction_certificate :: D=43 even=false D=43 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [41] lean.coefficient_profile_parity_obstruction_certificate :: D=44 even=true D=44 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [42] lean.coefficient_profile_parity_obstruction_certificate :: D=45 even=false D=45 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [43] lean.coefficient_profile_parity_obstruction_certificate :: D=46 even=true D=46 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [44] lean.coefficient_profile_parity_obstruction_certificate :: D=47 even=false D=47 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [45] lean.coefficient_profile_parity_obstruction_certificate :: D=48 even=true D=48 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [46] lean.coefficient_profile_parity_obstruction_certificate :: D=49 even=false D=49 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [47] lean.coefficient_profile_parity_obstruction_certificate :: D=50 even=true D=50 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [48] lean.coefficient_profile_parity_obstruction_certificate :: D=51 even=false D=51 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [49] lean.coefficient_profile_parity_obstruction_certificate :: D=52 even=true D=52 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [50] lean.coefficient_profile_parity_obstruction_certificate :: D=53 even=false D=53 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [51] lean.coefficient_profile_parity_obstruction_certificate :: D=54 even=true D=54 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [52] lean.coefficient_profile_parity_obstruction_certificate :: D=55 even=false D=55 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [53] lean.coefficient_profile_parity_obstruction_certificate :: D=56 even=true D=56 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [54] lean.coefficient_profile_parity_obstruction_certificate :: D=57 even=false D=57 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [55] lean.coefficient_profile_parity_obstruction_certificate :: D=58 even=true D=58 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [56] lean.coefficient_profile_parity_obstruction_certificate :: D=59 even=false D=59 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [57] lean.coefficient_profile_parity_obstruction_certificate :: D=60 even=true D=60 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [58] lean.coefficient_profile_parity_obstruction_certificate :: D=61 even=false D=61 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [59] lean.coefficient_profile_parity_obstruction_certificate :: D=62 even=true D=62 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [60] lean.coefficient_profile_parity_obstruction_certificate :: D=63 even=false D=63 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [61] lean.coefficient_profile_parity_obstruction_certificate :: D=64 even=true D=64 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [62] lean.coefficient_profile_parity_obstruction_certificate :: D=65 even=false D=65 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [63] lean.coefficient_profile_parity_obstruction_certificate :: D=66 even=true D=66 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [64] lean.coefficient_profile_parity_obstruction_certificate :: D=67 even=false D=67 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [65] lean.coefficient_profile_parity_obstruction_certificate :: D=68 even=true D=68 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [66] lean.coefficient_profile_parity_obstruction_certificate :: D=69 even=false D=69 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [67] lean.coefficient_profile_parity_obstruction_certificate :: D=70 even=true D=70 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [68] lean.coefficient_profile_parity_obstruction_certificate :: D=71 even=false D=71 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [69] lean.coefficient_profile_parity_obstruction_certificate :: D=72 even=true D=72 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [70] lean.coefficient_profile_parity_obstruction_certificate :: D=73 even=false D=73 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [71] lean.coefficient_profile_parity_obstruction_certificate :: D=74 even=true D=74 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [72] lean.coefficient_profile_parity_obstruction_certificate :: D=75 even=false D=75 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [73] lean.coefficient_profile_parity_obstruction_certificate :: D=76 even=true D=76 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [74] lean.coefficient_profile_parity_obstruction_certificate :: D=77 even=false D=77 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [75] lean.coefficient_profile_parity_obstruction_certificate :: D=78 even=true D=78 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [76] lean.coefficient_profile_parity_obstruction_certificate :: D=79 even=false D=79 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [77] lean.coefficient_profile_parity_obstruction_certificate :: D=80 even=true D=80 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [78] lean.coefficient_profile_parity_obstruction_certificate :: D=81 even=false D=81 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [79] lean.coefficient_profile_parity_obstruction_certificate :: D=82 even=true D=82 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [80] lean.coefficient_profile_parity_obstruction_certificate :: D=83 even=false D=83 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [81] lean.coefficient_profile_parity_obstruction_certificate :: D=84 even=true D=84 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [82] lean.coefficient_profile_parity_obstruction_certificate :: D=85 even=false D=85 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [83] lean.coefficient_profile_parity_obstruction_certificate :: D=86 even=true D=86 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [84] lean.coefficient_profile_parity_obstruction_certificate :: D=87 even=false D=87 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [85] lean.coefficient_profile_parity_obstruction_certificate :: D=88 even=true D=88 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [86] lean.coefficient_profile_parity_obstruction_certificate :: D=89 even=false D=89 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [87] lean.coefficient_profile_parity_obstruction_certificate :: D=90 even=true D=90 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [88] lean.coefficient_profile_parity_obstruction_certificate :: D=91 even=false D=91 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [89] lean.coefficient_profile_parity_obstruction_certificate :: D=92 even=true D=92 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [90] lean.coefficient_profile_parity_obstruction_certificate :: D=93 even=false D=93 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [91] lean.coefficient_profile_parity_obstruction_certificate :: D=94 even=true D=94 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [92] lean.coefficient_profile_parity_obstruction_certificate :: D=95 even=false D=95 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [93] lean.coefficient_profile_parity_obstruction_certificate :: D=96 even=true D=96 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [94] lean.coefficient_profile_parity_obstruction_certificate :: D=97 even=false D=97 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [95] lean.coefficient_profile_parity_obstruction_certificate :: D=98 even=true D=98 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [96] lean.coefficient_profile_parity_obstruction_certificate :: D=99 even=false D=99 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [97] lean.coefficient_profile_parity_obstruction_certificate :: D=100 even=true D=100 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [98] lean.coefficient_profile_parity_obstruction_certificate :: D=101 even=false D=101 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [99] lean.coefficient_profile_parity_obstruction_certificate :: D=102 even=true D=102 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [100] lean.coefficient_profile_parity_obstruction_certificate :: D=103 even=false D=103 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [101] lean.coefficient_profile_parity_obstruction_certificate :: D=104 even=true D=104 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [102] lean.coefficient_profile_parity_obstruction_certificate :: D=105 even=false D=105 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [103] lean.coefficient_profile_parity_obstruction_certificate :: D=106 even=true D=106 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [104] lean.coefficient_profile_parity_obstruction_certificate :: D=107 even=false D=107 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [105] lean.coefficient_profile_parity_obstruction_certificate :: D=108 even=true D=108 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [106] lean.coefficient_profile_parity_obstruction_certificate :: D=109 even=false D=109 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [107] lean.coefficient_profile_parity_obstruction_certificate :: D=110 even=true D=110 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [108] lean.coefficient_profile_parity_obstruction_certificate :: D=111 even=false D=111 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [109] lean.coefficient_profile_parity_obstruction_certificate :: D=112 even=true D=112 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [110] lean.coefficient_profile_parity_obstruction_certificate :: D=113 even=false D=113 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [111] lean.coefficient_profile_parity_obstruction_certificate :: D=114 even=true D=114 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [112] lean.coefficient_profile_parity_obstruction_certificate :: D=115 even=false D=115 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [113] lean.coefficient_profile_parity_obstruction_certificate :: D=116 even=true D=116 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [114] lean.coefficient_profile_parity_obstruction_certificate :: D=117 even=false D=117 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [115] lean.coefficient_profile_parity_obstruction_certificate :: D=118 even=true D=118 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [116] lean.coefficient_profile_parity_obstruction_certificate :: D=119 even=false D=119 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [117] lean.coefficient_profile_parity_obstruction_certificate :: D=120 even=true D=120 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [118] lean.coefficient_profile_parity_obstruction_certificate :: D=121 even=false D=121 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [119] lean.coefficient_profile_parity_obstruction_certificate :: D=122 even=true D=122 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [120] lean.coefficient_profile_parity_obstruction_certificate :: D=123 even=false D=123 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [121] lean.coefficient_profile_parity_obstruction_certificate :: D=124 even=true D=124 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [122] lean.coefficient_profile_parity_obstruction_certificate :: D=125 even=false D=125 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [123] lean.coefficient_profile_parity_obstruction_certificate :: D=126 even=true D=126 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [124] lean.coefficient_profile_parity_obstruction_certificate :: D=127 even=false D=127 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [125] lean.coefficient_profile_parity_obstruction_certificate :: D=128 even=true D=128 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [126] lean.coefficient_profile_parity_obstruction_certificate :: D=129 even=false D=129 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [127] lean.coefficient_profile_parity_obstruction_certificate :: D=130 even=true D=130 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [128] lean.coefficient_profile_parity_obstruction_certificate :: D=131 even=false D=131 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [129] lean.coefficient_profile_parity_obstruction_certificate :: D=132 even=true D=132 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [130] lean.coefficient_profile_parity_obstruction_certificate :: D=133 even=false D=133 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [131] lean.coefficient_profile_parity_obstruction_certificate :: D=134 even=true D=134 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [132] lean.coefficient_profile_parity_obstruction_certificate :: D=135 even=false D=135 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [133] lean.coefficient_profile_parity_obstruction_certificate :: D=136 even=true D=136 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [134] lean.coefficient_profile_parity_obstruction_certificate :: D=137 even=false D=137 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [135] lean.coefficient_profile_parity_obstruction_certificate :: D=138 even=true D=138 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [136] lean.coefficient_profile_parity_obstruction_certificate :: D=139 even=false D=139 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [137] lean.coefficient_profile_parity_obstruction_certificate :: D=140 even=true D=140 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [138] lean.coefficient_profile_parity_obstruction_certificate :: D=141 even=false D=141 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [139] lean.coefficient_profile_parity_obstruction_certificate :: D=142 even=true D=142 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [140] lean.coefficient_profile_parity_obstruction_certificate :: D=143 even=false D=143 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [141] lean.coefficient_profile_parity_obstruction_certificate :: D=144 even=true D=144 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [142] lean.coefficient_profile_parity_obstruction_certificate :: D=145 even=false D=145 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [143] lean.coefficient_profile_parity_obstruction_certificate :: D=146 even=true D=146 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [144] lean.coefficient_profile_parity_obstruction_certificate :: D=147 even=false D=147 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [145] lean.coefficient_profile_parity_obstruction_certificate :: D=148 even=true D=148 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [146] lean.coefficient_profile_parity_obstruction_certificate :: D=149 even=false D=149 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [147] lean.coefficient_profile_parity_obstruction_certificate :: D=150 even=true D=150 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [148] lean.coefficient_profile_parity_obstruction_certificate :: D=151 even=false D=151 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [149] lean.coefficient_profile_parity_obstruction_certificate :: D=152 even=true D=152 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [150] lean.coefficient_profile_parity_obstruction_certificate :: D=153 even=false D=153 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [151] lean.coefficient_profile_parity_obstruction_certificate :: D=154 even=true D=154 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [152] lean.coefficient_profile_parity_obstruction_certificate :: D=155 even=false D=155 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [153] lean.coefficient_profile_parity_obstruction_certificate :: D=156 even=true D=156 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [154] lean.coefficient_profile_parity_obstruction_certificate :: D=157 even=false D=157 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [155] lean.coefficient_profile_parity_obstruction_certificate :: D=158 even=true D=158 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [156] lean.coefficient_profile_parity_obstruction_certificate :: D=159 even=false D=159 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [157] lean.coefficient_profile_parity_obstruction_certificate :: D=160 even=true D=160 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [158] lean.coefficient_profile_parity_obstruction_certificate :: D=161 even=false D=161 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [159] lean.coefficient_profile_parity_obstruction_certificate :: D=162 even=true D=162 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [160] lean.coefficient_profile_parity_obstruction_certificate :: D=163 even=false D=163 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [161] lean.coefficient_profile_parity_obstruction_certificate :: D=164 even=true D=164 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [162] lean.coefficient_profile_parity_obstruction_certificate :: D=165 even=false D=165 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [163] lean.coefficient_profile_parity_obstruction_certificate :: D=166 even=true D=166 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [164] lean.coefficient_profile_parity_obstruction_certificate :: D=167 even=false D=167 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [165] lean.coefficient_profile_parity_obstruction_certificate :: D=168 even=true D=168 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [166] lean.coefficient_profile_parity_obstruction_certificate :: D=169 even=false D=169 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [167] lean.coefficient_profile_parity_obstruction_certificate :: D=170 even=true D=170 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [168] lean.coefficient_profile_parity_obstruction_certificate :: D=171 even=false D=171 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [169] lean.coefficient_profile_parity_obstruction_certificate :: D=172 even=true D=172 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [170] lean.coefficient_profile_parity_obstruction_certificate :: D=173 even=false D=173 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [171] lean.coefficient_profile_parity_obstruction_certificate :: D=174 even=true D=174 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [172] lean.coefficient_profile_parity_obstruction_certificate :: D=175 even=false D=175 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [173] lean.coefficient_profile_parity_obstruction_certificate :: D=176 even=true D=176 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [174] lean.coefficient_profile_parity_obstruction_certificate :: D=177 even=false D=177 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [175] lean.coefficient_profile_parity_obstruction_certificate :: D=178 even=true D=178 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [176] lean.coefficient_profile_parity_obstruction_certificate :: D=179 even=false D=179 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [177] lean.coefficient_profile_parity_obstruction_certificate :: D=180 even=true D=180 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [178] lean.coefficient_profile_parity_obstruction_certificate :: D=181 even=false D=181 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [179] lean.coefficient_profile_parity_obstruction_certificate :: D=182 even=true D=182 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [180] lean.coefficient_profile_parity_obstruction_certificate :: D=183 even=false D=183 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [181] lean.coefficient_profile_parity_obstruction_certificate :: D=184 even=true D=184 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [182] lean.coefficient_profile_parity_obstruction_certificate :: D=185 even=false D=185 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [183] lean.coefficient_profile_parity_obstruction_certificate :: D=186 even=true D=186 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [184] lean.coefficient_profile_parity_obstruction_certificate :: D=187 even=false D=187 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [185] lean.coefficient_profile_parity_obstruction_certificate :: D=188 even=true D=188 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [186] lean.coefficient_profile_parity_obstruction_certificate :: D=189 even=false D=189 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [187] lean.coefficient_profile_parity_obstruction_certificate :: D=190 even=true D=190 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [188] lean.coefficient_profile_parity_obstruction_certificate :: D=191 even=false D=191 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [189] lean.coefficient_profile_parity_obstruction_certificate :: D=192 even=true D=192 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [190] lean.coefficient_profile_parity_obstruction_certificate :: D=193 even=false D=193 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [191] lean.coefficient_profile_parity_obstruction_certificate :: D=194 even=true D=194 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [192] lean.coefficient_profile_parity_obstruction_certificate :: D=195 even=false D=195 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [193] lean.coefficient_profile_parity_obstruction_certificate :: D=196 even=true D=196 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [194] lean.coefficient_profile_parity_obstruction_certificate :: D=197 even=false D=197 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [195] lean.coefficient_profile_parity_obstruction_certificate :: D=198 even=true D=198 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [196] lean.coefficient_profile_parity_obstruction_certificate :: D=199 even=false D=199 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [197] lean.coefficient_profile_parity_obstruction_certificate :: D=200 even=true D=200 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [198] lean.coefficient_profile_parity_obstruction_certificate :: D=201 even=false D=201 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [199] lean.coefficient_profile_parity_obstruction_certificate :: D=202 even=true D=202 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [200] lean.coefficient_profile_parity_obstruction_certificate :: D=203 even=false D=203 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [201] lean.coefficient_profile_parity_obstruction_certificate :: D=204 even=true D=204 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [202] lean.coefficient_profile_parity_obstruction_certificate :: D=205 even=false D=205 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [203] lean.coefficient_profile_parity_obstruction_certificate :: D=206 even=true D=206 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [204] lean.coefficient_profile_parity_obstruction_certificate :: D=207 even=false D=207 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [205] lean.coefficient_profile_parity_obstruction_certificate :: D=208 even=true D=208 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [206] lean.coefficient_profile_parity_obstruction_certificate :: D=209 even=false D=209 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [207] lean.coefficient_profile_parity_obstruction_certificate :: D=210 even=true D=210 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [208] lean.coefficient_profile_parity_obstruction_certificate :: D=211 even=false D=211 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [209] lean.coefficient_profile_parity_obstruction_certificate :: D=212 even=true D=212 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [210] lean.coefficient_profile_parity_obstruction_certificate :: D=213 even=false D=213 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [211] lean.coefficient_profile_parity_obstruction_certificate :: D=214 even=true D=214 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [212] lean.coefficient_profile_parity_obstruction_certificate :: D=215 even=false D=215 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [213] lean.coefficient_profile_parity_obstruction_certificate :: D=216 even=true D=216 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [214] lean.coefficient_profile_parity_obstruction_certificate :: D=217 even=false D=217 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [215] lean.coefficient_profile_parity_obstruction_certificate :: D=218 even=true D=218 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [216] lean.coefficient_profile_parity_obstruction_certificate :: D=219 even=false D=219 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [217] lean.coefficient_profile_parity_obstruction_certificate :: D=220 even=true D=220 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [218] lean.coefficient_profile_parity_obstruction_certificate :: D=221 even=false D=221 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [219] lean.coefficient_profile_parity_obstruction_certificate :: D=222 even=true D=222 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [220] lean.coefficient_profile_parity_obstruction_certificate :: D=223 even=false D=223 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [221] lean.coefficient_profile_parity_obstruction_certificate :: D=224 even=true D=224 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [222] lean.coefficient_profile_parity_obstruction_certificate :: D=225 even=false D=225 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [223] lean.coefficient_profile_parity_obstruction_certificate :: D=226 even=true D=226 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [224] lean.coefficient_profile_parity_obstruction_certificate :: D=227 even=false D=227 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [225] lean.coefficient_profile_parity_obstruction_certificate :: D=228 even=true D=228 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [226] lean.coefficient_profile_parity_obstruction_certificate :: D=229 even=false D=229 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [227] lean.coefficient_profile_parity_obstruction_certificate :: D=230 even=true D=230 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [228] lean.coefficient_profile_parity_obstruction_certificate :: D=231 even=false D=231 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [229] lean.coefficient_profile_parity_obstruction_certificate :: D=232 even=true D=232 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [230] lean.coefficient_profile_parity_obstruction_certificate :: D=233 even=false D=233 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [231] lean.coefficient_profile_parity_obstruction_certificate :: D=234 even=true D=234 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [232] lean.coefficient_profile_parity_obstruction_certificate :: D=235 even=false D=235 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [233] lean.coefficient_profile_parity_obstruction_certificate :: D=236 even=true D=236 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [234] lean.coefficient_profile_parity_obstruction_certificate :: D=237 even=false D=237 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [235] lean.coefficient_profile_parity_obstruction_certificate :: D=238 even=true D=238 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [236] lean.coefficient_profile_parity_obstruction_certificate :: D=239 even=false D=239 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [237] lean.coefficient_profile_parity_obstruction_certificate :: D=240 even=true D=240 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [238] lean.coefficient_profile_parity_obstruction_certificate :: D=241 even=false D=241 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [239] lean.coefficient_profile_parity_obstruction_certificate :: D=242 even=true D=242 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [240] lean.coefficient_profile_parity_obstruction_certificate :: D=243 even=false D=243 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [241] lean.coefficient_profile_parity_obstruction_certificate :: D=244 even=true D=244 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [242] lean.coefficient_profile_parity_obstruction_certificate :: D=245 even=false D=245 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [243] lean.coefficient_profile_parity_obstruction_certificate :: D=246 even=true D=246 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [244] lean.coefficient_profile_parity_obstruction_certificate :: D=247 even=false D=247 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [245] lean.coefficient_profile_parity_obstruction_certificate :: D=248 even=true D=248 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [246] lean.coefficient_profile_parity_obstruction_certificate :: D=249 even=false D=249 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [247] lean.coefficient_profile_parity_obstruction_certificate :: D=250 even=true D=250 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [248] lean.coefficient_profile_parity_obstruction_certificate :: D=251 even=false D=251 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [249] lean.coefficient_profile_parity_obstruction_certificate :: D=252 even=true D=252 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [250] lean.coefficient_profile_parity_obstruction_certificate :: D=253 even=false D=253 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [251] lean.coefficient_profile_parity_obstruction_certificate :: D=254 even=true D=254 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [252] lean.coefficient_profile_parity_obstruction_certificate :: D=255 even=false D=255 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
  [253] lean.coefficient_profile_parity_obstruction_certificate :: D=256 even=true D=256 nearest-left profile parity instance -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value
-/

def reflectedNodeCount : Nat := 254

end RavelGenerated
