# Theta macro-renewal closure report — 2026-08-04

## Result

The two previously unclassified nine-state recurrent boundary SCCs at the
`6 -> 7` transition are now closed by replayable exact evidence.

Their coarsest equitable quotients are identical seven-class weighted cycles:

```text
[0,0,0,0,1,0,0]
[1,0,0,0,0,0,0]
[0,0,0,1,0,0,0]
[0,0,0,0,0,1,0]
[0,0,0,0,0,0,1]
[0,1,0,0,0,0,0]
[0,0,2,0,0,0,0]
```

The new engine operation `derive_macro_renewal_dominance` derives the exact
block-power identity

```text
A^7 = 2 I.
```

Thus the competitor Perron root satisfies `rho(A)^7 = 2`.  The operation does
not numerically extract this root.  It searches for a rational separator `q`
and checks both obligations exactly:

1. `2 < q^7`, by integer exponentiation;
2. `q < rho(core)`, by an exact rational Collatz lower bracket on the
   replay-selected predicted-core quotient.

The resulting proof is recorded on each boundary component as
`macro_renewal=true`, `renewal=7/2`.

## Engineering correction

Four lower grade-two SCC requests at `n=6` do not individually materialize an
upper transported family.  Previously this set the whole run to failure before
the rejected-boundary theorem was interpreted.

That was incorrect bookkeeping.  The failed request shadows are already added
to the rejected grade-two seed set.  The replayed boundary theorem is the
obligation that classifies or dominates every recurrent successor of those
seeds.  Therefore an individual `NO_UPPER_FAMILY` is now explicitly deferred to
boundary exhaustion; it becomes a failure only if that theorem remains open.

## Validation

- `rejected_boundary_exhaustion_test`: PASS
- `corona_projection_test`: PASS
- projected `4 -> 5`: PASS
- projected `5 -> 6`: PASS
- projected `6 -> 7`: PASS

The `n=7` finite complexity seam is closed.  The next proof task is to lift the
macro-renewal recognizer from this fixed quotient to a structural family theorem
or derive the general boundary grammar that guarantees a scalar block-power
identity (or its appropriate generalization).
