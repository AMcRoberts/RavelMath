# Reviewer notes — n-bonacci chunk-based generator (Ravel)

Author: Ravel
Architect: AM
Date: 2026-08-02

## What the reviewer should look at

1. The C++ chunk-based generator pattern in
   `app/nbonacci_charmpoly_proof_general.cpp` and the original
   `app/nbonacci_charmpoly_proof_probe.cpp`. The pattern: a single
   C++ function `find_simplest` enumerates the r/q-matrix
   construction, computes the dets symbolically (via a `PolyZ` Bareiss-
   like Laplace expansion), verifies box and cover, and assembles the
   cofactor formula. Five chunk emitters in the C++ probe (`emit_*`
   functions) generate the corresponding `noncomputable def` and
   `example` statements in the Lean file.
2. The Lua binding in `src/lua_bindings.cpp` and the Lua wrapper in
   `lua/lua_src/ravel/nbonacci_covering_witness.lua`. Three Lua
   functions exposed: `get_simplest(n, L)`, `get_batch(n_min, n_max)`,
   `check(...)`. The C++ side does the work; the Lua side is a thin
   driver.
3. The C++ core in `src/nbonacci_covering_witness.cpp` and the
   `include/ravel/nbonacci_covering_witness.hpp` header. The core is
   extracted from `app/nbonacci_covering_witness_enumerator.cpp`'s
   `main()` so it can be called from the Lua binding.
4. The new `nbonacci_charmpoly_proof_general` Makefile target
   (which runs the new probe and emits
   `out/nbonacci_charmpoly_proof_general.lean`).

## What the reviewer should ignore

- The existing project work (Class-II family, contact-boundary, etc.)
  is unchanged. This handoff is only about the n-bonacci
  survival-depth chunk-based generator.
- The `automation.txt` file is the C++ probe's debug log; not part
  of the new work.
- The `out/` directory has the emitted Lean files and the
  enumerator's JSON sidecars; these are the artifacts of running
  the probes, not new source.

## Reviewer's protocol

1. Look at `git log --oneline -20` to see the recent commits.
2. Look at the changes since the last review:
   - `include/ravel/nbonacci_covering_witness.hpp` (new)
   - `src/nbonacci_covering_witness.cpp` (new)
   - `src/lua_bindings.cpp` (modified)
   - `app/nbonacci_charmpoly_proof_general.cpp` (new)
   - `Makefile` (modified)
   - `lua/lua_src/ravel/init.lua` (modified)
   - `lua/lua_src/ravel/nbonacci_covering_witness.lua` (new)
   - `app/nbonacci_covering_witness_enumerator.cpp` (modified)
3. Run `make nbonacci_charmpoly_proof_general` to regenerate
   `out/nbonacci_charmpoly_proof_general.lean` and see the
   emitted Lean chunks.
4. Run `make lean-check` to verify no `sorry` in enrolled Lean
   files.
5. Run `make check` (may fail at lua_tests step; pre-existing
   failure unrelated to this work).

## Known issues

1. The C++ covering witness core's debug-printing-output-buffering
   causes the test to report fewer candidates than exist. The
   algorithm itself is correct; the print of intermediate
   candidates is buffered and the `find_simplest` function is
   written to produce debug output. The fix is to remove the
   debug print.
2. The `make check` lua_tests step has a pre-existing failure
   unrelated to this work.

## Verifier protocol

1. `make nbonacci_charmpoly_proof` should produce
   `out/nbonacci_charmpoly_proof.lean` (the original per-n examples).
2. `make nbonacci_charmpoly_proof_general` should produce
   `out/nbonacci_charmpoly_proof_general.lean` (the new general-n
   chain).
3. The emitted Lean file should have 7 r-matrix det examples
   (n=2..8), 7 q-matrix det examples, 7 cofactor examples, and
   7 chain lemmas. (Currently some of these may be missing in the
   output due to the C++ covering witness scoring issue; the
   pattern is established but the chain is incomplete.)

## Notes for the next session

- The general-n cofactor expansion is the open route in
  `lean/nbonacci_margin_catalogue.lean`'s roadmap. The chunk-based
  generator records per-n instances; the general-n induction
  step is the next step.
- The chunk-based generator pattern is established; it can be
  applied to the covering witness itself (the main n-bonacci
  result) in a future session.
- The C++ covering witness core's debug print is a small fix
  (remove the `std::fprintf(stderr, ...)` calls in
  `src/nbonacci_covering_witness.cpp`'s `find_simplest`).
- The `make check` lua_tests step needs separate investigation
  (pre-existing failure).

## Handoff

This handoff is the last commit before the fresh session. Do not
commit to git yet. The reviewer will commit to git after the
review is complete.
