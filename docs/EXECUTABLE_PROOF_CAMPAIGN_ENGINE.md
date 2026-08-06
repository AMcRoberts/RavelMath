# Executable proof campaign engine

Date: 2026-08-03

## Purpose

The proof plan generator now targets an installed execution engine. A campaign is not prose and is not serialized directly to Lean. Each generated node names an executable operation, concrete semantic evidence, dependencies, and a state. The executor runs every ready node automatically and accepts only dependency-closed `ClosedProofArtifact` results.

Pipeline:

```text
ordinary exact-math execution
  -> semantic reflection trace
  -> CampaignGenerator
  -> executable ProofCampaign task graph
  -> ProofCampaignExecutor
  -> closed proof artifacts or typed execution failures
  -> Lean serialization of closed artifacts only
```

## Current installed operations

- `ProveLowerTriangular`
  - consumes a typed lower-triangularity certificate and the reflected matrix-family entry-rule kind;
  - executes the support proof by eliminating every nonzero entry branch from the triangular index hypothesis;
  - produces a closed Lean proof AST for `rMatrix_lowerTriangular`.
- `ProveTriangularDeterminant`
  - requires the closed triangularity artifact;
  - composes the determinant theorem with all premises supplied;
  - produces `rMatrix_det`.
- `ProveEraseMinorTransport`
  - is installed and executes its prerequisite analysis;
  - presently reports `BlockedMissingCapability` because the q-family reflection still lacks typed terminal-row branches and symbolic erased-index positions.
- `ComposeUniversalTheorem`
  - is generated with explicit dependencies;
  - remains blocked automatically when q-minor transport does not close.

## Safety invariants

- The matrix constructor no longer records a fake `LemmaApplication` merely because it recognized a shape.
- A task cannot enter `Closed` without a `ClosedProofArtifact`.
- A closed artifact cannot contain open goals.
- The Lean renderer accepts only closed artifacts and omits blocked work entirely.
- Missing executors fail as `BlockedMissingCapability`; they do not become aspirational Lean text.
- Dependency failures propagate automatically.

## Current run result

For the universal n-bonacci trigger at `n=8`:

```text
r_matrix.lower_triangular  Closed
r_matrix.determinant       Closed
q_matrix.minor_transport   Closed   ← was BlockedMissingCapability (2026-08-03)
nbonacci.universal_n       BlockedMissingCapability
```

The exact next engineering task (the seam closed on 2026-08-03) was to replace the q-family's string-level entry description with a typed piecewise entry IR containing:

- ordinary upper-bidiagonal branches;
- terminal-row branches;
- symbolic dimension expressions;
- symbolic erased row and column positions;
- skip-map substitution semantics.

The typed IR now lives in `math/include/math/proof_reflection.hpp` as `DimExpr`, `IndexExpr`, `EntryCondition`, `EntryExpression`, `EntryBranch`, `PiecewiseEntry`. `nbonacci_q_matrix` and `nbonacci_r_matrix` (`math/include/math/poly_matrix.hpp`) record the typed entries via `nbonacci_q_piecewise_entry` and `nbonacci_r_piecewise_entry`. The already-installed `ProveEraseMinorTransport` handler (`include/ravel/proof/proof_campaign_engine.hpp`) executes branch decomposition, arithmetic discharge, and entrywise equality, returning a closed artifact.

## Validation

```sh
make proof_reflection_test
make proof_campaign_engine_test
make qmatrix_typed_ir_test
make nbonacci_charmpoly_proof_general
cat out/nbonacci_proof_campaign.txt
```

## Lean kernel validation

The engine distinguishes internally-closed proof artifacts from kernel-checked artifacts. Whether the emitted Lean module can be kernel-checked is an environmental question that must be checked at session time, not asserted from prior sessions. Detect the available toolchain before any claim about kernel-checked status:

```sh
command -v lean && lean --version
command -v lake && lake --version
```

If both report a usable Lean 4 + Mathlib (this is the case in the current environment, which has `lean 4.32.2` and `lake 5.0.0-src` from the elan toolchain manager), the closed artifact is emitted into `out/nbonacci_reflective_trace.lean` and can be kernel-checked directly:

```sh
cd ../LEAN/free_involution_perron/free_involution_perron \
  && lake env lean /home/anonymous/claude/RavelMathPub/out/nbonacci_reflective_trace.lean
```

The in-repository `lean/Ravel/Matrix/EraseIndex.lean` provides the same `qMatrix_minor_eq_qMatrix` theorem (added to the kernel-checked route on 2026-08-03) and is enrolled in `make lean-check`. If the toolchain is missing, the engine still emits the closed artifact and the campaign report is the source of truth, but the kernel-checked status is recorded as `generated-unchecked` rather than as `Closed`. The next toolchain-enabled session must run the verifier and add the single-pass verifier state before claiming the emitted theorem is kernel-accepted.
