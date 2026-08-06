# Universal n-bonacci runbook

## Build and execute

```sh
make proof_reflection_test
make proof_campaign_engine_test
make qmatrix_typed_ir_test
make nbonacci_charmpoly_proof_general
./out/nbonacci_charmpoly_proof_general --n=8
cat out/nbonacci_proof_campaign.txt
```

## Expected campaign tasks

```text
r_matrix.lower_triangular
r_matrix.determinant
q_matrix.minor_transport
q_matrix.determinant_recurrence
q_matrix.determinant_closed_form
nbonacci.characteristic_split
nbonacci.polynomial_closure
nbonacci.universal_n
```

Every task must close automatically or report a typed missing capability. Manual editing of generated Lean is forbidden.

## Kernel check

After installing the complete matching toolchain:

```sh
scripts/safe_lean_check.sh out/GeneratedCampaign.lean
```

Use the generated path reported by the application if it differs.

## Acceptance condition

Pressing `Go` must:

1. activate reflection from Lua;
2. examine the symbolic problem through the math library;
3. generate an executable campaign;
4. run every dependency and derivation operation automatically;
5. reject unsupported or open operations;
6. emit one completed Lean module; and
7. pass one Lean kernel check under the pinned environment.

A generated plan, raw proof block, manually repaired module, or finite-only confirmation does not satisfy the moonshot.
