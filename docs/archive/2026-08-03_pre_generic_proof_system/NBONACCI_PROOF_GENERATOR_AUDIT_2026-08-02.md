# n-bonacci proof-generator audit and continuation plan

Date: 2026-08-02

## Executive finding

The recent work contains a promising certificate-generation architecture, but its original handoff overstated what was proved. The bounded Lean output proved identities among hard-coded polynomials, not initially the determinants of the emitted matrices, and its alleged bounded general statement ended in `sorry`. The reusable covering-witness extraction also propagated only the constant coefficient of the recurrence, so it solved the wrong linear systems for `n>2`. The regression test then passed when it checked no evidence files at all.

This audit corrects those source defects and makes the remaining universal theorem boundary explicit.

## Corrections applied

1. `src/nbonacci_covering_witness.cpp`
   - Propagates every basis coefficient through `a[t+n] = a[t] - sum(a[t+1..t+n-1])`.
   - Rejects candidates unless the entire reconstructed sequence lies in `[-1,1]`, not merely the free parameters.
   - The corrected reusable core returns witnesses for `n=2..8` whose public box and cover validators pass.

2. `tests/nbonacci_covering_witness_test.cpp`
   - Exercises the reusable C++ implementation directly.
   - Refuses vacuous success: all seven expected evidence sidecars must be present and contain candidates.
   - Retains exact rational reconstruction, pin, box, and cover checks.

3. `Makefile`
   - Links the covering-witness regression test against the reusable implementation it now tests.

4. `app/nbonacci_charmpoly_proof_general.cpp`
   - Corrects the emitted `rMatrix`: diagonal `-1`, subdiagonal `X`.
   - Corrects the emitted `qMatrix`: diagonal `X`, superdiagonal `-1`, final row of ones ending in `1+X`.
   - Emits finite matrix/determinant linkage checks `Matrix.det (rMatrix n) = rMatrix_det_at n` and similarly for `qMatrix`.
   - Replaces the `sorry`-containing bounded wrapper with a finite interval proof intended to close by `interval_cases` and `native_decide`.
   - Keeps the universal theorem explicitly open.

## What remains unproved

The project still lacks a theorem for arbitrary `n` connecting the characteristic matrix determinant to the n-bonacci polynomial. Finite checks through `n=8` are useful regression evidence, but they are not induction and cannot establish the universal claim.

The universal proof should be split into four independently testable lemmas:

1. **Characteristic-matrix cofactor decomposition.** Define one canonical `charmatrix n`. Prove expansion along the chosen column produces `X * det(qMatrix n) + (-1)^n * det(rMatrix n)` with exact index transports.
2. **`rMatrix` determinant.** Prove lower-triangularity and obtain `det(rMatrix n) = (-1)^(n-1)` from the diagonal product.
3. **`qMatrix` determinant recurrence.** Prove `qdet (n+1) = X*qdet n + 1`, with base `qdet 2 = 1+X`; conclude `qdet n = sum_{k<n} X^k`.
4. **Polynomial closure.** Rewrite the cofactor expression using the two determinant formulas and normalize to `sum_{k<n} X^(k+1) - 1`.

## Lua-defined, inline C++ symbolic-proof generation strategy

The canonical proof-object definitions are Lua modules returning declarative tables. See `NBONACCI_LUA_PROOF_GENERATOR_ARCHITECTURE.md` and `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua`. JSON is not part of this design. Lua is outside the trust kernel: C++ must parse into closed typed variants and independently validate every node before Lean emission.

The C++ prover should generate certificates, not substitute for proofs. Add a small proof IR with nodes such as `MatrixEntry`, `Minor`, `DetExpansion`, `TriangularProduct`, `PolynomialRecurrence`, `RingIdentity`, and `IndexBijection`. Each node must carry:

- a machine-checkable statement;
- dependencies by stable identifier;
- the exact dimensions and index maps;
- a Lean rendering;
- a C++ verifier that recomputes the claimed transformation.

For the universal seam, generate a **schema certificate** rather than one certificate per numeric `n`:

- a symbolic description of the nonzero pattern of each matrix row;
- the minor-isomorphism map after deleting a row/column;
- the recurrence for the remaining determinant;
- base cases;
- the final polynomial recurrence and closed form.

Lean should check the schema once for arbitrary `n`. Numeric instances remain regression tests that compare the schema-generated form with direct determinant computation.

### Recommended implementation sequence

1. Introduce `include/ravel/proof_ir.hpp` and `src/proof_ir.cpp` with a Lua-table loader, closed typed proof nodes, validators, and Lean emitters.
2. Move matrix constructors into a shared header used by both the direct determinant checker and the emitter.
3. Add `--schema=<lua module>`, `--lean-out`, and `--self-check` modes to the C++ generator. Do not add a JSON certificate format.
4. Emit a lemma for the `qMatrix` minor equivalence at symbolic size `n+1 -> n`.
5. Emit the determinant recurrence theorem and prove it in Lean using the generated index equivalence.
6. Emit the geometric-sum solution as a generic polynomial induction.
7. Only after those pass, emit the characteristic-matrix cofactor theorem.

## Required tests

### Build and source tests

- Basis propagation unit test: compare every coefficient generated by `later_values` with sequences reconstructed from each standard basis vector.
- Candidate soundness test: every returned witness passes exact recurrence, exact box, exact pins, and exact cover.
- Enumerator/library parity test: the reusable core and standalone enumerator return the same canonical best witness under a fully specified tie-break.
- Empty-evidence failure test: regression test must return nonzero when sidecars are missing.
- Malformed evidence tests: reject truncation, bad fractions, inconsistent lengths, out-of-range indices, and duplicate pins.
- Overflow stress test: replace `long long` rationals with project `BigInt`/`PolyQ` machinery or prove boundedness before increasing `n`.

### Generator tests

- Generated Lean contains no `sorry`, `admit`, or undeclared axiom.
- Every emitted determinant constant is linked to `Matrix.det` of the emitted matrix.
- C++ direct determinant equals emitted closed form for a configurable range.
- Generated output is deterministic byte-for-byte.
- Lean compilation is mandatory in CI; absence of Lean is a failed or explicitly skipped job, never a pass.
- Mutation tests: deliberately swap `X` and `1`, reverse the subdiagonal condition, or alter one sign; matrix/determinant linkage must fail.

## Operator walkthrough

1. Install C++20 compiler, GNU Make, Lua 5.4 development files, Lean toolchain, and Mathlib dependencies.
2. From the repository root, run `make clean` and `make -j2 all`.
3. Run `make nbonacci_covering_witness_enumerate`; confirm seven nonempty `n2_L3` through `n8_L9` sidecars exist.
4. Run `make nbonacci_covering_witness_test`; any missing evidence or invalid candidate is a stop condition.
5. Run `make nbonacci_charmpoly_proof_general`.
6. Search the generated Lean file for `sorry`, `admit`, and `axiom`; there must be no unauthorized occurrence.
7. Compile the generated file with the repository Lean environment.
8. Run the direct C++ determinant cross-check over at least `n=2..12`.
9. Run the universal-schema tests separately from the finite-range tests. Do not label a finite bound as “general n.”
10. Save command logs and generated certificate hashes under `out/audit/<date>/`.

## Current validation boundary

C++ compilation of the modified generator and witness test succeeded in the audit environment. The corrected reusable witness core was directly exercised for `n=2..8` and returned validator-clean witnesses. The evidence regression now fails as intended when generated sidecars are absent. Lean itself was not installed in the audit environment, so the newly emitted Lean file could not be kernel-compiled here; that remains a mandatory validation step on a configured project machine.
