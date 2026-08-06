# Adding a derivation operation

## Decide whether an operation is missing

Use this test:

> Would this capability still belong in the math library or proof engine if the current theorem disappeared?

If yes, implement a generic operation. If no, place the theorem data in the campaign specification and use existing operations.

## Implementation sequence

1. Define the typed input and output structures.
2. Add the operation identifier to the campaign operation vocabulary.
3. Implement the executor using only typed inputs and dependency artifacts.
4. Return a typed failure when a required structure is absent.
5. Add renderer support only for reusable proof primitives.
6. Wire theorem-specific parameters through the campaign specification.
7. Manually audit dimensions, indices, signs, branches, and dependency order.
8. Confirm the executor contains no theorem-family identifiers.
9. Regenerate the campaign. Do not edit the output.
10. Kernel-check once the matching Lean environment is available.

## Examples

### Correct

A generic erased-index matrix-equality operation accepts source and target matrix-family programs plus a typed row/column transform. It emits extensionality, simplification, and arithmetic discharge.

### Incorrect

A `prove_q_minor` handler contains the completed q-matrix case tree or a raw Lean string.

### Correct

A sparse-cofactor operation consumes a sparse-support certificate, pivot expression, cofactor sign, and minor-transport artifact.

### Incorrect

A handler writes the n-bonacci determinant recurrence directly.

## Review grep

Before accepting a generic executor change, search its implementation for theorem-family names and raw Lean channels:

```sh
rg -n 'nbonacci|qMatrix|rMatrix|Raw|sorry' include/ravel/proof math/include/math
```

Matches in campaign data may be legitimate. Matches in generic operation bodies require inspection.
