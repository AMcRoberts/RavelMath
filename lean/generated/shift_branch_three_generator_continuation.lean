import Mathlib

/-! Generated from typed semantic nodes produced inside the exact math library.
    Concrete observations are comments; only registered, structurally justified
    lemma applications become theorem declarations. -/

namespace RavelGenerated

open Matrix

inductive SignedDefect
  | neutral | positive | negative
  deriving DecidableEq, Repr

theorem two_prefixes_give_three_signed_defects
    (p q : Bool) :
    (p = q) ∨ (p = false ∧ q = true) ∨ (p = true ∧ q = false) := by
  cases p <;> cases q <;> simp

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

/-- `SignedDefect`'s three-generator continuation is the direct Γ := SignedDefect
instantiation of `word_fold_intertwiner` -- no separate induction is needed since
the general theorem was already stated with no constraint on the alphabet type. -/
theorem three_generator_word_induction
    {α : Type*} [Preorder α] [Monoid α]
    (mul_left_mono : ∀ a : α, ∀ {b c : α}, b ≤ c → a*b ≤ a*c)
    (mul_right_mono : ∀ c : α, ∀ {a b : α}, a ≤ b → a*c ≤ b*c)
    (MC MK : SignedDefect → α) (P : α)
    (h : ∀ g, MC g * P ≤ P * MK g) :
    ∀ w : List SignedDefect,
      w.foldr (fun g z => MC g * z) 1 * P ≤
      P * w.foldr (fun g z => MK g * z) 1 :=
  word_fold_intertwiner mul_left_mono mul_right_mono MC MK P h

/-- Mechanically emitted: D=2 D=2 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_0 :
    (5 : ℕ) + 2 * 2 = (2 + 1) * (2 + 1) := by decide

/-- Mechanically emitted: D=3 D=3 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_1 :
    (10 : ℕ) + 2 * 3 = (3 + 1) * (3 + 1) := by decide

/-- Mechanically emitted: D=4 D=4 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_2 :
    (17 : ℕ) + 2 * 4 = (4 + 1) * (4 + 1) := by decide

/-- Mechanically emitted: D=5 D=5 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_3 :
    (26 : ℕ) + 2 * 5 = (5 + 1) * (5 + 1) := by decide

/-- Mechanically emitted: D=6 D=6 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_4 :
    (37 : ℕ) + 2 * 6 = (6 + 1) * (6 + 1) := by decide

/-- Mechanically emitted: D=7 D=7 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_5 :
    (50 : ℕ) + 2 * 7 = (7 + 1) * (7 + 1) := by decide

/-- Mechanically emitted: D=8 D=8 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_6 :
    (65 : ℕ) + 2 * 8 = (8 + 1) * (8 + 1) := by decide

/-- Mechanically emitted: D=9 D=9 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_7 :
    (82 : ℕ) + 2 * 9 = (9 + 1) * (9 + 1) := by decide

/-- Mechanically emitted: D=10 D=10 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_8 :
    (101 : ℕ) + 2 * 10 = (10 + 1) * (10 + 1) := by decide

/-- Mechanically emitted: D=11 D=11 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_9 :
    (122 : ℕ) + 2 * 11 = (11 + 1) * (11 + 1) := by decide

/-- Mechanically emitted: D=12 D=12 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_10 :
    (145 : ℕ) + 2 * 12 = (12 + 1) * (12 + 1) := by decide

/-- Mechanically emitted: D=13 D=13 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_11 :
    (170 : ℕ) + 2 * 13 = (13 + 1) * (13 + 1) := by decide

/-- Mechanically emitted: D=14 D=14 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_12 :
    (197 : ℕ) + 2 * 14 = (14 + 1) * (14 + 1) := by decide

/-- Mechanically emitted: D=15 D=15 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_13 :
    (226 : ℕ) + 2 * 15 = (15 + 1) * (15 + 1) := by decide

/-- Mechanically emitted: D=16 D=16 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_14 :
    (257 : ℕ) + 2 * 16 = (16 + 1) * (16 + 1) := by decide

/-- Mechanically emitted: D=17 D=17 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_15 :
    (290 : ℕ) + 2 * 17 = (17 + 1) * (17 + 1) := by decide

/-- Mechanically emitted: D=18 D=18 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_16 :
    (325 : ℕ) + 2 * 18 = (18 + 1) * (18 + 1) := by decide

/-- Mechanically emitted: D=19 D=19 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_17 :
    (362 : ℕ) + 2 * 19 = (19 + 1) * (19 + 1) := by decide

/-- Mechanically emitted: D=20 D=20 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_18 :
    (401 : ℕ) + 2 * 20 = (20 + 1) * (20 + 1) := by decide

/-- Mechanically emitted: D=21 D=21 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_19 :
    (442 : ℕ) + 2 * 21 = (21 + 1) * (21 + 1) := by decide

/-- Mechanically emitted: D=22 D=22 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_20 :
    (485 : ℕ) + 2 * 22 = (22 + 1) * (22 + 1) := by decide

/-- Mechanically emitted: D=23 D=23 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_21 :
    (530 : ℕ) + 2 * 23 = (23 + 1) * (23 + 1) := by decide

/-- Mechanically emitted: D=24 D=24 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_22 :
    (577 : ℕ) + 2 * 24 = (24 + 1) * (24 + 1) := by decide

/-- Mechanically emitted: D=25 D=25 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_23 :
    (626 : ℕ) + 2 * 25 = (25 + 1) * (25 + 1) := by decide

/-- Mechanically emitted: D=26 D=26 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_24 :
    (677 : ℕ) + 2 * 26 = (26 + 1) * (26 + 1) := by decide

/-- Mechanically emitted: D=27 D=27 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_25 :
    (730 : ℕ) + 2 * 27 = (27 + 1) * (27 + 1) := by decide

/-- Mechanically emitted: D=28 D=28 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_26 :
    (785 : ℕ) + 2 * 28 = (28 + 1) * (28 + 1) := by decide

/-- Mechanically emitted: D=29 D=29 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_27 :
    (842 : ℕ) + 2 * 29 = (29 + 1) * (29 + 1) := by decide

/-- Mechanically emitted: D=30 D=30 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_28 :
    (901 : ℕ) + 2 * 30 = (30 + 1) * (30 + 1) := by decide

/-- Mechanically emitted: D=31 D=31 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_29 :
    (962 : ℕ) + 2 * 31 = (31 + 1) * (31 + 1) := by decide

/-- Mechanically emitted: D=32 D=32 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_30 :
    (1025 : ℕ) + 2 * 32 = (32 + 1) * (32 + 1) := by decide

/-- Mechanically emitted: D=33 D=33 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_31 :
    (1090 : ℕ) + 2 * 33 = (33 + 1) * (33 + 1) := by decide

/-- Mechanically emitted: D=34 D=34 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_32 :
    (1157 : ℕ) + 2 * 34 = (34 + 1) * (34 + 1) := by decide

/-- Mechanically emitted: D=35 D=35 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_33 :
    (1226 : ℕ) + 2 * 35 = (35 + 1) * (35 + 1) := by decide

/-- Mechanically emitted: D=36 D=36 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_34 :
    (1297 : ℕ) + 2 * 36 = (36 + 1) * (36 + 1) := by decide

/-- Mechanically emitted: D=37 D=37 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_35 :
    (1370 : ℕ) + 2 * 37 = (37 + 1) * (37 + 1) := by decide

/-- Mechanically emitted: D=38 D=38 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_36 :
    (1445 : ℕ) + 2 * 38 = (38 + 1) * (38 + 1) := by decide

/-- Mechanically emitted: D=39 D=39 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_37 :
    (1522 : ℕ) + 2 * 39 = (39 + 1) * (39 + 1) := by decide

/-- Mechanically emitted: D=40 D=40 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_38 :
    (1601 : ℕ) + 2 * 40 = (40 + 1) * (40 + 1) := by decide

/-- Mechanically emitted: D=41 D=41 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_39 :
    (1682 : ℕ) + 2 * 41 = (41 + 1) * (41 + 1) := by decide

/-- Mechanically emitted: D=42 D=42 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_40 :
    (1765 : ℕ) + 2 * 42 = (42 + 1) * (42 + 1) := by decide

/-- Mechanically emitted: D=43 D=43 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_41 :
    (1850 : ℕ) + 2 * 43 = (43 + 1) * (43 + 1) := by decide

/-- Mechanically emitted: D=44 D=44 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_42 :
    (1937 : ℕ) + 2 * 44 = (44 + 1) * (44 + 1) := by decide

/-- Mechanically emitted: D=45 D=45 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_43 :
    (2026 : ℕ) + 2 * 45 = (45 + 1) * (45 + 1) := by decide

/-- Mechanically emitted: D=46 D=46 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_44 :
    (2117 : ℕ) + 2 * 46 = (46 + 1) * (46 + 1) := by decide

/-- Mechanically emitted: D=47 D=47 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_45 :
    (2210 : ℕ) + 2 * 47 = (47 + 1) * (47 + 1) := by decide

/-- Mechanically emitted: D=48 D=48 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_46 :
    (2305 : ℕ) + 2 * 48 = (48 + 1) * (48 + 1) := by decide

/-- Mechanically emitted: D=49 D=49 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_47 :
    (2402 : ℕ) + 2 * 49 = (49 + 1) * (49 + 1) := by decide

/-- Mechanically emitted: D=50 D=50 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_48 :
    (2501 : ℕ) + 2 * 50 = (50 + 1) * (50 + 1) := by decide

/-- Mechanically emitted: D=51 D=51 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_49 :
    (2602 : ℕ) + 2 * 51 = (51 + 1) * (51 + 1) := by decide

/-- Mechanically emitted: D=52 D=52 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_50 :
    (2705 : ℕ) + 2 * 52 = (52 + 1) * (52 + 1) := by decide

/-- Mechanically emitted: D=53 D=53 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_51 :
    (2810 : ℕ) + 2 * 53 = (53 + 1) * (53 + 1) := by decide

/-- Mechanically emitted: D=54 D=54 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_52 :
    (2917 : ℕ) + 2 * 54 = (54 + 1) * (54 + 1) := by decide

/-- Mechanically emitted: D=55 D=55 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_53 :
    (3026 : ℕ) + 2 * 55 = (55 + 1) * (55 + 1) := by decide

/-- Mechanically emitted: D=56 D=56 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_54 :
    (3137 : ℕ) + 2 * 56 = (56 + 1) * (56 + 1) := by decide

/-- Mechanically emitted: D=57 D=57 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_55 :
    (3250 : ℕ) + 2 * 57 = (57 + 1) * (57 + 1) := by decide

/-- Mechanically emitted: D=58 D=58 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_56 :
    (3365 : ℕ) + 2 * 58 = (58 + 1) * (58 + 1) := by decide

/-- Mechanically emitted: D=59 D=59 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_57 :
    (3482 : ℕ) + 2 * 59 = (59 + 1) * (59 + 1) := by decide

/-- Mechanically emitted: D=60 D=60 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_58 :
    (3601 : ℕ) + 2 * 60 = (60 + 1) * (60 + 1) := by decide

/-- Mechanically emitted: D=61 D=61 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_59 :
    (3722 : ℕ) + 2 * 61 = (61 + 1) * (61 + 1) := by decide

/-- Mechanically emitted: D=62 D=62 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_60 :
    (3845 : ℕ) + 2 * 62 = (62 + 1) * (62 + 1) := by decide

/-- Mechanically emitted: D=63 D=63 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_61 :
    (3970 : ℕ) + 2 * 63 = (63 + 1) * (63 + 1) := by decide

/-- Mechanically emitted: D=64 D=64 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_62 :
    (4097 : ℕ) + 2 * 64 = (64 + 1) * (64 + 1) := by decide

/-- Mechanically emitted: D=65 D=65 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_63 :
    (4226 : ℕ) + 2 * 65 = (65 + 1) * (65 + 1) := by decide

/-- Mechanically emitted: D=66 D=66 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_64 :
    (4357 : ℕ) + 2 * 66 = (66 + 1) * (66 + 1) := by decide

/-- Mechanically emitted: D=67 D=67 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_65 :
    (4490 : ℕ) + 2 * 67 = (67 + 1) * (67 + 1) := by decide

/-- Mechanically emitted: D=68 D=68 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_66 :
    (4625 : ℕ) + 2 * 68 = (68 + 1) * (68 + 1) := by decide

/-- Mechanically emitted: D=69 D=69 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_67 :
    (4762 : ℕ) + 2 * 69 = (69 + 1) * (69 + 1) := by decide

/-- Mechanically emitted: D=70 D=70 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_68 :
    (4901 : ℕ) + 2 * 70 = (70 + 1) * (70 + 1) := by decide

/-- Mechanically emitted: D=71 D=71 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_69 :
    (5042 : ℕ) + 2 * 71 = (71 + 1) * (71 + 1) := by decide

/-- Mechanically emitted: D=72 D=72 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_70 :
    (5185 : ℕ) + 2 * 72 = (72 + 1) * (72 + 1) := by decide

/-- Mechanically emitted: D=73 D=73 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_71 :
    (5330 : ℕ) + 2 * 73 = (73 + 1) * (73 + 1) := by decide

/-- Mechanically emitted: D=74 D=74 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_72 :
    (5477 : ℕ) + 2 * 74 = (74 + 1) * (74 + 1) := by decide

/-- Mechanically emitted: D=75 D=75 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_73 :
    (5626 : ℕ) + 2 * 75 = (75 + 1) * (75 + 1) := by decide

/-- Mechanically emitted: D=76 D=76 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_74 :
    (5777 : ℕ) + 2 * 76 = (76 + 1) * (76 + 1) := by decide

/-- Mechanically emitted: D=77 D=77 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_75 :
    (5930 : ℕ) + 2 * 77 = (77 + 1) * (77 + 1) := by decide

/-- Mechanically emitted: D=78 D=78 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_76 :
    (6085 : ℕ) + 2 * 78 = (78 + 1) * (78 + 1) := by decide

/-- Mechanically emitted: D=79 D=79 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_77 :
    (6242 : ℕ) + 2 * 79 = (79 + 1) * (79 + 1) := by decide

/-- Mechanically emitted: D=80 D=80 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_78 :
    (6401 : ℕ) + 2 * 80 = (80 + 1) * (80 + 1) := by decide

/-- Mechanically emitted: D=81 D=81 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_79 :
    (6562 : ℕ) + 2 * 81 = (81 + 1) * (81 + 1) := by decide

/-- Mechanically emitted: D=82 D=82 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_80 :
    (6725 : ℕ) + 2 * 82 = (82 + 1) * (82 + 1) := by decide

/-- Mechanically emitted: D=83 D=83 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_81 :
    (6890 : ℕ) + 2 * 83 = (83 + 1) * (83 + 1) := by decide

/-- Mechanically emitted: D=84 D=84 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_82 :
    (7057 : ℕ) + 2 * 84 = (84 + 1) * (84 + 1) := by decide

/-- Mechanically emitted: D=85 D=85 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_83 :
    (7226 : ℕ) + 2 * 85 = (85 + 1) * (85 + 1) := by decide

/-- Mechanically emitted: D=86 D=86 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_84 :
    (7397 : ℕ) + 2 * 86 = (86 + 1) * (86 + 1) := by decide

/-- Mechanically emitted: D=87 D=87 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_85 :
    (7570 : ℕ) + 2 * 87 = (87 + 1) * (87 + 1) := by decide

/-- Mechanically emitted: D=88 D=88 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_86 :
    (7745 : ℕ) + 2 * 88 = (88 + 1) * (88 + 1) := by decide

/-- Mechanically emitted: D=89 D=89 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_87 :
    (7922 : ℕ) + 2 * 89 = (89 + 1) * (89 + 1) := by decide

/-- Mechanically emitted: D=90 D=90 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_88 :
    (8101 : ℕ) + 2 * 90 = (90 + 1) * (90 + 1) := by decide

/-- Mechanically emitted: D=91 D=91 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_89 :
    (8282 : ℕ) + 2 * 91 = (91 + 1) * (91 + 1) := by decide

/-- Mechanically emitted: D=92 D=92 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_90 :
    (8465 : ℕ) + 2 * 92 = (92 + 1) * (92 + 1) := by decide

/-- Mechanically emitted: D=93 D=93 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_91 :
    (8650 : ℕ) + 2 * 93 = (93 + 1) * (93 + 1) := by decide

/-- Mechanically emitted: D=94 D=94 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_92 :
    (8837 : ℕ) + 2 * 94 = (94 + 1) * (94 + 1) := by decide

/-- Mechanically emitted: D=95 D=95 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_93 :
    (9026 : ℕ) + 2 * 95 = (95 + 1) * (95 + 1) := by decide

/-- Mechanically emitted: D=96 D=96 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_94 :
    (9217 : ℕ) + 2 * 96 = (96 + 1) * (96 + 1) := by decide

/-- Mechanically emitted: D=97 D=97 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_95 :
    (9410 : ℕ) + 2 * 97 = (97 + 1) * (97 + 1) := by decide

/-- Mechanically emitted: D=98 D=98 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_96 :
    (9605 : ℕ) + 2 * 98 = (98 + 1) * (98 + 1) := by decide

/-- Mechanically emitted: D=99 D=99 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_97 :
    (9802 : ℕ) + 2 * 99 = (99 + 1) * (99 + 1) := by decide

/-- Mechanically emitted: D=100 D=100 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_98 :
    (10001 : ℕ) + 2 * 100 = (100 + 1) * (100 + 1) := by decide

/-- Mechanically emitted: D=101 D=101 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_99 :
    (10202 : ℕ) + 2 * 101 = (101 + 1) * (101 + 1) := by decide

/-- Mechanically emitted: D=102 D=102 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_100 :
    (10405 : ℕ) + 2 * 102 = (102 + 1) * (102 + 1) := by decide

/-- Mechanically emitted: D=103 D=103 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_101 :
    (10610 : ℕ) + 2 * 103 = (103 + 1) * (103 + 1) := by decide

/-- Mechanically emitted: D=104 D=104 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_102 :
    (10817 : ℕ) + 2 * 104 = (104 + 1) * (104 + 1) := by decide

/-- Mechanically emitted: D=105 D=105 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_103 :
    (11026 : ℕ) + 2 * 105 = (105 + 1) * (105 + 1) := by decide

/-- Mechanically emitted: D=106 D=106 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_104 :
    (11237 : ℕ) + 2 * 106 = (106 + 1) * (106 + 1) := by decide

/-- Mechanically emitted: D=107 D=107 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_105 :
    (11450 : ℕ) + 2 * 107 = (107 + 1) * (107 + 1) := by decide

/-- Mechanically emitted: D=108 D=108 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_106 :
    (11665 : ℕ) + 2 * 108 = (108 + 1) * (108 + 1) := by decide

/-- Mechanically emitted: D=109 D=109 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_107 :
    (11882 : ℕ) + 2 * 109 = (109 + 1) * (109 + 1) := by decide

/-- Mechanically emitted: D=110 D=110 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_108 :
    (12101 : ℕ) + 2 * 110 = (110 + 1) * (110 + 1) := by decide

/-- Mechanically emitted: D=111 D=111 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_109 :
    (12322 : ℕ) + 2 * 111 = (111 + 1) * (111 + 1) := by decide

/-- Mechanically emitted: D=112 D=112 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_110 :
    (12545 : ℕ) + 2 * 112 = (112 + 1) * (112 + 1) := by decide

/-- Mechanically emitted: D=113 D=113 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_111 :
    (12770 : ℕ) + 2 * 113 = (113 + 1) * (113 + 1) := by decide

/-- Mechanically emitted: D=114 D=114 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_112 :
    (12997 : ℕ) + 2 * 114 = (114 + 1) * (114 + 1) := by decide

/-- Mechanically emitted: D=115 D=115 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_113 :
    (13226 : ℕ) + 2 * 115 = (115 + 1) * (115 + 1) := by decide

/-- Mechanically emitted: D=116 D=116 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_114 :
    (13457 : ℕ) + 2 * 116 = (116 + 1) * (116 + 1) := by decide

/-- Mechanically emitted: D=117 D=117 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_115 :
    (13690 : ℕ) + 2 * 117 = (117 + 1) * (117 + 1) := by decide

/-- Mechanically emitted: D=118 D=118 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_116 :
    (13925 : ℕ) + 2 * 118 = (118 + 1) * (118 + 1) := by decide

/-- Mechanically emitted: D=119 D=119 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_117 :
    (14162 : ℕ) + 2 * 119 = (119 + 1) * (119 + 1) := by decide

/-- Mechanically emitted: D=120 D=120 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_118 :
    (14401 : ℕ) + 2 * 120 = (120 + 1) * (120 + 1) := by decide

/-- Mechanically emitted: D=121 D=121 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_119 :
    (14642 : ℕ) + 2 * 121 = (121 + 1) * (121 + 1) := by decide

/-- Mechanically emitted: D=122 D=122 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_120 :
    (14885 : ℕ) + 2 * 122 = (122 + 1) * (122 + 1) := by decide

/-- Mechanically emitted: D=123 D=123 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_121 :
    (15130 : ℕ) + 2 * 123 = (123 + 1) * (123 + 1) := by decide

/-- Mechanically emitted: D=124 D=124 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_122 :
    (15377 : ℕ) + 2 * 124 = (124 + 1) * (124 + 1) := by decide

/-- Mechanically emitted: D=125 D=125 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_123 :
    (15626 : ℕ) + 2 * 125 = (125 + 1) * (125 + 1) := by decide

/-- Mechanically emitted: D=126 D=126 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_124 :
    (15877 : ℕ) + 2 * 126 = (126 + 1) * (126 + 1) := by decide

/-- Mechanically emitted: D=127 D=127 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_125 :
    (16130 : ℕ) + 2 * 127 = (127 + 1) * (127 + 1) := by decide

/-- Mechanically emitted: D=128 D=128 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_126 :
    (16385 : ℕ) + 2 * 128 = (128 + 1) * (128 + 1) := by decide

/-- Mechanically emitted: D=129 D=129 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_127 :
    (16642 : ℕ) + 2 * 129 = (129 + 1) * (129 + 1) := by decide

/-- Mechanically emitted: D=130 D=130 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_128 :
    (16901 : ℕ) + 2 * 130 = (130 + 1) * (130 + 1) := by decide

/-- Mechanically emitted: D=131 D=131 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_129 :
    (17162 : ℕ) + 2 * 131 = (131 + 1) * (131 + 1) := by decide

/-- Mechanically emitted: D=132 D=132 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_130 :
    (17425 : ℕ) + 2 * 132 = (132 + 1) * (132 + 1) := by decide

/-- Mechanically emitted: D=133 D=133 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_131 :
    (17690 : ℕ) + 2 * 133 = (133 + 1) * (133 + 1) := by decide

/-- Mechanically emitted: D=134 D=134 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_132 :
    (17957 : ℕ) + 2 * 134 = (134 + 1) * (134 + 1) := by decide

/-- Mechanically emitted: D=135 D=135 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_133 :
    (18226 : ℕ) + 2 * 135 = (135 + 1) * (135 + 1) := by decide

/-- Mechanically emitted: D=136 D=136 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_134 :
    (18497 : ℕ) + 2 * 136 = (136 + 1) * (136 + 1) := by decide

/-- Mechanically emitted: D=137 D=137 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_135 :
    (18770 : ℕ) + 2 * 137 = (137 + 1) * (137 + 1) := by decide

/-- Mechanically emitted: D=138 D=138 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_136 :
    (19045 : ℕ) + 2 * 138 = (138 + 1) * (138 + 1) := by decide

/-- Mechanically emitted: D=139 D=139 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_137 :
    (19322 : ℕ) + 2 * 139 = (139 + 1) * (139 + 1) := by decide

/-- Mechanically emitted: D=140 D=140 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_138 :
    (19601 : ℕ) + 2 * 140 = (140 + 1) * (140 + 1) := by decide

/-- Mechanically emitted: D=141 D=141 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_139 :
    (19882 : ℕ) + 2 * 141 = (141 + 1) * (141 + 1) := by decide

/-- Mechanically emitted: D=142 D=142 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_140 :
    (20165 : ℕ) + 2 * 142 = (142 + 1) * (142 + 1) := by decide

/-- Mechanically emitted: D=143 D=143 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_141 :
    (20450 : ℕ) + 2 * 143 = (143 + 1) * (143 + 1) := by decide

/-- Mechanically emitted: D=144 D=144 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_142 :
    (20737 : ℕ) + 2 * 144 = (144 + 1) * (144 + 1) := by decide

/-- Mechanically emitted: D=145 D=145 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_143 :
    (21026 : ℕ) + 2 * 145 = (145 + 1) * (145 + 1) := by decide

/-- Mechanically emitted: D=146 D=146 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_144 :
    (21317 : ℕ) + 2 * 146 = (146 + 1) * (146 + 1) := by decide

/-- Mechanically emitted: D=147 D=147 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_145 :
    (21610 : ℕ) + 2 * 147 = (147 + 1) * (147 + 1) := by decide

/-- Mechanically emitted: D=148 D=148 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_146 :
    (21905 : ℕ) + 2 * 148 = (148 + 1) * (148 + 1) := by decide

/-- Mechanically emitted: D=149 D=149 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_147 :
    (22202 : ℕ) + 2 * 149 = (149 + 1) * (149 + 1) := by decide

/-- Mechanically emitted: D=150 D=150 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_148 :
    (22501 : ℕ) + 2 * 150 = (150 + 1) * (150 + 1) := by decide

/-- Mechanically emitted: D=151 D=151 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_149 :
    (22802 : ℕ) + 2 * 151 = (151 + 1) * (151 + 1) := by decide

/-- Mechanically emitted: D=152 D=152 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_150 :
    (23105 : ℕ) + 2 * 152 = (152 + 1) * (152 + 1) := by decide

/-- Mechanically emitted: D=153 D=153 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_151 :
    (23410 : ℕ) + 2 * 153 = (153 + 1) * (153 + 1) := by decide

/-- Mechanically emitted: D=154 D=154 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_152 :
    (23717 : ℕ) + 2 * 154 = (154 + 1) * (154 + 1) := by decide

/-- Mechanically emitted: D=155 D=155 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_153 :
    (24026 : ℕ) + 2 * 155 = (155 + 1) * (155 + 1) := by decide

/-- Mechanically emitted: D=156 D=156 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_154 :
    (24337 : ℕ) + 2 * 156 = (156 + 1) * (156 + 1) := by decide

/-- Mechanically emitted: D=157 D=157 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_155 :
    (24650 : ℕ) + 2 * 157 = (157 + 1) * (157 + 1) := by decide

/-- Mechanically emitted: D=158 D=158 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_156 :
    (24965 : ℕ) + 2 * 158 = (158 + 1) * (158 + 1) := by decide

/-- Mechanically emitted: D=159 D=159 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_157 :
    (25282 : ℕ) + 2 * 159 = (159 + 1) * (159 + 1) := by decide

/-- Mechanically emitted: D=160 D=160 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_158 :
    (25601 : ℕ) + 2 * 160 = (160 + 1) * (160 + 1) := by decide

/-- Mechanically emitted: D=161 D=161 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_159 :
    (25922 : ℕ) + 2 * 161 = (161 + 1) * (161 + 1) := by decide

/-- Mechanically emitted: D=162 D=162 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_160 :
    (26245 : ℕ) + 2 * 162 = (162 + 1) * (162 + 1) := by decide

/-- Mechanically emitted: D=163 D=163 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_161 :
    (26570 : ℕ) + 2 * 163 = (163 + 1) * (163 + 1) := by decide

/-- Mechanically emitted: D=164 D=164 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_162 :
    (26897 : ℕ) + 2 * 164 = (164 + 1) * (164 + 1) := by decide

/-- Mechanically emitted: D=165 D=165 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_163 :
    (27226 : ℕ) + 2 * 165 = (165 + 1) * (165 + 1) := by decide

/-- Mechanically emitted: D=166 D=166 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_164 :
    (27557 : ℕ) + 2 * 166 = (166 + 1) * (166 + 1) := by decide

/-- Mechanically emitted: D=167 D=167 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_165 :
    (27890 : ℕ) + 2 * 167 = (167 + 1) * (167 + 1) := by decide

/-- Mechanically emitted: D=168 D=168 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_166 :
    (28225 : ℕ) + 2 * 168 = (168 + 1) * (168 + 1) := by decide

/-- Mechanically emitted: D=169 D=169 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_167 :
    (28562 : ℕ) + 2 * 169 = (169 + 1) * (169 + 1) := by decide

/-- Mechanically emitted: D=170 D=170 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_168 :
    (28901 : ℕ) + 2 * 170 = (170 + 1) * (170 + 1) := by decide

/-- Mechanically emitted: D=171 D=171 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_169 :
    (29242 : ℕ) + 2 * 171 = (171 + 1) * (171 + 1) := by decide

/-- Mechanically emitted: D=172 D=172 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_170 :
    (29585 : ℕ) + 2 * 172 = (172 + 1) * (172 + 1) := by decide

/-- Mechanically emitted: D=173 D=173 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_171 :
    (29930 : ℕ) + 2 * 173 = (173 + 1) * (173 + 1) := by decide

/-- Mechanically emitted: D=174 D=174 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_172 :
    (30277 : ℕ) + 2 * 174 = (174 + 1) * (174 + 1) := by decide

/-- Mechanically emitted: D=175 D=175 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_173 :
    (30626 : ℕ) + 2 * 175 = (175 + 1) * (175 + 1) := by decide

/-- Mechanically emitted: D=176 D=176 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_174 :
    (30977 : ℕ) + 2 * 176 = (176 + 1) * (176 + 1) := by decide

/-- Mechanically emitted: D=177 D=177 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_175 :
    (31330 : ℕ) + 2 * 177 = (177 + 1) * (177 + 1) := by decide

/-- Mechanically emitted: D=178 D=178 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_176 :
    (31685 : ℕ) + 2 * 178 = (178 + 1) * (178 + 1) := by decide

/-- Mechanically emitted: D=179 D=179 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_177 :
    (32042 : ℕ) + 2 * 179 = (179 + 1) * (179 + 1) := by decide

/-- Mechanically emitted: D=180 D=180 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_178 :
    (32401 : ℕ) + 2 * 180 = (180 + 1) * (180 + 1) := by decide

/-- Mechanically emitted: D=181 D=181 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_179 :
    (32762 : ℕ) + 2 * 181 = (181 + 1) * (181 + 1) := by decide

/-- Mechanically emitted: D=182 D=182 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_180 :
    (33125 : ℕ) + 2 * 182 = (182 + 1) * (182 + 1) := by decide

/-- Mechanically emitted: D=183 D=183 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_181 :
    (33490 : ℕ) + 2 * 183 = (183 + 1) * (183 + 1) := by decide

/-- Mechanically emitted: D=184 D=184 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_182 :
    (33857 : ℕ) + 2 * 184 = (184 + 1) * (184 + 1) := by decide

/-- Mechanically emitted: D=185 D=185 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_183 :
    (34226 : ℕ) + 2 * 185 = (185 + 1) * (185 + 1) := by decide

/-- Mechanically emitted: D=186 D=186 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_184 :
    (34597 : ℕ) + 2 * 186 = (186 + 1) * (186 + 1) := by decide

/-- Mechanically emitted: D=187 D=187 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_185 :
    (34970 : ℕ) + 2 * 187 = (187 + 1) * (187 + 1) := by decide

/-- Mechanically emitted: D=188 D=188 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_186 :
    (35345 : ℕ) + 2 * 188 = (188 + 1) * (188 + 1) := by decide

/-- Mechanically emitted: D=189 D=189 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_187 :
    (35722 : ℕ) + 2 * 189 = (189 + 1) * (189 + 1) := by decide

/-- Mechanically emitted: D=190 D=190 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_188 :
    (36101 : ℕ) + 2 * 190 = (190 + 1) * (190 + 1) := by decide

/-- Mechanically emitted: D=191 D=191 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_189 :
    (36482 : ℕ) + 2 * 191 = (191 + 1) * (191 + 1) := by decide

/-- Mechanically emitted: D=192 D=192 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_190 :
    (36865 : ℕ) + 2 * 192 = (192 + 1) * (192 + 1) := by decide

/-- Mechanically emitted: D=193 D=193 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_191 :
    (37250 : ℕ) + 2 * 193 = (193 + 1) * (193 + 1) := by decide

/-- Mechanically emitted: D=194 D=194 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_192 :
    (37637 : ℕ) + 2 * 194 = (194 + 1) * (194 + 1) := by decide

/-- Mechanically emitted: D=195 D=195 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_193 :
    (38026 : ℕ) + 2 * 195 = (195 + 1) * (195 + 1) := by decide

/-- Mechanically emitted: D=196 D=196 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_194 :
    (38417 : ℕ) + 2 * 196 = (196 + 1) * (196 + 1) := by decide

/-- Mechanically emitted: D=197 D=197 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_195 :
    (38810 : ℕ) + 2 * 197 = (197 + 1) * (197 + 1) := by decide

/-- Mechanically emitted: D=198 D=198 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_196 :
    (39205 : ℕ) + 2 * 198 = (198 + 1) * (198 + 1) := by decide

/-- Mechanically emitted: D=199 D=199 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_197 :
    (39602 : ℕ) + 2 * 199 = (199 + 1) * (199 + 1) := by decide

/-- Mechanically emitted: D=200 D=200 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_198 :
    (40001 : ℕ) + 2 * 200 = (200 + 1) * (200 + 1) := by decide

/-- Mechanically emitted: D=201 D=201 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_199 :
    (40402 : ℕ) + 2 * 201 = (201 + 1) * (201 + 1) := by decide

/-- Mechanically emitted: D=202 D=202 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_200 :
    (40805 : ℕ) + 2 * 202 = (202 + 1) * (202 + 1) := by decide

/-- Mechanically emitted: D=203 D=203 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_201 :
    (41210 : ℕ) + 2 * 203 = (203 + 1) * (203 + 1) := by decide

/-- Mechanically emitted: D=204 D=204 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_202 :
    (41617 : ℕ) + 2 * 204 = (204 + 1) * (204 + 1) := by decide

/-- Mechanically emitted: D=205 D=205 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_203 :
    (42026 : ℕ) + 2 * 205 = (205 + 1) * (205 + 1) := by decide

/-- Mechanically emitted: D=206 D=206 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_204 :
    (42437 : ℕ) + 2 * 206 = (206 + 1) * (206 + 1) := by decide

/-- Mechanically emitted: D=207 D=207 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_205 :
    (42850 : ℕ) + 2 * 207 = (207 + 1) * (207 + 1) := by decide

/-- Mechanically emitted: D=208 D=208 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_206 :
    (43265 : ℕ) + 2 * 208 = (208 + 1) * (208 + 1) := by decide

/-- Mechanically emitted: D=209 D=209 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_207 :
    (43682 : ℕ) + 2 * 209 = (209 + 1) * (209 + 1) := by decide

/-- Mechanically emitted: D=210 D=210 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_208 :
    (44101 : ℕ) + 2 * 210 = (210 + 1) * (210 + 1) := by decide

/-- Mechanically emitted: D=211 D=211 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_209 :
    (44522 : ℕ) + 2 * 211 = (211 + 1) * (211 + 1) := by decide

/-- Mechanically emitted: D=212 D=212 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_210 :
    (44945 : ℕ) + 2 * 212 = (212 + 1) * (212 + 1) := by decide

/-- Mechanically emitted: D=213 D=213 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_211 :
    (45370 : ℕ) + 2 * 213 = (213 + 1) * (213 + 1) := by decide

/-- Mechanically emitted: D=214 D=214 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_212 :
    (45797 : ℕ) + 2 * 214 = (214 + 1) * (214 + 1) := by decide

/-- Mechanically emitted: D=215 D=215 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_213 :
    (46226 : ℕ) + 2 * 215 = (215 + 1) * (215 + 1) := by decide

/-- Mechanically emitted: D=216 D=216 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_214 :
    (46657 : ℕ) + 2 * 216 = (216 + 1) * (216 + 1) := by decide

/-- Mechanically emitted: D=217 D=217 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_215 :
    (47090 : ℕ) + 2 * 217 = (217 + 1) * (217 + 1) := by decide

/-- Mechanically emitted: D=218 D=218 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_216 :
    (47525 : ℕ) + 2 * 218 = (218 + 1) * (218 + 1) := by decide

/-- Mechanically emitted: D=219 D=219 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_217 :
    (47962 : ℕ) + 2 * 219 = (219 + 1) * (219 + 1) := by decide

/-- Mechanically emitted: D=220 D=220 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_218 :
    (48401 : ℕ) + 2 * 220 = (220 + 1) * (220 + 1) := by decide

/-- Mechanically emitted: D=221 D=221 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_219 :
    (48842 : ℕ) + 2 * 221 = (221 + 1) * (221 + 1) := by decide

/-- Mechanically emitted: D=222 D=222 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_220 :
    (49285 : ℕ) + 2 * 222 = (222 + 1) * (222 + 1) := by decide

/-- Mechanically emitted: D=223 D=223 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_221 :
    (49730 : ℕ) + 2 * 223 = (223 + 1) * (223 + 1) := by decide

/-- Mechanically emitted: D=224 D=224 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_222 :
    (50177 : ℕ) + 2 * 224 = (224 + 1) * (224 + 1) := by decide

/-- Mechanically emitted: D=225 D=225 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_223 :
    (50626 : ℕ) + 2 * 225 = (225 + 1) * (225 + 1) := by decide

/-- Mechanically emitted: D=226 D=226 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_224 :
    (51077 : ℕ) + 2 * 226 = (226 + 1) * (226 + 1) := by decide

/-- Mechanically emitted: D=227 D=227 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_225 :
    (51530 : ℕ) + 2 * 227 = (227 + 1) * (227 + 1) := by decide

/-- Mechanically emitted: D=228 D=228 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_226 :
    (51985 : ℕ) + 2 * 228 = (228 + 1) * (228 + 1) := by decide

/-- Mechanically emitted: D=229 D=229 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_227 :
    (52442 : ℕ) + 2 * 229 = (229 + 1) * (229 + 1) := by decide

/-- Mechanically emitted: D=230 D=230 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_228 :
    (52901 : ℕ) + 2 * 230 = (230 + 1) * (230 + 1) := by decide

/-- Mechanically emitted: D=231 D=231 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_229 :
    (53362 : ℕ) + 2 * 231 = (231 + 1) * (231 + 1) := by decide

/-- Mechanically emitted: D=232 D=232 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_230 :
    (53825 : ℕ) + 2 * 232 = (232 + 1) * (232 + 1) := by decide

/-- Mechanically emitted: D=233 D=233 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_231 :
    (54290 : ℕ) + 2 * 233 = (233 + 1) * (233 + 1) := by decide

/-- Mechanically emitted: D=234 D=234 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_232 :
    (54757 : ℕ) + 2 * 234 = (234 + 1) * (234 + 1) := by decide

/-- Mechanically emitted: D=235 D=235 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_233 :
    (55226 : ℕ) + 2 * 235 = (235 + 1) * (235 + 1) := by decide

/-- Mechanically emitted: D=236 D=236 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_234 :
    (55697 : ℕ) + 2 * 236 = (236 + 1) * (236 + 1) := by decide

/-- Mechanically emitted: D=237 D=237 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_235 :
    (56170 : ℕ) + 2 * 237 = (237 + 1) * (237 + 1) := by decide

/-- Mechanically emitted: D=238 D=238 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_236 :
    (56645 : ℕ) + 2 * 238 = (238 + 1) * (238 + 1) := by decide

/-- Mechanically emitted: D=239 D=239 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_237 :
    (57122 : ℕ) + 2 * 239 = (239 + 1) * (239 + 1) := by decide

/-- Mechanically emitted: D=240 D=240 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_238 :
    (57601 : ℕ) + 2 * 240 = (240 + 1) * (240 + 1) := by decide

/-- Mechanically emitted: D=241 D=241 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_239 :
    (58082 : ℕ) + 2 * 241 = (241 + 1) * (241 + 1) := by decide

/-- Mechanically emitted: D=242 D=242 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_240 :
    (58565 : ℕ) + 2 * 242 = (242 + 1) * (242 + 1) := by decide

/-- Mechanically emitted: D=243 D=243 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_241 :
    (59050 : ℕ) + 2 * 243 = (243 + 1) * (243 + 1) := by decide

/-- Mechanically emitted: D=244 D=244 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_242 :
    (59537 : ℕ) + 2 * 244 = (244 + 1) * (244 + 1) := by decide

/-- Mechanically emitted: D=245 D=245 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_243 :
    (60026 : ℕ) + 2 * 245 = (245 + 1) * (245 + 1) := by decide

/-- Mechanically emitted: D=246 D=246 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_244 :
    (60517 : ℕ) + 2 * 246 = (246 + 1) * (246 + 1) := by decide

/-- Mechanically emitted: D=247 D=247 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_245 :
    (61010 : ℕ) + 2 * 247 = (247 + 1) * (247 + 1) := by decide

/-- Mechanically emitted: D=248 D=248 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_246 :
    (61505 : ℕ) + 2 * 248 = (248 + 1) * (248 + 1) := by decide

/-- Mechanically emitted: D=249 D=249 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_247 :
    (62002 : ℕ) + 2 * 249 = (249 + 1) * (249 + 1) := by decide

/-- Mechanically emitted: D=250 D=250 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_248 :
    (62501 : ℕ) + 2 * 250 = (250 + 1) * (250 + 1) := by decide

/-- Mechanically emitted: D=251 D=251 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_249 :
    (63002 : ℕ) + 2 * 251 = (251 + 1) * (251 + 1) := by decide

/-- Mechanically emitted: D=252 D=252 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_250 :
    (63505 : ℕ) + 2 * 252 = (252 + 1) * (252 + 1) := by decide

/-- Mechanically emitted: D=253 D=253 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_251 :
    (64010 : ℕ) + 2 * 253 = (253 + 1) * (253 + 1) := by decide

/-- Mechanically emitted: D=254 D=254 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_252 :
    (64517 : ℕ) + 2 * 254 = (254 + 1) * (254 + 1) := by decide

/-- Mechanically emitted: D=255 D=255 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_253 :
    (65026 : ℕ) + 2 * 255 = (255 + 1) * (255 + 1) := by decide

/-- Mechanically emitted: D=256 D=256 shift-branch three-generator continuation instance -- ravel::proof::stage_shift_branch_three_generator_continuation
    independently reconfirmed ordered_prefix_pair_counts[0] + 2*D = (D+1)^2. -/
theorem shift_branch_three_generator_continuation_instance_254 :
    (65537 : ℕ) + 2 * 256 = (256 + 1) * (256 + 1) := by decide

/- Semantic proof graph for: shift_branch_three_generator_continuation_batch
  [0] lean.shift_branch_three_generator_continuation_certificate :: D=2 parents=3 D=2 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [1] lean.shift_branch_three_generator_continuation_certificate :: D=3 parents=4 D=3 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [2] lean.shift_branch_three_generator_continuation_certificate :: D=4 parents=5 D=4 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [3] lean.shift_branch_three_generator_continuation_certificate :: D=5 parents=6 D=5 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [4] lean.shift_branch_three_generator_continuation_certificate :: D=6 parents=7 D=6 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [5] lean.shift_branch_three_generator_continuation_certificate :: D=7 parents=8 D=7 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [6] lean.shift_branch_three_generator_continuation_certificate :: D=8 parents=9 D=8 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [7] lean.shift_branch_three_generator_continuation_certificate :: D=9 parents=10 D=9 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [8] lean.shift_branch_three_generator_continuation_certificate :: D=10 parents=11 D=10 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [9] lean.shift_branch_three_generator_continuation_certificate :: D=11 parents=12 D=11 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [10] lean.shift_branch_three_generator_continuation_certificate :: D=12 parents=13 D=12 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [11] lean.shift_branch_three_generator_continuation_certificate :: D=13 parents=14 D=13 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [12] lean.shift_branch_three_generator_continuation_certificate :: D=14 parents=15 D=14 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [13] lean.shift_branch_three_generator_continuation_certificate :: D=15 parents=16 D=15 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [14] lean.shift_branch_three_generator_continuation_certificate :: D=16 parents=17 D=16 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [15] lean.shift_branch_three_generator_continuation_certificate :: D=17 parents=18 D=17 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [16] lean.shift_branch_three_generator_continuation_certificate :: D=18 parents=19 D=18 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [17] lean.shift_branch_three_generator_continuation_certificate :: D=19 parents=20 D=19 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [18] lean.shift_branch_three_generator_continuation_certificate :: D=20 parents=21 D=20 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [19] lean.shift_branch_three_generator_continuation_certificate :: D=21 parents=22 D=21 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [20] lean.shift_branch_three_generator_continuation_certificate :: D=22 parents=23 D=22 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [21] lean.shift_branch_three_generator_continuation_certificate :: D=23 parents=24 D=23 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [22] lean.shift_branch_three_generator_continuation_certificate :: D=24 parents=25 D=24 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [23] lean.shift_branch_three_generator_continuation_certificate :: D=25 parents=26 D=25 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [24] lean.shift_branch_three_generator_continuation_certificate :: D=26 parents=27 D=26 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [25] lean.shift_branch_three_generator_continuation_certificate :: D=27 parents=28 D=27 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [26] lean.shift_branch_three_generator_continuation_certificate :: D=28 parents=29 D=28 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [27] lean.shift_branch_three_generator_continuation_certificate :: D=29 parents=30 D=29 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [28] lean.shift_branch_three_generator_continuation_certificate :: D=30 parents=31 D=30 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [29] lean.shift_branch_three_generator_continuation_certificate :: D=31 parents=32 D=31 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [30] lean.shift_branch_three_generator_continuation_certificate :: D=32 parents=33 D=32 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [31] lean.shift_branch_three_generator_continuation_certificate :: D=33 parents=34 D=33 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [32] lean.shift_branch_three_generator_continuation_certificate :: D=34 parents=35 D=34 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [33] lean.shift_branch_three_generator_continuation_certificate :: D=35 parents=36 D=35 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [34] lean.shift_branch_three_generator_continuation_certificate :: D=36 parents=37 D=36 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [35] lean.shift_branch_three_generator_continuation_certificate :: D=37 parents=38 D=37 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [36] lean.shift_branch_three_generator_continuation_certificate :: D=38 parents=39 D=38 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [37] lean.shift_branch_three_generator_continuation_certificate :: D=39 parents=40 D=39 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [38] lean.shift_branch_three_generator_continuation_certificate :: D=40 parents=41 D=40 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [39] lean.shift_branch_three_generator_continuation_certificate :: D=41 parents=42 D=41 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [40] lean.shift_branch_three_generator_continuation_certificate :: D=42 parents=43 D=42 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [41] lean.shift_branch_three_generator_continuation_certificate :: D=43 parents=44 D=43 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [42] lean.shift_branch_three_generator_continuation_certificate :: D=44 parents=45 D=44 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [43] lean.shift_branch_three_generator_continuation_certificate :: D=45 parents=46 D=45 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [44] lean.shift_branch_three_generator_continuation_certificate :: D=46 parents=47 D=46 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [45] lean.shift_branch_three_generator_continuation_certificate :: D=47 parents=48 D=47 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [46] lean.shift_branch_three_generator_continuation_certificate :: D=48 parents=49 D=48 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [47] lean.shift_branch_three_generator_continuation_certificate :: D=49 parents=50 D=49 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [48] lean.shift_branch_three_generator_continuation_certificate :: D=50 parents=51 D=50 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [49] lean.shift_branch_three_generator_continuation_certificate :: D=51 parents=52 D=51 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [50] lean.shift_branch_three_generator_continuation_certificate :: D=52 parents=53 D=52 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [51] lean.shift_branch_three_generator_continuation_certificate :: D=53 parents=54 D=53 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [52] lean.shift_branch_three_generator_continuation_certificate :: D=54 parents=55 D=54 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [53] lean.shift_branch_three_generator_continuation_certificate :: D=55 parents=56 D=55 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [54] lean.shift_branch_three_generator_continuation_certificate :: D=56 parents=57 D=56 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [55] lean.shift_branch_three_generator_continuation_certificate :: D=57 parents=58 D=57 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [56] lean.shift_branch_three_generator_continuation_certificate :: D=58 parents=59 D=58 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [57] lean.shift_branch_three_generator_continuation_certificate :: D=59 parents=60 D=59 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [58] lean.shift_branch_three_generator_continuation_certificate :: D=60 parents=61 D=60 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [59] lean.shift_branch_three_generator_continuation_certificate :: D=61 parents=62 D=61 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [60] lean.shift_branch_three_generator_continuation_certificate :: D=62 parents=63 D=62 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [61] lean.shift_branch_three_generator_continuation_certificate :: D=63 parents=64 D=63 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [62] lean.shift_branch_three_generator_continuation_certificate :: D=64 parents=65 D=64 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [63] lean.shift_branch_three_generator_continuation_certificate :: D=65 parents=66 D=65 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [64] lean.shift_branch_three_generator_continuation_certificate :: D=66 parents=67 D=66 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [65] lean.shift_branch_three_generator_continuation_certificate :: D=67 parents=68 D=67 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [66] lean.shift_branch_three_generator_continuation_certificate :: D=68 parents=69 D=68 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [67] lean.shift_branch_three_generator_continuation_certificate :: D=69 parents=70 D=69 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [68] lean.shift_branch_three_generator_continuation_certificate :: D=70 parents=71 D=70 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [69] lean.shift_branch_three_generator_continuation_certificate :: D=71 parents=72 D=71 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [70] lean.shift_branch_three_generator_continuation_certificate :: D=72 parents=73 D=72 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [71] lean.shift_branch_three_generator_continuation_certificate :: D=73 parents=74 D=73 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [72] lean.shift_branch_three_generator_continuation_certificate :: D=74 parents=75 D=74 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [73] lean.shift_branch_three_generator_continuation_certificate :: D=75 parents=76 D=75 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [74] lean.shift_branch_three_generator_continuation_certificate :: D=76 parents=77 D=76 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [75] lean.shift_branch_three_generator_continuation_certificate :: D=77 parents=78 D=77 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [76] lean.shift_branch_three_generator_continuation_certificate :: D=78 parents=79 D=78 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [77] lean.shift_branch_three_generator_continuation_certificate :: D=79 parents=80 D=79 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [78] lean.shift_branch_three_generator_continuation_certificate :: D=80 parents=81 D=80 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [79] lean.shift_branch_three_generator_continuation_certificate :: D=81 parents=82 D=81 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [80] lean.shift_branch_three_generator_continuation_certificate :: D=82 parents=83 D=82 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [81] lean.shift_branch_three_generator_continuation_certificate :: D=83 parents=84 D=83 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [82] lean.shift_branch_three_generator_continuation_certificate :: D=84 parents=85 D=84 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [83] lean.shift_branch_three_generator_continuation_certificate :: D=85 parents=86 D=85 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [84] lean.shift_branch_three_generator_continuation_certificate :: D=86 parents=87 D=86 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [85] lean.shift_branch_three_generator_continuation_certificate :: D=87 parents=88 D=87 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [86] lean.shift_branch_three_generator_continuation_certificate :: D=88 parents=89 D=88 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [87] lean.shift_branch_three_generator_continuation_certificate :: D=89 parents=90 D=89 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [88] lean.shift_branch_three_generator_continuation_certificate :: D=90 parents=91 D=90 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [89] lean.shift_branch_three_generator_continuation_certificate :: D=91 parents=92 D=91 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [90] lean.shift_branch_three_generator_continuation_certificate :: D=92 parents=93 D=92 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [91] lean.shift_branch_three_generator_continuation_certificate :: D=93 parents=94 D=93 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [92] lean.shift_branch_three_generator_continuation_certificate :: D=94 parents=95 D=94 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [93] lean.shift_branch_three_generator_continuation_certificate :: D=95 parents=96 D=95 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [94] lean.shift_branch_three_generator_continuation_certificate :: D=96 parents=97 D=96 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [95] lean.shift_branch_three_generator_continuation_certificate :: D=97 parents=98 D=97 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [96] lean.shift_branch_three_generator_continuation_certificate :: D=98 parents=99 D=98 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [97] lean.shift_branch_three_generator_continuation_certificate :: D=99 parents=100 D=99 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [98] lean.shift_branch_three_generator_continuation_certificate :: D=100 parents=101 D=100 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [99] lean.shift_branch_three_generator_continuation_certificate :: D=101 parents=102 D=101 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [100] lean.shift_branch_three_generator_continuation_certificate :: D=102 parents=103 D=102 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [101] lean.shift_branch_three_generator_continuation_certificate :: D=103 parents=104 D=103 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [102] lean.shift_branch_three_generator_continuation_certificate :: D=104 parents=105 D=104 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [103] lean.shift_branch_three_generator_continuation_certificate :: D=105 parents=106 D=105 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [104] lean.shift_branch_three_generator_continuation_certificate :: D=106 parents=107 D=106 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [105] lean.shift_branch_three_generator_continuation_certificate :: D=107 parents=108 D=107 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [106] lean.shift_branch_three_generator_continuation_certificate :: D=108 parents=109 D=108 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [107] lean.shift_branch_three_generator_continuation_certificate :: D=109 parents=110 D=109 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [108] lean.shift_branch_three_generator_continuation_certificate :: D=110 parents=111 D=110 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [109] lean.shift_branch_three_generator_continuation_certificate :: D=111 parents=112 D=111 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [110] lean.shift_branch_three_generator_continuation_certificate :: D=112 parents=113 D=112 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [111] lean.shift_branch_three_generator_continuation_certificate :: D=113 parents=114 D=113 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [112] lean.shift_branch_three_generator_continuation_certificate :: D=114 parents=115 D=114 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [113] lean.shift_branch_three_generator_continuation_certificate :: D=115 parents=116 D=115 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [114] lean.shift_branch_three_generator_continuation_certificate :: D=116 parents=117 D=116 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [115] lean.shift_branch_three_generator_continuation_certificate :: D=117 parents=118 D=117 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [116] lean.shift_branch_three_generator_continuation_certificate :: D=118 parents=119 D=118 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [117] lean.shift_branch_three_generator_continuation_certificate :: D=119 parents=120 D=119 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [118] lean.shift_branch_three_generator_continuation_certificate :: D=120 parents=121 D=120 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [119] lean.shift_branch_three_generator_continuation_certificate :: D=121 parents=122 D=121 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [120] lean.shift_branch_three_generator_continuation_certificate :: D=122 parents=123 D=122 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [121] lean.shift_branch_three_generator_continuation_certificate :: D=123 parents=124 D=123 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [122] lean.shift_branch_three_generator_continuation_certificate :: D=124 parents=125 D=124 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [123] lean.shift_branch_three_generator_continuation_certificate :: D=125 parents=126 D=125 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [124] lean.shift_branch_three_generator_continuation_certificate :: D=126 parents=127 D=126 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [125] lean.shift_branch_three_generator_continuation_certificate :: D=127 parents=128 D=127 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [126] lean.shift_branch_three_generator_continuation_certificate :: D=128 parents=129 D=128 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [127] lean.shift_branch_three_generator_continuation_certificate :: D=129 parents=130 D=129 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [128] lean.shift_branch_three_generator_continuation_certificate :: D=130 parents=131 D=130 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [129] lean.shift_branch_three_generator_continuation_certificate :: D=131 parents=132 D=131 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [130] lean.shift_branch_three_generator_continuation_certificate :: D=132 parents=133 D=132 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [131] lean.shift_branch_three_generator_continuation_certificate :: D=133 parents=134 D=133 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [132] lean.shift_branch_three_generator_continuation_certificate :: D=134 parents=135 D=134 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [133] lean.shift_branch_three_generator_continuation_certificate :: D=135 parents=136 D=135 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [134] lean.shift_branch_three_generator_continuation_certificate :: D=136 parents=137 D=136 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [135] lean.shift_branch_three_generator_continuation_certificate :: D=137 parents=138 D=137 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [136] lean.shift_branch_three_generator_continuation_certificate :: D=138 parents=139 D=138 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [137] lean.shift_branch_three_generator_continuation_certificate :: D=139 parents=140 D=139 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [138] lean.shift_branch_three_generator_continuation_certificate :: D=140 parents=141 D=140 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [139] lean.shift_branch_three_generator_continuation_certificate :: D=141 parents=142 D=141 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [140] lean.shift_branch_three_generator_continuation_certificate :: D=142 parents=143 D=142 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [141] lean.shift_branch_three_generator_continuation_certificate :: D=143 parents=144 D=143 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [142] lean.shift_branch_three_generator_continuation_certificate :: D=144 parents=145 D=144 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [143] lean.shift_branch_three_generator_continuation_certificate :: D=145 parents=146 D=145 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [144] lean.shift_branch_three_generator_continuation_certificate :: D=146 parents=147 D=146 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [145] lean.shift_branch_three_generator_continuation_certificate :: D=147 parents=148 D=147 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [146] lean.shift_branch_three_generator_continuation_certificate :: D=148 parents=149 D=148 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [147] lean.shift_branch_three_generator_continuation_certificate :: D=149 parents=150 D=149 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [148] lean.shift_branch_three_generator_continuation_certificate :: D=150 parents=151 D=150 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [149] lean.shift_branch_three_generator_continuation_certificate :: D=151 parents=152 D=151 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [150] lean.shift_branch_three_generator_continuation_certificate :: D=152 parents=153 D=152 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [151] lean.shift_branch_three_generator_continuation_certificate :: D=153 parents=154 D=153 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [152] lean.shift_branch_three_generator_continuation_certificate :: D=154 parents=155 D=154 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [153] lean.shift_branch_three_generator_continuation_certificate :: D=155 parents=156 D=155 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [154] lean.shift_branch_three_generator_continuation_certificate :: D=156 parents=157 D=156 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [155] lean.shift_branch_three_generator_continuation_certificate :: D=157 parents=158 D=157 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [156] lean.shift_branch_three_generator_continuation_certificate :: D=158 parents=159 D=158 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [157] lean.shift_branch_three_generator_continuation_certificate :: D=159 parents=160 D=159 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [158] lean.shift_branch_three_generator_continuation_certificate :: D=160 parents=161 D=160 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [159] lean.shift_branch_three_generator_continuation_certificate :: D=161 parents=162 D=161 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [160] lean.shift_branch_three_generator_continuation_certificate :: D=162 parents=163 D=162 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [161] lean.shift_branch_three_generator_continuation_certificate :: D=163 parents=164 D=163 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [162] lean.shift_branch_three_generator_continuation_certificate :: D=164 parents=165 D=164 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [163] lean.shift_branch_three_generator_continuation_certificate :: D=165 parents=166 D=165 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [164] lean.shift_branch_three_generator_continuation_certificate :: D=166 parents=167 D=166 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [165] lean.shift_branch_three_generator_continuation_certificate :: D=167 parents=168 D=167 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [166] lean.shift_branch_three_generator_continuation_certificate :: D=168 parents=169 D=168 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [167] lean.shift_branch_three_generator_continuation_certificate :: D=169 parents=170 D=169 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [168] lean.shift_branch_three_generator_continuation_certificate :: D=170 parents=171 D=170 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [169] lean.shift_branch_three_generator_continuation_certificate :: D=171 parents=172 D=171 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [170] lean.shift_branch_three_generator_continuation_certificate :: D=172 parents=173 D=172 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [171] lean.shift_branch_three_generator_continuation_certificate :: D=173 parents=174 D=173 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [172] lean.shift_branch_three_generator_continuation_certificate :: D=174 parents=175 D=174 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [173] lean.shift_branch_three_generator_continuation_certificate :: D=175 parents=176 D=175 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [174] lean.shift_branch_three_generator_continuation_certificate :: D=176 parents=177 D=176 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [175] lean.shift_branch_three_generator_continuation_certificate :: D=177 parents=178 D=177 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [176] lean.shift_branch_three_generator_continuation_certificate :: D=178 parents=179 D=178 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [177] lean.shift_branch_three_generator_continuation_certificate :: D=179 parents=180 D=179 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [178] lean.shift_branch_three_generator_continuation_certificate :: D=180 parents=181 D=180 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [179] lean.shift_branch_three_generator_continuation_certificate :: D=181 parents=182 D=181 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [180] lean.shift_branch_three_generator_continuation_certificate :: D=182 parents=183 D=182 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [181] lean.shift_branch_three_generator_continuation_certificate :: D=183 parents=184 D=183 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [182] lean.shift_branch_three_generator_continuation_certificate :: D=184 parents=185 D=184 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [183] lean.shift_branch_three_generator_continuation_certificate :: D=185 parents=186 D=185 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [184] lean.shift_branch_three_generator_continuation_certificate :: D=186 parents=187 D=186 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [185] lean.shift_branch_three_generator_continuation_certificate :: D=187 parents=188 D=187 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [186] lean.shift_branch_three_generator_continuation_certificate :: D=188 parents=189 D=188 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [187] lean.shift_branch_three_generator_continuation_certificate :: D=189 parents=190 D=189 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [188] lean.shift_branch_three_generator_continuation_certificate :: D=190 parents=191 D=190 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [189] lean.shift_branch_three_generator_continuation_certificate :: D=191 parents=192 D=191 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [190] lean.shift_branch_three_generator_continuation_certificate :: D=192 parents=193 D=192 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [191] lean.shift_branch_three_generator_continuation_certificate :: D=193 parents=194 D=193 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [192] lean.shift_branch_three_generator_continuation_certificate :: D=194 parents=195 D=194 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [193] lean.shift_branch_three_generator_continuation_certificate :: D=195 parents=196 D=195 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [194] lean.shift_branch_three_generator_continuation_certificate :: D=196 parents=197 D=196 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [195] lean.shift_branch_three_generator_continuation_certificate :: D=197 parents=198 D=197 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [196] lean.shift_branch_three_generator_continuation_certificate :: D=198 parents=199 D=198 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [197] lean.shift_branch_three_generator_continuation_certificate :: D=199 parents=200 D=199 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [198] lean.shift_branch_three_generator_continuation_certificate :: D=200 parents=201 D=200 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [199] lean.shift_branch_three_generator_continuation_certificate :: D=201 parents=202 D=201 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [200] lean.shift_branch_three_generator_continuation_certificate :: D=202 parents=203 D=202 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [201] lean.shift_branch_three_generator_continuation_certificate :: D=203 parents=204 D=203 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [202] lean.shift_branch_three_generator_continuation_certificate :: D=204 parents=205 D=204 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [203] lean.shift_branch_three_generator_continuation_certificate :: D=205 parents=206 D=205 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [204] lean.shift_branch_three_generator_continuation_certificate :: D=206 parents=207 D=206 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [205] lean.shift_branch_three_generator_continuation_certificate :: D=207 parents=208 D=207 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [206] lean.shift_branch_three_generator_continuation_certificate :: D=208 parents=209 D=208 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [207] lean.shift_branch_three_generator_continuation_certificate :: D=209 parents=210 D=209 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [208] lean.shift_branch_three_generator_continuation_certificate :: D=210 parents=211 D=210 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [209] lean.shift_branch_three_generator_continuation_certificate :: D=211 parents=212 D=211 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [210] lean.shift_branch_three_generator_continuation_certificate :: D=212 parents=213 D=212 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [211] lean.shift_branch_three_generator_continuation_certificate :: D=213 parents=214 D=213 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [212] lean.shift_branch_three_generator_continuation_certificate :: D=214 parents=215 D=214 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [213] lean.shift_branch_three_generator_continuation_certificate :: D=215 parents=216 D=215 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [214] lean.shift_branch_three_generator_continuation_certificate :: D=216 parents=217 D=216 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [215] lean.shift_branch_three_generator_continuation_certificate :: D=217 parents=218 D=217 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [216] lean.shift_branch_three_generator_continuation_certificate :: D=218 parents=219 D=218 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [217] lean.shift_branch_three_generator_continuation_certificate :: D=219 parents=220 D=219 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [218] lean.shift_branch_three_generator_continuation_certificate :: D=220 parents=221 D=220 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [219] lean.shift_branch_three_generator_continuation_certificate :: D=221 parents=222 D=221 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [220] lean.shift_branch_three_generator_continuation_certificate :: D=222 parents=223 D=222 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [221] lean.shift_branch_three_generator_continuation_certificate :: D=223 parents=224 D=223 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [222] lean.shift_branch_three_generator_continuation_certificate :: D=224 parents=225 D=224 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [223] lean.shift_branch_three_generator_continuation_certificate :: D=225 parents=226 D=225 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [224] lean.shift_branch_three_generator_continuation_certificate :: D=226 parents=227 D=226 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [225] lean.shift_branch_three_generator_continuation_certificate :: D=227 parents=228 D=227 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [226] lean.shift_branch_three_generator_continuation_certificate :: D=228 parents=229 D=228 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [227] lean.shift_branch_three_generator_continuation_certificate :: D=229 parents=230 D=229 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [228] lean.shift_branch_three_generator_continuation_certificate :: D=230 parents=231 D=230 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [229] lean.shift_branch_three_generator_continuation_certificate :: D=231 parents=232 D=231 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [230] lean.shift_branch_three_generator_continuation_certificate :: D=232 parents=233 D=232 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [231] lean.shift_branch_three_generator_continuation_certificate :: D=233 parents=234 D=233 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [232] lean.shift_branch_three_generator_continuation_certificate :: D=234 parents=235 D=234 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [233] lean.shift_branch_three_generator_continuation_certificate :: D=235 parents=236 D=235 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [234] lean.shift_branch_three_generator_continuation_certificate :: D=236 parents=237 D=236 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [235] lean.shift_branch_three_generator_continuation_certificate :: D=237 parents=238 D=237 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [236] lean.shift_branch_three_generator_continuation_certificate :: D=238 parents=239 D=238 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [237] lean.shift_branch_three_generator_continuation_certificate :: D=239 parents=240 D=239 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [238] lean.shift_branch_three_generator_continuation_certificate :: D=240 parents=241 D=240 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [239] lean.shift_branch_three_generator_continuation_certificate :: D=241 parents=242 D=241 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [240] lean.shift_branch_three_generator_continuation_certificate :: D=242 parents=243 D=242 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [241] lean.shift_branch_three_generator_continuation_certificate :: D=243 parents=244 D=243 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [242] lean.shift_branch_three_generator_continuation_certificate :: D=244 parents=245 D=244 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [243] lean.shift_branch_three_generator_continuation_certificate :: D=245 parents=246 D=245 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [244] lean.shift_branch_three_generator_continuation_certificate :: D=246 parents=247 D=246 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [245] lean.shift_branch_three_generator_continuation_certificate :: D=247 parents=248 D=247 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [246] lean.shift_branch_three_generator_continuation_certificate :: D=248 parents=249 D=248 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [247] lean.shift_branch_three_generator_continuation_certificate :: D=249 parents=250 D=249 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [248] lean.shift_branch_three_generator_continuation_certificate :: D=250 parents=251 D=250 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [249] lean.shift_branch_three_generator_continuation_certificate :: D=251 parents=252 D=251 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [250] lean.shift_branch_three_generator_continuation_certificate :: D=252 parents=253 D=252 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [251] lean.shift_branch_three_generator_continuation_certificate :: D=253 parents=254 D=253 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [252] lean.shift_branch_three_generator_continuation_certificate :: D=254 parents=255 D=254 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [253] lean.shift_branch_three_generator_continuation_certificate :: D=255 parents=256 D=255 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
  [254] lean.shift_branch_three_generator_continuation_certificate :: D=256 parents=257 D=256 shift-branch three-generator continuation instance -- instantiates three_generator_word_induction
-/

def reflectedNodeCount : Nat := 255

end RavelGenerated
