# Minimax theorem-automation route

## Read first

1. `PROOF_SYSTEM_MANUAL.md`
2. `PROOF_SYSTEM_CONTRACTS.md`
3. `PROOF_SYSTEM_EXTENSION_GUIDE.md`
4. `THEOREM_STATUS.md`
5. The mathematical documents and source files for the selected theorem

Older Minimax continuation notes are archived and are not instructions.

## Operating rule

The theorem is the integration test. The engine is the work product.

For every required step:

1. express the goal and theorem-specific data in the campaign specification;
2. select an installed executable derivation operation;
3. when none applies, implement the missing reusable operation;
4. execute the campaign automatically;
5. render only a closed proof artifact;
6. distinguish internal closure from Lean kernel acceptance.

Never manually perform a missing derivation and place its result in C++, Lean strings, tests, or campaign data.

## Minimax pivots

A pivot is an executable alternative route. It must state:

- when it applies;
- which typed evidence it requires;
- which installed operations it runs;
- what artifact it produces; and
- which typed failure enables the next pivot.

Prefer the route that guarantees the most progress with the fewest assumptions and already-supported operations. Preserve alternate executable routes for known failure states. Do not generate aspirational plans.

## Delivered `Go` contract

Pressing `Go` must generate the campaign, verify that every operation has an executor, run all ready tasks, compose a closed proof, emit Lean, and invoke Lean once when the toolchain is present. No manual translation or generated-proof editing is part of the workflow.
