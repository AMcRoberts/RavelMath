# Structural block-power derivation report — 2026-08-04

## Result

The theta quotient block-power identity is no longer discovered by searching
successive matrix powers.

The new reusable engine operation
`derive_weighted_cycle_block_power` derives the identity from the quotient's
weighted successor grammar:

1. every quotient row has exactly one positive entry;
2. the successor map has indegree one at every class;
3. following successors from one class visits every quotient class exactly once
   and returns to the start;
4. the multiplicity is the product of the edge weights around that cycle.

For a quotient with cycle length `p` and cycle-weight product `m`, every
`p`-step walk returns uniquely to its starting class with weight `m`. Therefore

```text
A^p = m I.
```

The operation records the successor map, edge weights, cycle order, period, and
multiplicity. Matrix powering is retained only as an independent replay check of
the structurally derived certificate; it is not used to search for `p` or `m`.

For the two `n=7` theta SCC quotients, the derived values are

```text
p = 7
m = 2
A^7 = 2 I.
```

`derive_macro_renewal_dominance` now consumes this structural certificate before
performing the existing exact rational separation from the predicted core.

## Negative gate

A quotient with an additional positive successor is rejected as non-functional;
a successor permutation with more than one cycle is also rejected. Thus this
operation cannot silently reinterpret a branching or disconnected recurrence as
a scalar block-power family.

## Validation

- `rejected_boundary_exhaustion_test`: PASS
  - structural derivation of the seven-class quotient;
  - exact replay of `A^7 = 2 I`;
  - rejection of a deliberately branching quotient.
- `corona_projection_test`: PASS
- `recurrent_family_exhaustion_real_test`: PASS

The remaining universal step is to derive, from corona boundary state grammar,
that every relevant periodic quotient has this weighted-cycle structure (or to
classify the alternatives). The matrix-level block-power theorem itself is now
an evidence-producing engine operation.
