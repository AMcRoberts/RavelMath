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

/-- Class II's first two left-eigenvector coordinate equations have column
difference `(0,1,0)`, hence give `b*(b-c)=c` directly. -/
theorem classII_column_difference
    (a beta b c : ℝ) (hbeta : b = beta)
    (h0 : a * b + c + 1 = beta * b)
    (h1 : a * b + 1 = beta * c) :
    b * (b - c) = c := by
  subst b
  nlinarith [h0, h1]

/-- Tribonacci's first two left-eigenvector coordinate equations have column
difference `(0,1,-1)`, hence give `b*(b-c)=c-1` directly. -/
theorem tribonacci_column_difference
    (beta b c : ℝ) (hbeta : b = beta)
    (h0 : b + c = beta * b)
    (h1 : b + 1 = beta * c) :
    b * (b - c) = c - 1 := by
  subst b
  nlinarith [h0, h1]
