# Supergolden three-generator word intertwiner — kernel-checked checkpoint

Date: 2026-08-05

## Status

**Lean kernel checked.**

- `lean/generated/supergolden_three_generator_intertwiner.lean`
- SHA-256: `48fcbe7eca1b1694b23ce6b5d5f8d39093319203ff66d056adeaf31e7152ffd1`
- Checked with `scripts/safe_lean_check.sh` (fixed this session — see
  `docs/THEOREM_STATUS.md`'s 2026-08-05 top note) against Lean 4.32.1 /
  Mathlib pinned by the project's ridealong bundle. Clean build, no errors.

## What is proved

`RavelGenerated.supergolden_word_intertwiner`: in any preordered monoid,
three base generator inequalities (`neutral`, `positive`, `negative`)
between a "boundary" assignment `GB` and a "universal" assignment `GU`,
transported by a fixed element `P`, extend by induction to every finite
word over the three generators. Same statement and proof shape as the
already-checked `RavelGenerated.plastic_word_intertwiner`
(`lean/generated/plastic_three_generator_intertwiner.lean`) — the
combinatorial fact does not depend on which Pisot substitution produced the
base inequalities, only on there being exactly three of them.

## What supplies the base inequalities

`ravel::proof::derive_supergolden_three_generator_intertwiner()`
(`include/ravel/proof/supergolden_three_generator_intertwiner.hpp`) computes
them concretely and exactly for the supergolden number (dominant root of
`x^3-x^2-1`):

- substitution `0->01, 1->2, 2->0`, derived (not copied) via
  `include/ravel/canonical_beta_substitution.hpp` from the exact greedy
  beta-expansion of 1 (`1 0 1`) — see
  `docs/SUPERGOLDEN_QRS_CLOSURE...` / `THEOREM_STATUS.md`'s 2026-08-05
  entries;
- concrete contact-boundary graph: 20 boundary states, 25 boundary edges
  (`ravel::compute_contact_boundary_from_subst<3>`);
- universal 9-role parent-pair catalogue: 16 edges, split 10/3/3 across the
  neutral/positive/negative generators;
- displacement-forgetting role projection `P` (0/1 matrix, boundary states
  to the 9 role states) checked to be exact;
- all three simultaneous comparison inequalities `G_B P <= P G_U` checked
  directly, not assumed;
- fed into the theorem-neutral `ravel::proof::derive_finite_positive_grammar_majorant`
  generic operation, which also reports `proved = true`.

`tests/supergolden_three_generator_intertwiner_test.cpp` is the executable
regression; `tests/supergolden_qrs_test.cpp` is the substitution-level
closure regression it builds on.

## Scope boundary

This proves the word-closure consequence of three already-established base
inequalities, exactly as the plastic checkpoint does. It does not by itself
establish spectral dominance, norm-weighted twists, or the cyclotomic-sector
audit (moot here at the raw-substitution level: the incidence polynomial
already equals `x^3-x^2-1` exactly, no cofactor). Full contact-boundary
*enumeration completeness* (as opposed to the bounded computation actually
run, capped well above the 20 states/25 edges actually found) is not
separately certified; the caps used (`closure_cap=20000`,
`corona_cap=100000`, `max_corona_rounds=10`) were not approached.
