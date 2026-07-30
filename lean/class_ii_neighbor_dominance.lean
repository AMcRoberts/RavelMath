/-
  class_ii_neighbor_dominance.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Universal recurrent-SCC dominance and exact polynomial identities for the Class-II neighbor family.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  Candidate-shell dominance for the three fixed-light Class-II neighbors.

  This file proves only the polynomial sign comparisons.  It does not
  claim that the candidate SCC catalogues exhaust the complete contact
  boundary graph; that remains the combinatorial theorem target.
-/

import Mathlib.Tactic

def neighbor0CoreFactor (a x : ℤ) : ℤ :=
  x^8 - (a^2 + 2) * x^6 + (2*a^2 + 3*a + 2) * x^4
    - (a^2 + 5*a + 3) * x^2 + 2*a + 2

def neighbor1CoreFactor (a x : ℤ) : ℤ :=
  x^7 - (a^2 + 1) * x^5 + a^2 * x^3 - (a + 2) * x^2 + a

def neighbor2CoreFactor (a x : ℤ) : ℤ :=
  x^14 - (a^2 - 1)*x^12 - x^11 - (a^2+a+1)*x^10
    - (2*a^2+2*a-1)*x^9 - 3*a*x^8 - (5*a^2+2)*x^7
    - (2*a^2-3*a+8)*x^6 + (2*a^2-3*a+4)*x^5
    + (4*a^2-2*a)*x^4 - (3*a^2+6*a-5)*x^3
    - (8*a^2-3*a+5)*x^2 + (2*a^2-2)*x
    + 2*a^2+2*a-4

theorem neighbor0_eval_pred (a : ℤ) :
    neighbor0CoreFactor a (a-1)
      = a*(a-2)*(-2*a^5 + 9*a^4 - 11*a^3 + 4*a^2 - 3*a - 1) := by
  simp [neighbor0CoreFactor]
  ring

theorem neighbor1_eval_self (a : ℤ) :
    neighbor1CoreFactor a a = -a^3 - 2*a^2 + a := by
  simp [neighbor1CoreFactor]
  ring

theorem neighbor2_eval_self (a : ℤ) :
    neighbor2CoreFactor a a =
      -4*a^11 - 3*a^10 - 7*a^9 - 2*a^8 + 3*a^7 - 7*a^6
      - a^5 - 14*a^4 + 10*a^3 - 3*a^2 - 4 := by
  simp [neighbor2CoreFactor]
  ring

theorem neighbor0_core_negative_at_pred
    (a : ℤ) (ha : 3 ≤ a) :
    neighbor0CoreFactor a (a-1) < 0 := by
  rw [neighbor0_eval_pred]
  have ha0 : 0 < a := by omega
  have ha2 : 0 < a - 2 := by omega
  have hinner :
      -2*a^5 + 9*a^4 - 11*a^3 + 4*a^2 - 3*a - 1 < 0 := by
    by_cases h5 : 5 ≤ a
    · have hp4 : 0 < a^4 := pow_pos ha0 4
      have hp2 : 0 < a^2 := pow_pos ha0 2
      have hc1 : 9 - 2*a ≤ -1 := by omega
      have hc2 : 4 - 11*a < 0 := by omega
      have ht1 : a^4 * (9 - 2*a) ≤ a^4 * (-1) :=
        mul_le_mul_of_nonneg_left hc1 (le_of_lt hp4)
      have ht2 : a^2 * (4 - 11*a) < 0 :=
        mul_neg_of_pos_of_neg hp2 hc2
      nlinarith
    · have ha4 : a ≤ 4 := by omega
      interval_cases a <;> norm_num
  exact mul_neg_of_pos_of_neg (mul_pos ha0 ha2) hinner

theorem neighbor1_core_negative_at_self
    (a : ℤ) (ha : 3 ≤ a) :
    neighbor1CoreFactor a a < 0 := by
  rw [neighbor1_eval_self]
  have ha0 : 0 < a := by omega
  have hsquare : 0 < a^2 := pow_pos ha0 2
  nlinarith [mul_pos ha0 hsquare]

theorem neighbor2_core_negative_at_self
    (a : ℤ) (ha : 3 ≤ a) :
    neighbor2CoreFactor a a < 0 := by
  rw [neighbor2_eval_self]
  have ha0 : 0 < a := by omega
  have hp3 : 0 < a^3 := pow_pos ha0 3
  have hp7 : 0 < a^7 := pow_pos ha0 7
  have hp5 : 1 ≤ a^5 := by
    have : 0 < a^5 := pow_pos ha0 5
    omega
  have hp2 : 9 ≤ a^2 := by
    nlinarith [sq_nonneg (a-3)]
  have hp7lower : 9 ≤ a^7 := by
    have hmul := mul_le_mul hp2 hp5 (by norm_num) (by nlinarith)
    nlinarith [hmul]
  have hc1 : 3 - 2*a < 0 := by omega
  have hc2 : 10 - 3*a^7 < 0 := by nlinarith
  have ht1 : a^7 * (3 - 2*a) < 0 :=
    mul_neg_of_pos_of_neg hp7 hc1
  have ht2 : a^3 * (10 - 3*a^7) < 0 :=
    mul_neg_of_pos_of_neg hp3 hc2
  nlinarith [pow_pos ha0 11, pow_pos ha0 9, pow_pos ha0 6,
    pow_pos ha0 5, pow_pos ha0 4, pow_pos ha0 2]

theorem neighbor0_shell_below_pred
    (a t lambda : ℝ)
    (_ha : 3 ≤ a) (ht : t ≤ a-3) (hlambda : 0 < lambda)
    (heq : lambda + 1/lambda = t+2) :
    lambda < a-1 := by
  have hinv : 0 < 1/lambda := one_div_pos.mpr hlambda
  nlinarith

theorem neighbor1_shell_below_pred
    (a t lambda : ℝ)
    (ha : 3 ≤ a) (ht0 : 0 ≤ t) (ht : t ≤ a-2)
    (_hlambda : 0 ≤ lambda)
    (heq : lambda^2 = t*(t+2)) :
    lambda < a-1 := by
  nlinarith [sq_nonneg (a-1-lambda), mul_nonneg ht0 (by nlinarith)]

theorem neighbor2_shell_below_self
    (a k lambda : ℝ)
    (_ha : 3 ≤ a) (hk : k ≤ a-1) (heq : lambda = k) :
    lambda < a := by
  nlinarith
