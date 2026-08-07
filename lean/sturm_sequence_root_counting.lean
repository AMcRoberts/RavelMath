/-
  sturm_sequence_root_counting.lean

  Author: Ravel.
  Date:   2026-08-07.
  Architect: AM (project coordinator and project architect).
  Project: RavelMathPub (work repo at
           ~/claude/RavelMathPub/).
  Aim:    Port of Sturm's theorem (real polynomial root counting via
          sign-change sequences) into Lean 4 / Mathlib, following
          Manuel Eberl's Isabelle/HOL formalisation (Archive of Formal
          Proofs, "A Formalisation of Sturm's Theorem", 2014) directly
          -- see refs/references.bib key EberlSturmSequences2014 and
          docs/WEB_AND_API_BIBLIOGRAPHY_USAGE_2026-08-07.md for the
          full correspondence between that development's structure and
          this file's.
  Audit:  lean/signatures/MANIFEST.txt (signed manifest).
  GPG key fingerprint: C254E9BC303372F75B33038C3F3D3FD47F09D7E5.
  X.509 certificate: lean/signatures/ravel-cert.pem (self-signed).
-/

/-
  This file exists to close a real gap identified during the
  mathlib::reflection retrofit work: `math/src/exact_pisot.c` already
  computes exact Sturm-chain root isolation for arbitrary-degree
  integer polynomials (used by `pisot_classify_degree_n`, Finding 30),
  but Mathlib has no formalization of Sturm's theorem itself, so none
  of that C++ computation could be backed by a kernel-checked Lean
  fact the way this project's other reflection connections are.

  SCOPE. This file targets exactly the case `exact_pisot.c` uses:
  SQUAREFREE polynomials (the classifier already checks
  `poly_is_squarefree` before building a Sturm chain), counting
  DISTINCT real roots in a half-open interval `(a, b]`. Eberl's
  original development covers this case in full; the multiplicity-
  aware extension (Li--Paulson 2018, Budan--Fourier) is not needed
  here and not attempted.

  WHAT IS REUSED, NOT REINVENTED (see the bibliography note for
  detail): `List.destutter` (Mathlib, already used by
  `Polynomial.signVariations` for the analogous coefficient-sign-change
  count in Descartes' rule of signs) for collapsing adjacent equal
  signs; Mathlib's intermediate value theorem, polynomial continuity,
  and `Filter.eventually` machinery in place of Eberl's bespoke
  neighbourhood lemmas.
-/

import Mathlib

open Polynomial

/- ============================================================
   Section 1: sign-change counting for a list of polynomials,
   evaluated at a point. Mirrors Eberl's `sign_changes`
   (Sturm_Theorem.thy, Section 2.1), reusing the same
   filter-nonzero / map-to-sign / destutter-adjacent-duplicates /
   length-minus-one pattern Mathlib's `Polynomial.signVariations`
   already uses for coefficient sign changes (RuleOfSigns.lean).
   ============================================================ -/

/-- The number of sign changes of the sequence `p₁(x), ..., pₙ(x)`,
ignoring zero values. A "sign change" is a maximal run boundary
between a positive and a negative value (adjacent EQUAL nonzero signs
are collapsed by `List.destutter`, exactly as `Polynomial.
signVariations` collapses adjacent equal coefficient signs). -/
noncomputable def signChanges (ps : List (Polynomial ℝ)) (x : ℝ) : ℕ :=
  let signs := (ps.map (fun p => SignType.sign (p.eval x))).filter (· ≠ 0)
  (signs.destutter (· ≠ ·)).length - 1

@[simp]
theorem signChanges_nil (x : ℝ) : signChanges [] x = 0 := by
  simp [signChanges]

/-- Sign changes of a length-1 list is always 0 (there's nothing to
change sign relative to). -/
@[simp]
theorem signChanges_singleton (p : Polynomial ℝ) (x : ℝ) :
    signChanges [p] x = 0 := by
  simp only [signChanges, List.map, List.filter]
  rcases h : SignType.sign (p.eval x) with _ | _ | _ <;> simp [h]

/-- Pure combinatorial core of the "sturm triple" fact, isolated from
any real-number reasoning: for any three signs `sp, sq, sr` with
`sp ≠ 0` and `sr = -sp` (so `sp`,`sr` are the two nonzero, opposite
"ends"), the sign-change count of `[sp, sq, sr]` is exactly 1, no
matter what `sq` is. Decidable finite case check over `SignType`
(3 constructors) -- no free real variables, so `decide` applies
directly here (unlike at the `Polynomial.eval`-valued level). -/
theorem signChanges_core_triple {sp sq sr : SignType}
    (hsp : sp ≠ 0) (hr : sr = -sp) :
    (([sp, sq, sr].filter (· ≠ 0)).destutter (· ≠ ·)).length - 1 = 1 := by
  subst hr
  rcases sp with _ | _ | _ <;> rcases sq with _ | _ | _ <;> revert hsp <;> decide

/-- For a length-3 sequence `[p,q,r]`, if `p(x) ≠ 0` and `r(x)` has the
OPPOSITE sign to `p(x)`, the sign-change count is exactly 1,
regardless of `q`'s sign (matching Eberl's `sign-changes-sturm-triple`,
outline p.8 -- this is the key combinatorial fact that lets the middle
polynomial's sign be irrelevant at a root of the middle polynomial). -/
theorem signChanges_sturm_triple {p q r : Polynomial ℝ} {x : ℝ}
    (hp : p.eval x ≠ 0) (hr : SignType.sign (r.eval x) = -SignType.sign (p.eval x)) :
    signChanges [p, q, r] x = 1 := by
  have hsp : SignType.sign (p.eval x) ≠ 0 := by
    simpa [sign_eq_zero_iff] using hp
  simp only [signChanges, List.map]
  exact signChanges_core_triple hsp hr

/-- `List.getLastD` on a NONEMPTY list is independent of the default
value supplied (only used when the list is actually empty) -- reduces
immediately via `getLastD_cons`, which drops the default entirely in
favour of the list's own head as soon as one element is available. -/
theorem List.getLastD_congr {α : Type*} {l : List α} (h : l ≠ []) (d1 d2 : α) :
    l.getLastD d1 = l.getLastD d2 := by
  cases l with
  | nil => exact absurd rfl h
  | cons x l' => rw [List.getLastD_cons, List.getLastD_cons]

/-- Structural fact about `List.destutter'` not already in Mathlib:
destuttering a concatenation `l1 ++ l2`, starting from a seed `a`,
equals destuttering `l1` from `a`, DROPPING its trailing "current
seed" marker, followed by destuttering `l2` starting from whatever
that trailing seed was. (`List.destutter'`'s own recursive law keeps
the SEED as head on each transition and re-threads the just-consumed
element as the new seed -- `destutter'_cons_pos (h : R b a) : (a ::
l).destutter' R b = b :: l.destutter' R a` -- so the seed always
survives as the very LAST element of the result, via the `destutter'
_nil : destutter' R a [] = [a]` base case; that trailing occurrence is
exactly the connection point to a following `l2`, and must be dropped
from the "prefix" contribution or it would be double-counted.) Proved
by structural induction on `l1`, generalizing the seed. -/
theorem List.destutter'_append {α : Type*} (R : α → α → Prop) [DecidableRel R]
    (a : α) (l1 l2 : List α) :
    (l1 ++ l2).destutter' R a =
      (l1.destutter' R a).dropLast ++
        l2.destutter' R ((l1.destutter' R a).getLastD a) := by
  induction l1 generalizing a with
  | nil => simp
  | cons b rest ih =>
    rw [List.cons_append, List.destutter'_cons, List.destutter'_cons]
    split_ifs with hab
    · rw [ih b, List.dropLast_cons_of_ne_nil (rest.destutter'_ne_nil R),
        List.getLastD_cons, List.cons_append,
        List.getLastD_congr (rest.destutter'_ne_nil R) b a]
    · exact ih a

/-- Sign changes distribute over list concatenation at a shared
pivot: the sign-change count of `ps₁ ++ [p] ++ ps₂` (at a point where
`p` is nonzero) equals the sign-change count of the LEFT half
`ps₁ ++ [p]` plus the sign-change count of the RIGHT half `[p] ++ ps₂`
(matching Eberl's `sign-changes-distrib`, outline p.8). Reduces to a
length computation via `destutter'_append`, case-split on whether the
seed reached after consuming `ps₁` differs from the pivot's own sign
or coincides with it (both cases give the same length identity, shown
directly). -/
theorem signChanges_distrib {ps1 ps2 : List (Polynomial ℝ)} {p : Polynomial ℝ} {x : ℝ}
    (hp : p.eval x ≠ 0) :
    signChanges (ps1 ++ [p] ++ ps2) x =
      signChanges (ps1 ++ [p]) x + signChanges ([p] ++ ps2) x := by
  classical
  have hsp : SignType.sign (p.eval x) ≠ 0 := by simpa [sign_eq_zero_iff] using hp
  simp only [signChanges, List.map_append, List.map_cons, List.map_nil, List.filter_append]
  set s1 := (ps1.map (fun q => SignType.sign (q.eval x))) with hs1def
  set s2 := (ps2.map (fun q => SignType.sign (q.eval x))) with hs2def
  set sp := SignType.sign (p.eval x) with hspdef
  have hspf : ([sp] : List SignType).filter (· ≠ 0) = [sp] := by simp [hsp]
  simp only [hspf]
  set l1 := s1.filter (· ≠ 0) with hl1def
  set l2 := s2.filter (· ≠ 0) with hl2def
  clear_value l1 l2 s1 s2 sp
  clear hspf hs1def hs2def hspdef hp
  rcases l1 with _ | ⟨a0, l1'⟩
  · simp
  · have hassoc : l1' ++ [sp] ++ l2 = l1' ++ (sp :: l2) := by
      rw [List.append_assoc]; rfl
    simp only [List.cons_append, List.nil_append, List.destutter_cons', hassoc]
    rw [List.destutter'_append (· ≠ ·) a0 l1' (sp :: l2),
        List.destutter'_append (· ≠ ·) a0 l1' [sp]]
    set g := (l1'.destutter' (· ≠ ·) a0).getLastD a0 with hg
    set D := (l1'.destutter' (· ≠ ·) a0).dropLast with hD
    -- [sp].destutter' (≠) g unfolds via destutter'_cons on the empty
    -- tail; (sp::l2).destutter' (≠) g unfolds the same way on l2.
    rw [List.destutter'_cons (R := (· ≠ ·)) (a := g) (b := sp) (l := l2),
        List.destutter'_cons (R := (· ≠ ·)) (a := g) (b := sp) (l := ([] : List SignType))]
    have hlen_pos : 1 ≤ (l2.destutter' (· ≠ ·) sp).length :=
      List.length_pos_of_ne_nil (l2.destutter'_ne_nil (· ≠ ·))
    split_ifs with hgsp
    · simp only [List.destutter'_nil]
      simp only [List.length_append, List.length_cons, List.length_nil, List.length_singleton]
      omega
    · rw [not_not] at hgsp
      subst hgsp
      simp only [List.destutter'_nil]
      simp only [List.length_append, List.length_nil, List.length_singleton]
      omega

/- ============================================================
   Section 2: the quasi-Sturm-sequence / Sturm-sequence structure
   hierarchy. Mirrors Eberl's `quasi_sturm_seq`/`sturm_seq` locales
   (outline p.9): `QuasiSturmSeq` captures the properties satisfied by
   any nonempty SUFFIX of a genuine Sturm sequence (weaker); `SturmSeq`
   adds the properties that only hold for the FULL sequence (p₁ = p
   itself, the sign-flip-at-a-root relationship between p and p₂, and
   p, p₂ having no common roots). Encoded as `structure`s bundling the
   defining properties as fields, rather than Isabelle's `locale`
   (Lean has no direct locale analogue; a structure over the sequence
   plus a separate `Prop`-valued predicate is the standard idiom).
   ============================================================ -/

/-- A nonempty list of real polynomials satisfying the three
properties any nonempty suffix of a genuine Sturm sequence has (Eberl's
`quasi_sturm_seq`): the last polynomial never changes sign; and if the
MIDDLE one of any three consecutive polynomials has a root at `x`, the
other two have opposite, nonzero sign at `x`. -/
structure QuasiSturmSeq (ps : List (Polynomial ℝ)) : Prop where
  ne_nil : ps ≠ []
  last_sign_const : ∀ x y : ℝ,
    SignType.sign ((ps.getLast ne_nil).eval x) = SignType.sign ((ps.getLast ne_nil).eval y)
  signs : ∀ (i : ℕ) (x : ℝ), (hi : i + 2 < ps.length) → (ps[i+1]'(by omega)).eval x = 0 →
    SignType.sign ((ps[i+2]'(by omega)).eval x) = -SignType.sign ((ps[i]'(by omega)).eval x)

/-- A genuine Sturm sequence of `p` (Eberl's `sturm_seq`, extending
`quasi_sturm_seq`): additionally, `p` is the FIRST polynomial; at any
root `x` of `p`, the second polynomial `p₂` has the opposite sign
immediately left of `x` and the SAME sign immediately right of `x`
(the derivative-like separating property); and `p`, `p₂` share no
common roots (so the sequence doesn't degenerate at the very start). -/
structure SturmSeq (p : Polynomial ℝ) (ps : List (Polynomial ℝ)) : Prop extends
    QuasiSturmSeq ps where
  length_ge_two : 2 ≤ ps.length
  head_eq_p : ps.head (by rintro rfl; simp at length_ge_two) = p
  deriv_sign : ∀ x0 : ℝ, p.eval x0 = 0 →
    ∀ᶠ x in nhds x0, SignType.sign ((p * ps[1]'(by omega)).eval x) =
      if x > x0 then 1 else -1
  squarefree_pair : ∀ x : ℝ, ¬(p.eval x = 0 ∧ (ps[1]'(by omega)).eval x = 0)

/- ============================================================
   Section 3: local constancy of `signChanges` away from roots.
   Eberl's general form (outline p.10-12) handles ANY point, splitting
   the sequence wherever an INTERIOR polynomial has a root there (via
   `split_sign_changes` and the `sturm_triple` fact). This file proves
   the case actually needed for root-COUNTING at a RATIONAL bracket
   (matching how `math/src/exact_pisot.c` isolates roots -- bisection
   always chooses bracket endpoints where every Sturm-chain polynomial
   is checked nonzero before use, never picking an endpoint that is an
   exact root of anything in the chain): local constancy at a point
   where EVERY polynomial in the sequence is individually nonzero.
   This sidesteps the root-splitting machinery entirely -- pure
   continuity, via Mathlib's `ContinuousAt.eventually_lt`.
   ============================================================ -/

/-- If a polynomial's value at `x0` is nonzero, its SIGN is locally
constant near `x0` (standard continuity fact: an open half-line
condition transports back through `ContinuousAt`). -/
theorem sign_eval_eventually_eq (p : Polynomial ℝ) (x0 : ℝ) (h : p.eval x0 ≠ 0) :
    ∀ᶠ x in nhds x0, SignType.sign (p.eval x) = SignType.sign (p.eval x0) := by
  have hcont : ContinuousAt (fun x => p.eval x) x0 := (Polynomial.continuous p).continuousAt
  rcases lt_or_gt_of_ne h with hneg | hpos
  · have hev := hcont.eventually_lt continuousAt_const hneg
    filter_upwards [hev] with x hx
    rw [sign_neg hneg, sign_neg hx]
  · have hev := continuousAt_const.eventually_lt hcont hpos
    filter_upwards [hev] with x hx
    rw [sign_pos hpos, sign_pos hx]

/-- If EVERY polynomial in a list is individually nonzero at `x0`, the
whole (filtered, mapped-to-sign) list is eventually equal to its value
at `x0`, near `x0` -- finite conjunction of the single-polynomial
local-constancy fact above, by induction on the list. -/
theorem signList_eventually_eq {ps : List (Polynomial ℝ)} {x0 : ℝ}
    (h : ∀ p ∈ ps, p.eval x0 ≠ 0) :
    ∀ᶠ x in nhds x0, ps.map (fun p => SignType.sign (p.eval x)) =
      ps.map (fun p => SignType.sign (p.eval x0)) := by
  induction ps with
  | nil => simp
  | cons p rest ih =>
    have hp : p.eval x0 ≠ 0 := h p (List.mem_cons_self ..)
    have hrest : ∀ q ∈ rest, q.eval x0 ≠ 0 := fun q hq => h q (List.mem_cons_of_mem _ hq)
    filter_upwards [sign_eval_eventually_eq p x0 hp, ih hrest] with x hx1 hx2
    simp [hx1, hx2]

/-- `signChanges` is locally constant near any point where every
polynomial in the sequence is individually nonzero -- the case this
project actually needs (rational bisection bracket endpoints, chosen
exactly so no chain polynomial vanishes there). -/
theorem signChanges_eventually_const_of_all_ne {ps : List (Polynomial ℝ)} {x0 : ℝ}
    (h : ∀ p ∈ ps, p.eval x0 ≠ 0) :
    ∀ᶠ x in nhds x0, signChanges ps x = signChanges ps x0 := by
  filter_upwards [signList_eventually_eq h] with x hx
  simp [signChanges, hx]

/- ============================================================
   Section 4: the FULL general local-constancy theorem, following
   Eberl's actual proof directly (`sturm_document.pdf`, "2.3 Auxiliary
   lemmas about roots and sign changes", the `split_sign_changes`
   function and `split_sign_changes_correct(_nbh)`), not just the
   all-nonzero special case above. The real hypothesis is much weaker
   than "every polynomial nonzero at x0": only the FIRST polynomial
   needs to be nonzero there. `split_sign_changes` recursively peels
   off either a 2-window or a 3-window from the front, always keeping
   the invariant "current head nonzero at x0" -- when the SECOND
   element of a 3-window is a root, the `QuasiSturmSeq.signs` property
   forces the THIRD element nonzero (opposite sign to the first),
   which becomes the new head, maintaining the invariant.
   ============================================================ -/

/-- Peels a Sturm(-like) sequence into windows (of length 2 or 3) at a
point `x`, such that each window's own sign-change count sums to the
whole list's. Matches Eberl's `split_sign_changes` exactly (structure,
not just intent): a length-3 window `[p,q,r]` is taken exactly when
`p x ≠ 0 ∧ q x = 0` (the case `signChanges_sturm_triple` handles
uniformly regardless of `q`'s own local wobble); otherwise only `[p,q]`
is peeled and the rest continues from `q`. -/
noncomputable def splitSignChanges (ps : List (Polynomial ℝ)) (x : ℝ) : List (List (Polynomial ℝ)) :=
  match ps with
  | [] => []
  | [p] => [[p]]
  | [p, q] => [[p, q]]
  | p :: q :: r :: rest =>
      if p.eval x ≠ 0 ∧ q.eval x = 0 then
        [p, q, r] :: splitSignChanges (r :: rest) x
      else
        [p, q] :: splitSignChanges (q :: r :: rest) x
  termination_by ps.length

/-- The total sign-change count of a peeled window list -- the
quantity `split_sign_changes_correct(_nbh)` relates to `signChanges`
of the original list. -/
noncomputable def splitSignChangesTotal (pieces : List (List (Polynomial ℝ))) (x : ℝ) : ℕ :=
  (pieces.map (fun piece => signChanges piece x)).sum

/-- The peeled-window total, computed with the split fixed AT `x0` but
each piece's sign changes evaluated at the (possibly different) point
`x` -- Eberl's `sign_changes'`. This is the quantity that turns out to
be BOTH (a) exactly `signChanges ps x0` when `x = x0` (`splitSignChanges
_correct`) and (b) exactly `signChanges ps x` for `x` NEAR `x0`
(`splitSignChanges_correct_nbh`), which together give local constancy. -/
noncomputable def splitSignChangesTotalAt (ps : List (Polynomial ℝ)) (x0 x : ℝ) : ℕ :=
  splitSignChangesTotal (splitSignChanges ps x0) x

/-- The total sign-change count of the peeled windows (evaluated AT
the same point `x0` used to compute the split) equals `signChanges ps
x0` -- matches Eberl's `split_sign_changes_correct` exactly, proved by
the SAME induction the recursive definition itself uses (Lean's
auto-generated equation-compiler induction principle
`splitSignChanges.induct`, standing in for Eberl's hand-written
`split_sign_changes_induct`). -/
theorem splitSignChangesTotalAt_self (ps : List (Polynomial ℝ)) (x0 : ℝ) :
    ∀ (hqs : QuasiSturmSeq ps), (ps.head hqs.ne_nil).eval x0 ≠ 0 →
      splitSignChangesTotalAt ps x0 x0 = signChanges ps x0 := by
  induction ps using splitSignChanges.induct (x := x0) with
  | case1 => intro hqs _; exact absurd rfl hqs.ne_nil
  | case2 p => intro _ _; simp [splitSignChangesTotalAt, splitSignChangesTotal, splitSignChanges]
  | case3 p q => intro _ _; simp [splitSignChangesTotalAt, splitSignChangesTotal, splitSignChanges]
  | case4 p q r rest h ih1 =>
    intro hqs hp
    have hp0 : p.eval x0 ≠ 0 := by simpa [List.head] using hp
    have hq0 := h.2
    have hsigns := hqs.signs 0 x0 (by simp) (by simpa using hq0)
    simp only [List.getElem_cons_succ, List.getElem_cons_zero] at hsigns
    have hr0 : r.eval x0 ≠ 0 := by
      intro hr0
      rw [hr0] at hsigns
      have hsp0 : SignType.sign (p.eval x0) = 0 := by
        have hh := hsigns.symm
        simpa using hh
      exact hp0 (sign_eq_zero_iff.mp hsp0)
    have hlen : (p :: q :: r :: rest).length = (r :: rest).length + 2 := by simp
    have hqsr : QuasiSturmSeq (r :: rest) := by
      refine ⟨List.cons_ne_nil r rest, ?_, ?_⟩
      · intro a b
        have h1 := hqs.last_sign_const a b
        simpa using h1
      · intro i x' hi hzero
        have := hqs.signs (i+2) x' (by omega) (by simpa using hzero)
        simpa using this
    have hd_r : ((r :: rest).head (List.cons_ne_nil r rest)).eval x0 ≠ 0 := by simpa using hr0
    have key := ih1 hqsr hd_r
    have hdist := signChanges_distrib (ps1 := [p, q]) (ps2 := rest)
      (p := r) (x := x0) hr0
    simp only [splitSignChangesTotalAt, splitSignChanges, if_pos h,
      splitSignChangesTotal, List.map_cons, List.sum_cons] at key ⊢
    rw [key]
    have hsc3 : signChanges [p, q, r] x0 = 1 := signChanges_sturm_triple hp0 hsigns
    have hcombine : signChanges (p :: q :: r :: rest) x0 =
        signChanges [p, q, r] x0 + signChanges (r :: rest) x0 := by
      have := hdist
      simpa using this
    rw [hcombine, hsc3]
  | case5 p q r rest h ih2 =>
    intro hqs hp
    have hp0 : p.eval x0 ≠ 0 := by simpa [List.head] using hp
    have hq0 : q.eval x0 ≠ 0 := fun hq0 => h ⟨hp0, hq0⟩
    have hlen : (p :: q :: r :: rest).length = (q :: r :: rest).length + 1 := by simp
    have hqsqr : QuasiSturmSeq (q :: r :: rest) := by
      refine ⟨List.cons_ne_nil q (r :: rest), ?_, ?_⟩
      · intro a b
        have h1 := hqs.last_sign_const a b
        simpa using h1
      · intro i x' hi hzero
        have := hqs.signs (i+1) x' (by omega) (by simpa using hzero)
        simpa using this
    have hd_q : ((q :: r :: rest).head (List.cons_ne_nil q (r :: rest))).eval x0 ≠ 0 := by
      simpa using hq0
    have key := ih2 hqsqr hd_q
    have hdist := signChanges_distrib (ps1 := [p]) (ps2 := r :: rest)
      (p := q) (x := x0) hq0
    simp only [splitSignChangesTotalAt, splitSignChanges, if_neg h,
      splitSignChangesTotal, List.map_cons, List.sum_cons] at key ⊢
    rw [key]
    have hcombine : signChanges (p :: q :: r :: rest) x0 =
        signChanges [p, q] x0 + signChanges (q :: r :: rest) x0 := by
      have := hdist
      simpa using this
    rw [hcombine]

/-- If a polynomial's sign transports from `x0` to `x` (i.e. is equal
there) and it's nonzero at `x0`, it's nonzero at `x` too -- immediate
from `sign_eq_zero_iff`, but needed repeatedly below to convert a
sign-equality into a nonzero-transport fact. -/
theorem ne_zero_of_sign_eq {p : Polynomial ℝ} {x0 x : ℝ}
    (heq : SignType.sign (p.eval x) = SignType.sign (p.eval x0)) (h0 : p.eval x0 ≠ 0) :
    p.eval x ≠ 0 := by
  intro hx
  rw [hx] at heq
  simp only [sign_zero] at heq
  exact h0 (sign_eq_zero_iff.mp heq.symm)

/-- The split (computed AT `x0`), with each piece's sign changes
evaluated at a NEARBY point `x` where every polynomial that was
nonzero at `x0` keeps the same sign, still totals to `signChanges ps
x` (not just `signChanges ps x0`) -- Eberl's `split_sign_changes_
correct_nbh`, same induction as `splitSignChangesTotalAt_self`, now
threading an extra evaluation point plus the sign-transport
hypothesis. The window that's a root of `q` at `x0` still contributes
exactly 1 at `x` regardless of `q`'s own local behaviour, since
`signChanges_sturm_triple` only needs the FLANKING polynomials
nonzero+opposite, and those transport by hypothesis. -/
theorem splitSignChangesTotalAt_eq (ps : List (Polynomial ℝ)) (x0 : ℝ) :
    ∀ (hqs : QuasiSturmSeq ps), (ps.head hqs.ne_nil).eval x0 ≠ 0 →
      ∀ (x : ℝ), (∀ p ∈ ps, p.eval x0 ≠ 0 → SignType.sign (p.eval x) = SignType.sign (p.eval x0)) →
        splitSignChangesTotalAt ps x0 x = signChanges ps x := by
  induction ps using splitSignChanges.induct (x := x0) with
  | case1 => intro hqs _ _ _; exact absurd rfl hqs.ne_nil
  | case2 p => intro _ _ x _; simp [splitSignChangesTotalAt, splitSignChangesTotal, splitSignChanges]
  | case3 p q => intro _ _ x _; simp [splitSignChangesTotalAt, splitSignChangesTotal, splitSignChanges]
  | case4 p q r rest h ih1 =>
    intro hqs hp x nbh
    have hp0 : p.eval x0 ≠ 0 := by simpa [List.head] using hp
    have hq0 := h.2
    have hsigns := hqs.signs 0 x0 (by simp) (by simpa using hq0)
    simp only [List.getElem_cons_succ, List.getElem_cons_zero] at hsigns
    have hr0 : r.eval x0 ≠ 0 := by
      intro hr0
      rw [hr0] at hsigns
      have hsp0 : SignType.sign (p.eval x0) = 0 := by
        have hh := hsigns.symm
        simpa using hh
      exact hp0 (sign_eq_zero_iff.mp hsp0)
    have hlen : (p :: q :: r :: rest).length = (r :: rest).length + 2 := by simp
    have hqsr : QuasiSturmSeq (r :: rest) := by
      refine ⟨List.cons_ne_nil r rest, ?_, ?_⟩
      · intro a b
        have h1 := hqs.last_sign_const a b
        simpa using h1
      · intro i x' hi hzero
        have := hqs.signs (i+2) x' (by omega) (by simpa using hzero)
        simpa using this
    have hd_r : ((r :: rest).head (List.cons_ne_nil r rest)).eval x0 ≠ 0 := by simpa using hr0
    have nbh_r : ∀ p' ∈ (r :: rest), p'.eval x0 ≠ 0 →
        SignType.sign (p'.eval x) = SignType.sign (p'.eval x0) :=
      fun p' hp' => nbh p' (List.mem_cons_of_mem _ (List.mem_cons_of_mem _ hp'))
    have key := ih1 hqsr hd_r x nbh_r
    have hp_x : p.eval x ≠ 0 := ne_zero_of_sign_eq (nbh p (by simp) hp0) hp0
    have hr_x : r.eval x ≠ 0 := ne_zero_of_sign_eq (nbh r (by simp) hr0) hr0
    have hsigns_x : SignType.sign (r.eval x) = -SignType.sign (p.eval x) := by
      rw [nbh r (by simp) hr0, nbh p (by simp) hp0]
      exact hsigns
    have hdist := signChanges_distrib (ps1 := [p, q]) (ps2 := rest)
      (p := r) (x := x) hr_x
    simp only [splitSignChangesTotalAt, splitSignChanges, if_pos h,
      splitSignChangesTotal, List.map_cons, List.sum_cons] at key ⊢
    rw [key]
    have hsc3 : signChanges [p, q, r] x = 1 := signChanges_sturm_triple hp_x hsigns_x
    have hcombine : signChanges (p :: q :: r :: rest) x =
        signChanges [p, q, r] x + signChanges (r :: rest) x := by
      have := hdist
      simpa using this
    rw [hcombine, hsc3]
  | case5 p q r rest h ih2 =>
    intro hqs hp x nbh
    have hp0 : p.eval x0 ≠ 0 := by simpa [List.head] using hp
    have hq0 : q.eval x0 ≠ 0 := fun hq0 => h ⟨hp0, hq0⟩
    have hlen : (p :: q :: r :: rest).length = (q :: r :: rest).length + 1 := by simp
    have hqsqr : QuasiSturmSeq (q :: r :: rest) := by
      refine ⟨List.cons_ne_nil q (r :: rest), ?_, ?_⟩
      · intro a b
        have h1 := hqs.last_sign_const a b
        simpa using h1
      · intro i x' hi hzero
        have := hqs.signs (i+1) x' (by omega) (by simpa using hzero)
        simpa using this
    have hd_q : ((q :: r :: rest).head (List.cons_ne_nil q (r :: rest))).eval x0 ≠ 0 := by
      simpa using hq0
    have nbh_q : ∀ p' ∈ (q :: r :: rest), p'.eval x0 ≠ 0 →
        SignType.sign (p'.eval x) = SignType.sign (p'.eval x0) :=
      fun p' hp' => nbh p' (List.mem_cons_of_mem _ hp')
    have key := ih2 hqsqr hd_q x nbh_q
    have hq_x : q.eval x ≠ 0 := ne_zero_of_sign_eq (nbh q (by simp) hq0) hq0
    have hdist := signChanges_distrib (ps1 := [p]) (ps2 := r :: rest)
      (p := q) (x := x) hq_x
    simp only [splitSignChangesTotalAt, splitSignChanges, if_neg h,
      splitSignChangesTotal, List.map_cons, List.sum_cons] at key ⊢
    rw [key]
    have hcombine : signChanges (p :: q :: r :: rest) x =
        signChanges [p, q] x + signChanges (q :: r :: rest) x := by
      have := hdist
      simpa using this
    rw [hcombine]

/-- Every polynomial in `ps` that is nonzero at `x0` is eventually
(near `x0`) the same sign as at `x0` -- finite conjunction of
`sign_eval_eventually_eq`, restricted to the nonzero-at-`x0` subset
(the version `splitSignChangesTotalAt_eq` actually needs). -/
theorem signList_eventually_eq' (ps : List (Polynomial ℝ)) (x0 : ℝ) :
    ∀ᶠ x in nhds x0, ∀ p ∈ ps, p.eval x0 ≠ 0 → SignType.sign (p.eval x) = SignType.sign (p.eval x0) := by
  induction ps with
  | nil => simp
  | cons p rest ih =>
    by_cases hp : p.eval x0 ≠ 0
    · filter_upwards [sign_eval_eventually_eq p x0 hp, ih] with x hx1 hx2
      intro q hq hq0
      rw [List.mem_cons] at hq
      rcases hq with rfl | hq
      · exact hx1
      · exact hx2 q hq hq0
    · filter_upwards [ih] with x hx2
      intro q hq hq0
      rw [List.mem_cons] at hq
      rcases hq with rfl | hq
      · exact absurd hq0 hp
      · exact hx2 q hq hq0

/-- A `[p,q]` PAIR (where `p` is nonzero at `x0` and `q` has a
GLOBALLY constant sign -- true of every genuine "last pair" tail)
has eventually-constant `signChanges` near `x0`: `p`'s sign transports
by continuity, `q`'s needs no continuity at all since it never
changes. -/
theorem signChanges_pair_eventually_eq {p q : Polynomial ℝ} {x0 : ℝ}
    (hp0 : p.eval x0 ≠ 0) (hqlast : ∀ x y : ℝ, SignType.sign (q.eval x) = SignType.sign (q.eval y)) :
    ∀ᶠ x in nhds x0, signChanges [p, q] x = signChanges [p, q] x0 := by
  filter_upwards [sign_eval_eventually_eq p x0 hp0] with x hpx
  simp only [signChanges, List.map_cons, List.map_nil]
  rw [hpx, hqlast x x0]

/-- The split (a FIXED, finite list of windows, computed once at
`x0`), summed with each piece evaluated at a varying `x`, is itself
eventually constant near `x0` -- the missing link connecting
`splitSignChangesTotalAt_eq` (relates the total-at-x to `signChanges
ps x`) and `splitSignChangesTotalAt_self` (relates the total-at-x0 to
`signChanges ps x0`) into genuine local constancy of `signChanges`
itself. Same induction once more: length-1 pieces are trivially
constant (always 0); `[p,q]` tail pieces via `signChanges_pair_
eventually_eq`; `[p,q,r]` triples via `signChanges_sturm_triple`,
which is identically 1 at BOTH `x` and `x0` once `p`,`r` are confirmed
nonzero+opposite at each (continuity transports this from `x0`). -/
theorem splitSignChangesTotalAt_eventually_const (ps : List (Polynomial ℝ)) (x0 : ℝ) :
    ∀ (hqs : QuasiSturmSeq ps), (ps.head hqs.ne_nil).eval x0 ≠ 0 →
      ∀ᶠ x in nhds x0, splitSignChangesTotalAt ps x0 x = splitSignChangesTotalAt ps x0 x0 := by
  induction ps using splitSignChanges.induct (x := x0) with
  | case1 => intro hqs _; exact absurd rfl hqs.ne_nil
  | case2 p => intro _ _; simp [splitSignChangesTotalAt, splitSignChangesTotal, splitSignChanges]
  | case3 p q =>
    intro hqs hp
    have hp0 : p.eval x0 ≠ 0 := by simpa [List.head] using hp
    filter_upwards [signChanges_pair_eventually_eq hp0 hqs.last_sign_const] with x hx
    simp only [splitSignChangesTotalAt, splitSignChanges, splitSignChangesTotal,
      List.map_cons, List.map_nil, List.sum_cons, List.sum_nil, add_zero]
    exact hx
  | case4 p q r rest h ih1 =>
    intro hqs hp
    have hp0 : p.eval x0 ≠ 0 := by simpa [List.head] using hp
    have hq0 := h.2
    have hsigns := hqs.signs 0 x0 (by simp) (by simpa using hq0)
    simp only [List.getElem_cons_succ, List.getElem_cons_zero] at hsigns
    have hr0 : r.eval x0 ≠ 0 := by
      intro hr0
      rw [hr0] at hsigns
      have hsp0 : SignType.sign (p.eval x0) = 0 := by
        have hh := hsigns.symm
        simpa using hh
      exact hp0 (sign_eq_zero_iff.mp hsp0)
    have hlen : (p :: q :: r :: rest).length = (r :: rest).length + 2 := by simp
    have hqsr : QuasiSturmSeq (r :: rest) := by
      refine ⟨List.cons_ne_nil r rest, ?_, ?_⟩
      · intro a b
        have h1 := hqs.last_sign_const a b
        simpa using h1
      · intro i x' hi hzero
        have := hqs.signs (i+2) x' (by omega) (by simpa using hzero)
        simpa using this
    have hd_r : ((r :: rest).head (List.cons_ne_nil r rest)).eval x0 ≠ 0 := by simpa using hr0
    have key := ih1 hqsr hd_r
    have hsc3' : signChanges [p, q, r] x0 = 1 := signChanges_sturm_triple hp0 hsigns
    filter_upwards [key, sign_eval_eventually_eq p x0 hp0, sign_eval_eventually_eq r x0 hr0]
      with x hkey hpx hrx
    simp only [splitSignChangesTotalAt, splitSignChanges, if_pos h,
      splitSignChangesTotal, List.map_cons, List.sum_cons] at hkey ⊢
    rw [hkey]
    have hsc3 : signChanges [p, q, r] x = 1 :=
      signChanges_sturm_triple (ne_zero_of_sign_eq hpx hp0)
        (by rw [hpx, hrx]; exact hsigns)
    rw [hsc3, hsc3']
  | case5 p q r rest h ih2 =>
    intro hqs hp
    have hp0 : p.eval x0 ≠ 0 := by simpa [List.head] using hp
    have hq0 : q.eval x0 ≠ 0 := fun hq0 => h ⟨hp0, hq0⟩
    have hlen : (p :: q :: r :: rest).length = (q :: r :: rest).length + 1 := by simp
    have hqsqr : QuasiSturmSeq (q :: r :: rest) := by
      refine ⟨List.cons_ne_nil q (r :: rest), ?_, ?_⟩
      · intro a b
        have h1 := hqs.last_sign_const a b
        simpa using h1
      · intro i x' hi hzero
        have := hqs.signs (i+1) x' (by omega) (by simpa using hzero)
        simpa using this
    have hd_q : ((q :: r :: rest).head (List.cons_ne_nil q (r :: rest))).eval x0 ≠ 0 := by
      simpa using hq0
    have key := ih2 hqsqr hd_q
    filter_upwards [key, sign_eval_eventually_eq p x0 hp0, sign_eval_eventually_eq q x0 hq0]
      with x hkey hpx hqx
    simp only [splitSignChangesTotalAt, splitSignChanges, if_neg h,
      splitSignChangesTotal, List.map_cons, List.sum_cons] at hkey ⊢
    rw [hkey]
    have hsc2 : signChanges [p, q] x = signChanges [p, q] x0 := by
      simp only [signChanges, List.map_cons, List.map_nil]
      rw [hpx, hqx]
    rw [hsc2]

/-- THE full general local-constancy theorem (Eberl's ultimate goal of
this section): `signChanges` is eventually constant near ANY point
`x0` that is not a root of the FIRST polynomial in a genuine
quasi-Sturm sequence -- not just points where every polynomial happens
to be individually nonzero. Combines all three split-based facts
above: `signChanges ps x = splitSignChangesTotalAt ps x0 x` (nbh
transport) `= splitSignChangesTotalAt ps x0 x0` (the fixed split's own
local constancy) `= signChanges ps x0` (the base correctness fact). -/
theorem signChanges_eventually_const (ps : List (Polynomial ℝ)) (x0 : ℝ)
    (hqs : QuasiSturmSeq ps) (hp : (ps.head hqs.ne_nil).eval x0 ≠ 0) :
    ∀ᶠ x in nhds x0, signChanges ps x = signChanges ps x0 := by
  filter_upwards [signList_eventually_eq' ps x0,
    splitSignChangesTotalAt_eventually_const ps x0 hqs hp] with x hnbh hconst
  rw [← splitSignChangesTotalAt_eq ps x0 hqs hp x hnbh, hconst,
      splitSignChangesTotalAt_self ps x0 hqs hp]

/- ============================================================
   Section 5: the sign-change DROP at a root of `p` itself (Eberl's
   `p_zero`), and the pieces it needs: dropping a `SturmSeq`'s head
   gives a `QuasiSturmSeq` of the tail (`quasi_sturm_seq_Cons`), and a
   nonzero polynomial's zero set is finite, hence isolated -- so
   `p.eval x ≠ 0` holds on a PUNCTURED neighbourhood of any point
   (root or not) of `p`.
   ============================================================ -/

/-- Dropping the head of a `QuasiSturmSeq` (length ≥ 2) still gives a
`QuasiSturmSeq` of the tail -- Eberl's `quasi_sturm_seq_Cons`, proved
inline several times above; extracted here as its own reusable lemma. -/
theorem QuasiSturmSeq.tail {p : Polynomial ℝ} {rest : List (Polynomial ℝ)}
    (hqs : QuasiSturmSeq (p :: rest)) (hne : rest ≠ []) : QuasiSturmSeq rest := by
  refine ⟨hne, ?_, ?_⟩
  · intro a b
    have h1 := hqs.last_sign_const a b
    simpa [List.getLast_cons hne] using h1
  · intro i x hi hzero
    have := hqs.signs (i+1) x (by simp; omega) (by simpa using hzero)
    simpa using this

/-- A nonzero polynomial's zero set is finite (`Polynomial.
finite_setOf_isRoot`), hence any point has a PUNCTURED neighbourhood
avoiding every OTHER root -- combined with the puncture itself
excluding `x0`, `p` is nonzero throughout a punctured neighbourhood of
ANY point (whether or not that point is itself a root). -/
theorem eventually_ne_zero_nhdsWithin (p : Polynomial ℝ) (hp : p ≠ 0) (x0 : ℝ) :
    ∀ᶠ x in nhdsWithin x0 {x0}ᶜ, p.eval x ≠ 0 := by
  have hfin : Set.Finite {x : ℝ | p.eval x = 0} := Polynomial.finite_setOf_isRoot hp
  have hfin' : Set.Finite ({x : ℝ | p.eval x = 0} \ {x0}) := hfin.sdiff
  have hopen : IsOpen ({x : ℝ | p.eval x = 0} \ {x0})ᶜ := hfin'.isClosed.isOpen_compl
  have hmem : x0 ∈ ({x : ℝ | p.eval x = 0} \ {x0})ᶜ := by simp
  have hnhds : ({x : ℝ | p.eval x = 0} \ {x0})ᶜ ∈ nhds x0 := hopen.mem_nhds hmem
  have hnhds' : ({x : ℝ | p.eval x = 0} \ {x0})ᶜ ∈ nhdsWithin x0 {x0}ᶜ :=
    mem_nhdsWithin_of_mem_nhds hnhds
  have hpunc : {x0}ᶜ ∈ nhdsWithin x0 {x0}ᶜ := self_mem_nhdsWithin
  filter_upwards [hnhds', hpunc] with x hx1 hx2
  intro hx0
  exact hx1 ⟨hx0, hx2⟩

/-- If `x` is a root of `p` (`p ≠ 0`), the number of sign changes of a
genuine Sturm sequence of `p` drops by exactly 1 crossing `x0` from
below -- Eberl's `p_zero`. Combines: `q` (the second chain element) is
nonzero at `x0` (`squarefree_pair`); the SUFFIX `q :: rest` is a
`QuasiSturmSeq` with nonzero head there, so `signChanges_eventually_
const` applies to it; `p` itself is eventually nonzero on the
PUNCTURED neighbourhood (`eventually_ne_zero_nhdsWithin`); and
`deriv_sign` pins the sign of `p*q` on each side, which -- combined
with `p`'s own now-known sign -- pins `q`'s sign relative to `p`,
hence `signChanges [p,q]` exactly (0 right at/after `x0`, 1 just
before), via `signChanges_distrib` splitting the pair off from the
rest at the shared pivot `q`. -/
theorem SturmSeq.p_zero {p : Polynomial ℝ} {ps : List (Polynomial ℝ)} (hss : SturmSeq p ps)
    {x0 : ℝ} (hx0 : p.eval x0 = 0) (hpne : p ≠ 0) :
    ∀ᶠ x in nhdsWithin x0 {x0}ᶜ,
      signChanges ps x = signChanges ps x0 + (if x < x0 then 1 else 0) := by
  obtain ⟨q, rest, hform⟩ : ∃ q rest, ps = p :: q :: rest := by
    rcases ps with _ | ⟨p', _ | ⟨q', rest'⟩⟩
    · exact absurd hss.length_ge_two (by simp)
    · exact absurd hss.length_ge_two (by simp)
    · have hp' : p' = p := by simpa [List.head] using hss.head_eq_p
      exact ⟨q', rest', by rw [hp']⟩
  subst hform
  have hq0 : q.eval x0 ≠ 0 := fun hq0 => hss.squarefree_pair x0 ⟨hx0, by simpa using hq0⟩
  have hqsr : QuasiSturmSeq (q :: rest) := hss.toQuasiSturmSeq.tail (List.cons_ne_nil q rest)
  have hqrconst := signChanges_eventually_const (q :: rest) x0 hqsr (by simpa [List.head] using hq0)
  have hderiv := hss.deriv_sign x0 hx0
  have hpnz := eventually_ne_zero_nhdsWithin p hpne x0
  have hqrconst' := hqrconst.filter_mono (nhdsWithin_le_nhds (s := ({x0}ᶜ : Set ℝ)))
  have hderiv' := hderiv.filter_mono (nhdsWithin_le_nhds (s := ({x0}ᶜ : Set ℝ)))
  have hxne := self_mem_nhdsWithin (α := ℝ) (a := x0) (s := {x0}ᶜ)
  filter_upwards [hqrconst', hderiv', hpnz, hxne] with x hqr hsgn hpx hxne0
  have hxne0' : x ≠ x0 := hxne0
  have hsgn' : SignType.sign ((p * (p :: q :: rest)[1]'(by simp)).eval x) =
      (if x > x0 then 1 else -1 : SignType) := hsgn
  simp only [List.getElem_cons_succ, List.getElem_cons_zero] at hsgn'
  rw [Polynomial.eval_mul, sign_mul] at hsgn'
  have hpsign : SignType.sign (p.eval x) ≠ 0 := by simpa [sign_eq_zero_iff] using hpx
  have hqx : q.eval x ≠ 0 := by
    intro hqx0
    have hqsign0 : SignType.sign (q.eval x) = 0 := by simpa [sign_eq_zero_iff] using hqx0
    rw [hqsign0, mul_zero] at hsgn'
    split_ifs at hsgn' <;> exact absurd hsgn' (by decide)
  have hqsign : SignType.sign (q.eval x) ≠ 0 := by simpa [sign_eq_zero_iff] using hqx
  have hsign_rel : SignType.sign (q.eval x) =
      (if x < x0 then -SignType.sign (p.eval x) else SignType.sign (p.eval x)) := by
    by_cases hgt : x > x0
    · have hnlt : ¬ x < x0 := by linarith
      simp only [hgt, if_true] at hsgn'
      simp only [hnlt, if_false]
      rcases hps : SignType.sign (p.eval x) with _ | _ | _ <;>
        rcases hqs : SignType.sign (q.eval x) with _ | _ | _ <;>
        simp only [hps, hqs] at hpsign hqsign hsgn' <;> first | rfl | (exfalso; revert hsgn'; decide)
    · have hlt : x < x0 := lt_of_le_of_ne (not_lt.mp hgt) hxne0'
      simp only [hgt, if_false] at hsgn'
      simp only [hlt, if_true]
      rcases hps : SignType.sign (p.eval x) with _ | _ | _ <;>
        rcases hqs : SignType.sign (q.eval x) with _ | _ | _ <;>
        simp only [hps, hqs] at hpsign hqsign hsgn' <;> first | rfl | (exfalso; revert hsgn'; decide)
  have hpair : signChanges [p, q] x = if x < x0 then 1 else 0 := by
    simp only [signChanges, List.map_cons, List.map_nil]
    rw [hsign_rel]
    split_ifs with hlt
    · rcases hps : SignType.sign (p.eval x) with _ | _ | _ <;>
        simp only [hps] at hpsign <;> first | rfl | exact absurd rfl hpsign
    · rcases hps : SignType.sign (p.eval x) with _ | _ | _ <;>
        simp only [hps] at hpsign <;> first | rfl | exact absurd rfl hpsign
  have hdist := signChanges_distrib (ps1 := [p]) (ps2 := rest) (p := q) (x := x) hqx
  have hcombine : signChanges (p :: q :: rest) x = signChanges [p, q] x + signChanges (q :: rest) x := by
    have := hdist; simpa using this
  have hdist0 := signChanges_distrib (ps1 := [p]) (ps2 := rest) (p := q) (x := x0) hq0
  have hcombine0 : signChanges (p :: q :: rest) x0 = signChanges [p, q] x0 + signChanges (q :: rest) x0 := by
    have := hdist0; simpa using this
  have hpair0 : signChanges [p, q] x0 = 0 := by
    simp only [signChanges, List.map_cons, List.map_nil]
    have hpsign0 : SignType.sign (p.eval x0) = 0 := by simp [hx0]
    rcases hqs0 : SignType.sign (q.eval x0) with _ | _ | _ <;> simp [hpsign0, hqs0]
  rw [hcombine, hpair, hqr, hcombine0, hpair0]
  ring

/- ============================================================
   Section 6: the main root-counting theorem. Eberl's `count_roots_
   between_aux` (locally constant at every point of an interval ⇒
   constant across it) is exactly Mathlib's `IsPreconnected.constant`
   applied to a discrete-valued (`ℕ`) function -- reused directly,
   not re-derived via Eberl's own bespoke real-analysis lemma.
   ============================================================ -/

/-- If `f : ℝ → ℕ` is locally constant (w.r.t. the subspace topology)
at every point of `Icc a b`, it is constant across the whole interval
-- Eberl's `fun_eq_in_ivl`/`count_roots_between_aux`, but via Mathlib's
`IsPreconnected.constant` (a preconnected set mapped continuously into
a discrete space is constant on it) instead of a bespoke proof: local
constancy w.r.t. `nhdsWithin` is exactly `ContinuousWithinAt` into a
discrete codomain, since `nhds` of a point in a discrete space is the
principal filter at that point. -/
theorem constant_of_eventually_locally_constant_Icc {f : ℝ → ℕ} {a b : ℝ}
    (hloc : ∀ x ∈ Set.Icc a b, ∀ᶠ y in nhdsWithin x (Set.Icc a b), f y = f x) :
    ∀ x ∈ Set.Icc a b, ∀ y ∈ Set.Icc a b, f x = f y := by
  have hcont : ContinuousOn f (Set.Icc a b) := by
    intro x hx
    have : ContinuousWithinAt f (Set.Icc a b) x := by
      unfold ContinuousWithinAt
      rw [nhds_discrete, Filter.tendsto_pure]
      exact hloc x hx
    exact this
  intro x hx y hy
  exact (isPreconnected_Icc).constant hcont hx hy


/-- Extracts a concrete `δ` from an "eventually on the punctured
neighbourhood" fact -- Eberl's `guess ε` step (`eventually_at`,
`dist_real_def` unfolded by hand). Built as its own reusable lemma,
verified in isolation, before using it in the main induction. -/
theorem exists_delta_of_eventually_nhdsWithin_compl {x0 : ℝ} {P : ℝ → Prop}
    (h : ∀ᶠ x in nhdsWithin x0 {x0}ᶜ, P x) :
    ∃ δ > 0, ∀ x : ℝ, x ≠ x0 → |x - x0| < δ → P x := by
  rw [Filter.eventually_iff, mem_nhdsWithin] at h
  obtain ⟨U, hUopen, hx0U, hUsub⟩ := h
  obtain ⟨δ, hδpos, hδball⟩ := Metric.isOpen_iff.mp hUopen x0 hx0U
  refine ⟨δ, hδpos, fun x hxne hxdist => hUsub ⟨hδball (by rwa [Metric.mem_ball, Real.dist_eq]), hxne⟩⟩

/-- The MINIMUM element of a nonempty finite set of reals, with proofs
of membership and minimality -- Eberl's `Min-in`/`Min-le` pair,
packaged together via `Finset.min'`. -/
theorem Set.Finite.exists_min {s : Set ℝ} (hfin : s.Finite) (hne : s.Nonempty) :
    ∃ m ∈ s, ∀ y ∈ s, m ≤ y := by
  have hne' : hfin.toFinset.Nonempty := by rwa [Set.Finite.toFinset_nonempty]
  refine ⟨hfin.toFinset.min' hne', ?_, ?_⟩
  · have := hfin.toFinset.min'_mem hne'
    rwa [Set.Finite.mem_toFinset] at this
  · intro y hy
    apply hfin.toFinset.min'_le
    rwa [Set.Finite.mem_toFinset]

/-- The empty-root-set case of `count_roots_between`: if `p` has no
root in `(a,b]`, `signChanges ps` agrees at `a` and `b`. Two
subcases: if `p a ≠ 0`, `p` is nonzero throughout `[a,b]` (nothing in
`(a,b]` is a root, and `a` itself isn't either), so `signChanges_
eventually_const` gives local constancy at every point, hence global
constancy via `constant_of_eventually_locally_constant_Icc`. If
`p a = 0`, approach `a` from the right (where `p_zero`'s formula gives
EXACT equality, not `+1` -- that only applies approaching from the
LEFT) to land on a point `x1 > a` where `p` is already nonzero
throughout `[x1,b]`, then apply the same constancy argument there. -/
theorem SturmSeq.count_roots_between_empty {p : Polynomial ℝ} {ps : List (Polynomial ℝ)}
    (hss : SturmSeq p ps) (hpne : p ≠ 0) {a b : ℝ} (hab : a ≤ b)
    (hempty : {x : ℝ | a < x ∧ x ≤ b ∧ p.eval x = 0} = ∅) :
    signChanges ps a = signChanges ps b := by
  have hnoroot : ∀ x : ℝ, a < x → x ≤ b → p.eval x ≠ 0 := by
    intro x hxa hxb hcontra
    have : x ∈ {x : ℝ | a < x ∧ x ≤ b ∧ p.eval x = 0} := ⟨hxa, hxb, hcontra⟩
    rw [hempty] at this
    exact this
  by_cases hpa : p.eval a = 0
  · -- Approach a from the right to x1; p is nonzero on all of [x1, b].
    have hpz := hss.p_zero hpa hpne
    obtain ⟨δ, hδpos, hδprop⟩ := exists_delta_of_eventually_nhdsWithin_compl hpz
    by_cases hab' : a = b
    · rw [hab']
    · have hab'' : a < b := lt_of_le_of_ne hab hab'
      set x1 := min (a + δ / 2) b with hx1def
      have hx1a : a < x1 := lt_min (by linarith) hab''
      have hx1b : x1 ≤ b := min_le_right _ _
      have hx1dist : |x1 - a| < δ := by
        rw [abs_of_pos (by linarith)]
        calc x1 - a ≤ (a + δ / 2) - a := by rw [hx1def]; linarith [min_le_left (a + δ/2) b]
          _ < δ := by linarith
      have hx1eq : signChanges ps x1 = signChanges ps a := by
        have := hδprop x1 (ne_of_gt hx1a) hx1dist
        rwa [if_neg (not_lt.mpr hx1a.le)] at this
      have hconst : ∀ x ∈ Set.Icc x1 b, ∀ᶠ y in nhdsWithin x (Set.Icc x1 b),
          signChanges ps y = signChanges ps x := by
        intro x hx
        have hxa : a < x := lt_of_lt_of_le hx1a hx.1
        have hxne0 : p.eval x ≠ 0 := hnoroot x hxa hx.2
        exact (signChanges_eventually_const ps x hss.toQuasiSturmSeq
          (by rw [hss.head_eq_p]; exact hxne0)).filter_mono nhdsWithin_le_nhds
      have := constant_of_eventually_locally_constant_Icc hconst x1 ⟨le_refl x1, hx1b⟩ b ⟨hx1b, le_refl b⟩
      rw [← hx1eq, this]
  · -- p a ≠ 0: p is nonzero throughout [a, b].
    have hconst : ∀ x ∈ Set.Icc a b, ∀ᶠ y in nhdsWithin x (Set.Icc a b),
        signChanges ps y = signChanges ps x := by
      intro x hx
      have hxne0 : p.eval x ≠ 0 := by
        rcases eq_or_lt_of_le hx.1 with heq | hlt
        · rw [← heq]; exact hpa
        · exact hnoroot x hlt hx.2
      exact (signChanges_eventually_const ps x hss.toQuasiSturmSeq
        (by rw [hss.head_eq_p]; exact hxne0)).filter_mono nhdsWithin_le_nhds
    exact constant_of_eventually_locally_constant_Icc hconst a ⟨le_refl a, hab⟩ b ⟨hab, le_refl b⟩

/-- Given a root `x2` of `p` and any lower bound `lb < x2`, there is a
concrete point `x1` strictly between `lb` and `x2` where `signChanges`
is exactly one MORE than at `x2` -- `p_zero`'s `+1` branch,
instantiated at a specific nearby point (not a whole interval; the
caller establishes the rest of the interval separately, typically via
the induction hypothesis itself). -/
theorem SturmSeq.exists_left_drop {p : Polynomial ℝ} {ps : List (Polynomial ℝ)}
    (hss : SturmSeq p ps) (hpne : p ≠ 0) {x2 lb : ℝ} (hx2root : p.eval x2 = 0)
    (hlb : lb < x2) :
    ∃ x1, lb < x1 ∧ x1 < x2 ∧ signChanges ps x1 = signChanges ps x2 + 1 := by
  have hpz := hss.p_zero hx2root hpne
  obtain ⟨δ, hδpos, hδprop⟩ := exists_delta_of_eventually_nhdsWithin_compl hpz
  set δ' := min δ (x2 - lb) with hδ'def
  have hδ'pos : 0 < δ' := lt_min hδpos (by linarith)
  have hδ'leδ : δ' ≤ δ := min_le_left _ _
  have hδ'lelb : δ' ≤ x2 - lb := min_le_right _ _
  refine ⟨x2 - δ' / 2, by linarith, by linarith, ?_⟩
  have hdist : |x2 - δ' / 2 - x2| < δ := by
    rw [show x2 - δ'/2 - x2 = -(δ'/2) from by ring, abs_neg, abs_of_pos (by linarith)]
    linarith
  have hlt : x2 - δ'/2 < x2 := by linarith
  have := hδprop (x2 - δ' / 2) (ne_of_lt hlt) hdist
  rwa [if_pos hlt] at this

/-- THE main theorem (Eberl's `count_roots_between`): for `p ≠ 0`, the
sign-change difference between two points is exactly the number of
roots of `p` in the half-open interval `(a,b]`. Strong induction on
that root count: empty case via `count_roots_between_empty`; nonempty
case splits at the FIRST (minimum) root `x2`, uses `exists_left_drop`
plus the induction hypothesis itself (applied to `(a,x1)`, which is
provably root-free, so IT correctly dispatches to the empty case
regardless of whether `p a = 0`) for the drop across `x2`, and the
induction hypothesis again on `(x2,b]`. -/
theorem SturmSeq.count_roots_between {p : Polynomial ℝ} {ps : List (Polynomial ℝ)}
    (hss : SturmSeq p ps) (hpne : p ≠ 0) :
    ∀ a b : ℝ, a ≤ b →
      (signChanges ps a : ℤ) - signChanges ps b =
        ({x : ℝ | a < x ∧ x ≤ b ∧ p.eval x = 0}.ncard : ℤ) := by
  suffices h : ∀ n : ℕ, ∀ a b : ℝ, a ≤ b →
      {x : ℝ | a < x ∧ x ≤ b ∧ p.eval x = 0}.ncard = n →
      (signChanges ps a : ℤ) - signChanges ps b = n by
    intro a b hab; exact h _ a b hab rfl
  intro n
  induction n using Nat.strong_induction_on with
  | _ n ih =>
  intro a b hab hn
  have hrootsfin : Set.Finite {x : ℝ | a < x ∧ x ≤ b ∧ p.eval x = 0} :=
    (Polynomial.finite_setOf_isRoot hpne).subset (fun x hx => hx.2.2)
  by_cases hex : {x : ℝ | a < x ∧ x ≤ b ∧ p.eval x = 0}.Nonempty
  · -- Nonempty: split at the minimum root x2.
    obtain ⟨x2, hx2mem, hx2min⟩ := hrootsfin.exists_min hex
    obtain ⟨hx2a, hx2b, hx2root⟩ := hx2mem
    have hleft_eq : {x : ℝ | a < x ∧ x ≤ x2 ∧ p.eval x = 0} = {x2} := by
      ext y
      simp only [Set.mem_setOf_eq, Set.mem_singleton_iff]
      constructor
      · rintro ⟨hya, hyx2, hyroot⟩
        exact le_antisymm hyx2 (hx2min y ⟨hya, le_trans hyx2 hx2b, hyroot⟩)
      · rintro rfl; exact ⟨hx2a, le_refl _, hx2root⟩
    -- Left drop: find x1 ∈ (a, x2) with signChanges ps x1 = signChanges ps x2 + 1,
    -- then delegate (a, x1) to the induction hypothesis (root-free there).
    obtain ⟨x1, hax1, hx1x2, hx1eq⟩ := hss.exists_left_drop hpne hx2root hx2a
    have hax1_empty : {x : ℝ | a < x ∧ x ≤ x1 ∧ p.eval x = 0} = ∅ := by
      ext y
      simp only [Set.mem_setOf_eq, Set.mem_empty_iff_false, iff_false]
      rintro ⟨hya, hyx1, hyroot⟩
      have hymem : y ∈ {x : ℝ | a < x ∧ x ≤ x2 ∧ p.eval x = 0} :=
        ⟨hya, le_trans hyx1 hx1x2.le, hyroot⟩
      rw [hleft_eq] at hymem
      exact absurd hymem (ne_of_lt (lt_of_le_of_lt hyx1 hx1x2))
    have hax1_card : ({x : ℝ | a < x ∧ x ≤ x1 ∧ p.eval x = 0}).ncard = 0 := by
      rw [hax1_empty]; simp
    have hlt_n : 0 < n := hn ▸ Set.Nonempty.ncard_pos hrootsfin hex
    have hax1 := ih 0 hlt_n a x1 hax1.le hax1_card
    simp only [Nat.cast_zero] at hax1
    have hsc_a_x1 : signChanges ps a = signChanges ps x1 := by
      have h1 : (signChanges ps a : ℤ) = signChanges ps x1 := by linarith
      exact_mod_cast h1
    -- Right side: (x2, b] has n - 1 roots.
    have hsplit_disj : Disjoint {x : ℝ | a < x ∧ x ≤ x2 ∧ p.eval x = 0}
        {x : ℝ | x2 < x ∧ x ≤ b ∧ p.eval x = 0} := by
      rw [Set.disjoint_left]
      rintro y ⟨_, hy1, _⟩ ⟨hy2, _, _⟩
      exact absurd hy1 (not_le.mpr hy2)
    have hsplit_union : {x : ℝ | a < x ∧ x ≤ b ∧ p.eval x = 0} =
        {x : ℝ | a < x ∧ x ≤ x2 ∧ p.eval x = 0} ∪ {x : ℝ | x2 < x ∧ x ≤ b ∧ p.eval x = 0} := by
      ext y
      simp only [Set.mem_setOf_eq, Set.mem_union]
      constructor
      · rintro ⟨hya, hyb, hyroot⟩
        rcases le_or_gt y x2 with h | h
        · exact Or.inl ⟨hya, h, hyroot⟩
        · exact Or.inr ⟨h, hyb, hyroot⟩
      · rintro (⟨hya, hyx2, hyroot⟩ | ⟨hyx2, hyb, hyroot⟩)
        · exact ⟨hya, le_trans hyx2 hx2b, hyroot⟩
        · exact ⟨lt_trans hx2a hyx2, hyb, hyroot⟩
    have hright_fin : Set.Finite {x : ℝ | x2 < x ∧ x ≤ b ∧ p.eval x = 0} :=
      (Polynomial.finite_setOf_isRoot hpne).subset (fun x hx => hx.2.2)
    have hleft_fin : Set.Finite {x : ℝ | a < x ∧ x ≤ x2 ∧ p.eval x = 0} :=
      (Polynomial.finite_setOf_isRoot hpne).subset (fun x hx => hx.2.2)
    have hcard_eq : ({x : ℝ | a < x ∧ x ≤ x2 ∧ p.eval x = 0}).ncard +
        ({x : ℝ | x2 < x ∧ x ≤ b ∧ p.eval x = 0}).ncard = n := by
      rw [← Set.ncard_union_eq hsplit_disj hleft_fin hright_fin, ← hsplit_union, hn]
    rw [hleft_eq] at hcard_eq
    simp only [Set.ncard_singleton] at hcard_eq
    have hright_card : ({x : ℝ | x2 < x ∧ x ≤ b ∧ p.eval x = 0}).ncard = n - 1 := by omega
    have hright_lt : n - 1 < n := by omega
    have hright := ih (n - 1) hright_lt x2 b hx2b hright_card
    -- Combine.
    have hn1 : (1 : ℤ) ≤ n := by exact_mod_cast Nat.one_le_iff_ne_zero.mpr (by omega)
    have hcast : ((n - 1 : ℕ) : ℤ) = (n : ℤ) - 1 := by omega
    rw [hcast] at hright
    rw [hsc_a_x1, hx1eq]
    push_cast
    linarith
  · -- Empty case.
    rw [Set.not_nonempty_iff_eq_empty] at hex
    have hn0 : n = 0 := by rw [hex, Set.ncard_empty] at hn; omega
    have hdiff := hss.count_roots_between_empty hpne hab hex
    rw [hdiff, hn0]
    simp
