import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

theorem pow_intertwiner
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a * b ≤ a * c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a * c ≤ b * c)
    (x y P : α) (hxy : x * P ≤ P * y) :
    ∀ n : ℕ, x ^ n * P ≤ P * y ^ n := by
  intro n
  induction n with
  | zero => simpa using le_refl P
  | succ n ih =>
      calc
        x ^ (n + 1) * P = x * (x ^ n * P) := by rw [pow_succ', mul_assoc]
        _ ≤ x * (P * y ^ n) := mul_left_mono x ih
        _ = (x * P) * y ^ n := by rw [mul_assoc]
        _ ≤ (P * y) * y ^ n := mul_right_mono _ hxy
        _ = P * y ^ (n + 1) := by rw [mul_assoc, ← pow_succ']

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

-- Connects the word-evaluator's replicate-R case to the plain monoid power
-- `pow_intertwiner` is stated over -- a small, genuinely new (not duplicated)
-- structural fact, not an independent re-proof of the sandwich induction itself.
theorem evalWord_replicate_r {α : Type*} [Monoid α] (Q R : α) (d : ℕ) :
    evalWord Q R (List.replicate d QR.r) = R ^ d := by
  induction d with
  | zero => simp [evalWord]
  | succ d ih => rw [List.replicate_succ, evalWord, ih, pow_succ']

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
  · rw [evalWord_replicate_r, evalWord_replicate_r]
    exact pow_intertwiner mul_left_mono mul_right_mono Rc Rk P hR d

theorem cut_pair_count (m : ℕ) :
    Fintype.card (Fin (m + 1) × Fin (m + 1)) = (m + 1) * (m + 1) := by
  simp [Fintype.card_prod, Fintype.card_fin]

/-- Mechanically emitted scalar witness: m=1 m=1 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4 symbolic cut states and independently
    re-checked 2 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_0 :
    (2 : ℕ) = 2 := by decide

/-- Mechanically emitted scalar witness: m=2 m=2 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    9 symbolic cut states and independently
    re-checked 3 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_1 :
    (3 : ℕ) = 3 := by decide

/-- Mechanically emitted scalar witness: m=3 m=3 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    16 symbolic cut states and independently
    re-checked 4 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_2 :
    (4 : ℕ) = 4 := by decide

/-- Mechanically emitted scalar witness: m=4 m=4 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    25 symbolic cut states and independently
    re-checked 5 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_3 :
    (5 : ℕ) = 5 := by decide

/-- Mechanically emitted scalar witness: m=5 m=5 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    36 symbolic cut states and independently
    re-checked 6 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_4 :
    (6 : ℕ) = 6 := by decide

/-- Mechanically emitted scalar witness: m=6 m=6 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    49 symbolic cut states and independently
    re-checked 7 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_5 :
    (7 : ℕ) = 7 := by decide

/-- Mechanically emitted scalar witness: m=7 m=7 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    64 symbolic cut states and independently
    re-checked 8 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_6 :
    (8 : ℕ) = 8 := by decide

/-- Mechanically emitted scalar witness: m=8 m=8 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    81 symbolic cut states and independently
    re-checked 9 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_7 :
    (9 : ℕ) = 9 := by decide

/-- Mechanically emitted scalar witness: m=9 m=9 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    100 symbolic cut states and independently
    re-checked 10 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_8 :
    (10 : ℕ) = 10 := by decide

/-- Mechanically emitted scalar witness: m=10 m=10 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    121 symbolic cut states and independently
    re-checked 11 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_9 :
    (11 : ℕ) = 11 := by decide

/-- Mechanically emitted scalar witness: m=11 m=11 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    144 symbolic cut states and independently
    re-checked 12 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_10 :
    (12 : ℕ) = 12 := by decide

/-- Mechanically emitted scalar witness: m=12 m=12 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    169 symbolic cut states and independently
    re-checked 13 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_11 :
    (13 : ℕ) = 13 := by decide

/-- Mechanically emitted scalar witness: m=13 m=13 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    196 symbolic cut states and independently
    re-checked 14 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_12 :
    (14 : ℕ) = 14 := by decide

/-- Mechanically emitted scalar witness: m=14 m=14 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    225 symbolic cut states and independently
    re-checked 15 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_13 :
    (15 : ℕ) = 15 := by decide

/-- Mechanically emitted scalar witness: m=15 m=15 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    256 symbolic cut states and independently
    re-checked 16 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_14 :
    (16 : ℕ) = 16 := by decide

/-- Mechanically emitted scalar witness: m=16 m=16 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    289 symbolic cut states and independently
    re-checked 17 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_15 :
    (17 : ℕ) = 17 := by decide

/-- Mechanically emitted scalar witness: m=17 m=17 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    324 symbolic cut states and independently
    re-checked 18 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_16 :
    (18 : ℕ) = 18 := by decide

/-- Mechanically emitted scalar witness: m=18 m=18 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    361 symbolic cut states and independently
    re-checked 19 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_17 :
    (19 : ℕ) = 19 := by decide

/-- Mechanically emitted scalar witness: m=19 m=19 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    400 symbolic cut states and independently
    re-checked 20 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_18 :
    (20 : ℕ) = 20 := by decide

/-- Mechanically emitted scalar witness: m=20 m=20 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    441 symbolic cut states and independently
    re-checked 21 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_19 :
    (21 : ℕ) = 21 := by decide

/-- Mechanically emitted scalar witness: m=21 m=21 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    484 symbolic cut states and independently
    re-checked 22 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_20 :
    (22 : ℕ) = 22 := by decide

/-- Mechanically emitted scalar witness: m=22 m=22 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    529 symbolic cut states and independently
    re-checked 23 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_21 :
    (23 : ℕ) = 23 := by decide

/-- Mechanically emitted scalar witness: m=23 m=23 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    576 symbolic cut states and independently
    re-checked 24 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_22 :
    (24 : ℕ) = 24 := by decide

/-- Mechanically emitted scalar witness: m=24 m=24 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    625 symbolic cut states and independently
    re-checked 25 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_23 :
    (25 : ℕ) = 25 := by decide

/-- Mechanically emitted scalar witness: m=25 m=25 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    676 symbolic cut states and independently
    re-checked 26 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_24 :
    (26 : ℕ) = 26 := by decide

/-- Mechanically emitted scalar witness: m=26 m=26 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    729 symbolic cut states and independently
    re-checked 27 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_25 :
    (27 : ℕ) = 27 := by decide

/-- Mechanically emitted scalar witness: m=27 m=27 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    784 symbolic cut states and independently
    re-checked 28 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_26 :
    (28 : ℕ) = 28 := by decide

/-- Mechanically emitted scalar witness: m=28 m=28 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    841 symbolic cut states and independently
    re-checked 29 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_27 :
    (29 : ℕ) = 29 := by decide

/-- Mechanically emitted scalar witness: m=29 m=29 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    900 symbolic cut states and independently
    re-checked 30 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_28 :
    (30 : ℕ) = 30 := by decide

/-- Mechanically emitted scalar witness: m=30 m=30 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    961 symbolic cut states and independently
    re-checked 31 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_29 :
    (31 : ℕ) = 31 := by decide

/-- Mechanically emitted scalar witness: m=31 m=31 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1024 symbolic cut states and independently
    re-checked 32 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_30 :
    (32 : ℕ) = 32 := by decide

/-- Mechanically emitted scalar witness: m=32 m=32 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1089 symbolic cut states and independently
    re-checked 33 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_31 :
    (33 : ℕ) = 33 := by decide

/-- Mechanically emitted scalar witness: m=33 m=33 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1156 symbolic cut states and independently
    re-checked 34 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_32 :
    (34 : ℕ) = 34 := by decide

/-- Mechanically emitted scalar witness: m=34 m=34 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1225 symbolic cut states and independently
    re-checked 35 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_33 :
    (35 : ℕ) = 35 := by decide

/-- Mechanically emitted scalar witness: m=35 m=35 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1296 symbolic cut states and independently
    re-checked 36 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_34 :
    (36 : ℕ) = 36 := by decide

/-- Mechanically emitted scalar witness: m=36 m=36 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1369 symbolic cut states and independently
    re-checked 37 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_35 :
    (37 : ℕ) = 37 := by decide

/-- Mechanically emitted scalar witness: m=37 m=37 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1444 symbolic cut states and independently
    re-checked 38 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_36 :
    (38 : ℕ) = 38 := by decide

/-- Mechanically emitted scalar witness: m=38 m=38 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1521 symbolic cut states and independently
    re-checked 39 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_37 :
    (39 : ℕ) = 39 := by decide

/-- Mechanically emitted scalar witness: m=39 m=39 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1600 symbolic cut states and independently
    re-checked 40 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_38 :
    (40 : ℕ) = 40 := by decide

/-- Mechanically emitted scalar witness: m=40 m=40 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1681 symbolic cut states and independently
    re-checked 41 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_39 :
    (41 : ℕ) = 41 := by decide

/-- Mechanically emitted scalar witness: m=41 m=41 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1764 symbolic cut states and independently
    re-checked 42 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_40 :
    (42 : ℕ) = 42 := by decide

/-- Mechanically emitted scalar witness: m=42 m=42 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1849 symbolic cut states and independently
    re-checked 43 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_41 :
    (43 : ℕ) = 43 := by decide

/-- Mechanically emitted scalar witness: m=43 m=43 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    1936 symbolic cut states and independently
    re-checked 44 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_42 :
    (44 : ℕ) = 44 := by decide

/-- Mechanically emitted scalar witness: m=44 m=44 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2025 symbolic cut states and independently
    re-checked 45 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_43 :
    (45 : ℕ) = 45 := by decide

/-- Mechanically emitted scalar witness: m=45 m=45 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2116 symbolic cut states and independently
    re-checked 46 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_44 :
    (46 : ℕ) = 46 := by decide

/-- Mechanically emitted scalar witness: m=46 m=46 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2209 symbolic cut states and independently
    re-checked 47 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_45 :
    (47 : ℕ) = 47 := by decide

/-- Mechanically emitted scalar witness: m=47 m=47 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2304 symbolic cut states and independently
    re-checked 48 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_46 :
    (48 : ℕ) = 48 := by decide

/-- Mechanically emitted scalar witness: m=48 m=48 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2401 symbolic cut states and independently
    re-checked 49 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_47 :
    (49 : ℕ) = 49 := by decide

/-- Mechanically emitted scalar witness: m=49 m=49 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2500 symbolic cut states and independently
    re-checked 50 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_48 :
    (50 : ℕ) = 50 := by decide

/-- Mechanically emitted scalar witness: m=50 m=50 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2601 symbolic cut states and independently
    re-checked 51 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_49 :
    (51 : ℕ) = 51 := by decide

/-- Mechanically emitted scalar witness: m=51 m=51 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2704 symbolic cut states and independently
    re-checked 52 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_50 :
    (52 : ℕ) = 52 := by decide

/-- Mechanically emitted scalar witness: m=52 m=52 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2809 symbolic cut states and independently
    re-checked 53 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_51 :
    (53 : ℕ) = 53 := by decide

/-- Mechanically emitted scalar witness: m=53 m=53 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    2916 symbolic cut states and independently
    re-checked 54 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_52 :
    (54 : ℕ) = 54 := by decide

/-- Mechanically emitted scalar witness: m=54 m=54 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3025 symbolic cut states and independently
    re-checked 55 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_53 :
    (55 : ℕ) = 55 := by decide

/-- Mechanically emitted scalar witness: m=55 m=55 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3136 symbolic cut states and independently
    re-checked 56 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_54 :
    (56 : ℕ) = 56 := by decide

/-- Mechanically emitted scalar witness: m=56 m=56 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3249 symbolic cut states and independently
    re-checked 57 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_55 :
    (57 : ℕ) = 57 := by decide

/-- Mechanically emitted scalar witness: m=57 m=57 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3364 symbolic cut states and independently
    re-checked 58 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_56 :
    (58 : ℕ) = 58 := by decide

/-- Mechanically emitted scalar witness: m=58 m=58 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3481 symbolic cut states and independently
    re-checked 59 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_57 :
    (59 : ℕ) = 59 := by decide

/-- Mechanically emitted scalar witness: m=59 m=59 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3600 symbolic cut states and independently
    re-checked 60 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_58 :
    (60 : ℕ) = 60 := by decide

/-- Mechanically emitted scalar witness: m=60 m=60 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3721 symbolic cut states and independently
    re-checked 61 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_59 :
    (61 : ℕ) = 61 := by decide

/-- Mechanically emitted scalar witness: m=61 m=61 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3844 symbolic cut states and independently
    re-checked 62 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_60 :
    (62 : ℕ) = 62 := by decide

/-- Mechanically emitted scalar witness: m=62 m=62 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    3969 symbolic cut states and independently
    re-checked 63 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_61 :
    (63 : ℕ) = 63 := by decide

/-- Mechanically emitted scalar witness: m=63 m=63 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4096 symbolic cut states and independently
    re-checked 64 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_62 :
    (64 : ℕ) = 64 := by decide

/-- Mechanically emitted scalar witness: m=64 m=64 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4225 symbolic cut states and independently
    re-checked 65 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_63 :
    (65 : ℕ) = 65 := by decide

/-- Mechanically emitted scalar witness: m=65 m=65 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4356 symbolic cut states and independently
    re-checked 66 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_64 :
    (66 : ℕ) = 66 := by decide

/-- Mechanically emitted scalar witness: m=66 m=66 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4489 symbolic cut states and independently
    re-checked 67 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_65 :
    (67 : ℕ) = 67 := by decide

/-- Mechanically emitted scalar witness: m=67 m=67 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4624 symbolic cut states and independently
    re-checked 68 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_66 :
    (68 : ℕ) = 68 := by decide

/-- Mechanically emitted scalar witness: m=68 m=68 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4761 symbolic cut states and independently
    re-checked 69 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_67 :
    (69 : ℕ) = 69 := by decide

/-- Mechanically emitted scalar witness: m=69 m=69 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    4900 symbolic cut states and independently
    re-checked 70 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_68 :
    (70 : ℕ) = 70 := by decide

/-- Mechanically emitted scalar witness: m=70 m=70 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    5041 symbolic cut states and independently
    re-checked 71 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_69 :
    (71 : ℕ) = 71 := by decide

/-- Mechanically emitted scalar witness: m=71 m=71 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    5184 symbolic cut states and independently
    re-checked 72 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_70 :
    (72 : ℕ) = 72 := by decide

/-- Mechanically emitted scalar witness: m=72 m=72 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    5329 symbolic cut states and independently
    re-checked 73 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_71 :
    (73 : ℕ) = 73 := by decide

/-- Mechanically emitted scalar witness: m=73 m=73 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    5476 symbolic cut states and independently
    re-checked 74 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_72 :
    (74 : ℕ) = 74 := by decide

/-- Mechanically emitted scalar witness: m=74 m=74 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    5625 symbolic cut states and independently
    re-checked 75 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_73 :
    (75 : ℕ) = 75 := by decide

/-- Mechanically emitted scalar witness: m=75 m=75 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    5776 symbolic cut states and independently
    re-checked 76 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_74 :
    (76 : ℕ) = 76 := by decide

/-- Mechanically emitted scalar witness: m=76 m=76 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    5929 symbolic cut states and independently
    re-checked 77 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_75 :
    (77 : ℕ) = 77 := by decide

/-- Mechanically emitted scalar witness: m=77 m=77 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    6084 symbolic cut states and independently
    re-checked 78 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_76 :
    (78 : ℕ) = 78 := by decide

/-- Mechanically emitted scalar witness: m=78 m=78 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    6241 symbolic cut states and independently
    re-checked 79 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_77 :
    (79 : ℕ) = 79 := by decide

/-- Mechanically emitted scalar witness: m=79 m=79 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    6400 symbolic cut states and independently
    re-checked 80 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_78 :
    (80 : ℕ) = 80 := by decide

/-- Mechanically emitted scalar witness: m=80 m=80 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    6561 symbolic cut states and independently
    re-checked 81 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_79 :
    (81 : ℕ) = 81 := by decide

/-- Mechanically emitted scalar witness: m=81 m=81 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    6724 symbolic cut states and independently
    re-checked 82 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_80 :
    (82 : ℕ) = 82 := by decide

/-- Mechanically emitted scalar witness: m=82 m=82 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    6889 symbolic cut states and independently
    re-checked 83 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_81 :
    (83 : ℕ) = 83 := by decide

/-- Mechanically emitted scalar witness: m=83 m=83 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    7056 symbolic cut states and independently
    re-checked 84 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_82 :
    (84 : ℕ) = 84 := by decide

/-- Mechanically emitted scalar witness: m=84 m=84 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    7225 symbolic cut states and independently
    re-checked 85 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_83 :
    (85 : ℕ) = 85 := by decide

/-- Mechanically emitted scalar witness: m=85 m=85 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    7396 symbolic cut states and independently
    re-checked 86 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_84 :
    (86 : ℕ) = 86 := by decide

/-- Mechanically emitted scalar witness: m=86 m=86 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    7569 symbolic cut states and independently
    re-checked 87 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_85 :
    (87 : ℕ) = 87 := by decide

/-- Mechanically emitted scalar witness: m=87 m=87 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    7744 symbolic cut states and independently
    re-checked 88 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_86 :
    (88 : ℕ) = 88 := by decide

/-- Mechanically emitted scalar witness: m=88 m=88 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    7921 symbolic cut states and independently
    re-checked 89 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_87 :
    (89 : ℕ) = 89 := by decide

/-- Mechanically emitted scalar witness: m=89 m=89 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    8100 symbolic cut states and independently
    re-checked 90 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_88 :
    (90 : ℕ) = 90 := by decide

/-- Mechanically emitted scalar witness: m=90 m=90 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    8281 symbolic cut states and independently
    re-checked 91 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_89 :
    (91 : ℕ) = 91 := by decide

/-- Mechanically emitted scalar witness: m=91 m=91 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    8464 symbolic cut states and independently
    re-checked 92 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_90 :
    (92 : ℕ) = 92 := by decide

/-- Mechanically emitted scalar witness: m=92 m=92 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    8649 symbolic cut states and independently
    re-checked 93 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_91 :
    (93 : ℕ) = 93 := by decide

/-- Mechanically emitted scalar witness: m=93 m=93 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    8836 symbolic cut states and independently
    re-checked 94 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_92 :
    (94 : ℕ) = 94 := by decide

/-- Mechanically emitted scalar witness: m=94 m=94 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    9025 symbolic cut states and independently
    re-checked 95 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_93 :
    (95 : ℕ) = 95 := by decide

/-- Mechanically emitted scalar witness: m=95 m=95 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    9216 symbolic cut states and independently
    re-checked 96 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_94 :
    (96 : ℕ) = 96 := by decide

/-- Mechanically emitted scalar witness: m=96 m=96 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    9409 symbolic cut states and independently
    re-checked 97 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_95 :
    (97 : ℕ) = 97 := by decide

/-- Mechanically emitted scalar witness: m=97 m=97 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    9604 symbolic cut states and independently
    re-checked 98 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_96 :
    (98 : ℕ) = 98 := by decide

/-- Mechanically emitted scalar witness: m=98 m=98 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    9801 symbolic cut states and independently
    re-checked 99 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_97 :
    (99 : ℕ) = 99 := by decide

/-- Mechanically emitted scalar witness: m=99 m=99 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    10000 symbolic cut states and independently
    re-checked 100 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_98 :
    (100 : ℕ) = 100 := by decide

/-- Mechanically emitted scalar witness: m=100 m=100 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    10201 symbolic cut states and independently
    re-checked 101 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_99 :
    (101 : ℕ) = 101 := by decide

/-- Mechanically emitted scalar witness: m=101 m=101 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    10404 symbolic cut states and independently
    re-checked 102 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_100 :
    (102 : ℕ) = 102 := by decide

/-- Mechanically emitted scalar witness: m=102 m=102 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    10609 symbolic cut states and independently
    re-checked 103 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_101 :
    (103 : ℕ) = 103 := by decide

/-- Mechanically emitted scalar witness: m=103 m=103 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    10816 symbolic cut states and independently
    re-checked 104 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_102 :
    (104 : ℕ) = 104 := by decide

/-- Mechanically emitted scalar witness: m=104 m=104 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    11025 symbolic cut states and independently
    re-checked 105 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_103 :
    (105 : ℕ) = 105 := by decide

/-- Mechanically emitted scalar witness: m=105 m=105 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    11236 symbolic cut states and independently
    re-checked 106 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_104 :
    (106 : ℕ) = 106 := by decide

/-- Mechanically emitted scalar witness: m=106 m=106 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    11449 symbolic cut states and independently
    re-checked 107 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_105 :
    (107 : ℕ) = 107 := by decide

/-- Mechanically emitted scalar witness: m=107 m=107 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    11664 symbolic cut states and independently
    re-checked 108 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_106 :
    (108 : ℕ) = 108 := by decide

/-- Mechanically emitted scalar witness: m=108 m=108 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    11881 symbolic cut states and independently
    re-checked 109 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_107 :
    (109 : ℕ) = 109 := by decide

/-- Mechanically emitted scalar witness: m=109 m=109 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    12100 symbolic cut states and independently
    re-checked 110 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_108 :
    (110 : ℕ) = 110 := by decide

/-- Mechanically emitted scalar witness: m=110 m=110 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    12321 symbolic cut states and independently
    re-checked 111 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_109 :
    (111 : ℕ) = 111 := by decide

/-- Mechanically emitted scalar witness: m=111 m=111 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    12544 symbolic cut states and independently
    re-checked 112 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_110 :
    (112 : ℕ) = 112 := by decide

/-- Mechanically emitted scalar witness: m=112 m=112 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    12769 symbolic cut states and independently
    re-checked 113 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_111 :
    (113 : ℕ) = 113 := by decide

/-- Mechanically emitted scalar witness: m=113 m=113 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    12996 symbolic cut states and independently
    re-checked 114 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_112 :
    (114 : ℕ) = 114 := by decide

/-- Mechanically emitted scalar witness: m=114 m=114 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    13225 symbolic cut states and independently
    re-checked 115 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_113 :
    (115 : ℕ) = 115 := by decide

/-- Mechanically emitted scalar witness: m=115 m=115 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    13456 symbolic cut states and independently
    re-checked 116 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_114 :
    (116 : ℕ) = 116 := by decide

/-- Mechanically emitted scalar witness: m=116 m=116 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    13689 symbolic cut states and independently
    re-checked 117 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_115 :
    (117 : ℕ) = 117 := by decide

/-- Mechanically emitted scalar witness: m=117 m=117 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    13924 symbolic cut states and independently
    re-checked 118 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_116 :
    (118 : ℕ) = 118 := by decide

/-- Mechanically emitted scalar witness: m=118 m=118 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    14161 symbolic cut states and independently
    re-checked 119 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_117 :
    (119 : ℕ) = 119 := by decide

/-- Mechanically emitted scalar witness: m=119 m=119 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    14400 symbolic cut states and independently
    re-checked 120 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_118 :
    (120 : ℕ) = 120 := by decide

/-- Mechanically emitted scalar witness: m=120 m=120 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    14641 symbolic cut states and independently
    re-checked 121 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_119 :
    (121 : ℕ) = 121 := by decide

/-- Mechanically emitted scalar witness: m=121 m=121 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    14884 symbolic cut states and independently
    re-checked 122 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_120 :
    (122 : ℕ) = 122 := by decide

/-- Mechanically emitted scalar witness: m=122 m=122 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    15129 symbolic cut states and independently
    re-checked 123 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_121 :
    (123 : ℕ) = 123 := by decide

/-- Mechanically emitted scalar witness: m=123 m=123 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    15376 symbolic cut states and independently
    re-checked 124 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_122 :
    (124 : ℕ) = 124 := by decide

/-- Mechanically emitted scalar witness: m=124 m=124 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    15625 symbolic cut states and independently
    re-checked 125 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_123 :
    (125 : ℕ) = 125 := by decide

/-- Mechanically emitted scalar witness: m=125 m=125 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    15876 symbolic cut states and independently
    re-checked 126 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_124 :
    (126 : ℕ) = 126 := by decide

/-- Mechanically emitted scalar witness: m=126 m=126 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    16129 symbolic cut states and independently
    re-checked 127 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_125 :
    (127 : ℕ) = 127 := by decide

/-- Mechanically emitted scalar witness: m=127 m=127 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    16384 symbolic cut states and independently
    re-checked 128 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_126 :
    (128 : ℕ) = 128 := by decide

/-- Mechanically emitted scalar witness: m=128 m=128 simultaneous Q/R intertwiner sweep
    -- ravel::proof::stage_generalized_multinacci_general_m_intertwiner found
    16641 symbolic cut states and independently
    re-checked 129 roof-word channels against `roof_word_intertwiner`. -/
theorem generalized_multinacci_general_m_intertwiner_instance_127 :
    (129 : ℕ) = 129 := by decide

/- Semantic proof graph for: generalized_multinacci_general_m_intertwiner_batch
  [0] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=1 states=4 words=2 m=1 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [1] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=2 states=9 words=3 m=2 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [2] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=3 states=16 words=4 m=3 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [3] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=4 states=25 words=5 m=4 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [4] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=5 states=36 words=6 m=5 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [5] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=6 states=49 words=7 m=6 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [6] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=7 states=64 words=8 m=7 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [7] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=8 states=81 words=9 m=8 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [8] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=9 states=100 words=10 m=9 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [9] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=10 states=121 words=11 m=10 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [10] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=11 states=144 words=12 m=11 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [11] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=12 states=169 words=13 m=12 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [12] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=13 states=196 words=14 m=13 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [13] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=14 states=225 words=15 m=14 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [14] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=15 states=256 words=16 m=15 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [15] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=16 states=289 words=17 m=16 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [16] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=17 states=324 words=18 m=17 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [17] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=18 states=361 words=19 m=18 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [18] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=19 states=400 words=20 m=19 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [19] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=20 states=441 words=21 m=20 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [20] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=21 states=484 words=22 m=21 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [21] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=22 states=529 words=23 m=22 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [22] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=23 states=576 words=24 m=23 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [23] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=24 states=625 words=25 m=24 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [24] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=25 states=676 words=26 m=25 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [25] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=26 states=729 words=27 m=26 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [26] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=27 states=784 words=28 m=27 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [27] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=28 states=841 words=29 m=28 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [28] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=29 states=900 words=30 m=29 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [29] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=30 states=961 words=31 m=30 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [30] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=31 states=1024 words=32 m=31 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [31] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=32 states=1089 words=33 m=32 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [32] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=33 states=1156 words=34 m=33 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [33] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=34 states=1225 words=35 m=34 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [34] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=35 states=1296 words=36 m=35 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [35] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=36 states=1369 words=37 m=36 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [36] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=37 states=1444 words=38 m=37 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [37] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=38 states=1521 words=39 m=38 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [38] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=39 states=1600 words=40 m=39 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [39] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=40 states=1681 words=41 m=40 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [40] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=41 states=1764 words=42 m=41 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [41] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=42 states=1849 words=43 m=42 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [42] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=43 states=1936 words=44 m=43 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [43] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=44 states=2025 words=45 m=44 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [44] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=45 states=2116 words=46 m=45 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [45] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=46 states=2209 words=47 m=46 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [46] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=47 states=2304 words=48 m=47 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [47] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=48 states=2401 words=49 m=48 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [48] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=49 states=2500 words=50 m=49 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [49] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=50 states=2601 words=51 m=50 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [50] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=51 states=2704 words=52 m=51 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [51] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=52 states=2809 words=53 m=52 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [52] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=53 states=2916 words=54 m=53 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [53] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=54 states=3025 words=55 m=54 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [54] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=55 states=3136 words=56 m=55 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [55] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=56 states=3249 words=57 m=56 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [56] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=57 states=3364 words=58 m=57 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [57] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=58 states=3481 words=59 m=58 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [58] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=59 states=3600 words=60 m=59 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [59] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=60 states=3721 words=61 m=60 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [60] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=61 states=3844 words=62 m=61 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [61] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=62 states=3969 words=63 m=62 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [62] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=63 states=4096 words=64 m=63 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [63] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=64 states=4225 words=65 m=64 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [64] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=65 states=4356 words=66 m=65 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [65] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=66 states=4489 words=67 m=66 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [66] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=67 states=4624 words=68 m=67 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [67] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=68 states=4761 words=69 m=68 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [68] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=69 states=4900 words=70 m=69 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [69] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=70 states=5041 words=71 m=70 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [70] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=71 states=5184 words=72 m=71 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [71] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=72 states=5329 words=73 m=72 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [72] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=73 states=5476 words=74 m=73 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [73] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=74 states=5625 words=75 m=74 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [74] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=75 states=5776 words=76 m=75 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [75] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=76 states=5929 words=77 m=76 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [76] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=77 states=6084 words=78 m=77 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [77] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=78 states=6241 words=79 m=78 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [78] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=79 states=6400 words=80 m=79 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [79] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=80 states=6561 words=81 m=80 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [80] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=81 states=6724 words=82 m=81 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [81] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=82 states=6889 words=83 m=82 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [82] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=83 states=7056 words=84 m=83 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [83] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=84 states=7225 words=85 m=84 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [84] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=85 states=7396 words=86 m=85 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [85] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=86 states=7569 words=87 m=86 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [86] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=87 states=7744 words=88 m=87 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [87] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=88 states=7921 words=89 m=88 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [88] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=89 states=8100 words=90 m=89 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [89] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=90 states=8281 words=91 m=90 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [90] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=91 states=8464 words=92 m=91 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [91] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=92 states=8649 words=93 m=92 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [92] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=93 states=8836 words=94 m=93 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [93] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=94 states=9025 words=95 m=94 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [94] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=95 states=9216 words=96 m=95 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [95] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=96 states=9409 words=97 m=96 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [96] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=97 states=9604 words=98 m=97 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [97] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=98 states=9801 words=99 m=98 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [98] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=99 states=10000 words=100 m=99 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [99] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=100 states=10201 words=101 m=100 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [100] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=101 states=10404 words=102 m=101 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [101] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=102 states=10609 words=103 m=102 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [102] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=103 states=10816 words=104 m=103 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [103] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=104 states=11025 words=105 m=104 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [104] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=105 states=11236 words=106 m=105 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [105] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=106 states=11449 words=107 m=106 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [106] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=107 states=11664 words=108 m=107 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [107] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=108 states=11881 words=109 m=108 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [108] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=109 states=12100 words=110 m=109 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [109] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=110 states=12321 words=111 m=110 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [110] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=111 states=12544 words=112 m=111 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [111] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=112 states=12769 words=113 m=112 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [112] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=113 states=12996 words=114 m=113 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [113] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=114 states=13225 words=115 m=114 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [114] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=115 states=13456 words=116 m=115 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [115] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=116 states=13689 words=117 m=116 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [116] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=117 states=13924 words=118 m=117 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [117] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=118 states=14161 words=119 m=118 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [118] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=119 states=14400 words=120 m=119 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [119] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=120 states=14641 words=121 m=120 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [120] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=121 states=14884 words=122 m=121 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [121] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=122 states=15129 words=123 m=122 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [122] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=123 states=15376 words=124 m=123 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [123] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=124 states=15625 words=125 m=124 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [124] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=125 states=15876 words=126 m=125 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [125] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=126 states=16129 words=127 m=126 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [126] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=127 states=16384 words=128 m=127 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
  [127] lean.generalized_multinacci_general_m_intertwiner_certificate :: m=128 states=16641 words=129 m=128 simultaneous Q/R intertwiner sweep -- instantiates roof_word_intertwiner
-/

def reflectedNodeCount : Nat := 128

end RavelGenerated
