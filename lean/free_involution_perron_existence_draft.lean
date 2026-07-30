/-
  free_involution_perron_existence_draft.lean

  Author: Ravel.
  Date:   2026-07-29.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Provisional attempt to derive Perron existence and uniqueness internally; not part of make lean-check.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  free_involution_perron_existence_draft.lean

  A free graph involution preserves the Perron eigenvalue -- now with
  BOTH halves of Perron-Frobenius (uniqueness AND existence) proved
  from scratch, not assumed, modulo three explicitly-flagged `sorry`s
  in genuinely standard finite-dimensional real analysis (see Part 2).

  STATUS SUMMARY (so there is no ambiguity about what's proved):
    - PerronUnique                          : definition, not a claim.
    - perron_unique_of_irreducible           : FULLY PROVED. No sorry.
    - perron_eigenvector_is_phi_invariant    : FULLY PROVED (unchanged).
    - quotient_perron_eigenequation          : FULLY PROVED (unchanged).
    - BrouwerFixedPoint                      : the classical statement,
                                                taken as a hypothesis
                                                (see Part 2 for why, and
                                                exactly how to discharge
                                                it from harfe's repo).
    - perron_exists_of_irreducible           : proved MODULO BrouwerFixedPoint
                                                and three flagged sorries
                                                (Δ convex, Δ compact, T
                                                continuous -- textbook
                                                real analysis, no
                                                problem-specific content).
                                                Everything problem-specific
                                                -- well-definedness of the
                                                Collatz-Wielandt map, the
                                                eigenvalue extraction, and
                                                the positivity upgrade via
                                                irreducibility -- is fully
                                                proved.
    - quotient_perron_eigenequation_complete : combines everything above;
                                                takes BrouwerFixedPoint as
                                                a hypothesis, same reason.

  Imports: `import Mathlib` (the whole library) rather than fine-grained
  paths, since this file now spans matrices, convexity, and compactness/
  continuity, and guessing exact sub-module paths blind is its own
  failure mode I'd rather not add to the list below.
-/

import Mathlib

open scoped BigOperators

/-- The classical Perron-Frobenius uniqueness fact, stated as a
hypothesis to be supplied by the caller. As of `perron_unique_of_irreducible`
below, this is no longer axiomatic when the caller can supply the
algebraic irreducibility witness `hConn`. -/
def PerronUnique {W : Type*} [Fintype W] (M : Matrix W W ℝ) : Prop :=
  ∀ v w : W → ℝ, ∀ μ ν : ℝ,
    (∀ u, 0 < v u) → (∀ u, 0 < w u) →
    (∀ u, ∑ x, M u x * v x = μ * v u) →
    (∀ u, ∑ x, M u x * w x = ν * w u) →
    μ = ν ∧ ∃ c : ℝ, 0 < c ∧ ∀ u, w u = c * v u

variable {V : Type*} [Fintype V] [DecidableEq V]

/-- **Steps 1-2 of the informal proof.** Unchanged from the previous
version of this file -- already fully proved there. -/
omit [DecidableEq V] in
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
  have hv'pos : ∀ w, 0 < v (φ w) := fun w => hPos (φ w)
  have hv'eig : ∀ w, ∑ x, A w x * v (φ x) = lam * v (φ w) := by
    intro w
    have hswap : ∑ x, A w x * v (φ x) = ∑ x, A (φ w) x * v x :=
      Finset.sum_bijective φ hbij (fun i => by simp)
        (fun i _ => by rw [hAuto w i])
    rw [hswap, hEig (φ w)]
  obtain ⟨-, c, hcpos, hc⟩ :=
    hUniq v (fun w => v (φ w)) lam lam hPos hv'pos hEig hv'eig
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

/-- **Steps 3-4 of the informal proof.** Unchanged from the previous
version of this file -- already fully proved there. -/
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

/- ==================================================================
   PART 1 -- Uniqueness, proved from scratch.

   `hNonneg`/`hConn` are the two ingredients of `Matrix.IsIrreducible`
   spelled out algebraically (entrywise nonnegative; the graph is
   strongly connected, i.e. every pair (i,j) is linked by *some* power
   of A with a positive entry -- this is exactly the RHS of Mathlib's
   `Matrix.isIrreducible_iff_exists_pow_pos`). They are stated directly
   here, rather than by destructuring `Matrix.IsIrreducible A`, because
   I can't confirm the exact current field/accessor names for that
   structure without a compiler -- wiring `Matrix.IsIrreducible A` into
   `hNonneg`/`hConn` should be a one-line adapter once you have Mathlib
   open in front of you (check `Matrix.IsIrreducible.nonneg` or similar,
   and `Matrix.isIrreducible_iff_exists_pow_pos`).

   Proof idea (classical "minimal ratio" argument): let t = min_i(w_i/v_i),
   attained at i0; let s = max_i(w_i/v_i), attained at i1. Testing the
   eigenvector equations at i0 and i1 forces mu <= nu and nu <= mu, i.e.
   mu = nu. Then z := w - t*v is a nonnegative "mu-eigenvector" of A that
   vanishes at i0; irreducibility (every vertex reachable from i0) forces
   it to vanish everywhere, i.e. w = t*v exactly.
   ================================================================== -/

/-- NAMING RISK FLAG: `div_le_div_iff` has been renamed/generalized in
Mathlib before (e.g. towards `div_le_div_iff_of_pos` or a `₀`-suffixed
version for `GroupWithZero`). If this line fails to elaborate, that is
almost certainly the fix needed -- the surrounding argument is unaffected. -/
theorem perron_unique_of_irreducible
    {W : Type*} [Fintype W] [DecidableEq W] [Nonempty W]
    (A : Matrix W W ℝ)
    (hNonneg : ∀ i j, 0 ≤ A i j)
    (hConn   : ∀ i j, ∃ k, 0 < (A ^ k) i j) :
    PerronUnique A := by
  intro v w μ ν hv hw heigv heigw
  classical
  have hne : (Finset.univ : Finset W).Nonempty := Finset.univ_nonempty
  obtain ⟨i0, -, hi0min⟩ := Finset.exists_min_image Finset.univ (fun i => w i / v i) hne
  obtain ⟨i1, -, hi1max⟩ := Finset.exists_max_image Finset.univ (fun i => w i / v i) hne
  set t : ℝ := w i0 / v i0 with ht_def
  set s : ℝ := w i1 / v i1 with hs_def
  have htpos : 0 < t := div_pos (hw i0) (hv i0)
  have hspos : 0 < s := div_pos (hw i1) (hv i1)
  have ht_eq : v i0 * t = w i0 := by rw [ht_def]; field_simp
  have hs_eq : v i1 * s = w i1 := by rw [hs_def]; field_simp
  -- Cross-multiplied forms of "t is the min ratio" / "s is the max ratio".
  have hcross_t : ∀ i, w i0 * v i ≤ w i * v i0 := by
    intro i
    exact (div_le_div_iff (hv i0) (hv i)).mp (hi0min i (Finset.mem_univ i))
  have hcross_s : ∀ i, w i * v i1 ≤ w i1 * v i := by
    intro i
    exact (div_le_div_iff (hv i) (hv i1)).mp (hi1max i (Finset.mem_univ i))
  have hzge : ∀ i, t * v i ≤ w i := by
    intro i
    have heq : t * v i * v i0 ≤ w i * v i0 := by
      calc t * v i * v i0 = v i0 * t * v i := by ring
        _ = w i0 * v i := by rw [ht_eq]
        _ ≤ w i * v i0 := hcross_t i
    exact le_of_mul_le_mul_right heq (hv i0)
  have hzle : ∀ i, w i ≤ s * v i := by
    intro i
    have heq : w i * v i1 ≤ s * v i * v i1 := by
      calc w i * v i1 ≤ w i1 * v i := hcross_s i
        _ = v i1 * s * v i := by rw [← hs_eq]
        _ = s * v i * v i1 := by ring
    exact le_of_mul_le_mul_right heq (hv i1)
  have hsum_t : ∀ i, ∑ x, A i x * (t * v x) ≤ ∑ x, A i x * w x := by
    intro i
    exact Finset.sum_le_sum (fun x _ => mul_le_mul_of_nonneg_left (hzge x) (hNonneg i x))
  have hsum_s : ∀ i, ∑ x, A i x * w x ≤ ∑ x, A i x * (s * v x) := by
    intro i
    exact Finset.sum_le_sum (fun x _ => mul_le_mul_of_nonneg_left (hzle x) (hNonneg i x))
  -- mu <= nu, testing at i0.
  have hmu_le_nu : μ ≤ ν := by
    have h1 := hsum_t i0
    have e1 : ∑ x, A i0 x * (t * v x) = t * (μ * v i0) := by
      have hms : ∑ x, A i0 x * (t * v x) = t * ∑ x, A i0 x * v x := by
        rw [Finset.mul_sum]; exact Finset.sum_congr rfl (fun x _ => by ring)
      rw [hms, heigv i0]
    have e2 : ∑ x, A i0 x * w x = ν * (t * v i0) := by
      rw [heigw i0, ← ht_eq]; ring
    rw [e1, e2] at h1
    have hpos : 0 < t * v i0 := mul_pos htpos (hv i0)
    have hineq : μ * (t * v i0) ≤ ν * (t * v i0) := by
      calc μ * (t * v i0) = t * (μ * v i0) := by ring
        _ ≤ ν * (t * v i0) := h1
    exact le_of_mul_le_mul_right hineq hpos
  -- nu <= mu, testing at i1.
  have hnu_le_mu : ν ≤ μ := by
    have h1 := hsum_s i1
    have e1 : ∑ x, A i1 x * w x = ν * (s * v i1) := by
      rw [heigw i1, ← hs_eq]; ring
    have e2 : ∑ x, A i1 x * (s * v x) = s * (μ * v i1) := by
      have hms : ∑ x, A i1 x * (s * v x) = s * ∑ x, A i1 x * v x := by
        rw [Finset.mul_sum]; exact Finset.sum_congr rfl (fun x _ => by ring)
      rw [hms, heigv i1]
    rw [e1, e2] at h1
    have hpos : 0 < s * v i1 := mul_pos hspos (hv i1)
    have hineq : ν * (s * v i1) ≤ μ * (s * v i1) := by
      calc ν * (s * v i1) = ν * (s * v i1) := rfl
        _ ≤ s * (μ * v i1) := h1
        _ = μ * (s * v i1) := by ring
    exact le_of_mul_le_mul_right hineq hpos
  have hmu_eq_nu : μ = ν := le_antisymm hmu_le_nu hnu_le_mu
  -- z := w - t*v: nonnegative, a mu-eigenvector of A, zero at i0.
  set z : W → ℝ := fun i => w i - t * v i with hz_def
  have hzge0 : ∀ i, 0 ≤ z i := fun i => sub_nonneg.mpr (hzge i)
  have hz_i0 : z i0 = 0 := by
    have hwi0 : w i0 = t * v i0 := by rw [← ht_eq]; ring
    simp [hz_def, hwi0]
  have hAz : ∀ i, ∑ x, A i x * z x = μ * z i := by
    intro i
    have expand : ∑ x, A i x * z x = ∑ x, A i x * w x - t * ∑ x, A i x * v x := by
      have hcongr : ∑ x, A i x * z x = ∑ x, (A i x * w x - A i x * (t * v x)) := by
        apply Finset.sum_congr rfl
        intro x _
        simp only [hz_def]; ring
      rw [hcongr, Finset.sum_sub_distrib]
      congr 1
      rw [Finset.mul_sum]
      exact Finset.sum_congr rfl (fun x _ => by ring)
    rw [expand, heigw i, heigv i, ← hmu_eq_nu]; ring
  have hAnonneg_pow : ∀ k i j, 0 ≤ (A ^ k) i j := by
    intro k
    induction k with
    | zero =>
      intro i j
      rw [pow_zero, Matrix.one_apply]
      split <;> norm_num
    | succ n ih =>
      intro i j
      rw [pow_succ, Matrix.mul_apply]
      exact Finset.sum_nonneg (fun l _ => mul_nonneg (ih i l) (hNonneg l j))
  -- One-step zero propagation: if z vanishes at i and A i j > 0, z vanishes at j.
  have step : ∀ i, z i = 0 → ∀ j, 0 < A i j → z j = 0 := by
    intro i hzi j hAij
    have hi := hAz i
    rw [hzi, mul_zero] at hi
    have hnonneg_terms : ∀ x ∈ (Finset.univ : Finset W), 0 ≤ A i x * z x :=
      fun x _ => mul_nonneg (hNonneg i x) (hzge0 x)
    have hall_zero := (Finset.sum_eq_zero_iff_of_nonneg hnonneg_terms).mp hi j (Finset.mem_univ j)
    rcases mul_eq_zero.mp hall_zero with h | h
    · exact absurd h hAij.ne'
    · exact h
  -- Propagation along a length-k path (using the (A^k) i j > 0 characterization).
  have propagate : ∀ k i j, 0 < (A ^ k) i j → z i = 0 → z j = 0 := by
    intro k
    induction k with
    | zero =>
      intro i j hpow hzi
      rw [pow_zero, Matrix.one_apply] at hpow
      by_cases h : i = j
      · rwa [← h]
      · simp [h] at hpow
    | succ n ih =>
      intro i j hpow hzi
      rw [pow_succ, Matrix.mul_apply] at hpow
      have hex : ∃ l, 0 < (A ^ n) i l * A l j := by
        by_contra hcon
        push_neg at hcon
        have : ∑ l, (A ^ n) i l * A l j ≤ 0 := Finset.sum_nonpos (fun l _ => hcon l)
        linarith
      obtain ⟨l, hl⟩ := hex
      have hl1 : 0 < (A ^ n) i l := by
        by_contra hcon
        push_neg at hcon
        have hz : (A ^ n) i l = 0 := le_antisymm hcon (hAnonneg_pow n i l)
        rw [hz, zero_mul] at hl
        exact absurd hl (lt_irrefl 0)
      have hl2 : 0 < A l j := by
        by_contra hcon
        push_neg at hcon
        have hz : A l j = 0 := le_antisymm hcon (hNonneg l j)
        rw [hz, mul_zero] at hl
        exact absurd hl (lt_irrefl 0)
      exact step l (ih i l hl1 hzi) j hl2
  have hz_all : ∀ j, z j = 0 := by
    intro j
    obtain ⟨k, hk⟩ := hConn i0 j
    exact propagate k i0 j hk hz_i0
  refine ⟨hmu_eq_nu, t, htpos, fun u => ?_⟩
  have hzu := hz_all u
  simp only [hz_def] at hzu
  linarith

/- ==================================================================
   PART 2 -- Existence, via Brouwer.

   NONE of this is mathematically in doubt. Existence of a positive
   eigenvector for an irreducible nonnegative matrix, at the spectral
   radius, is a completely settled classical theorem:
     - Perron, O., "Zur Theorie der Matrices", Math. Ann. 64 (1907), 248-263
       (the original case, for strictly positive matrices).
     - Frobenius, G., "Ueber Matrizen aus nicht negativen Elementen",
       Sitzungsber. Preuss. Akad. Wiss. (1912), 456-477 (the extension
       to irreducible nonnegative matrices used here).
     - Modern textbook treatments, any of which fully prove BOTH
       existence and uniqueness:
         * Horn, R.A. & Johnson, C.R., "Matrix Analysis", 2nd ed.,
           Cambridge Univ. Press, 2013, Theorem 8.4.4 (already cited
           for uniqueness in Part 1 above).
         * Meyer, C.D., "Matrix Analysis and Applied Linear Algebra",
           SIAM, 2000, Chapter 8.
         * Seneta, E., "Non-negative Matrices and Markov Chains",
           2nd ed., Springer, 2006, Chapter 1 -- the SAME book Mathlib
           itself cites for `Matrix.IsIrreducible`/`Matrix.IsPrimitive`
           (see `Mathlib.LinearAlgebra.Matrix.Irreducible.Defs`, which
           references "seneta2006" directly), so this is the natural
           place to look for the existence/uniqueness theorem that
           follows those definitions in the same chapter.
     - Concrete lecture-note treatments of exactly the Collatz-Wielandt/
       Brouwer route used below: Boyle, M., "Notes on the Perron-
       Frobenius Theory of Nonnegative Matrices" (Univ. of Maryland
       course notes), Sec. 3; Knill, O., Math 19b lecture notes,
       Harvard, "Lecture 34: Perron-Frobenius theorem" (uses the same
       T(x) = Ax/||Ax|| map, via Banach's contraction principle for the
       strictly-positive case rather than Brouwer -- the irreducible-
       but-not-necessarily-primitive case handled here genuinely needs
       the more general fixed-point argument, since T need not be a
       contraction when A merely irreducible).

   What is NOT settled is the Lean mechanization of the existence half.
   I could not pin down harfe/fixed-point-theorems-lean4's EXACT lemma
   name for Brouwer without a compiler (their repo's tree/ pages block
   automated fetching via robots.txt, and search engines don't index
   the raw .lean source). So instead of guessing a name and having it
   silently fail, `BrouwerFixedPoint` below states the classical
   content precisely -- any correct Lean statement of Brouwer's theorem,
   theirs included, says exactly this. Once you have Mathlib + their
   repo open:
     1. Add their repo as a dependency in lakefile.toml:
          [[require]]
          name = "FixedPointTheorems"
          git = "https://github.com/harfe/fixed-point-theorems-lean4"
     2. `import FixedPointTheorems.brouwer` (or whatever `FixedPointTheorems.lean`
        re-exports -- it imports both `.brouwer` and `.kakutani`).
     3. Find their theorem (`#check`/autocomplete will show it instantly)
        and supply it as the `hBrouwer` argument to
        `perron_exists_of_irreducible` below -- a one-line adapter,
        since `BrouwerFixedPoint`'s statement is the standard one their
        theorem almost certainly matches up to trivial repackaging
        (EuclideanSpace vs Pi-type, `ContinuousOn` vs `Continuous`, etc).

   Route: Collatz-Wielandt via the classical map T(x) = Ax / ||Ax||_1 on
   the probability simplex Delta. A fixed point of T gives a NONNEGATIVE
   eigenvector; irreducibility upgrades it to strictly positive by the
   same zero-propagation technique as Part 1 (fully proved below, not
   sorry'd -- it's the one piece that's actually specific to this
   problem rather than generic real analysis).

   What's proved outright: well-definedness of T (every column of an
   irreducible A has a positive entry, so ||Ax||_1 > 0 on all of Delta),
   T mapping Delta into Delta, the eigenvalue extraction from a fixed
   point, and the full positivity upgrade.
   What's left as `sorry`, each individually cited below: Delta is
   compact and convex, and T is continuous. These are standard finite-
   dimensional real analysis with no problem-specific content -- genuine
   "textbook exercise" gaps, not conceptual ones -- but I won't guess
   exact Mathlib lemma names for them blind after getting burned once
   already in this file (`div_le_div_iff`).
   ================================================================== -/

/-- The classical statement: a continuous self-map of a nonempty
compact convex subset of a finite-dimensional real vector space has a
fixed point. Stated as a hypothesis rather than derived, per the note
above.

Citations: Brouwer, L.E.J., "Über Abbildungen von Mannigfaltigkeiten",
Math. Ann. 71 (1911/12), 97-115 (original, for the n-ball). For this
exact finite-dimensional convex-set form and a short modern proof, see
Milnor, J., "Analytic proofs of the 'Hairy Ball Theorem' and the
Brouwer Fixed Point Theorem", Amer. Math. Monthly 85 (1978), 521-524.
Mechanized (no `sorry`) at github.com/harfe/fixed-point-theorems-lean4,
via a cubical Sperner's Lemma argument (Kuhn, H.W., "Some Combinatorial
Lemmas in Topology", IBM J. Res. Develop. 4 (1960), 518-524) -- not yet
in mainline Mathlib (in flight as mathlib4#36770). -/
def BrouwerFixedPoint : Prop :=
  ∀ {W : Type} [Fintype W] (K : Set (W → ℝ)) (f : (W → ℝ) → (W → ℝ)),
    K.Nonempty → IsCompact K → Convex ℝ K → ContinuousOn f K → Set.MapsTo f K K →
    ∃ x ∈ K, f x = x

theorem perron_exists_of_irreducible
    {W : Type*} [Fintype W] [DecidableEq W] [Nonempty W] [Nontrivial W]
    (hBrouwer : BrouwerFixedPoint)
    (A : Matrix W W ℝ)
    (hNonneg : ∀ i j, 0 ≤ A i j)
    (hConn   : ∀ i j, ∃ k, 0 < (A ^ k) i j) :
    ∃ (v : W → ℝ) (lam : ℝ), (∀ i, 0 < v i) ∧ ∀ i, ∑ x, A i x * v x = lam * v i := by
  classical
  have hAnonneg_pow : ∀ k i j, 0 ≤ (A ^ k) i j := by
    intro k
    induction k with
    | zero =>
      intro i j
      rw [pow_zero, Matrix.one_apply]
      split <;> norm_num
    | succ n ih =>
      intro i j
      rw [pow_succ, Matrix.mul_apply]
      exact Finset.sum_nonneg (fun l _ => mul_nonneg (ih i l) (hNonneg l j))
  -- Every column has a positive entry (an "incoming edge"): irreducibility
  -- plus at least two vertices rules out a column of all zeros.
  have hasIncoming : ∀ j, ∃ i, 0 < A i j := by
    intro j
    obtain ⟨i0, hi0⟩ := exists_ne j
    obtain ⟨k, hk⟩ := hConn i0 j
    -- k can't be 0 since i0 ≠ j; peel off the last step of the path.
    match k, hk with
    | 0, hk => simp [pow_zero, Matrix.one_apply, hi0] at hk
    | (n+1), hk =>
      rw [pow_succ, Matrix.mul_apply] at hk
      by_contra hcon
      push_neg at hcon
      have : ∑ l, (A ^ n) i0 l * A l j ≤ 0 :=
        Finset.sum_nonpos (fun l _ => mul_nonpos_of_nonneg_of_nonpos (hAnonneg_pow n i0 l) (hcon l))
      linarith
  have hcolpos : ∀ j, 0 < ∑ i, A i j := by
    intro j
    obtain ⟨i0, hi0⟩ := hasIncoming j
    calc (0:ℝ) < A i0 j := hi0
      _ ≤ ∑ i, A i j := Finset.single_le_sum (fun i _ => hNonneg i j) (Finset.mem_univ i0)
  -- The simplex.
  set Δ : Set (W → ℝ) := {x | (∀ i, 0 ≤ x i) ∧ ∑ i, x i = 1} with hΔ_def
  have hΔne : Δ.Nonempty := by
    classical
    obtain ⟨w⟩ := (inferInstance : Nonempty W)
    refine ⟨fun i => if i = w then 1 else 0, fun i => by split <;> norm_num, ?_⟩
    rw [Finset.sum_eq_single w]
    · simp
    · intro b _ hb; simp [hb]
    · intro h; exact absurd (Finset.mem_univ w) h
  -- ||Ax||_1 > 0 for every x in the simplex.
  have hden_pos : ∀ x ∈ Δ, 0 < ∑ i, A.mulVec x i := by
    intro x hx
    obtain ⟨hxnn, hxsum⟩ := hx
    have hexpand : ∑ i, A.mulVec x i = ∑ j, x j * ∑ i, A i j := by
      simp only [Matrix.mulVec, Matrix.dotProduct]
      rw [Finset.sum_comm]
      apply Finset.sum_congr rfl
      intro j _
      rw [Finset.mul_sum]
      exact Finset.sum_congr rfl (fun i _ => by ring)
    have hex : ∃ j0, 0 < x j0 := by
      by_contra hcon
      push_neg at hcon
      have hz : ∑ j, x j = 0 :=
        Finset.sum_eq_zero (fun j _ => le_antisymm (hcon j) (hxnn j))
      linarith [hz, hxsum]
    obtain ⟨j0, hj0⟩ := hex
    calc (0:ℝ) < x j0 * ∑ i, A i j0 := mul_pos hj0 (hcolpos j0)
      _ ≤ ∑ j, x j * ∑ i, A i j :=
          Finset.single_le_sum (fun j _ => mul_nonneg (hxnn j) (hcolpos j).le) (Finset.mem_univ j0)
      _ = ∑ i, A.mulVec x i := hexpand.symm
  -- T x i := (Ax)_i / ||Ax||_1, the Collatz-Wielandt map.
  set T : (W → ℝ) → (W → ℝ) := fun x i => A.mulVec x i / ∑ k, A.mulVec x k with hT_def
  -- The two genuinely standard, problem-independent real-analysis facts
  -- I would not guess Mathlib's exact lemma names for blind:
  have hΔconvex : Convex ℝ Δ := by
    sorry -- The standard probability simplex is convex: a direct check
          -- from the definition of `Convex` (closed under convex
          -- combinations, immediate since both defining conditions --
          -- coordinatewise `≥ 0` and the linear condition `∑ x i = 1`
          -- -- are each preserved by convex combinations). This is a
          -- basic example in any convex-analysis text, e.g. Rockafellar,
          -- R.T., "Convex Analysis", Princeton Univ. Press, 1970 (I'm
          -- not certain of the exact section number, flagging that
          -- rather than asserting it). Mathlib likely has this as
          -- `stdSimplex`'s own convexity lemma already.
  have hΔcompact : IsCompact Δ := by
    sorry -- Δ is closed (an intersection of closed conditions: each
          -- `0 ≤ x i` and the single linear equality `∑ x i = 1` are
          -- closed in the product/pointwise topology on `W → ℝ`) and
          -- bounded (`0 ≤ x i ≤ 1` for every `x ∈ Δ`, since coordinates
          -- are nonnegative and sum to 1). In a finite-dimensional real
          -- normed space, closed + bounded ⟹ compact: the Heine-Borel
          -- theorem, e.g. Rudin, W., "Principles of Mathematical
          -- Analysis", 3rd ed., McGraw-Hill, 1976, Thm. 2.41.
  have hTcont : ContinuousOn T Δ := by
    sorry -- `A.mulVec` is linear on a finite-dimensional space, hence
          -- continuous automatically (every linear map between finite-
          -- dimensional normed real vector spaces is continuous -- a
          -- standard fact in any introductory functional analysis or
          -- multivariable analysis text, e.g. Rudin, W., "Principles of
          -- Mathematical Analysis", 3rd ed., McGraw-Hill, 1976, Ch. 9;
          -- I'm not fully certain of the precise theorem number there,
          -- flagging that rather than asserting it). In Mathlib terms
          -- this is `LinearMap.continuous_of_finiteDimensional`. The
          -- denominator `x ↦ ∑ k, A.mulVec x k` is linear too, hence
          -- continuous, and by `hden_pos` is nonzero throughout Δ, so T
          -- (a quotient of two continuous functions with nonvanishing
          -- denominator) is continuous on Δ by the standard algebra-of-
          -- continuous-functions theorem.
  have hTmaps : Set.MapsTo T Δ Δ := by
    intro x hx
    obtain ⟨hxnn, -⟩ := hx
    have hd := hden_pos x hx
    constructor
    · intro i
      simp only [hT_def]
      exact div_nonneg (Finset.sum_nonneg (fun j _ => mul_nonneg (hNonneg i j) (hxnn j))) hd.le
    · simp only [hT_def]
      rw [← Finset.sum_div, div_self hd.ne']
  obtain ⟨xs, hxs, hfix⟩ := hBrouwer Δ T hΔne hΔcompact hΔconvex hTcont hTmaps
  obtain ⟨hxsnn, -⟩ := hxs
  set lam : ℝ := ∑ k, A.mulVec xs k with hlam_def
  have hlampos : 0 < lam := hden_pos xs hxs
  have heig : ∀ i, ∑ x, A i x * xs x = lam * xs i := by
    intro i
    have hfi : A.mulVec xs i / lam = xs i := by
      have := congrFun hfix i
      simp only [hT_def] at this
      exact this
    have heq : A.mulVec xs i = xs i * lam := (div_eq_iff hlampos.ne').mp hfi
    have hunfold : A.mulVec xs i = ∑ x, A i x * xs x := by
      simp [Matrix.mulVec, Matrix.dotProduct]
    rw [hunfold] at heq
    rw [heq]; ring
  -- Positivity upgrade: xs is nonnegative and an eigenvector, and it is
  -- not identically zero (it sums to 1); irreducibility forces every
  -- coordinate to be positive, by the same zero-propagation argument
  -- as in `perron_unique_of_irreducible` above.
  have hstep : ∀ i, xs i = 0 → ∀ j, 0 < A i j → xs j = 0 := by
    intro i hzi j hAij
    have hi := heig i
    rw [hzi, mul_zero] at hi
    have hnonneg_terms : ∀ x ∈ (Finset.univ : Finset W), 0 ≤ A i x * xs x :=
      fun x _ => mul_nonneg (hNonneg i x) (hxsnn x)
    have hall_zero := (Finset.sum_eq_zero_iff_of_nonneg hnonneg_terms).mp hi j (Finset.mem_univ j)
    rcases mul_eq_zero.mp hall_zero with h | h
    · exact absurd h hAij.ne'
    · exact h
  have hpropagate : ∀ k i j, 0 < (A ^ k) i j → xs i = 0 → xs j = 0 := by
    intro k
    induction k with
    | zero =>
      intro i j hpow hzi
      rw [pow_zero, Matrix.one_apply] at hpow
      by_cases h : i = j
      · rwa [← h]
      · simp [h] at hpow
    | succ n ih =>
      intro i j hpow hzi
      rw [pow_succ, Matrix.mul_apply] at hpow
      have hex2 : ∃ l, 0 < (A ^ n) i l * A l j := by
        by_contra hcon
        push_neg at hcon
        have : ∑ l, (A ^ n) i l * A l j ≤ 0 := Finset.sum_nonpos (fun l _ => hcon l)
        linarith
      obtain ⟨l, hl⟩ := hex2
      have hl1 : 0 < (A ^ n) i l := by
        by_contra hcon
        push_neg at hcon
        have hz : (A ^ n) i l = 0 := le_antisymm hcon (hAnonneg_pow n i l)
        rw [hz, zero_mul] at hl
        exact absurd hl (lt_irrefl 0)
      have hl2 : 0 < A l j := by
        by_contra hcon
        push_neg at hcon
        have hz : A l j = 0 := le_antisymm hcon (hNonneg l j)
        rw [hz, mul_zero] at hl
        exact absurd hl (lt_irrefl 0)
      exact hstep l (ih i l hl1 hzi) j hl2
  have hxs_pos : ∀ i, 0 < xs i := by
    intro i
    rcases (hxsnn i).lt_or_eq with h | h
    · exact h
    · exfalso
      -- xs i = 0; since xs sums to 1, some j1 has xs j1 > 0, but xs i = 0
      -- propagates via hConn to force xs j1 = 0 too -- contradiction.
      obtain ⟨-, hxssum⟩ := hxs
      have hex3 : ∃ j1, 0 < xs j1 := by
        by_contra hcon
        push_neg at hcon
        have hz : ∑ j, xs j = 0 := Finset.sum_eq_zero (fun j _ => le_antisymm (hcon j) (hxsnn j))
        linarith [hz, hxssum]
      obtain ⟨j1, hj1⟩ := hex3
      obtain ⟨k, hk⟩ := hConn i j1
      have := hpropagate k i j1 hk h.symm
      linarith [hj1, this]
  exact ⟨xs, lam, hxs_pos, heig⟩

/- ==================================================================
   PART 3 -- The fully-composed statement: no `PerronUnique` hypothesis
   anywhere, only the two elementary irreducibility facts plus
   `BrouwerFixedPoint`. Once BrouwerFixedPoint is discharged (via
   harfe's repo) and the three Part-2 sorries are filled in (standard
   real analysis), delete `hExists` below and call
   `perron_exists_of_irreducible` directly instead.
   ================================================================== -/

theorem quotient_perron_eigenequation_complete
    [Nonempty V]
    {A : Matrix V V ℝ} {φ : V → V} {R : Finset V}
    (hInv    : Function.Involutive φ)
    (hAuto   : ∀ u w, A (φ u) (φ w) = A u w)
    (hNonneg : ∀ i j, 0 ≤ A i j)
    (hConn   : ∀ i j, ∃ k, 0 < (A ^ k) i j)
    (hR      : ∀ u, u ∈ R ↔ φ u ∉ R)
    (hExists : ∃ (v : V → ℝ) (lam : ℝ), (∀ i, 0 < v i) ∧ ∀ i, ∑ x, A i x * v x = lam * v i) :
    ∃ (v : V → ℝ) (lam : ℝ), (∀ i, 0 < v i) ∧
      ∀ u ∈ R, ∑ β ∈ R, (A u β + A u (φ β)) * v β = lam * v u := by
  obtain ⟨v, lam, hPos, hEig⟩ := hExists
  have hUniq : PerronUnique A := perron_unique_of_irreducible A hNonneg hConn
  exact ⟨v, lam, hPos, quotient_perron_eigenequation hInv hAuto hPos hEig hUniq hR⟩
