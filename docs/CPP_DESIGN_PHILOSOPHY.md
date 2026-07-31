# C++ design philosophy

Conventions the C++ side of the project follows across `include/`,
`src/`, `app/`, `math/`, and `lua/`. These are rules for extending
the codebase consistently.

## 0. The four C++ locations and what goes where

| Location | Contains |
|---|---|
| `include/ravel/*.hpp` | Reusable C++ headers (the public `ravel::` library). Header-only when possible; paired with `tests/*_test.cpp` per §3. |
| `src/*.cpp` | The small set of non-header-only implementations paired with the header library. Currently: `lua_bindings.cpp` (Lua-C bridge), `rauzy_fractal.cpp` (explicit template instantiation). |
| `app/*.cpp` | Standalone driver programs (`main()`-bearing). Each one exercises a piece of mathematical machinery and prints a report. NOT tests, NOT library code. |
| `tests/*_test.cpp` | Test binaries. Paired one-per-header per §3. |

`math/` is a separate C++ sub-library with its own `Makefile`; it
produces `math/out/libmath.a` and is linked into the main project
via the top-level `Makefile`.  See `math/TOOLS.md` for the math
library's tier breakdown.

C++ `.cpp` files with a `main()` do NOT belong in `src/` (that's
non-header-only library code), `tests/` (that's test binaries
with specific header-pair semantics), or `lua/scripts/` (that's
interpreted-orchestration only).  They go in `app/`.  This was
the project's mistake before the reorganization; see
`docs/GKW_SCRIPTS_NOTE.md` for the GKW-specific rationale and the
general principle.

## 1. Header-only where possible, templated on alphabet size

`Substitution<d>`, `RauzyFractal<d>`, `ANode<d>`, and the
ambient-graph/corona/contact-boundary/faces/D_cont machinery are
class templates parameterized on alphabet size `d`, resolved at
compile time. Fixed-size `std::array` members, no heap allocation in
hot paths, dimension mismatches caught at compile time. Cost: call
sites spell out `<d>` explicitly; a new `d` is a new instantiation.

**Rule**: a templated pipeline is only as general as its
least-general dependency. Before assuming a new `d` works end to end,
check every function it transitively calls, not just the layer
you're editing — `spectral.hpp`'s eigenvalue solver was closed-form
2×2/3×3-only for a long time while everything downstream of it was
already fully templated, silently gating the whole pipeline at
`d ≤ 3`. Fixed in `spectral_invariants_general` (`spectral.hpp`);
`survey.hpp::classify_matrix_spectral` now dispatches to it for
`n ≥ 4`.

## 2. Double precision by default; exact arithmetic only when demonstrated necessary

No FLINT, no Boost. mini-gmp (vendored but treated as native project
code, lives at `math/include/mini-gmp/` and `math/src/`) provides
`mpz_t` and `mpq_t`. It's used by the math library's exact
arithmetic primitives and by the Sturm-based Pisot classifier; the
rest of the main project uses `double` and power iteration with
documented tolerances. State the tradeoff in a comment at the
point you take it (see `core.hpp`, `d_cont_check.hpp` for the
pattern).

**Rule**: build the exact-arithmetic path when a specific numerical
question demonstrably needs it (a result that looks like it might be
a precision artifact), not preemptively.  The classic example: the
12 "EXPLODED" near-Salem Pisot candidates from the 39-substitution
survey were resolved by the math library's exact `Q(β)` sign test
via `Substitution::in_H_sigma_exact` (see `core.hpp`).

## 3. Every header ships a paired, standalone self-test

`foo.hpp` in `include/ravel/` → `foo_test.cpp` in `tests/`, using
the `CHECK`/`CHECK_NEAR`/`EXPECT` macro pair (any `*_test.cpp` shows
the pattern), printing `[ok]`/`[FAIL]` per case, nonzero exit on
any failure. Wired into the top-level `Makefile` as an independent
target, runnable with no Lua dependency. Test against a closed-form
value where one exists, or against a retained deterministic fixture
where it doesn't.

**Rule**: an untested header is not exercised by `make check` and
will not be caught by it. If a `.cpp` file isn't in the
`Makefile`, add it before trusting anything it computes.

For the math library, the equivalent is `math/tests/test_*.cpp`
under `tests/` there; `math/TOOLS.md` documents the per-tier
mapping.

## 4. Code mirrors the paper's own structure

Types and functions are named after the reference paper's
definitions (`D_cont` for Def 2.2/3.1, the corona construction for
Def 3.9, `algorithm2` for Algorithm 2) rather than renamed into more
idiomatic C++ names, so the paper and the code can be read
side by side, definition by definition. Comments name which paper
section or which reference Python file a given piece mirrors — this
is what makes the numerical cross-check table in
`docs/RESEARCH_STATUS.md` traceable.

## 5. One decimal constant, one source

The decimal mantissa of `sqrt(3)/2` lives only in
`include/ravel/cyclotomic.hpp`.  Every other constant is either
exact in binary floating point (`0.5`, `1.0`, `0.0`) or derived
from that source. Don't re-type a decimal expansion of an irrational
in a second file.

The transcendentals are the one principled exception:
`include/ravel/rational_transcendentals.hpp` is the source of
truth for π, sin, and cos, computed via Machin's formula +
alternating-series remainder bound (and Taylor + Lagrange
remainder for sin/cos), with no floating point anywhere.  The
default paths through the math library that need a π or a sin
bracket it from this header rather than using `std::cos` /
`M_PI`.

## 6. main() lives in app/, not src/

C++ files with a `main()` function are standalone driver programs
and go in `app/`.  They exercise some part of the library and
print a report; they are not reusable code, they are not tests of
a specific header, and they are not Lua/Python/shell scripts (the
`lua/scripts/` folder is for the latter).  See §0 above for the
four-location summary; the pre-reorganization mistake was
collecting all `.cpp` files (both library code and `main()`-bearing
drivers) under one `lua/src/` + `lua/scripts/` split, which mixed
the two concerns.

## Checklist for new numerical work

1. Pick the right location for the code:
   - Reusable header → `include/ravel/foo.hpp`.
   - Non-header-only implementation → `src/foo.cpp`.
   - Standalone driver with `main()` → `app/foo.cpp`.
   - Test of a specific header → `tests/foo_test.cpp`.
2. Template on `d` if it needs to generalize; verify every function
   it calls is equally general (§1) — grep for hardcoded array sizes,
   don't assume.
3. Double precision, tradeoff stated in a comment (§2).
4. Paired `_test.cpp` in `tests/`, wired into the top-level
   `Makefile`, checked against a closed-form value or an independent
   implementation (§3).
5. If implementing something from a paper, name it after the
   paper's definition numbers and say so in a comment (§4).
6. If it's `main()`-bearing, put it in `app/`, not `src/` (§6).
