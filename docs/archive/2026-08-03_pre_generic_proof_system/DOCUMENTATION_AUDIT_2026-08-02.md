# Documentation audit — 2026-08-02

Requested by AM during the work/NetHack cadence session: "put together
a full documentation audit, and cite the theorems of the project where
appropriate in the work." This file records what was actually checked,
what was fixed, and what remains flagged rather than fixed. It is a
record of the audit itself, not a replacement for the documents it
covers -- read `THEOREM_STATUS.md` and `FINDINGS_FOR_CITATION.md` for
current claim strength, not this file.

## Scope

Covers all 24 files directly under `docs/` (`docs/archive/` and
`docs/notes/` excluded as historical, not live reference). Checked for
three things:

1. **Staleness**: claims that were true when written but have since
   been superseded, without the superseding doc/commit being linked
   back.
2. **Structural integrity**: do file paths, function names, and
   doc-to-doc cross-references named in prose actually exist.
3. **Citation coverage**: is every claim of mathematical substance
   traceable to a numbered Theorem/Finite proposition
   (`THEOREM_STATUS.md`) or Finding (`FINDINGS_FOR_CITATION.md`), and
   do those canonical documents in turn point back to where the claim
   lives in detail.

## Citation index (built first, used throughout)

`THEOREM_STATUS.md`: Theorem 1 (return/contact lift projection, kernel
checked), Theorem 2 (sparse-exploration resource bound, paper proof),
Theorem 3 (free-involution Perron descent, kernel checked conditional),
Theorem 4 (n-bonacci correction determinant, paper proof + partial
kernel check), Theorem 5 (Class-II BP matrix, paper proof + exact check
`a<=64`), Theorem 6 (Class-II contact matrix, paper proof + exact check
`2<=a<=6`); Finite proposition 1 (labelled-lift factor comparison),
Finite proposition 2 (endpoint-phase lift zero modes), Finite
proposition 3 (literature-correct Class-II corona layers); plus a
Formalization queue covering the fixed-light neighbor matrix theorem
and several kernel-checked lemmas.

`FINDINGS_FOR_CITATION.md`: Findings 1-11 pre-existing (12-EXPLODED
pipeline, n-bonacci `ρ_nc=λ(G_B)`, Class-II dominant cores, Thread A4/A5
refutations and results, the BP core family, Item A/B involution work).
**Findings 12-14 added by this audit** (see below) -- the global
occurrence base-premises seam closure, recurrent-SCC exhaustion, and
tonight's A4/birth-round strengthening had no citable Finding number
before this audit despite being among the project's most substantial
recent closures.

## Fixes applied

### Stale-pointer pattern (found four times total, three by this session before the audit, one new)

A recurring, specific failure mode: a doc or code comment states "X is
open" or "not yet attempted," is true at the time, and then a *later*
file or commit closes X -- but the original statement is never updated
to point at the closure. Every instance found was a genuine claim that
was correct when written, not a fabrication; the bug is the missing
forward link, not the original text.

1. `app/class_ii_neighbor2_recurrent_exhaustion_check.cpp` (fixed
   pre-audit, `d7e474a`): claimed items 3/4 of recurrent-SCC exhaustion
   were "genuinely open," written 19:59 2026-07-31; closed 46 minutes
   later the same night by `class_ii_neighbor2_round_stratified_
   transient_check.cpp`. Fixed with a dated correction citing the
   closing file.
2. `docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s "Next
   implementation" section (fixed pre-audit, `9af0027`): described
   Round 1's reverse-inclusion as the "next real mathematical step, not
   yet attempted," written before the same-night literature-check
   closure. Fixed with a correction note preserving the original
   investigation history (including a real dead end) rather than
   rewriting it away.
3. `docs/RESEARCH_STATUS.md` (found and fixed by this audit): stated
   Round 1's reverse-inclusion "remains open," same stale claim as #2,
   independently drifted in a second document. Fixed with the same
   correction, cross-referencing #2's account.
4. `include/ravel/class_ii_neighbor2_pruning.hpp`'s
   `class_ii_neighbor2_base_round_premises` (flagged, not fixed,
   pre-audit `9b2351e`): the actual proof-tracking *code* -- not just a
   comment -- still hardcodes all four base rounds as open, dated
   "2026-07-30," predating both the Round 1 and Round 4 closures. This
   is load-bearing (an enrolled test asserts the stale values). Left
   flagged rather than corrected: the two closures used arguments
   outside this function's specific technical definition of "reverse
   inclusion," and getting the fix wrong would introduce a false
   `closed` into exactly the infrastructure this project is most
   careful about. Needs a dedicated session to verify precisely before
   touching.

**Lesson for future sessions**: when a fix lands in file B that
contradicts a claim in file A, check whether A's own text points
forward to B. It usually doesn't, because nothing forces that link.
Worth a standing habit: after closing a "genuinely open" item, grep for
the closed claim's own prior phrasing elsewhere in `docs/` and
`include/`, not just update the one file where the work happened.

### Citation gap: three major results had no Finding number

Findings 12, 13, and 14 added to `FINDINGS_FOR_CITATION.md`
(`0252179`), covering:

- **Finding 12**: the global occurrence theorem's four-round
  base-premises seam, closed for every `a>=7` (Round 1 for every
  `a>=3`). Previously described only in `GLOBAL_CATALOGUE_
  OCCURRENCE_EXHAUSTION.md` and `RESEARCH_STATUS.md`, never given a
  citable Finding number despite being a multi-day closure with real
  commits (`1fda87c`, `1f82dbd`, and the round 2/3 closures).
- **Finding 13**: recurrent-SCC exhaustion items 1-5, verified together
  at `a` in `{7,...,20,30}` (`c17e879`, `d28c66b`), plus this session's
  round-by-round strengthening.
- **Finding 14**: Thread A4's persistent non-AR branching profile,
  strengthened from `n=60000` to `n=500000` this session, plus the
  (not yet promoted) observation that the constant profile does not
  generalize to `sigma_{0,2}`/`sigma_{0,3}`.

Two-way citations added: `THEOREM_STATUS.md` and `GLOBAL_CATALOGUE_
OCCURRENCE_EXHAUSTION.md` now point to Finding 12/13 where they discuss
the underlying results; `DIRECTION_AND_OPEN_THREADS.md`'s Thread A4
section points to Finding 6.7/14. Before this audit, the Findings
document and the Theorem/status documents were two citation graphs
that never referenced each other.

## Structural integrity: checked clean

- Every `app/*.cpp` file named in a doc (91 distinct references) exists
  except one placeholder example (`app/foo.cpp`, explicitly a
  hypothetical in `CPP_DESIGN_PHILOSOPHY.md`/`LUA_USAGE.md`, not a real
  broken link).
- Every `include/ravel/*.hpp` file named in a doc (37 distinct
  references) exists except two placeholders: `include/ravel/foo.hpp`
  (same hypothetical pattern) and `include/ravel/gkw.hpp`, which
  `GKW_SCRIPTS_NOTE.md` explicitly describes as a *future* location
  ("If/when the GKW work grows into a reusable... tool, it would move
  to `include/ravel/gkw.hpp`... Today it isn't there") -- correctly
  self-documented as not existing, not a stale claim.
- Every `docs/*.md` file named by another doc (18 distinct references)
  exists except `docs/RESEARCH_VECTORS.md`, which
  `DIRECTION_AND_OPEN_THREADS.md` already flags as checked-nonexistent
  as of 2026-07-31 (a prior version of that note pointed to it; the
  current text documents the correction rather than repeating the
  broken pointer).

No broken structural references were found beyond what the docs
already self-correct.

## Flagged, not fixed (deliberately)

- `include/ravel/class_ii_neighbor2_pruning.hpp`'s induction ledger
  (above) -- needs careful verification of exact technical scope before
  editing load-bearing proof-tracking code.
- `docs/CLASS_II_NEIGHBOR2_TRANSPORT_THEOREM.md` (self-labeled
  "Statement under construction," last touched 2026-07-31 04:00,
  predating the night's major closures): contains "Exclusion remains
  open" in a context (Red-exclusion for `r>=4` interior rounds) that
  may or may not be the same "exclusion" already closed elsewhere for
  base rounds 2-3. Ambiguous without deeper study; not edited to avoid
  a wrong claim in either direction.
- `docs/PYTHON_MIGRATION_INVENTORY.md`: states "40 live `.py` sources
  as of 2026-07-28"; actual count is now 42. Minor engineering-doc
  drift, not a mathematical claim; not reconciled file-by-file given
  the audit's time budget.

## What this audit did not cover

`docs/archive/` and `docs/notes/` (explicitly historical, not live
reference per `README.md`'s own description). The `.ravel/` continuity
tree (a separate repository with its own versioning policy -- out of
scope for a `RavelMathPub` documentation audit). Line-by-line reading
of every doc; the staleness sweep was pattern-based (grep for dated
"as of," "genuinely open," "not yet attempted," "TODO," "remains open"
markers, then individually verified each hit against current state),
which catches the specific failure mode this session kept encountering
but is not a guarantee against every possible drift.
