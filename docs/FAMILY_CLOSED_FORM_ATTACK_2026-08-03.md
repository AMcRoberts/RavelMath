# Family closed-form attack — 2026-08-03

## Purpose

Use the universal-n proof methodology on the family-of-families frontier:
replace a finite observed law with an explicit arbitrary-parameter certificate,
then isolate the remaining bridge required for a full Lean theorem.

## First promoted law: adjacent-swap counts

For the Class-II family

```text
0 -> 0^a 1^b 2
1 -> 0^a 2
2 -> 0
```

with `a,b>=1`, there are exactly three adjacent unequal run boundaries:
`0|1`, `1|2`, and the second image's `0|2`. Each produces a distinct
substitution and preserves every Parikh column.

For n-bonacci, every image except the last is the two-letter word
`0(i+1)`. Hence exactly one adjacent swap is available in each of the first
`n-1` images and none in the last: the count is `n-1`.

## Artifacts

- `include/ravel/family_closed_forms.hpp` gives closed forms, constructors, and
  certificates against the generic neighbor generator.
- `tests/family_closed_forms_test.cpp` checks Class-II through `a=4096` at five
  distinct positive `b` values and all encodable n-bonacci dimensions `2..127`.
- `lean/family_of_families_adjacent_swap_counts.lean` states the arbitrary-
  parameter cardinality theorems as a standalone validation file.

## Exact status boundary

The C++ theorem is executable and independently cross-checked against the
generic adjacency algorithm. The Lean file kernel-checks the closed-form site
catalogues. A later bridge can formalize that these site catalogues are exactly
the output of a Lean implementation of the generic adjacent-swap generator.

## Next hard target

Move from this small combinatorial family theorem to one of the explicit affine
Class-II catalogues. Use the same decomposition:

1. exact parametric grammar;
2. forward generation;
3. reverse exhaustion;
4. distinctness and validity;
5. dominance only after literal occurrence and exhaustion are closed.

The current best target is the neighbor dominant-core catalogue whose matrix is
`A(a)=A(3)+(a-3)D`: the determinant polynomial is already closed-form, while
complete-graph occurrence/dominance is the remaining bridge.
