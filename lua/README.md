# RavelMathPub — Lua reimplementation

A Lua reimplementation of a genuine subset of `../` (RavelMathPub,
the `ravel::` C++ project) using **Lua** for data definitions, test
orchestration, and pure-Lua logic, with a small **C/C++** shared
library for the "heavy math": the Pisot spectral invariants, the
contact-boundary / corona / ambient-graph construction, the exact
`Q(β)` arithmetic, and the Hall–Branciard KL divergence (the
CHSH-named helpers in `thermometer.hpp` and `tilt.hpp` are correlation
diagnostics borrowed from the Bell-inequality literature; see the
provenance notes in those headers). This tree also carries a
pure-Lua hex/tiling-coordinate layer with no C++ counterpart in this
repository -- see "Why this split?" below.

The goal is to mirror the C++ project's genuinely-shared machinery
(cyclotomic arithmetic, spectral invariants, substitution rules,
contact-boundary/corona/ambient-graph construction, the Pisot
survey/classifier) while moving every data table, every algorithm
that does not need to be fast, and every test runner to Lua. The
hex/tiling-coordinate layer below (hex kinds, the outer transition
table, vertex offsets, module lineage, the coordinate BFS) is a
**pure-Lua original with no C++ counterpart** in this repository --
comments elsewhere in this tree once claimed otherwise and were
corrected 2026-07-31. Re-running the data tables or test files never
invokes the C++ compiler; only the shared library is compiled.

## Why this split?

* **Lua** excels at iteration and data. The 9×36 hex outer transition
  table is over 1,000 lines of "fill the cell with `(neighbour,
  child)`"; that lives in `lua_src/ravel/data/hex_outer_table.lua`.
  The substitution-rule and balanced-pair refinement is a few
  hundred lines of pure string-and-counting code, also Lua. Tests
  are the most natural Lua citizen (no header dance, no main()
  boilerplate, no fetch-content).
* **C++** keeps the cyclotomic ring multiplication, the spectral
  invariants for Pisot substitution matrices (closed-form 2×2/3×3
  plus the general-n power-iteration / Wielandt-deflation path in
  `spectral.hpp`), the ambient-graph / corona / contact-boundary
  construction, and the exact `Q(β)` eigenvalue computation
  (`qbeta_eigenvalue.hpp`). These have real algebraic identities to
  maintain — keep them in a compiled library so the
  proof-of-correctness is a one-line edit in one place.  The math
  library at `../math/` (Sturm sequencing + exact `Q(β)`
  arithmetic + Bezout-based modular inverse + Cramer-rule
  eigenvectors + Sturm-isolated real-root exact sign test) backs
  the new `Substitution::in_H_sigma_exact` path in `core.hpp`
  that resolves the 12 "EXPLODED" near-Salem Pisot candidates
  previously rejected by the double-precision `in_H_sigma` tolerance.

The decimal mantissa of `√3/2` lives only in
`src/cyclotomic.hpp`. Every other file in the project reads it
through `ravel.constants`, exactly like the C++ side.

## Layout

```
lua/
├── Makefile                  builds the shared library and runs tests
├── README.md                 this file
├── src/                      C++ backend (header-only)
│   ├── algebraic.hpp         Z[β] / minpoly(β) arithmetic, 8 named Pisot rings
│   ├── ambient_graph.hpp     ambient graph construction
│   ├── balanced_pair.hpp     balanced-pair automaton and ρ_nc
│   ├── barge.hpp             characteristic polynomial + rational-root test
│   ├── contact_boundary.hpp  full D_cont → ±C → G_B pipeline
│   ├── core.hpp              Substitution<d> template (+ in_H_sigma_exact via mathlib)
│   ├── corona.hpp            corona construction
│   ├── cyclotomic.hpp        Cyclotomic12D ring (a, b, c, e) + to_xy()
│   ├── d_cont_check.hpp      geometric D_cont verifier (is_in_D_cont; uses exact path)
│   ├── exact_pisot.c         Sturm-based exact Pisot classifier (mpq_t internally)
│   ├── faces.hpp             face-intersection-dim filter
│   ├── qbeta_eigenvalue.hpp  exact Q(β) dominant eigenvalue, general d
│   ├── rauzy_fractal.hpp     RauzyFractal<d> + tiling checks
│   ├── spectral.hpp          spectral invariants 2x2 / 3x3 / general-n
│   ├── substitution.hpp      SubstitutionRule + Parikh balanced-pair helper
│   ├── survey.hpp            random Pisot survey + α-modulus classifier (general d)
│   ├── thermometer.hpp       CHSH-style correlation statistic (see provenance note)
│   ├── tilt.hpp              Hall–Branciard KL + cos(x-y) correlator
../math/                      exact-arithmetic math library (Tier 0-4 + tools doc)
│   ├── *_test.cpp            paired self-tests for every header
│   └── lua_bindings.cpp      Lua C-API exports (builds spectre_native.so)
├── out/                      build artefacts (created by `make`)
│   └── spectre_native.so     the compiled shared library
├── lua_src/                  Lua package root (require()-able modules)
│   └── ravel/
│       ├── init.lua          composes the public ravel.* API
│       ├── data/             static geometry/transition tables
│       ├── tests/            20 Lua test suites (see scripts/run_lua_tests.lua for the exact list)
│       ├── coord_bfs.lua     BFS walker over the hex-coordinate graph
│       ├── lineage.lua       lineage / LCA / tree distance
│       ├── predict_dimension.lua  W1 + W5 closed-form dimension predictor
│       └── substitution_rule.lua  SubstitutionRule + Parikh balanced-pair helper
└── scripts/
    ├── run_lua_tests.lua     top-level test orchestrator
    ├── coord_bfs.lua         pure-Lua BFS walker (no C++ counterpart)
    ├── contact_boundary_4x4.cpp  4-letter contact-boundary driver + in-process qbeta
    ├── cylinder_measure.cpp  empirical cylinder measure + factorization ratios
    ├── qbeta_eigenvalue.cpp  CLI wrapper for the in-process qbeta path
    ├── build_spectre_transition_table.lua  data-pipeline step
    └── parse_specmap.lua     data-pipeline step
```

`lua_src/` is the directory added to Lua's `package.path` — the
LuaRocks convention for a pure-Lua module root. (See
`docs/LUA_USAGE.md` and `docs/PROJECT_LOG_INDEX.md` for why the
original `lua/lua/` package-root rename to `lua/lua_src/` was done.)

## Build

Prerequisites: a C++20 compiler (`g++` is what was used here), the
Lua 5.4 development headers, and `pkg-config`. On Debian /
Ubuntu:

```sh
sudo apt-get install -y g++ make pkg-config lua5.4 liblua5.4-dev
```

Then in `lua/`:

```sh
make             # build everything and run all tests
make check      # build and run all C++ self-tests + Lua suite
make contact_boundary_4x4  # build the 4-letter contact-boundary driver
```

Individual `make` targets:

| Target | What it builds / runs |
|---|---|
| `d_cont_check_test` | d_cont_check.hpp self-test (uses exact in_H_sigma) |
| `ambient_graph_test` | ambient_graph.hpp self-test |
| `corona_test` | corona.hpp self-test |
| `contact_boundary_test` | contact_boundary.hpp full driver pipeline on σ₁ (d=2..8 dispatch) |
| `exact_pisot_test` | Sturm-based exact Pisot classifier (38 cases) |
| `spectral_general_test` | spectral.hpp's general-n eigenvalue path |
| `qbeta_eigenvalue_test` | exact Q(β) eigenvalue (general-d) |
| `cylinder_measure` | empirical cylinder measure + factorization ratios |
| `survey_test` | random Pisot survey at d=2..7 (uses default_K_for_n heuristic) |
| `mathlib_test` | the math library: Sturm + Bezout + linear algebra + in_h_sigma (248 tests across 8 tiers) |
| `contact_boundary_4x4` | 4-letter contact-boundary driver + in-process qbeta |

## What runs when

`make check` builds every standalone C++ test binary and then runs
the full Lua suite. With the in-tree data the test summary is
approximately:

```
[ok]   all 10 algebraic cases passed
[ok]   all 6 d_cont_check cases passed
[ok]   all 7 ambient_graph cases passed
[ok]   all 8 corona cases passed
[ok]   all 31 contact_boundary cases passed
[ok]   all 38 exact_pisot cases passed
[ok]   all 9 spectral_general cases passed
[ok]   all 11 qbeta_eigenvalue cases passed

14 Lua suites passed:
  test_cyclotomic  (10)  test_hex  (2)  test_transitions  (3)
  test_transitions_audit  (2)  test_spectre_transitions  (9)
  test_coordinate  (4)  test_tribonacci  (1)  test_closure  (3)
  test_pisot  (16)  test_pisot_survey  (10)  test_reference_sigma  (4)
  test_predict_dimension  (6)  test_gap_labels  (4)  test_contact_boundary  (52)

ALL SUITES PASSED.
```

The exact totals depend on which test cases are added each session;
the "ALL SUITES PASSED" line is the canonical pass/fail signal.

## Mapping back to the C++ project

| Lua | C++ | Purpose |
|---|---|---|
| `ravel.cyclo.*` | `ravel::Cyclotomic12D` (cyclotomic.hpp) | 12th cyclotomic ring arithmetic |
| `ravel.constants.*` | `ravel::k*` (cyclotomic.hpp) | Named decimal constants |
| `ravel.spectral.*` | `ravel::spectral_invariants_*` | Perron root, β₂, n-1 bound |
| `ravel.tilt.*` | `ravel::hb_mi_floor`, `tilt_correlator` | HB KL and cos(x-y) |
| `ravel.hex_kind` | *(pure Lua, no C++ counterpart)* | Hex enum |
| `ravel.outer` | *(pure Lua, no C++ counterpart)* | 9x36 hex transition table |
| `ravel.vertices` | *(pure Lua, no C++ counterpart)* | Unit hex shape |
| `ravel.substitution_rule` | `ravel::SubstitutionRule`, balanced pairs | Sigma + Parikh refinement |
| `ravel.lineage` | *(pure Lua, no C++ counterpart)* | Module lineage helpers |
| `ravel.coord_bfs.walk` | *(pure Lua, no C++ counterpart)* | BFS over the hex map |
| `ravel.contact_boundary.*` | `ravel::compute_contact_boundary*` | D_cont → ±C → G_B pipeline |
| `ravel.contact_boundary.from_subst` | `ravel::compute_contact_boundary_from_subst` | Auto-derive D_cont geometrically |
| `ravel.d_cont_check.*` | `ravel::is_in_D_cont`, `verify_D_cont_table` | Standalone geometric D_cont check |
| `ravel.rauzy_fractal.*` | `ravel::RauzyFractal<d>` | Rauzy fractal (d=4) |
| `ravel.survey.*` | `ravel::random_pisot_survey`, etc. | Random Pisot survey |
| `ravel.barge.*` | `ravel::barge_flags`, `charpoly_int` | Barge classification helpers |
| `ravel.tests.*` | `tests/test_*.cpp` | Per-binary test suites |

## Adding a new test or data file

* New constants go in `src/cyclotomic.hpp` and are exposed by
  adding one line to `src/lua_bindings.cpp::constants_funcs`. The
  Lua side picks them up via `ravel.constants.load(native)`.
* New data tables go under `lua_src/ravel/data/` and should be
  integer-indexed tables so the C++ cyclotomic helpers can read
  them directly (no name translation).
* New Lua tests go under `lua_src/ravel/tests/test_<name>.lua`,
  follow the pattern in `test_hex.lua`, and are registered in
  `scripts/run_lua_tests.lua::test_files`.
* New C++ math goes under `src/`, follows the header-only pattern
  of `cyclotomic.hpp` / `spectral.hpp` / `tilt.hpp`, and is exposed
  by adding one entry to `lua_bindings.cpp`.

## Where to look next

* `../docs/LUA_USAGE.md` — backend structure, build targets, when to
  use Lua vs. C++.
* `../docs/CPP_DESIGN_PHILOSOPHY.md` — the C++ conventions and how
  to extend them.
* `../docs/DIRECTION_AND_OPEN_THREADS.md` — what's still unfinished
  in the engineering layer (Thread C) and what research questions
  are open (Thread A).
* `../lua/TODO_GENERALIZATION.md` — live engineering TODO list.
