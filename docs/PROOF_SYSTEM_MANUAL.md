# Reflective proof system manual

Start with the `Reflective proof engine` section of `MATHEMATICAL_API.md`. That section is the public API and bootstrap breadcrumb. This manual owns the operating workflow; `PROOF_SYSTEM_CONTRACTS.md` owns invariants; `PROOF_SYSTEM_EXTENSION_GUIDE.md` owns extension procedure.

## Purpose

The system turns ordinary symbolic mathematics into a completed Lean proof through reusable reflective machinery.

The intended flow is:

```text
Lua activation
→ ordinary math-library execution
→ semantic reflection
→ campaign generation
→ executable derivation operations
→ closed proof artifact
→ Lean serialization
→ one kernel check
```

Lean is the verifier. C++ is not a replacement theorem prover. C++ recognizes structures and executes carefully implemented derivation operations that assemble the proof Lean will check.

## API and locations

The authoritative symbol and location table is in `MATHEMATICAL_API.md` under `Reflective proof engine`. Use `SOURCE_FILE_ATLAS.md` for repository-wide ownership. Do not maintain a second location list here.

## Division of responsibility

### Math library

Owns mathematical definitions, exact operations, structural recognizers, and reflective pivots. A theorem-specific application must not duplicate these.

### Lua

Activates reflection and selects requested goals or policies. Lua describes configuration, not proof text.

### Campaign specification

Contains theorem data: family identifiers, definitions, transforms, target signatures, and permitted generic operations.

### Campaign executor

Schedules dependencies and executes installed generic operations. It must not contain theorem-family names or completed theorem-specific proof scripts.

### Lean renderer

Serializes closed proof artifacts. It must reject open goals, unsupported operations, and raw proof text.

### Lean

Performs the formal check. A campaign state called `Closed` means internally dependency-closed; kernel acceptance must be reported separately.

## Running the universal-n campaign

```sh
make proof_reflection_test
make proof_campaign_engine_test
make qmatrix_typed_ir_test
make nbonacci_charmpoly_proof_general
./out/nbonacci_charmpoly_proof_general --n=8
cat out/nbonacci_proof_campaign.txt
```

The universal-n checkpoint has now been kernel checked. For new or regenerated campaigns, run:

```sh
scripts/safe_lean_check.sh out/GeneratedCampaign.lean
```

and record the exact artifact hash before claiming `KernelChecked`.

Use the actual generated filename if the application reports a different path.

## Reading campaign output

Task states emitted by the current executor are:

- `Discovered`, `DependenciesPending`, `Ready`, `Running`;
- `Closed` — all internal dependencies and proof steps are complete;
- `BlockedMissingCapability` — no installed executor or required typed capability is available;
- `BlockedDependency` — a prerequisite did not close;
- `Failed` — supplied evidence or specification was invalid.

`UnsupportedDerivationOperation` is the diagnostic meaning of a missing executor, normally reported through `BlockedMissingCapability`. `GeneratedUnchecked` and `KernelChecked` are external artifact-verification statuses, not `TaskState` values. Never translate `Closed` into `KernelChecked` without running Lean.

## Proper use

When a task blocks, inspect the missing capability. Add or improve a generic derivation operation if the same mathematical operation would be useful outside this theorem. Keep theorem data in the campaign specification.

Do not repair generated Lean by hand. A hand repair identifies a missing renderer feature, derivation operation, or incorrect specification. Fix that layer and regenerate.
