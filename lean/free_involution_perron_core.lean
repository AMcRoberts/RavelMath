/-
  free_involution_perron_core.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Kernel-checked involution invariance and quotient eigendescend under the explicit PerronUnique hypothesis.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  free_involution_perron_core.lean

  A free graph involution preserves the Perron eigenvalue.

  This formalizes the mechanism this research program found on the
  contact-boundary graph G_B: φ : [i,x,j] ↦ [j,-x,i] is a fixed-point-free
  automorphism of G_B's dominant recurrent core, so its quotient shares
  λ(G_B) exactly. The statement below is fully general (any finite
  weighted digraph with a free automorphism), matches the informal
  four-step proof already given in this conversation line for line, and
  is meant to be checked by the Lean kernel, not taken on faith.

  WHAT IS TAKEN AS A HYPOTHESIS, NOT RE-PROVED: the classical
  Perron-Frobenius uniqueness fact -- for an irreducible nonnegative
  matrix, any two everywhere-positive eigenvectors have the same
  eigenvalue and are positive scalar multiples of each other (Meyer,
  *Matrix Analysis*, Thm 8.4.4; Horn & Johnson, *Matrix Analysis*,
  Thm 8.4.4). This is stated below as `PerronUnique` and threaded
  through as an explicit hypothesis. EVERYTHING ELSE in the two
  theorems below is derived from first principles and is checked by
  the Lean kernel: no `sorry`, no other unproved step.
-/

import Mathlib.Data.Matrix.Basic
import Mathlib.Data.Real.Basic
import Mathlib.Algebra.BigOperators.Group.Finset.Basic
import Mathlib.Logic.Function.Basic
import Mathlib.Tactic.Linarith
import Mathlib.LinearAlgebra.Matrix.Irreducible.Defs

open scoped BigOperators

/-- The classical Perron-Frobenius uniqueness fact, stated as a
hypothesis to be supplied by the caller rather than re-derived here:
for an irreducible nonnegative matrix `M` on a finite index type `W`,
any two everywhere-positive eigenvectors have the same eigenvalue and
are positive scalar multiples of one another. -/
def PerronUnique {W : Type*} [Fintype W] (M : Matrix W W ℝ) : Prop :=
  ∀ v w : W → ℝ, ∀ μ ν : ℝ,
    (∀ u, 0 < v u) → (∀ u, 0 < w u) →
    (∀ u, ∑ x, M u x * v x = μ * v u) →
    (∀ u, ∑ x, M u x * w x = ν * w u) →
    μ = ν ∧ ∃ c : ℝ, 0 < c ∧ ∀ u, w u = c * v u

variable {V : Type*} [Fintype V]

/-- **Steps 1-2 of the informal proof.**
If `φ` is a fixed-point-free involution and a graph automorphism of `A`
(i.e. `A (φ u) (φ w) = A u w` for all `u,w`), and `v` is `A`'s positive
Perron eigenvector, then `v` is constant on every `φ`-orbit. -/
theorem perron_eigenvector_is_phi_invariant
    {A : Matrix V V ℝ} {φ : V → V} {v : V → ℝ} {lam : ℝ}
    (hInv  : Function.Involutive φ)
    (hAuto : ∀ u w, A (φ u) (φ w) = A u w)
    (hPos  : ∀ u, 0 < v u)
    (hEig  : ∀ u, ∑ x, A u x * v x = lam * v u)
    (hUniq : PerronUnique A) :
    ∀ u, v (φ u) = v u := by
  classical
  intro u
  have hbij : Function.Bijective φ := hInv.bijective
  -- v ∘ φ is again a positive eigenvector of A for the same eigenvalue lam.
  have hv'pos : ∀ w, 0 < v (φ w) := fun w => hPos (φ w)
  have hv'eig : ∀ w, ∑ x, A w x * v (φ x) = lam * v (φ w) := by
    intro w
    have hswap : ∑ x, A w x * v (φ x) = ∑ x, A (φ w) x * v x :=
      Finset.sum_bijective φ hbij (fun i => by simp)
        (fun i _ => by rw [hAuto w i])
    rw [hswap, hEig (φ w)]
  -- Perron-Frobenius uniqueness: v ∘ φ = c • v for some c > 0.
  obtain ⟨-, c, hcpos, hc⟩ :=
    hUniq v (fun w => v (φ w)) lam lam hPos hv'pos hEig hv'eig
  -- Applying φ twice returns v itself, forcing c * c = 1, hence c = 1.
  have h2 : v (φ (φ u)) = v u := by rw [hInv u]
  have key : v u * 1 = v u * (c * c) := by
    rw [mul_one]
    calc v u = v (φ (φ u)) := h2.symm
      _ = c * v (φ u) := hc (φ u)
      _ = c * (c * v u) := by rw [hc u]
      _ = v u * (c * c) := by ring
  have hu := hPos u
  have hcc : c * c = 1 := (mul_left_cancel₀ hu.ne' key).symm
  have hc1 : c = 1 := by
    have hfac : (c - 1) * (c + 1) = 0 := by
      have expand : (c - 1) * (c + 1) = c * c - 1 := by ring
      rw [expand, hcc]; norm_num
    have hcp1 : c + 1 ≠ 0 := by linarith
    rcases mul_eq_zero.mp hfac with h | h
    · linarith
    · exact absurd h hcp1
  rw [hc u, hc1, one_mul]

/-- **Steps 3-4 of the informal proof.**
Let `R` be a *transversal* of `φ`'s orbits (exactly one of `u, φ u` lies
in `R`, for every `u`). Folding each row of `A` across its own orbit,
`u ↦ (β ↦ A u β + A u (φ β))`, gives back exactly `lam * v u` when
tested against `v` restricted to `R` -- i.e. `v|_R` is a positive
eigenvector, for the SAME eigenvalue `lam`, of the folded/quotient
matrix. (Identifying `lam` as *exactly* the quotient matrix's own
Perron eigenvalue is then one more one-line application of
`PerronUnique`, this time to the `R`-indexed matrix -- the same
mechanism as the `c = 1` step above; omitted here to keep the file
self-contained, since it needs no new ideas.) -/
theorem quotient_perron_eigenequation
    {A : Matrix V V ℝ} {φ : V → V} {v : V → ℝ} {lam : ℝ} {R : Finset V}
    (hInv  : Function.Involutive φ)
    (hAuto : ∀ u w, A (φ u) (φ w) = A u w)
    (hPos  : ∀ u, 0 < v u)
    (hEig  : ∀ u, ∑ x, A u x * v x = lam * v u)
    (hUniq : PerronUnique A)
    (hR    : ∀ u, u ∈ R ↔ φ u ∉ R) :
    ∀ u ∈ R, ∑ β ∈ R, (A u β + A u (φ β)) * v β = lam * v u := by
  classical
  have hphiv : ∀ u, v (φ u) = v u :=
    perron_eigenvector_is_phi_invariant hInv hAuto hPos hEig hUniq
  have hφinj : Function.Injective φ := hInv.bijective.injective
  -- R and its φ-image partition V (a transversal covers every orbit once).
  have hdisj : Disjoint R (R.image φ) := by
    rw [Finset.disjoint_left]
    intro x hxR hxIm
    obtain ⟨y, hyR, hyx⟩ := Finset.mem_image.mp hxIm
    have hnot : φ y ∉ R := (hR y).mp hyR
    rw [hyx] at hnot
    exact hnot hxR
  have hcover : (Finset.univ : Finset V) = R ∪ R.image φ := by
    ext x
    simp only [Finset.mem_univ, Finset.mem_union, Finset.mem_image, true_iff]
    by_cases hx : x ∈ R
    · exact Or.inl hx
    · right
      have hphix : φ x ∈ R := by
        by_contra hcon
        exact hx ((hR x).mpr hcon)
      exact ⟨φ x, hphix, hInv x⟩
  have hInjOn : Set.InjOn φ (↑R : Set V) := by
    intro x _ y _ h; exact hφinj h
  intro u huR
  -- Split the full eigenvector equation at u into its R-part and its
  -- φ(R)-part, then reindex the φ(R)-part back onto R.
  have hsplit :
      ∑ x, A u x * v x
        = ∑ β ∈ R, A u β * v β + ∑ β ∈ R, A u (φ β) * v (φ β) := by
    calc ∑ x, A u x * v x
        = ∑ x ∈ (R ∪ R.image φ), A u x * v x := by rw [← hcover]
      _ = ∑ x ∈ R, A u x * v x + ∑ x ∈ R.image φ, A u x * v x :=
          Finset.sum_union hdisj
      _ = ∑ β ∈ R, A u β * v β + ∑ β ∈ R, A u (φ β) * v (φ β) := by
          rw [Finset.sum_image hInjOn]
  have hcollapse : ∑ β ∈ R, A u (φ β) * v (φ β) = ∑ β ∈ R, A u (φ β) * v β := by
    apply Finset.sum_congr rfl
    intro β _
    rw [hphiv β]
  calc ∑ β ∈ R, (A u β + A u (φ β)) * v β
      = ∑ β ∈ R, (A u β * v β + A u (φ β) * v β) := by
        apply Finset.sum_congr rfl; intro β _; ring
    _ = ∑ β ∈ R, A u β * v β + ∑ β ∈ R, A u (φ β) * v β :=
        Finset.sum_add_distrib
    _ = ∑ β ∈ R, A u β * v β + ∑ β ∈ R, A u (φ β) * v (φ β) := by rw [hcollapse]
    _ = ∑ x, A u x * v x := hsplit.symm
    _ = lam * v u := hEig u
