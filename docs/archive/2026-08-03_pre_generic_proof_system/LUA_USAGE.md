# Lua usage

## Division of responsibility

All numerics — Perron eigenvalue extraction, characteristic
polynomials, the ambient-graph/corona/contact-boundary construction,
Rauzy fractal generation, exact `Q(β)` arithmetic — are implemented
in C++ (`include/ravel/*.hpp`), compiled into `spectre_native.so`
along with the math library (`math/`, linked as
`math/out/libmath.a`). Lua does orchestration only: composing the
C++-backed primitives into a readable API, running the test suite,
and holding static geometric data tables. **No numerical algorithm
is implemented in Lua.** If you're about to write loop-heavy
numerical code in a `.lua` file, it belongs in a C++ header
instead (see `CPP_DESIGN_PHILOSOPHY.md`).

A meaningful part of the validation surface bypasses Lua entirely:
`d_cont_check_test`, `ambient_graph_test`, `corona_test`,
`contact_boundary_test`, `exact_pisot_test`,
`spectral_general_test`, `survey_test`, `qbeta_eigenvalue_test`,
`test_qbasis`, `test_12_exploded`, `validate_exploded`, and the math
library's `math/check` are standalone C++ binaries with their own
`main()`, buildable and runnable with `g++`/`make` alone.

## Directory layout

The C++ side of the project is reorganized under the new top-level
`include/` + `src/` + `app/` + `tests/` + `math/` layout.  The Lua
side keeps its `lua/lua_src/ravel/` package root.

```
RavelMathPub/
├── include/ravel/        # C++ headers (reusable library; see docs/CPP_DESIGN_PHILOSOPHY.md)
├── src/                  # C++ non-header-only impls (lua_bindings, rauzy_fractal)
├── app/                  # C++ main()-bearing drivers (cylinder_measure, gkw_*, etc.)
├── tests/                # C++ test binaries (paired one-per-header)
├── math/                 # math library subproject (proper C++ library, own Makefile)
│   ├── include/          #   headers (math/, mini-gmp/, exact_pisot.h)
│   ├── src/              #   impls (mini-gmp.c, mini-mpq.c, exact_pisot.c)
│   └── tests/            #   mathlib tests (319/319 across 10 tiers)
├── lua/                  # Lua package, unchanged
│   ├── lua_src/ravel/    #   LuaRocks package root (the Lua API)
│   │   ├── init.lua      #     composes the public ravel.* API
│   │   ├── data/         #     static geometry/transition tables
│   │   ├── tests/        #     14 Lua test suites
│   │   └── *.lua         #     pure-Lua logic (BFS, lineage, etc.)
│   ├── scripts/          #   Lua/shell orchestration (no Python remains here)
│   └── Makefile          #   thin wrapper around the top-level Makefile
└── Makefile              # top-level orchestration for everything C++
```

`lua_src/` is the directory added to Lua's `package.path` — the
LuaRocks convention for a pure-Lua module root.

C++ driver programs with `main()` go in `app/`, not in
`lua/scripts/`.  The `lua/scripts/` folder is for interpreted
orchestration. Former `.py` drivers were removed after native C++
cores, Lua drivers, and fixture parity were established. The
`main()`-bearing `.cpp` files used to live there were a misnomer
and are now in `app/`.  See `docs/GKW_SCRIPTS_NOTE.md` for the
worked example (GKW drivers) and `docs/CPP_DESIGN_PHILOSOPHY.md`
for the general principle.

## Connecting the two languages

`src/lua_bindings.cpp` compiles into `spectre_native.so`, loaded
from Lua as a C module. Every driver/test script follows this
pattern:

```lua
package.cpath = project_dir .. "/../out/?.so;" .. package.cpath
package.path = project_dir .. "/lua_src/?.lua;" .. package.path
local native = require("spectre_native")           -- compiled C++ bridge
local ravel = require("ravel").init(native)     -- composed Lua API
```

Callers then use `ravel.contact_boundary.compute(...)`,
`ravel.return_phase.build(substitution, marker, orbit_cap)`,
`ravel.pisot_survey(...)`, etc. without needing to know which parts
are C++ and which are Lua.

`ravel.return_phase.build` constructs marker return words, their
derived substitution, and the finite phase system with states
`(return_word, offset)`. Large reachable-state exploration remains in
C++; Lua receives compact results and controls experiments.

`ravel.fibonacci_selection.exact(d1, d2, shift, targets, max_cells)`
computes the infinite-chain piecewise-circle measure in native C++ for
four caller-supplied correlator targets. `exact_quantum` supplies the
standard CHSH targets. Both return correlators, accepted measures,
CHSH, and the partition-cell count; the optional cap bounds memory.
`lua/scripts/fibonacci_selection_theorem.lua` is an audit driver: the
historical fixtures contradict their claimed general even-stride
exactness and quadratic shift law.

`ravel.fibonacci_finite.run(N, L, d1, d2, targets, mode, steps,
baseline, max_sites)` is the bounded finite-chain companion. Mode is
`"polarizer"` or `"rule30"`; pass `targets=nil` to measure an unbiased
Rule-30 baseline before retargeting. Polarizer evaluation streams
without `N`-sized numeric temporaries. Rule-30 retains two byte buffers,
not the many NumPy arrays used by the prototype. The driver is
`lua/scripts/fibonacci_finite_family.lua`.

`ravel.fibonacci_dynamics.correlation_matrix` evolves a packed
periodic Fibonacci word and computes arbitrary XOR-offset functional
correlations with wordwise XOR/popcount. Its storage is `nf*N/8`
bytes, 64 times smaller than an `nf*N` double matrix.
`ravel.fibonacci_dynamics.retarget` applies the finite selection
model to four chosen packed functionals.
`ravel.fibonacci_dynamics.local_windows` snaps four polarizer
windows to the radius-`R` Fibonacci boundary partition, measures their
four correlators, and retargets them with `O(R)` auxiliary memory.
Drivers:
`lua/scripts/rule30_functional_search.lua` and
`lua/scripts/measurement_cost.lua`. The complete historical
dynamical-family audit is `lua/scripts/dynamical_family.lua`; it
reports the corrected failure of the proposed `Theta(1/R)` gap.

## Build targets and what runs when

All of the following work from the top-level directory (the
`lua/Makefile` is a thin wrapper that delegates to the top-level
Makefile, so `cd lua && make ...` still works for legacy callers).

| Command | What it does |
|---|---|
| `make check` | Builds `math/out/libmath.a`, the Lua shared library `out/spectre_native.so`, builds+runs all standalone C++ test binaries, then runs `lua5.4 lua/scripts/run_lua_tests.lua` (14 Lua test suites). Full validation pass. |
| `make data` / `make tables` | Pure-Lua data pipeline: `lua/scripts/parse_specmap.lua` and `lua/scripts/build_spectre_transition_table.lua` turn the raw SVG-derived geometry (`tatham_svg_parsed/`, `tatham_svg_parsed/`) into the Lua tables under `lua/lua_src/ravel/data/`. Doesn't touch the C++ backend. |
| `lua5.4 lua/scripts/coord_bfs.lua` | Interactive/exploratory BFS walk of the Spectre tiling's hex-coordinate graph. |
| `make math` | Build + test the math library in isolation (`math/Makefile`). |
| `make apps` | Build the apps in `app/`. |
| `make tests` | Build the tests in `tests/`. |
| `make gkw_sturm_certify` | Build and run the GKW Sturm-certify driver (the M=12 collocation eigenvalue certificate, ~25s). |
| `make rational_transcendentals_test`, `make contact_boundary_4x4`, `make cylinder_measure`, etc. | Individual C++ targets; no Lua needed. |

The math library at `math/` (Sturm sequencing + exact `Q(β)`
arithmetic + Bezout-based modular inverse + Cramer-rule left and
right eigenvectors + Sturm-isolated real-root exact sign test +
exact-rational and tunable-precision Collatz-Wielandt Perron-root
certification (`ball.hpp`, `bigfloat.hpp`), 319
tests across 10 tiers; see `math/TOOLS.md` for the full inventory) backs
the new `Substitution::in_H_sigma_exact` path in
`include/ravel/core.hpp` (which uses the dedicated
`left_eigenvector_via_qbeta` -- there was a real bug this project
shipped and fixed by conflating it with `right_eigenvector_via_qbeta`).
It resolves the 12 "EXPLODED" near-Salem Pisot candidates from the
39-substitution survey
(previously rejected by the double-precision `in_H_sigma`
tolerance) in the sense that the pipeline now terminates with finite,
definite values rather than diverging -- see
`docs/FINDINGS_FOR_CITATION.md` §A for exactly what is and isn't
claimed by that.

Numerical surveys, the contact-boundary pipeline, and the
cylinder-measure/factorization checks all run as standalone C++
binaries built from `app/*.cpp` (and tested via `tests/*_test.cpp`)
— Lua plays no role in any of them.

## Where new work goes

- New numerical/combinatorial algorithm, or anything templated on
  alphabet size → C++ header in `include/ravel/`, with a paired
  `*_test.cpp` in `tests/`.
- New non-header-only C++ implementation → `src/foo.cpp`.
- New standalone C++ driver with `main()` → `app/foo.cpp`.
- New math library tool → `math/include/math/foo.hpp` (header-only)
  or `math/src/foo.c` + `math/include/foo.h` (C), with a paired
  `math/tests/test_foo.cpp`.  See `math/TOOLS.md` for the tier
  breakdown.
- New test case for existing functionality → `.lua` file in
  `lua/lua_src/ravel/tests/`.
- New static table (geometry, transitions, constants) → `.lua` file
  in `lua/lua_src/ravel/data/`.
- Composing existing primitives with no new numerics → Lua, in
  `lua/lua_src/ravel/*.lua` or a `lua/scripts/*.lua` driver.
