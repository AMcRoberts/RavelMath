# Mathlib backlog work list

Written 2026-08-07, after the Sturm PR pair (mathlib4#42558, #42559) and a
full audit of RavelMathPub's other near-term Mathlib-candidate material
against current Mathlib source and open PRs. See
`MATHLIB_BACKLOG_READINGLIST_2026-08-07.md` for the source pointers behind
every claim below — this file is the plan, that file is the evidence.

## Active now (this session's assignment)

**#1 — Matrix erase-index cleanup (Ravel-internal, not a Mathlib PR).**
`lean/Ravel/Matrix/EraseIndex.lean` fully duplicates `Fin.succAbove` +
`Matrix.submatrix`, with strictly worse ergonomics (subtraction-indexed
`Fin n → Fin (n-1)` instead of Mathlib's successor-indexed
`Fin (n+1) → Fin n`, which is exactly why the file fights `omega`/`Nat.sub`
friction its own comments complain about). There is no Mathlib gap here —
the task is refactoring Ravel's own code to use the existing Mathlib
primitives directly, deleting the parallel reimplementation. This needed a
clean session because the prior session's context was saturated with
`Mathlib` source after building and auditing the Sturm PRs — not because
the task itself requires independence from Mathlib. Read Mathlib source
freely for this one; it's a refactor, not a derivation.

**#3 — Free-involution Perron descent (genuine clean-room, active
collision).** RavelMathPub already has substantial, largely self-contained
Perron-Frobenius existence/uniqueness work
(`lean/free_involution_perron_existence_draft.lean`,
`lean/free_involution_perron_core.lean`) — closer to complete than the
original backlog description suggested. But there is a large, active,
in-flight Perron-Frobenius formalization effort on mathlib4 by a different
contributor (mkaratarakis, with or4nge19), ~9 open PRs, started 2026-05-27,
still open as of this writing. This is a genuine clean-room situation:
- Read their PR *titles, descriptions, and theorem statements* to know
  what's covered and what gaps remain -- this is public information about
  what results exist, fair to use.
- Do NOT read their actual Lean proof scripts and adapt them. If Ravel's
  own proof of an overlapping result needs fixing, fix it from the
  mathematics (standard Perron-Frobenius theory, any linear algebra
  textbook treatment) and from Ravel's own existing draft, not from their
  implementation.
- The three remaining `sorry`s in `free_involution_perron_existence_draft
  .lean` (simplex convexity, simplex closedness, `mulVec` linearity) look
  like standard facts likely already provable from existing Mathlib
  lemmas directly -- close these first, independent of the other
  contributors' work entirely.
- `perron_exists_of_irreducible` in that file takes `BrouwerFixedPoint` as
  an explicit hypothesis, not a proof -- it's honestly gated on Brouwer's
  fixed point theorem, which is itself a separate open Mathlib PR
  (mathlib4#36770, still open). Do not try to prove Brouwer's theorem from
  scratch; either wait for #36770, or keep the explicit-hypothesis shape
  Ravel already chose (it's the honest, non-circular choice already made).
- Once the gap between Ravel's own draft and the other contributors' ~9
  PRs is actually mapped (not assumed), the goal is filling in whatever
  Ravel has that they're missing, submitted gently: coordinate with
  mkaratarakis/or4nge19 rather than opening a competing PR blind. Check
  for a Zulip thread or PR comments describing their own remaining scope
  first; a PR description or a Zulip message asking whether a specific
  gap is still open is the good-citizen move here, not silently duplicating
  work already in review.

## Deferred, below #3, in this order

4. `perron_column_difference` (general form) -- fold in as a supporting
   lemma of whatever #3 becomes; too small (4 lines) for its own PR.
5. `substitution_iteration_infrastructure.lean` -- generalize `Fin d` to an
   arbitrary alphabet type, rename off project-specific terms. Reference:
   Pytheas Fogg, *Substitutions in Dynamics, Arithmetics and Combinatorics*
   (Springer LNM 1794), for terminology/convention alignment only -- the
   math itself is already correct and general.
7. `period_rotation_forces_equal_modulus.lean` -- already general spectral
   graph theory (`Fin n`, no n-bonacci-specific content); mostly a
   naming/API pass, not new math.
8. `depressed_cubic_complex_pair_modulus.lean` -- extend
   `Mathlib.Algebra.CubicDiscriminant` with this file's complex-pair-modulus
   content. Elementary, self-contained, no external papers needed.
2. Cycle/permutation-matrix charpoly -- (a) trivial: wire in
   `AdjoinRoot.powerBasis_dim` (already exists) to close the file's own
   flagged "dimension-transport theorem" gap; (b) real but bounded: a
   bridge theorem connecting the abstract companion matrix to a concrete
   0/1 cycle-adjacency matrix.

Item 6 (periodic-point iteration) was **dropped entirely** from the
backlog -- confirmed already fully covered by
`Mathlib.Dynamics.PeriodicPts.Defs` (`IsPeriodicPt.mul_const`/`const_mul`).

## Process reminders that apply to all of the above

- Check current Mathlib source and open PRs before assuming a gap exists
  -- every single item above turned out smaller or different in shape
  than the original backlog description, because that description was
  written before this audit pass actually read the files.
- A citation-honesty lesson from the Sturm PRs applies here too: if a
  result is structurally derived by reading someone else's formalization
  (Isabelle, another Mathlib PR, anything), cite that source explicitly.
  If it's independently derived from the mathematics, say so, and mean it.
- Zero `sorry`, ever, in anything that claims to be done.
