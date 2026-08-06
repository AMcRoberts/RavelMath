# Reflective proof system contracts

These contracts are acceptance criteria, not suggestions.

## Architectural contracts

1. **The application is only a trigger.** It may select a problem and request outputs. It may not own determinant algorithms, matrix-family definitions, recurrence proofs, or theorem narration.
2. **The math library owns observation.** Structural facts must arise where the corresponding mathematical operation occurs.
3. **A plan is executable.** Every generated campaign operation must have an installed executor before `Go` is allowed.
4. **Implement the missing derivation operation.** Never manually perform a missing derivation and store its result as raw Lean, C++ strings, test fixtures, or theorem-specific handler code.
5. **The executor is theorem-neutral.** The generic executor must not contain `nbonacci`, `qMatrix`, `rMatrix`, or equivalent theorem-family names.
6. **Specifications contain data, not completed proofs.** They may supply definitions, transforms, signatures, and lemma identifiers. They may not supply prewritten proof-step vectors.
7. **No raw proof escape hatch.** Only typed proof operations may close a task.
8. **Only closed artifacts render.** Open goals, candidates, observations, and unsupported operations must be rejected before Lean serialization.
9. **Lean is the verifier.** C++ confidence checks must remain bounded engineering checks; do not recreate Lean in C++.
10. **Kernel status is explicit.** Internally closed, generated-unchecked, and kernel-checked are distinct states.

## Derivation-operation contract

A derivation operation must declare:

- accepted typed inputs;
- structural preconditions;
- produced proof artifact;
- required dependencies;
- renderer operations it may emit;
- typed failure states.

Its implementation must construct the proof program from those inputs. It may not contain the target theorem's completed case tree.

## Static review contract

Before accepting an operation, manually inspect:

- dimension changes;
- index orientation and `Fin` transport;
- erased row and column maps;
- cofactor signs;
- branch coverage and branch exclusivity;
- definition dependency order;
- recurrence base and successor alignment;
- theorem and hypothesis names supplied to the renderer;
- absence of theorem-specific names in executor code.

Tests support the review but do not replace it. Lean remains the final authority.

## Failure contract

A missing capability must be reported directly. Do not downgrade it into plausible generated text. The report must name the missing operation or typed input and list the downstream blocked tasks.
