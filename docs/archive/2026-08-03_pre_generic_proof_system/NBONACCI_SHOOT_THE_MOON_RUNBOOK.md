# Shoot-the-moon runbook: universal n-bonacci theorem

Date: 2026-08-02

Before changing source, read `docs/READINGLIST.NBONACCI_SHOOT_THE_MOON.md`. It is the authoritative map of the exact C++, Lua, Lean, arithmetic-library, test, build, and documentation files involved in this campaign.

## The button

From the repository root:

```sh
make shoot-the-moon
```

This is deliberately shell + C++ + Lua + Lean. It does not introduce Python into the universal-proof architecture.

The target builds `out/ravel_proof_runner`, validates the Lua theorem schema and strategy tree, runs every currently available foundation gate, and writes a timestamped campaign directory under:

```text
out/shoot_the_moon/YYYYMMDD-HHMMSS/
```

Open `NEXT_MOVES.md` first. Then inspect `FRONTIER.md`, `run_state.lua`, and `logs/`.

## What is automated now

The checked-in Lua tree gives every universal proof obligation at least two routes and gives the hard obligations four to seven routes. The C++ loader rejects missing obligations, duplicate course IDs, unresolved dependency edges, and invalid pivot references. The runner produces:

- a human-readable move tree;
- a Lua-encoded mutable run state;
- a command queue;
- per-step logs and status files;
- artifact directories for generated Lua certificates and Lean files.

The available foundation execution checks the Lua schemas, the corrected covering-witness regression, the existing finite characteristic-polynomial generator, proof-hole scans, and the pinned Lean suite when available.

## What remains to implement

The runner is now the campaign controller, not yet the full symbolic prover. Each course in the Lua strategy tree names the C++ validator and Lean artifact that should be added next. The next engineer should implement one course at a time behind a closed C++ course-handler registry. A handler must:

1. read only typed schema objects;
2. generate its proposed index maps, row operations, recurrence certificate, or Lean lemma;
3. independently validate the proposal over symbolic structure and finite mutation tests;
4. write its artifacts into the active run directory;
5. invoke Lean on the smallest standalone lemma;
6. classify failure against the course's `failure_signatures`;
7. update `run_state.lua` and select the first untried `next_on_failure` course.

Do not make Lua callbacks execute arbitrary C++ or emit arbitrary Lean text. Lua selects among closed, named mechanisms. C++ validates and renders. Lean checks.

## First implementation sequence

The best first attack is shared infrastructure, because it unlocks two obligations:

1. Implement `cm.reindex_library` as an inventory pass over the adjoining Mathlib sources.
2. If the necessary `Fin` deletion/reindex lemmas exist, implement `cm.direct_fin_equiv` and `qm.shared_erase_index` using them.
3. Otherwise implement `cm.custom_erase_index` once in `lean/Ravel/Matrix/EraseIndex.lean`.
4. Use that module for both `characteristic_minor_maps` and `q_minor_reduction`.
5. In parallel, implement `rt.triangular_api`, which should be comparatively independent.
6. Once `q_minor_reduction` is accepted, attack `qr.sparse_laplace`; pivot to the continuant route if support bookkeeping dominates.
7. Solve the recurrence with `qc.shifted_sequence` if lower-bound arithmetic makes direct induction noisy.
8. Finish the characteristic split and polynomial closure only after the matrix transports compile independently.

## Failure discipline

A failed route is evidence, not debris. Never overwrite its generated Lean file or log. Record:

- exact command;
- compiler and Mathlib revisions;
- first failing theorem or goal;
- whether the failure is definitional equality, index arithmetic, determinant API mismatch, simplifier behavior, or a mathematical contradiction;
- mutation-test results;
- selected next course and reason.

The strategy tree is intentionally deeper than a normal task list. It includes changes of representation—direct `Fin` maps, block matrices, recursive matrix families, permutation certificates, sparse Laplace lemmas, continuants, row operations, polynomial evaluation, and Berkowitz recursion—so a local API dead end does not end the campaign.

## Success condition

The campaign succeeds only when the pinned Lean kernel accepts an arbitrary-`n` theorem with no `sorry`, `admit`, unauthorized axiom, or concrete upper bound. Finite C++ checks remain regression evidence, not the universal proof.
