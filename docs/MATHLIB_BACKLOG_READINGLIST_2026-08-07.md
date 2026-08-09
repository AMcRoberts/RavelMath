# Mathlib backlog reading list

Companion to `MATHLIB_BACKLOG_WORKLIST_2026-08-07.md`. Read that first for
the plan; this file is where each claim in it comes from. Read in order.

## 0. Orientation (read first, every session)

- `docs/DOCUMENTATION_INDEX.md` for the work-repository reading route.
- `docs/REFLECTION_RETROFIT_PLAN.md` -- the reflection-pipeline discipline
  this whole project runs on (concrete C++ evidence -> typed trace ->
  mechanically rendered Lean), and the citation-pattern mistake caught and
  fixed in it. The same discipline about not hand-authoring proofs applies
  to anything transported to Mathlib.
- `docs/PROVENANCE_AND_DEPENDENCIES.md`-style thinking: see
  `~/ravel/ToStdMathlib/PROVENANCE_AND_DEPENDENCIES.md` from the Sturm
  port (if present in this package) for the exact shape of a provenance
  writeup that survives an honesty audit -- distinguishing "translated
  from a specific named source" from "independently derived" and getting
  that distinction *right*, not just asserted.

## 1. Item #1 -- matrix erase-index cleanup

Ravel source (the file to refactor away):
- `lean/Ravel/Matrix/EraseIndex.lean` (101 lines) -- read the whole thing,
  it's short. Contains `last`, `lastPred`, `skip`, `skip_injective`,
  `minor`, `minorShift`.

Mathlib equivalents (in a real Mathlib checkout, e.g. clone
`leanprover-community/mathlib4` fresh if this package didn't include one):
- `Mathlib/Data/Fin/SuccPred.lean` -- `Fin.succAbove`, `succAbove_right_
  injective`, and the surrounding case-split lemma set (`succAbove_of_
  castSucc_lt`, etc.) -- this is `skip`, done properly, with far more
  supporting lemmas already proven.
- `Mathlib/Data/Fin/Embedding.lean` -- `succAboveEmb` (the injective
  embedding form).
- `Mathlib/LinearAlgebra/Matrix/Defs.lean` -- `Matrix.submatrix` and its
  own lemma set (`submatrix_submatrix`, `submatrix_map`, etc.) -- this is
  `minor`, done generally (arbitrary reindexing functions, not just
  deletion).
- `Mathlib/LinearAlgebra/Matrix/Adjugate.lean`, header comment -- Mathlib's
  own explicit design note about why it avoids deletion-minors in favor of
  a row-replacement trick. Worth understanding before assuming
  deletion-minors are the "right" primitive to standardize on internally.

Task: find every call site of `Ravel.Matrix.EraseIndex.*` in RavelMathPub
(`grep -rln "EraseIndex\." --include=*.lean --include=*.cpp --include=*.hpp
.`), replace with the Mathlib primitives above, delete the file, confirm
nothing regresses (`make check` / `make lean-check` as appropriate).

## 2. Item #3 -- free-involution Perron descent

Ravel source (read both fully -- the second is long but the header
comment is an accurate map of what's proven vs. what's `sorry`d):
- `lean/free_involution_perron_core.lean` (175 lines) -- `PerronUnique`,
  `perron_eigenvector_is_phi_invariant`, `quotient_perron_eigenequation`.
  No `sorry`.
- `lean/free_involution_perron_existence_draft.lean` (674 lines) -- the
  larger draft. `perron_unique_of_irreducible` is fully proved. Three
  `sorry`s remain (lines ~529, ~540, ~549 as of this writing -- search for
  `sorry` to get current line numbers): standard-simplex convexity,
  standard-simplex closedness, and linearity of `Matrix.mulVec`. Also
  contains `BrouwerFixedPoint` (an explicit `Prop`-valued hypothesis, not
  a proof -- read its docstring, which cites Milnor 1978 and a specific
  external mechanization) and `perron_exists_of_irreducible`, which
  consumes that hypothesis honestly rather than asserting Brouwer's
  theorem for free.

Perron-Frobenius already in Mathlib (checked 2026-08-07, before any of
the below merges):
- `Mathlib/LinearAlgebra/Matrix/Stochastic.lean` -- `rowStochastic`,
  `colStochastic`. Existing, unrelated to the open PRs below.
- Nothing else -- no `PerronFrobenius` directory, no positive/nonnegative
  eigenvector uniqueness theorem, existed in mainline Mathlib as of
  2026-08-07.

The open PRs (all by mkaratarakis, co-authored with or4nge19, opened
2026-05-27, all OPEN as of 2026-08-07 -- **check current status before
doing anything**, this list will go stale):

| PR | Title |
|---|---|
| leanprover-community/mathlib4#39914 | Perron-Frobenius spectral lemmas (`LinearAlgebra/Matrix/Spectrum`) |
| #39915 | complex-analytic lemmas for Perron-Frobenius (`Analysis/CStarAlgebra`) |
| #39916 | paths for Perron-Frobenius (`Combinatorics/Quiver/Path`) |
| #39918 | core lemmas for non-negative matrices (`PerronFrobenius`) |
| #39920 | Perron-Frobenius for primitive matrices (`PerronFrobenius`) |
| #39921 | uniqueness of the Perron eigenvector (`PerronFrobenius`) -- depends on #39920 |
| #39922 | Perron-Frobenius for irreducible matrices (`PerronFrobenius`) -- depends on #39918, #39921 |
| #39925 | simplicity of the Perron root (`PerronFrobenius`) |
| #39927 | Perron-Frobenius for column-stochastic matrices (`PerronFrobenius`) |

Also relevant, separate open PR:
- leanprover-community/mathlib4#36770 -- "invariance of domain via
  Brouwer's fixed point theorem." This is what `BrouwerFixedPoint` in
  Ravel's draft is waiting on. Check its status before doing anything with
  `perron_exists_of_irreducible`.

For each PR above: read the **title, description, and the theorem
*statements*** (via `gh pr diff <n> --repo leanprover-community/mathlib4`,
looking only at signatures and docstrings, not proof bodies) to build an
accurate map of what's covered. Do not read their proof scripts and adapt
them -- see the clean-room note in the work list. If a signature can't be
understood without its proof, that's a sign to derive the fact
independently from the mathematics, not to read further into their code.

## 3. Deferred items -- pointers only, do not start without re-confirming priority

- Item 2 (cycle/permutation charpoly): `lean/cycle_charpoly_campaign_
  validation.lean`, `lean/graph_cycle_charpoly_factor_validation.lean`.
  Mathlib: `Mathlib/RingTheory/AdjoinRoot.lean`'s `powerBasis_dim`,
  `Mathlib/LinearAlgebra/Matrix/Charpoly/Basic.lean`'s
  `charpoly_fromBlocks_zero₂₁` (already used).
- Item 4: `lean/perron_column_difference.lean`, theorem
  `perron_column_difference` specifically (the other ~10 theorems in that
  file are n-bonacci-specific and stay internal).
- Item 5: `lean/substitution_iteration_infrastructure.lean`.
- Item 7: `lean/period_rotation_forces_equal_modulus.lean`.
- Item 8: `lean/depressed_cubic_complex_pair_modulus.lean`. Mathlib:
  `Mathlib/Algebra/CubicDiscriminant.lean` (the file to extend).

## Sturm PRs, for reference on what "done" looks like

- mathlib4#42558 (abstract Sturm theory) and #42559 (certified PRS
  bridge, stacked on #42558) -- the two-file split, the citation
  discipline in each file's `## References` section, and the terse PR
  body vs. the fuller staging notes are all worth reusing as a template
  for whatever comes out of items #1 and #3.
