# Lua-encoded universal n-bonacci proof generator

Date: 2026-08-02

## Decision

The project does **not** use JSON for proof-object or data-structure definitions. The universal proof generator uses Lua modules returning declarative tables. This follows the repository's existing boundary: Lua defines data and orchestration; bounded C++ performs exact symbolic work; Lean remains the final proof checker.

The initial schema is:

`lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua`

Its structural regression test is:

`lua/lua_src/ravel/tests/test_nbonacci_proof_schema.lua`

The Lua file is neither a proof nor trusted executable evidence. It names the objects, obligations, dependency graph, validators, and Lean emitters that the C++ prover must implement.

## Why the universal theorem is still open

The existing generator can compute and certify individual dimensions. That does not establish the theorem for arbitrary `n`. The obstruction is structural: the proof must show that deleting particular rows and columns from an `n`-dependent matrix always produces the claimed smaller matrix, with the correct index transport and cofactor sign.

Finite determinant calculations hide this problem because concrete matrices can simply be evaluated. For symbolic `n`, Lean must be given reusable maps between finite index types and proofs that every transported entry agrees. Until those minor-isomorphism lemmas exist, the recurrence is an observed pattern rather than a theorem.

The open chain is:

1. define one canonical characteristic matrix family;
2. prove its two nonzero cofactors transport exactly to `q_matrix` and `r_matrix`;
3. prove `r_matrix` is triangular and compute its determinant;
4. prove the dimension-lowering minor identity for `q_matrix`;
5. derive `qdet(n+1) = X*qdet(n) + 1`;
6. solve that recurrence by induction as a geometric sum;
7. normalize the final polynomial identity.

The difficult step is not serialization and not polynomial arithmetic. It is the parameterized finite-index bookkeeping in steps 2 and 4.

## Runtime architecture

### Lua layer

Lua modules return plain tables containing:

- schema version and stable identifiers;
- symbolic parameters and assumptions;
- matrix-family names and entry-rule identifiers;
- typed proof obligations;
- explicit dependency identifiers;
- names of closed C++ validators and Lean emitters;
- finite regression ranges.

Lua may select and compose known schema constructors, but it must not inject arbitrary Lean source or register arbitrary C++ callbacks. Unknown keys, kinds, validators, and emitters are errors.

### C++ layer

Add a closed typed representation under `include/ravel/proof/` and `src/proof/`. Suggested variants are:

- `MatrixFamily`
- `IndexBijection`
- `MinorEquivalence`
- `DetExpansion`
- `TriangularProduct`
- `DeterminantRecurrence`
- `BaseCase`
- `PolynomialInduction`
- `RingIdentity`
- `TheoremNode`

The loader uses the existing Lua 5.4 integration to read the returned table. It converts each table into a typed value and rejects malformed fields, duplicate identifiers, unsupported node kinds, cycles, unresolved dependencies, and dimension expressions outside the supported grammar.

Every validator recomputes its claim from matrix entry rules and index maps. The emitter is allowed to render Lean only after validation succeeds.

### Lean layer

Lean receives definitions and lemmas, not an assertion that C++ should be trusted. Generated files must:

- contain no `sorry`, `admit`, or unauthorized axiom;
- state the theorem for arbitrary `n` with its lower-bound assumption;
- prove index-map well-formedness and entry preservation;
- derive determinant recurrences from Mathlib determinant lemmas;
- solve the polynomial recurrence by induction;
- be accepted by the repository's pinned Lean/Mathlib environment.

## Required command-line surface

The C++ generator should converge on:

```text
nbonacci_charmpoly_proof_general \
  --schema=lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua \
  --lean-out=out/nbonacci_charmpoly_universal.lean \
  --self-check \
  --regression-min=2 \
  --regression-max=12
```

No `--certificate-json` mode should be added. A diagnostic dump, if needed, should be a canonical Lua module or a human-readable text report. The canonical input remains Lua.

## Implementation stages and gates

### Stage 1 — schema loader

Implement a Lua-table loader and typed dependency graph. Gate: the checked-in schema loads; malformed fixtures fail deterministically; no proof is emitted yet.

### Stage 2 — shared matrix families

Move the characteristic, `q`, and `r` entry rules into shared C++ code used by direct determinant computation, schema validation, and Lean emission. Gate: direct finite determinants are unchanged for `n=2..12`.

### Stage 3 — `r_matrix`

Generate the arbitrary-`n` triangular determinant proof. Gate: Lean accepts it independently.

### Stage 4 — `q_matrix` minor transport

Generate the explicit `Fin` index maps for the selected cofactor and prove entry equality. Gate: Lean accepts the minor-equivalence lemma independently; mutation of one index or sign fails.

### Stage 5 — recurrence and closed form

Generate and check the determinant recurrence and geometric-sum induction. Gate: Lean accepts both with no finite enumeration in the theorem proof.

### Stage 6 — characteristic cofactor split

Generate the characteristic-matrix expansion and both minor transports. Gate: Lean accepts the split independently.

### Stage 7 — universal theorem

Compose the checked lemmas and normalize the polynomial. Gate: the theorem is quantified over arbitrary `n >= 2`; the finite interval wrapper is retained only as regression evidence.

## Tests to build

1. Lua schema shape, unique IDs, dependency closure, and cycle rejection.
2. Unknown node kind, validator, emitter, or matrix entry rule rejection.
3. Missing field, wrong type, duplicate ID, missing dependency, and cyclic dependency fixtures.
4. Deterministic schema loading and deterministic Lean output.
5. Shared matrix entry rules agree with the current direct constructors for `n=2..12`.
6. Every generated minor map is bijective and preserves every entry in finite regressions.
7. Mutation tests for row/column deletion, index shifts, `X`/`1`, and cofactor signs.
8. Generated Lean contains no proof holes or unauthorized axioms.
9. Lean compilation is a required CI job, not silently treated as a pass when unavailable.
10. The universal theorem test rejects any proof whose conclusion or hypotheses contain a concrete upper bound on `n`.

## Beginner-safe continuation walkthrough

1. Run `make clean && make -j2 all`.
2. Run `make lua`; stop on any Lua or native-library failure.
3. Run `cd lua && lua5.4 scripts/run_lua_tests.lua`; confirm `n-bonacci Lua proof schema: PASS`.
4. Read the schema from top to bottom. Each obligation names exactly one missing mathematical step and its prerequisites.
5. Implement only the first unimplemented validator. Do not emit Lean for a node that does not self-check.
6. Add one valid fixture and at least one deliberately broken fixture for that validator.
7. Run direct finite comparisons across `n=2..12`.
8. Emit the corresponding standalone Lean lemma and compile it.
9. Only then mark that obligation implemented and move to the next dependency.
10. After every stage, run `make check` and `make lean-check`, save the command log, and update `docs/THEOREM_STATUS.md` without strengthening the status beyond what Lean accepted.

## Trust statement

Lua describes the proof plan. C++ checks and translates the plan. Lean decides whether the theorem follows. A successful Lua load or C++ finite regression is not itself the universal proof.

## Campaign controller and pivot tree

The implementation now includes a concrete strategy module at:

`lua/lua_src/ravel/proof/nbonacci_universal_strategy_tree.lua`

It contains 40 named proof courses distributed across every blocked obligation. Failure edges are explicit and validated by both Lua and C++. The one-button controller is:

```sh
make shoot-the-moon
```

The C++ runner is intentionally a closed parser/controller. Course-specific symbolic handlers are the next implementation layer; they must be registered in C++, independently validate their certificates, and emit standalone Lean lemmas before a course can be marked successful. See `docs/NBONACCI_SHOOT_THE_MOON_RUNBOOK.md`.
