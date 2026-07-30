# Why the GKW work lives in `app/` as `.cpp` drivers

This note documents the directory/file-type choice for the
Gauss–Kuzmin–Wirsing transfer-operator eigenvalue work
(`gkw_sturm_certify.cpp`, `gkw_fully_rigorous.cpp`) so the next
session doesn't have to reverse-engineer it.

## The one-paragraph answer

`app/` is this project's directory for *standalone C++ driver
programs* — `main()` entry points that exercise a piece of
mathematical machinery and print a report.  C++ files that
have a `main()` do not belong in `src/`, `include/`, or `tests/`
under this project's conventions:

- `include/spectre/*.hpp` is the *reusable C++ header library*
  (templated class definitions, function templates, no `main()`,
  called by other code).
- `src/*.cpp` is the small set of non-header-only implementations
  paired with the header library (currently just `lua_bindings.cpp`
  and `rauzy_fractal.cpp`; the Sturm internals live in the
  `math/` sub-library).
- `tests/*_test.cpp` is the test binaries (one per header;
  see `docs/CPP_DESIGN_PHILOSOPHY.md` §3).
- `app/*.cpp` is the main()-bearing standalone drivers.

The GKW work is two such drivers: each one builds a single
specific matrix (the M-node Chebyshev collocation matrix), feeds
it through a characteristic-polynomial + Sturm-isolation
pipeline, and prints the top-two eigenvalues to N bits of
certified precision.  Neither is a reusable library component,
neither is a test of an existing header, and neither is a glue
driver for the contact-boundary pipeline — so `app/` is correct
for both.

The earlier name `scripts/` was wrong: scripts/ is for
*interpreted* driver programs (`.lua`, `.sh`, `.py`) that the
shell or Lua orchestrator calls.  C++ programs with `main()`
are not "scripts" in any meaningful sense, and they have
different build, link, and dependency-graph concerns.  That
folder now contains only `.lua` / `.sh` / `.py` orchestration
under `lua/scripts/`, with the C++ programs split out into
`app/`.

## What "app/" is for, concretely

Look at the other `.cpp` files in `app/`:
- `cylinder_measure.cpp` — builds two specific substitutions,
  measures their cylinder measure and factor frequencies,
  prints the report.  Has its own `main()`.  No reusable
  library here.
- `qbeta_eigenvalue.cpp` — reads a matrix dump from a file,
  runs the exact-Q(β) eigenvalue routine on it, prints the
  result.  Standalone driver for `spectre/qbeta_eigenvalue.hpp`.
- `contact_boundary_4x4.cpp` — takes a 4×4 matrix as command-line
  args, runs the full contact-boundary pipeline on it, prints
  the report.  Standalone driver for the contact-boundary
  headers.
- the former `rauzy_fractal_natural.cpp` — i.i.d. projected samples
  (removed after the corrected graph-directed native generator gained
  a tested Lua surface);
  fractal, prints coordinates.  Standalone driver.
- `validate_exploded.cpp` / `test_12_exploded.cpp` — verify the
  12 EXPLODED candidates under the exact classifier / exact
  in_H_sigma path.  Standalone driver / verifier.
- `rational_transcendentals_test.cpp` — verifies the Machin-formula
  π and Taylor+remainder sin/cos brackets in
  `spectre/rational_transcendentals.hpp`.
- `verify_exploded_12.cpp` — forward-branch driver for the same
  12 candidates under the exact Sturm classifier (smaller
  surface area than `validate_exploded.cpp`; kept as an
  independent reference).
- `test_general_spectral.cpp` — quick visual smoke test for
  `spectral_invariants_general`; the canonical regression test
  is `tests/spectral_general_test.cpp` (paired with
  `spectral.hpp` per the §3 design philosophy).

The GKW `.cpp` files fit this pattern exactly.  Each one:
1. Builds a single specific matrix (M-node collocation).
2. Computes its characteristic polynomial exactly.
3. Isolates specific roots to high precision.
4. Prints a labeled report.

## Why not a header in `include/spectre/`?

`include/spectre/*.hpp` is reusable mathematical machinery —
typed once, instantiated many times, used by other code.
Examples: `spectre::Substitution<d>`, `spectral_invariants_general`,
`compute_contact_boundary_dispatch`.

The GKW work is the opposite shape: it has a single hardcoded
`M = 12` (or `M = 8` in the fully-rigorous variant) baked in, a
single fixed collocation-node construction, and a single printed
report.  Generalizing it to a reusable library would be premature:
the work currently is "certify that this specific matrix has a
specific eigenvalue to 90 digits" — a single fact, not a
generality.  If/when the GKW work grows into a reusable
spectral-radius-of-transfer-operator tool, it would move to
`include/spectre/gkw.hpp` (header-only, templated on M).
Today it isn't there.

## Why not a test in `tests/`?

`tests/*_test.cpp` follow the strict pattern: "test that header
X produces output Y for input Z, where Y is independently
verified."  They live in `tests/` and exist to gate the
header's correctness on every `make check`.

The GKW work is not testing a header — it IS the application.
`gkw_sturm_certify.cpp` does test that
`isolate_real_root_generic_mpz` works as advertised on a hard
case, but the test of the Sturm isolator itself already exists
in `tests/exact_pisot_test.cpp` (38 cases).  The GKW driver's
purpose is to produce a GKW eigenvalue certificate, not to gate
any header's correctness.

If the GKW eigenvalue computation is to be a long-lived test
target — i.e. if "GKW λ(G_B) to N digits" is a number we want to
keep verifying in `make check` — then it should grow its own
test in `tests/` (paired with whatever `gkw.hpp` becomes when
it stops being a script).  Until then, `app/` is correct.

## The math library is a sub-library, not a vendor dependency

The GKW drivers depend on `isolate_real_root_generic_mpz` from
the math library at `math/`.  That dependency is satisfied
through the normal sub-library link (the top-level `Makefile`
calls `make -C math lib` first, which produces
`math/out/libmath.a`, which the app targets link against).
mini-gmp lives in `math/include/mini-gmp/` and `math/src/` —
treated as native project code (the project's "roll your own,
FREE alternative, no Boost" stance), not as a third-party
vendor.  See `math/Makefile` and the top-level `Makefile` for
the wiring.

## The one open question

The GKW work's "user-facing API" is the *output*: a printed
eigenvalue bracket to N digits.  There's no good way to assert
"the GKW λ is X" inside `make check` without re-running the
matrix construction and Sturm isolation on every CI run, which
the forward-branch session's notes flag as too slow for a
default test target.  Practical recommendation: keep the GKW
files as optional `make gkw_sturm_certify` /
`make gkw_fully_rigorous` targets (NOT in `make all` or
`make check`), exactly as `cylinder_measure` was before
`TODO_GENERALIZATION.md` item 4 promoted it.  When a session
actually needs the GKW certificate, it builds and runs them
by hand; the result is logged in that session's notes, not
gated by CI.

## See also

- `docs/CPP_DESIGN_PHILOSOPHY.md` — the rules that put reusable
  code in `include/spectre/*.hpp`, non-header-only impls in
  `src/*.cpp`, tests in `tests/*_test.cpp`, and standalone
  drivers in `app/*.cpp`.
- `docs/DIRECTION_AND_OPEN_THREADS.md` — the GKW work's status
  in the broader project direction.
- `lua/TODO_GENERALIZATION.md` — the previous `cylinder_measure`
  was similarly orphan-before-promotion; same pattern, different
  numerical question.
- `gkw_fully_rigorous.cpp` was built but timed out at `M=8` and was
  left unfinished; this is why it is not wired into `make all`.
- `math/Makefile` and `math/TOOLS.md` — the math library that
  the GKW drivers consume.
