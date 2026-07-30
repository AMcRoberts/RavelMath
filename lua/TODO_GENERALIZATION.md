# TODO — generalizing the backend beyond alphabet size 3

Written originally to address the W13 caveat (n≥4 matrices silently
failed `classify_matrix_spectral`); all engineering items 1–6 are
now DONE, item 7 is still open.  This document tracks what changed
and what's still left, and reflects the post-`include/`+`src/`+`app/`+
`tests/`+`math/` reorganization layout.

## The headline finding (historical, 2026-07)

Most of the backend was already properly general. `Substitution<d>`
(`include/ravel/core.hpp`), `RauzyFractal<d>`
(`include/ravel/rauzy_fractal.hpp`), the graph-geometry side
(`include/ravel/ambient_graph.hpp`, `corona.hpp`,
`contact_boundary.hpp`, `d_cont_check.hpp`, `faces.hpp`) are all
templated on alphabet size `d` and don't assume 3 letters anywhere.
`charpoly_int` (`include/ravel/barge.hpp`) is also fully general —
it uses Faddeev-LeVerrier/Newton's identities, works for any `n`.

The gap was narrow but load-bearing: `spectral.hpp` only had
closed-form eigenvalue solvers for n=2 and n=3
(`spectral_invariants_2x2`, `_3x3`, via direct quadratic/Cardano
formulas), and `survey.hpp::classify_matrix_spectral` silently
returned a null result (`beta=0, pisot=false`) for any matrix that
isn't 2x2 or 3x3 — no error, no fallback, just a quiet failure:

```cpp
} else {
    return out;   // out is default-constructed: beta=0.0, pisot=false
}
```

That deferral made sense when the project's active surface was
3-letter substitutions (Tribonacci, plastic, Penrose, etc). It no
longer matched reality: rnd13, sigma_1, sigma_2 — the things actually
being worked on — are all 4-letter, and none of them could go
through the shared classification path.

## Status update — items 1–4 applied

All four verified by rebuilding **from scratch** and rerunning every
non-Lua test binary: `d_cont_check_test` (6/6),
`ambient_graph_test` (7/7), `corona_test` (8/8),
`contact_boundary_test` (31/31), `exact_pisot_test` (38/38) — all
pass, identical to their pre-change results, so nothing downstream
regressed.  (Test counts updated for the post-reorganization layout
in §"Current test count" below.)

- **Item 1+2 (done):** `spectral_invariants_general` added to
  `include/ravel/spectral.hpp` (power iteration for beta,
  Wielandt deflation + norm-growth-rate for beta2 — works whether
  the second eigenvalue is real or a complex-conjugate pair).
  `classify_matrix_spectral` in `include/ravel/survey.hpp` now
  calls it for n >= 4 instead of `return out;`.  Regression test
  `tests/spectral_general_test.cpp` (9/9 passing) checks it
  against the known Tribonacci/Tetrabonacci/rnd13 values and
  includes a negative case (a 4-cycle permutation matrix, correctly
  rejected as non-Pisot) so the fix can't silently regress into
  "always says yes."
- **Item 3 (done, 2 of 3):** `app/cylinder_measure.cpp`'s private
  power-iteration `Subst` constructor and
  `app/contact_boundary_4x4.cpp`'s private `compute_beta` both now
  call the shared `spectral_invariants_general` /
  `classify_matrix_spectral` instead of their own copies.  Both
  re-verified to produce byte-identical beta values to before
  (1.927562 / 5.623559).  `app/qbeta_eigenvalue.cpp` is kept
  alone — it does exact Q(β) arithmetic for precision at large
  boundary-graph sizes, a genuinely different tool, not just a
  fourth duplicate; the header `include/ravel/qbeta_eigenvalue.hpp`
  documents when to reach for it vs. the double-precision general
  path.
- **Item 4 (done):** `app/cylinder_measure.cpp` is now a real
  top-level `Makefile` target, built and run as part of
  `make cylinder_measure` and `make all`.  `spectral_general_test`
  is built and run as part of `make tests` and `make check`.

## Status update — items 5, 6 applied (in a follow-up audit pass)

- **Item 5 (done):** The `dump_gb_matrix` → `qbeta_eigenvalue`
  pipeline is now wired end-to-end in
  `app/contact_boundary_4x4.cpp`.  When the binary is invoked
  with a `matrix_path` argument, it dumps the G_B adjacency
  matrix to that file and then runs
  `ravel::qbeta_dominant_eigenvalue_4` on it in-process (using
  `charpoly_int` for the char poly, then exact Q(β) power
  iteration).  The output prints the exact λ(G_B) alongside the
  double-precision `boundary_eigenvalue` from the pipeline, so
  the two can be compared directly.  The
  `lua/scripts/run_qbeta_survey.sh` wrapper still exists (it
  loops over the 11 clean non-unimodular candidates), but each
  invocation is now a single binary call rather than a
  dump-then-invoke pattern.  Known minor arg-order bug in the
  shell script documented in the Makefile comment for the
  `qbeta_eigenvalue` target.
- **Item 6 (done):** `compute_contact_boundary_dispatch` and
  `compute_contact_boundary_from_subst_dispatch` (in
  `include/ravel/contact_boundary.hpp`) used to support only d
  in {2, 3, 4} via a hard-coded switch statement; they're now
  recursive templates that walk d from `MAX_DISPATCH_D = 8` down
  to 2, dispatching to the matching `compute_contact_boundary<d>`
  instantiation via `if constexpr`.  The random Pisot survey
  `random_pisot_survey` in `include/ravel/survey.hpp` got a
  `default_K_for_n(n)` heuristic that picks the matrix entry
  range K based on alphabet size (K=4 for n<=4 to preserve the
  3-letter canonical regime; K=n+1 for n>=5 to keep the
  primitive-Pisot density workable at higher d).  A new
  `tests/survey_test.cpp` (34 tests, `make survey_test`) covers
  d=2..7 and the `default_K_for_n` heuristic.

## Status update — Sturm bug, σ_2 matrix, weak assertions (already fixed)

The `math/src/exact_pisot.c` Sturm classifier no longer
infinite-loops on its first input.  `poly_divmod` was rewritten
over Q (mpq_t) — division in Q always terminates, and sign-change
counts V(x) are preserved under positive scalar multiples.  The
`classify_and_check` test helper now asserts `info.is_pisot == 1`
and β interval bracketing, so a return-code-only check can't
silently pass a non-classifier.  The σ_2 test matrix was corrected
from `{{2,3,1},{0,0,0},{1,1,0}}` (which has det=0 because row 1 is
all zeros) to the correct `{{2,3,1},{1,0,0},{0,1,0}}`.
`tests/exact_pisot_test.cpp` now passes 38/38.

## Status update — math library, in_H_sigma_exact (post-EXPLODED audit pass)

The 12 "EXPLODED" near-Salem Pisot candidates from the
39-substitution survey are now resolved via a new exact
`Substitution::in_H_sigma_exact(x, j)` method in
`include/ravel/core.hpp` that delegates to the math library at
`math/` (Sturm-isolated β interval + exact Q(β) sign test via
`math/include/math/in_h_sigma.hpp`).  The
`include/ravel/d_cont_check.hpp` `search_D_cont` was updated to
use the exact path, so the contact-boundary pipeline no longer
EXPLODES on the 12 candidates.  See `math/tests/test_exploded.cpp`
and `math/TOOLS.md` for the math library's tool inventory and
test results (248 tests across 8 tiers all passing).

The exact path is opt-in (the existing double-precision
`in_H_sigma` is kept for the σ_1 / σ_2 / Tribonacci surveys where
the boundary issue isn't observed).
`core.hpp::Substitution<d>` lazily caches the Q(β) ring,
Sturm-isolated β interval, and Cramer-rule eigenvector on the
first call to `in_H_sigma_exact`, so subsequent exact calls are
as cheap as the double-precision ones.

## Status update — exact `solve_Mx_eq_rhs_exact`

The forward-branch work added an exact `mpq_t` Gaussian-elimination
version of `solve_Mx_eq_rhs` next to the existing double-precision
version in `include/ravel/ambient_graph.hpp` and
`include/ravel/corona.hpp`.  Both are present; the existing
double-precision one is the default ("fast and messy" — cheap,
toleranced), the new one is the "checking" reference
(bit-exact, slower).  Each header has clear comments documenting
which to use when.  No call site was changed — the existing
defaults pass all tests; the exact path is opt-in for verification
of suspect contact-boundary candidates.

## Status update — reorganization

The C++ side of the project was reorganized per the user's
direction:

- `lua/src/*.hpp` → `include/ravel/*.hpp` (the reusable header
  library)
- `lua/src/{exact_pisot.c, rauzy_fractal.cpp, lua_bindings.cpp}` →
  `src/` (non-header-only implementations)
- `lua/src/*_test.cpp` → `tests/` (paired one-per-header)
- `lua/scripts/*.cpp` → `app/` (the `main()`-bearing drivers; the
  `scripts/` name was wrong, see `docs/GKW_SCRIPTS_NOTE.md`)
- `lua/src/mathlib/` → `math/` (proper C++ sub-library with its
  own `Makefile`; treats mini-gmp as native project code at
  `math/include/mini-gmp/` and `math/src/`, not as a vendor dep)
- `lua/src/mini-gmp/` → `math/{include/mini-gmp,src}/` (same)
- `lua/Makefile` → thin wrapper around the top-level `Makefile`

See `README.md` and `docs/CPP_DESIGN_PHILOSOPHY.md` for the
post-reorganization layout.

## Current test count

| Binary | Test count | Status |
|---|---|---|
| `d_cont_check_test` | 6 | pass |
| `ambient_graph_test` | 7 | pass |
| `corona_test` | 8 | pass |
| `contact_boundary_test` | 35 | pass (d=2..8 dispatch) |
| `exact_pisot_test` | 38 | pass |
| `spectral_general_test` | 9 | pass |
| `qbeta_eigenvalue_test` | (small) | pass |
| `survey_test` | 34 | pass (d=2..7 survey) |
| `test_qbasis` | (small) | pass |
| `validate_exploded` | 12 | pass (Sturm classification) |
| `test_12_exploded` | 67 | pass (12 EXPLODED, exact path) |
| `math/check` | 248 | pass (8 tier tests) |
| Lua test suites | 52/52 | pass |
| **Total C++ test cases** | **~520** | **all pass** |

`make check` ends with `ALL SUITES PASSED.`

## TODO, in priority order

1. ~~**Write a general eigenvalue path in `include/ravel/spectral.hpp` and stop
   silently failing for n>=4.**~~ **DONE** — `spectral_invariants_general`
   added; verified against the known Tribonacci/Tetrabonacci/rnd13
   values plus a negative case.  See `tests/spectral_general_test.cpp`
   (9/9 passing).

2. ~~**Make `classify_matrix_spectral` call the new general path for
   n>=4** instead of `return out;`.~~ **DONE** — `survey.hpp` dispatches
   `n >= 4` to `spectral_invariants_general`.

3. ~~**Retire the duplicate eigenvalue code once (1)+(2) land.**~~ **DONE** —
   `app/cylinder_measure.cpp` and `app/contact_boundary_4x4.cpp` were
   pointed at the shared path; both reproduced byte-identical beta
   values.  `app/qbeta_eigenvalue.cpp` kept for exact Q(β) arithmetic.

4. ~~**Wire `cylinder_measure` into the Makefile** as its own target.~~ **DONE** —
   `make cylinder_measure` and part of `make all`.

5. ~~**Finish or delete the `dump_gb_matrix` → `qbeta_eigenvalue` pipeline.**~~
   **DONE** — wired end-to-end in `app/contact_boundary_4x4.cpp`.

6. ~~**Generalize the random survey generator past n=3.**~~ **DONE** —
   `compute_contact_boundary_dispatch` and the survey generator are
   general-d (up to `MAX_DISPATCH_D = 8`).

7. **Smaller: reconcile the CHSH/Tsirelson-named files with the
   substitution-combinatorics files.** `include/ravel/thermometer.hpp`,
   `include/ravel/tilt.hpp`, `lua/lua_src/ravel/spectre_chsh.py`,
   `lua/lua_src/ravel/ultrametric_chsh.py` borrow CHSH's algebraic
   form (the four-term sign-pattern maximization) as a statistic on
   substitution correlation functions — that's a fine empirical
   tool, but nothing in the headers currently says in one place
   "this is a correlation statistic borrowed from Bell
   inequalities, not a claim about quantum mechanics in the
   substitution."  Worth a one-paragraph note at the top of
   `include/ravel/thermometer.hpp` saying that plainly, so it
   doesn't get mistaken for more than it is by someone reading the
   tree cold.  (Tracked in `docs/RESEARCH_STATUS.md` and in
   `docs/FINDINGS_FOR_CITATION.md`'s reference list, but not at the
   point of code.)

## What's already fine and doesn't need touching

The graph/geometry side (`include/ravel/ambient_graph.hpp`,
`corona.hpp`, `contact_boundary.hpp`, `d_cont_check.hpp`, `faces.hpp`,
`RauzyFractal<d>`) is templated correctly and there's no evidence
it special-cases alphabet size anywhere. The math library at
`math/` handles Sturm sequencing + exact Q(β) arithmetic +
Bezout-based modular inverse in Q(β), with 248 tests across 8
tiers; it continues to be the single source of truth for exact
Q(β) arithmetic.  Document any future changes carefully so the
math library continues to be that single source of truth.
