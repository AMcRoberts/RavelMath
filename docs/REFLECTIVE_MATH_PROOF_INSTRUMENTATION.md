# Reflective math-proof instrumentation

Date: 2026-08-03

## Corrected architectural intent

The proof system is not an external C++ program that computes an answer and then narrates that answer as Lean. The mathematical library itself carries optional, normally dormant reflective pivots. A Lua declaration activates a trace context. While the library examines a problem, the exact operations it actually performs record objects, observations, branch choices, minors, reductions, identities, and results. A renderer may then translate that provenance into debug-time proof material.

The test or application is only a trigger. It must not own matrix definitions, determinant algorithms, recurrence discovery, polynomial normalization, or theorem-specific proof narration.

## Implemented first vertical slice

- `math/include/math/proof_reflection.hpp`
  - optional thread-local trace context;
  - typed events and parent links;
  - zero behavioral effect when no trace is active.
- `math/include/math/poly_matrix.hpp`
  - shared exact polynomial-matrix construction;
  - shared minor deletion;
  - determinant computation with reflective sparse/first-row pivots;
  - n-bonacci q/r families and polynomial constructions moved out of the app.
- `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua`
  - `reflection.enabled = true` and `mode = "observe_and_prove"`;
  - names the internal pivot vocabulary and requested outputs.
- `include/ravel/proof/reflection_declaration.hpp`
  - closed, non-executing reader for the activation declaration;
  - the full Lua loader also parses and validates the same configuration where Lua 5.4 is available.
- `include/ravel/proof/reflective_lean_renderer.hpp`
  - refuses to render without operation provenance;
  - currently emits a trace-grounded Lean skeleton, not a false theorem.
- `app/nbonacci_charmpoly_proof_general.cpp`
  - reduced to schema activation, problem construction, ordinary math calls, and artifact writing.
- `tests/proof_reflection_test.cpp`
  - verifies that internal pivots are recorded when active and absent when inactive.

## What remains before the system proves the universal theorem

The trace vocabulary must become semantic rather than merely operational. In particular, matrix-family identities need stable symbolic object IDs, index maps, shape predicates, and parent-linked equality witnesses. Determinant pivots must expose reusable obligations such as entry preservation under `Fin` transport. Polynomial operations should record normalization/rewrite certificates at the primitive operation layer. The Lean renderer must consume only fully validated semantic events and turn unresolved transports into explicit proof obligations.

The important invariant is already established: no app-local computation may bypass the reflective library and then be printed as truth.

## Validation performed

```sh
make proof_reflection_test nbonacci_charmpoly_proof_general
```

The reflection test passed. The n-bonacci trigger produced 135 internal library events at `n=8` and wrote:

- `out/nbonacci_reflective_trace.txt`
- `out/nbonacci_reflective_trace.lean`

The environment lacked Lua 5.4 development metadata/headers, so the existing full Lua-linked runner was not rebuilt here. The activation path used the closed declaration reader; the full loader source was updated consistently.

## 2026-08-03 implementation boundary: semantic proof graph

The first vertical slice is implemented.

- `math/include/math/proof_reflection.hpp` now stores a typed DAG.  Matrix
  families, matrix instances, erase-index maps, sparse-support certificates,
  triangularity certificates, determinant identities, lemma applications, and
  unresolved obligations are distinct closed payload types.
- `PolyMatrixZ` retains family and construction provenance.  Every minor
  retains a parent edge and records the exact row/column skip maps intended for
  Lean's `Fin` transport layer.
- Constructing `nbonacci_r_matrix` recognizes lower-bidiagonal structure and
  registers a `Matrix.det_of_lowerTriangular` application.  The renderer emits
  a universal Lean theorem only from that registered structural application.
- Constructing `nbonacci_q_matrix` records `q_minor_reduction` as a typed
  blocked obligation.  Finite determinant equality is not promoted to a Lean
  theorem.
- `lean/Ravel/Matrix/EraseIndex.lean` contains the first shared symbolic skip
  map, value simplifiers, injectivity proof, and the corresponding minor
  operation.

An earlier Lean defect was corrected at the same boundary: the old
`rMatrix_proof_attempt.lean` placed `X` on the superdiagonal despite describing
and computing a lower-bidiagonal matrix.  Its definition now agrees with the
C++ family (`j + 1 = i`).

### Validation actually performed

```
make proof_reflection_test nbonacci_charmpoly_proof_general
./out/proof_reflection_test
./out/nbonacci_charmpoly_proof_general --n=8
```

The run produced 126 parent-linked semantic nodes.  Whether the emitted Lean
modules can be kernel-checked in this environment is detected at session
time, not asserted from prior sessions. Run:

```sh
command -v lean && lean --version
command -v lake && lake --version
```

If both report a usable Lean 4 + Mathlib, the new Lean modules can be
kernel-checked via `make lean-check`, which now includes
`lean/Ravel/Matrix/EraseIndex.lean` (the typed `qMatrix` is generated by the
campaign engine into `out/nbonacci_reflective_trace.lean`, separately
kernel-checkable). If the toolchain is missing, the emitted theorem is recorded
as `generated-unchecked`; the next toolchain-enabled session must run the
verifier before claiming the emitted theorem is kernel-accepted.

### Exact next seam

Stabilize the typed `qMatrix` definition and the in-repository
`qMatrix_minor_eq_qMatrix` proof. The campaign engine already emits a closed
artifact for the q-minor transport; the in-repository kernel-checked route
needs the same case-analysis proof to be parameter-checked through Lean's
`Fin` transport layer (currently the engine-emitted module is checked; the
in-repository file `lean/Ravel/Matrix/EraseIndex.lean` carries the typed
`minorShift` index operation and the per-branch decidable equality
infrastructure, and a finite set of `native_decide` witnesses at concrete
dimensions). Only after that node is checked for every dimension may the
determinant recurrence rule consume it.
