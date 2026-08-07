# Web and API bibliography usage — 2026-08-07

This record lists external web sources consulted while porting Sturm's
theorem (real polynomial root counting via sign-change sequences) into
`lean/sturm_sequence_root_counting.lean`, the missing formalization piece
identified as blocking Finding 30 (arbitrary-degree Pisot classification),
the general form of Finding 29, and Finding 22's Diophantine reduction.
Canonical BibTeX entries are in `refs/references.bib`.

## Sturm's theorem, Isabelle/HOL formalization (primary reference)

**Citation key:** `EberlSturmSequences2014`

Manuel Eberl's Archive of Formal Proofs entry "A Formalisation of Sturm's
Theorem" (2014) is the template this port follows directly. Full text
downloaded 2026-08-07 (proof outline PDF, 27pp, and full session document
PDF) and archived locally per `refs/FULLTEXT_ARCHIVE.md`'s policy; see
`refs/FULLTEXT_MANIFEST.tsv` for filenames/digests.

Structure reused directly (translated from Isabelle/HOL to Lean 4/Mathlib,
not re-derived from scratch):
- `sign_changes ps x`: the number of sign changes in a list of polynomials
  evaluated at `x`, defined via filtering zero values, mapping to sign, and
  collapsing adjacent duplicates (`remdups_adj` in Isabelle) before taking
  `length - 1`. Mathlib already has the exact same "filter zero, map to
  sign, `List.destutter` to collapse duplicates, take length" pattern in
  `Polynomial.signVariations` (`Mathlib/Algebra/Polynomial/RuleOfSigns.lean`,
  Descartes' rule of signs, 2025) -- reused `List.destutter` directly rather
  than reimplementing sign-change collapsing.
- The `quasi_sturm_seq`/`sturm_seq` locale hierarchy (weakest suffix
  property vs. the full sequence definition) -- ported as two Lean
  structures/predicates with the same three defining properties.
- The auxiliary lemmas connecting sign-change locality (constant away from
  roots, changes by exactly 1 at each root, distributes over list
  concatenation) -- ported with proofs re-derived in Lean tactics
  (`nlinarith`/`linarith`/case analysis) since Isabelle's `auto`/`simp`
  proof terms do not translate directly; the STATEMENTS and proof
  STRATEGY (case split on sign at three adjacent points, IVT for existence
  of intermediate roots) are taken from the paper, not reinvented.
- The canonical Sturm sequence construction via the polynomial remainder
  sequence (`sturm_aux p q = if degree q = 0 then [p,q] else p :: sturm_aux
  q (-(p mod q))`), matching `math/src/exact_pisot.c`'s own Sturm-chain
  construction (`sturm_chain_build`, confirmed independently identical in
  shape before this port started).
- The main theorem `count_roots_between` (sign-change difference between
  two points equals the number of roots in the half-open interval) --
  statement ported directly; Mathlib's own IVT (`intermediate_value_Icc`
  family), polynomial continuity (`Polynomial.continuous`), and
  eventually-nonzero-near-a-point machinery are used in place of Isabelle's
  bespoke `poly_neighbourhood_without_roots` etc. (Section 1.2.4/1.2.6 of
  the outline), since Mathlib already has these as standard topology/
  analysis lemmas -- not reimplemented.

**2026-08-07 update -- Sections 1-4 complete (the general local-
constancy theorem), zero `sorry`, kernel-checked.** Went back to the
FULL proof document (`2014_Eberl_Sturm-sequences-formalisation-
document.pdf`, not just the statement-only outline) after an initial
attempt at a scoped-down "all polynomials nonzero" special case was
explicitly rejected in favour of the real argument. Ported directly,
following the actual proof scripts (not reconstructed blind):
`sign_changes_distrib` (needed a NEW Mathlib-level structural lemma,
`List.destutter'_append`, not previously in Mathlib -- destuttering a
concatenation drops the trailing "current seed" marker from the
prefix contribution, proved by induction after initially getting the
`destutter'` recursion direction backwards and correcting via the
actual Mathlib source); the `QuasiSturmSeq`/`SturmSeq` structures;
`splitSignChanges` (Eberl's `split_sign_changes`, a well-founded
recursive peeling function, using Lean's auto-generated
`splitSignChanges.induct` equation-compiler induction in place of
Eberl's hand-written `split_sign_changes_induct`); and all three
supporting facts needed for full local constancy --
`splitSignChangesTotalAt_self` (`split_sign_changes_correct`),
`splitSignChangesTotalAt_eq` (`split_sign_changes_correct_nbh`), and
`splitSignChangesTotalAt_eventually_const` (the fixed split's own
local constancy in the evaluation point -- needed to correctly CHAIN
the other two into genuine local constancy; an initial attempt to
combine just the first two directly was circular and had to be
corrected). Final theorem `signChanges_eventually_const`: `signChanges`
is eventually constant near any point that is not a root of the FIRST
polynomial in a genuine quasi-Sturm sequence -- the real hypothesis
Eberl uses, not the artificially strong "every polynomial nonzero"
version. `math/src/exact_pisot.c` correspondence confirmed: the
recursion's invariant (current head nonzero at the split point,
maintained by the `signs` field forcing the third element of a
3-window nonzero whenever the second is a root) matches exactly how
`sturm_chain_build`'s Sturm-chain construction is used downstream.

## Budan-Fourier theorem, Isabelle/HOL formalization (secondary reference)

**Citation key:** `LiPaulson2018BudanFourier`

Wenda Li and Lawrence C. Paulson, "Counting Polynomial Roots in
Isabelle/HOL: A Formal Proof of the Budan-Fourier Theorem" (CPP 2018,
arXiv:1811.11093). Consulted for context on extending Sturm's theorem to
count roots WITH multiplicity -- not needed for the initial port, since
`exact_pisot.c`'s classifier already requires squarefree input
(`poly_is_squarefree` checked before Sturm-chain construction begins), so
the ported Lean development targets the squarefree/distinct-roots case
Eberl's original formalization already covers. Flagged here as the natural
next reference if a future multiplicity-aware extension is needed. Full
text downloaded and archived locally (see `refs/FULLTEXT_MANIFEST.tsv`).

## Mathlib sign-variation infrastructure (reused, not reimplemented)

**Citation key:** none (Mathlib API, not a paper) -- documented here per
the "don't reinvent a wheel we already have" instruction.

`Mathlib/Algebra/Polynomial/RuleOfSigns.lean` (Descartes' Rule of Signs,
Alex Meiburg, 2025) already provides `Polynomial.signVariations` using
exactly the `List.destutter`-based sign-collapsing technique this port
needs for `signChanges`. Reused `List.destutter` directly instead of
reimplementing sign-collapsing from first principles; the Sturm-sequence
version differs from `signVariations` only in operating on a list of
polynomial VALUES at a point rather than a list of COEFFICIENTS, so the
underlying list-processing lemma cannot be called directly but the
technique and several destutter-manipulation lemmas transfer.

## Documentation rule

Per `docs/WEB_AND_API_BIBLIOGRAPHY_USAGE_2026-08-03.md`'s established
convention: add canonical entries to `refs/references.bib`, preserve full
text in `refs/FullText/` with a `refs/FULLTEXT_MANIFEST.tsv` row, record
exactly which theorem/section informed which Lean declaration, and prefer
an already-available stronger project-local or Mathlib theorem over
reimplementing a generic one from an external source.
