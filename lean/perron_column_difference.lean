/-
  perron_column_difference.lean

  Author: Ravel.
  Date:   2026-08-02.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at ~/claude/RavelMathPub/).
  Aim:    Kernel-check the dimension-independent identity behind the
          differing Class-II/Tribonacci formulas: subtracting two
          coordinates of M^T v = beta v is exactly subtracting the
          corresponding incidence columns.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

import Mathlib.Tactic

/-- If `v` is a left eigenvector of `M` with eigenvalue `beta`, then the
scaled difference of any two coordinates is the same difference of the
corresponding columns of `M`, evaluated against `v`.  No positivity,
primitivity, Pisot, or unimodularity hypothesis is needed. -/
theorem perron_column_difference
    {n : ℕ} (M : Fin n → Fin n → ℝ) (v : Fin n → ℝ) (beta : ℝ)
    (hleft : ∀ i, ∑ k, M k i * v k = beta * v i)
    (i j : Fin n) :
    beta * (v i - v j) = ∑ k, (M k i - M k j) * v k := by
  rw [mul_sub, ← hleft i, ← hleft j, ← Finset.sum_sub_distrib]
  apply Finset.sum_congr rfl
  intro k _
  ring

/-- The three-coordinate specialization used by the executable probe.  If
the first Perron coordinate is `beta`, then `beta*(b-c)` is read directly
from incidence-column difference `(d0,d1,d2)`. -/
theorem perron_three_coordinate_identity
    (beta b c d0 d1 d2 : ℝ)
    (hbeta : b = beta)
    (hcolumn : beta * b - beta * c = d0 * b + d1 * c + d2) :
    b * (b - c) = d0 * b + d1 * c + d2 := by
  subst b
  nlinarith [hcolumn]

/-- Class II's column difference `(0,1,0)` gives `b*(b-c)=c`. -/
theorem classII_column_difference
    (b c : ℝ) (h : b * b - b * c = 0 * b + 1 * c + 0) :
    b * (b - c) = c := by
  nlinarith [h]

/-- Tribonacci's column difference `(0,1,-1)` gives `b*(b-c)=c-1`. -/
theorem tribonacci_column_difference
    (b c : ℝ) (h : b * b - b * c = 0 * b + 1 * c - 1) :
    b * (b - c) = c - 1 := by
  nlinarith [h]
