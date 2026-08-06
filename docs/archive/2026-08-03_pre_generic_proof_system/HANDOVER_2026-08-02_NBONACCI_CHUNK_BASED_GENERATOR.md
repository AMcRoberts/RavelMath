# Corrected handover — n-bonacci proof generator

Date: 2026-08-02

The authoritative audit is `docs/NBONACCI_PROOF_GENERATOR_AUDIT_2026-08-02.md`.

## Corrected status

The C++/Lean generator is a finite-certificate prototype, not yet a universal proof. The earlier generated bounded wrapper contained `sorry`, and the emitted matrix definitions did not match the C++ matrices. Both source issues have been corrected. The generator now emits finite `Matrix.det` linkage checks and a sorry-free bounded wrapper intended for Lean verification.

The reusable covering-witness extraction had a substantive recurrence bug: it propagated only the constant coefficient column. It now propagates every basis coefficient and validates the entire reconstructed sequence. Direct C++ checks produce validator-clean witnesses for `n=2..8`.

The covering-witness regression test no longer passes with zero evidence. Run the enumerator first; missing or empty sidecars are a hard failure.

## Universal theorem boundary

Still open:

- cofactor expansion of the canonical characteristic matrix for arbitrary `n`;
- universal determinant formula for `rMatrix`;
- universal determinant recurrence and geometric-sum formula for `qMatrix`;
- final polynomial normalization.

The next engineering objective is a Lua-defined, C++-validated proof schema that emits symbolic recurrence and index-bijection lemmas, with Lean checking the schema for arbitrary `n`. The checked-in starting schema is `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua`; JSON is not used. Numeric generation remains regression support only. See `docs/NBONACCI_LUA_PROOF_GENERATOR_ARCHITECTURE.md`.

## Required execution order

```text
make clean
make -j2 all
make nbonacci_covering_witness_enumerate
make nbonacci_covering_witness_test
make nbonacci_charmpoly_proof_general
make lean-check
```

Then independently compile `out/nbonacci_charmpoly_proof_general.lean` and reject any output containing `sorry`, `admit`, or unauthorized axioms.

## Files changed in this correction

- `src/nbonacci_covering_witness.cpp`
- `tests/nbonacci_covering_witness_test.cpp`
- `app/nbonacci_charmpoly_proof_general.cpp`
- `Makefile`
- this handover
- `docs/NBONACCI_PROOF_GENERATOR_AUDIT_2026-08-02.md`
