# Reading list: universal n-bonacci shoot-the-moon campaign

Use this list when resuming the universal n-bonacci characteristic-polynomial proof campaign. It is the authoritative project-local route to the mathematics, C++ machinery, Lua proof definitions, Lean proof seam, tests, and build controls that may need to change.

## Read in this order

1. `docs/NBONACCI_PROOF_GENERATOR_AUDIT_2026-08-02.md`
   - Corrected-source audit, false-positive gates that were removed, and the distinction between finite evidence and a universal proof.

2. `docs/NBONACCI_LUA_PROOF_GENERATOR_ARCHITECTURE.md`
   - Defines the Lua/C++/Lean trust boundary, proof-object architecture, acceptance gates, and staged arbitrary-`n` program.

3. `docs/NBONACCI_SHOOT_THE_MOON_RUNBOOK.md`
   - One-button execution path, campaign artifacts, failure discipline, and the first implementation sequence.

4. `docs/NBONACCI_COURSE_HANDLER_INTERFACE.md`
   - Required C++ course-handler contract, inputs, outputs, classification behavior, and pivot integration.

5. `HANDOVER_2026-08-02_NBONACCI_CHUNK_BASED_GENERATOR.md`
   - Historical handoff for the finite/chunk-based generator. Read critically; bounded generation is not the universal theorem.

6. `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua`
   - Declarative theorem and obligation definitions.

7. `lua/lua_src/ravel/proof/nbonacci_universal_strategy_tree.lua`
   - Ordered strategies and deeper pivots for every blocked obligation.

8. `app/ravel_proof_runner.cpp`
   - Typed campaign orchestration, graph validation, artifact generation, and the course-dispatch seam.

9. `Makefile`
   - Confirm `shoot-the-moon`, Lua linkage, test enrollment, generated Lean checks, and prerequisites before editing source.

## Exact C++ source surface: read before editing

These files either contain the exact machinery used by the campaign or are expected to be touched while implementing the universal proof.

### Primary implementation files — expected to change

1. `app/ravel_proof_runner.cpp`
   - Campaign coordinator, failure classification, artifact layout, run-state updates, and pivot selection.
   - Keep orchestration here; move substantial symbolic implementations into reusable proof-library units.

2. `include/ravel/proof/lua_strategy_loader.hpp`
   - Typed C++ model of Lua obligations, courses, dependencies, pivots, acceptance criteria, and failure signatures.
   - Extend when the declarative schema gains a supported node or certificate type.

3. `src/proof/lua_strategy_loader.cpp`
   - Lua-table decoding and structural validation.
   - Keep Lua declarative: do not allow arbitrary Lua proof logic or unchecked Lean text generation.

4. `app/nbonacci_charmpoly_proof_general.cpp`
   - Current finite determinant and Lean-emission machinery.
   - Contains or uses the concrete matrix families, polynomial determinant computation, geometric sums, n-bonacci characteristic-polynomial construction, and polynomial-to-Lean rendering.
   - Extract shared matrix, determinant, normalization, and Lean-emission code rather than copying it into course handlers.

5. `app/nbonacci_charmpoly_proof_probe.cpp`
   - Earlier finite-range implementation and regression comparison point.
   - Read before consolidating duplicated determinant or emission behavior.

6. `Makefile`
   - Enroll every new reusable C++ source, course-handler test, malformed fixture, Lean checker, and campaign gate.

### Shared n-bonacci witness machinery — preserve and reuse

7. `include/ravel/nbonacci_covering_witness.hpp`
   - Public witness API and data structures.
   - Change only when a proof course genuinely needs additional certified metadata.

8. `src/nbonacci_covering_witness.cpp`
   - Corrected recurrence-coefficient propagation, reconstruction, and full-sequence box validation.
   - Do not fork this implementation into the campaign runner.

9. `app/nbonacci_covering_witness_proof.cpp`
   - Certificate-facing use of the witness core and useful output conventions.

10. `app/nbonacci_covering_witness_enumerator.cpp`
    - Finite search and reconnaissance driver for counterexamples, parameter discovery, and witness generation.

11. `tests/nbonacci_covering_witness_test.cpp`
    - Non-vacuous regression gate. Missing or empty evidence must remain a failure.

### Exact arithmetic and polynomial machinery

12. `math/include/math/poly_z.hpp`
    - Integer-polynomial representation and arithmetic used by determinant generation.
    - Read before adding symbolic normalization, recurrence certificates, equality checks, or Lean serialization.

13. `math/include/math/bigint.hpp`
    - Exact coefficient arithmetic beneath `PolyZ`.
    - Avoid narrowing coefficients through machine integers.

14. `math/include/math/primality.hpp`
    - Included by the finite generator. Verify whether it is genuinely required before propagating the dependency into extracted proof-library code.

15. `tests/nbonacci_block_identity_test.cpp`
    - Nearby exact block-identity tests and fixtures.
    - Add cross-checks here or in a new focused test when matrix-family or recurrence logic is centralized.

## Lean side of the universal seam — expected to change

16. `lean/nbonacci_margin_catalogue.lean`
    - Current universal theorem location and principal arbitrary-`n` determinant/cofactor seam.
    - Generated lemmas must target definitions compatible with this file, not unrelated hard-coded polynomials.

17. `lean/rMatrix_proof_attempt.lean`
    - Prior direct `rMatrix` determinant attempt.
    - Read its index, triangularity, and simplification failures before generating a replacement.

18. `lean/Ravel/Matrix/EraseIndex.lean` *(planned; create if Mathlib lacks the needed reusable API)*
    - Intended shared row/column deletion and reindexing layer for parameterized minors.
    - Both characteristic-minor and `qMatrix` minor-reduction courses should use one implementation.

19. `out/nbonacci_charmpoly_proof_general.lean` *(generated)*
    - Finite generated certificate and regression artifact.
    - Never maintain it by hand or mistake its bounded statements for the universal theorem.

## Lua proof definitions and tests

20. `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua`
    - Declarative theorem objects and proof obligations.

21. `lua/lua_src/ravel/proof/nbonacci_universal_strategy_tree.lua`
    - Alternative courses, failure signatures, and deeper pivots.

22. `lua/lua_src/ravel/tests/test_nbonacci_proof_schema.lua`
    - Structural schema regression test.

23. `lua/lua_src/ravel/tests/test_nbonacci_strategy_tree.lua`
    - Obligation coverage, pivot integrity, and strategy-graph regression test.

24. `lua/scripts/run_lua_tests.lua`
    - Lua test enrollment. Every new schema or strategy fixture must be reachable here.

## Documentation that must stay synchronized

25. `docs/NBONACCI_PROOF_GENERATOR_AUDIT_2026-08-02.md`
26. `docs/NBONACCI_LUA_PROOF_GENERATOR_ARCHITECTURE.md`
27. `docs/NBONACCI_SHOOT_THE_MOON_RUNBOOK.md`
28. `docs/NBONACCI_COURSE_HANDLER_INTERFACE.md`
29. `docs/THEOREM_STATUS.md`
30. `docs/RESEARCH_STATUS.md`
31. `README.md`

Update these whenever the implementation boundary, accepted theorem strength, first command, strategy schema, or generated artifacts change. Do not document a bounded certificate as an arbitrary-`n` theorem.

## Source-edit discipline

Before implementing a course, identify every file above that owns the same mathematical concept. Extract one shared C++ implementation instead of copying matrix, determinant, polynomial, or Lean-emission logic between applications.

A course is not complete until all of the following agree:

- Lua declaration and pivot metadata;
- typed C++ loader representation;
- C++ symbolic implementation;
- build enrollment;
- native regression or mutation test;
- generated standalone Lean checker;
- Lean result or classified failure artifact;
- campaign run state and documentation.

## First command

From the repository root:

```sh
make shoot-the-moon
```

Open the newest `out/shoot_the_moon/<timestamp>/NEXT_MOVES.md` first. Do not weaken a failing proof gate. A blocked obligation must retain its artifacts, receive a failure classification, and pivot through a course encoded in the Lua strategy tree.

## Current implementation boundary

The schema, strategy graph, loader validation, campaign-state output, foundation gates, and launch target exist. The next engineering work is the closed registry of C++ course handlers. Each handler must:

1. generate one standalone symbolic lemma or certificate;
2. validate its own structural claims and finite mutation cases;
3. emit the smallest practical Lean checker file;
4. run or queue Lean checking;
5. classify the first failure without deleting artifacts;
6. choose the next compatible Lua-defined pivot;
7. update the Lua run state and human-readable frontier.

## Rules for this proof path

- Lua defines declarative proof objects, obligations, strategy courses, and campaign state.
- C++ owns typed loading, symbolic manipulation, validation, course dispatch, and Lean emission.
- Lean checks mathematical correctness.
- Shell and Make provide launch and build plumbing only.
- Do not add JSON or Python to this proof path.
- Keep finite-instance evidence distinct from a universal theorem.
- Never treat an emitted formula as proved until it is linked to the actual matrix family and accepted by Lean.
