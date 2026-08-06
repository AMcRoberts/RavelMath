# Rejected-State Shape Invariant — Round 48

## Result

The rejected-boundary proof now has a typed upstream shape certificate between
replay validation and symbolic label-schedule matching.

For every recurrent component that is neither:

- wholly transported/core, nor
- a terminal one-in/one-out permutation component,

`derive_rejected_state_shape_invariant` checks and records, in order:

1. the displacement has catalogue grade exactly two;
2. every coordinate lies in `{-1,0,1}`;
3. exactly one coordinate is zero;
4. deleting that zero leaves cyclic alternation.

Only components carrying this certificate may enter the alternating-zero
endpoint grammar and schedule proof.

## Why grade two is upstream

The adjacent transport campaign constructs the rejected boundary with an
acceptance request restricted to grade one or grade two. The independently
supplied transported/core predicate removes the grade-one branch. Therefore an
unresolved recurrent state must replay as grade two. The engine now checks this
explicitly instead of leaving it implicit in the caller.

## Evidence boundary

This round makes the finite replay implication explicit and reusable. It does
not yet constitute a dimension-free algebraic proof that the n-bonacci
transition equations force the four shape clauses. That remaining theorem is
now isolated cleanly: replace the finite checks inside
`derive_rejected_state_shape_invariant` with a symbolic defect/rank derivation
without changing downstream schedule, phase-twist, or dominance code.

## Validation

- `rejected_boundary_exhaustion_test`: PASS
- `corona_projection_test`: PASS
- projected `4 -> 5`: PASS
- projected `5 -> 6`: PASS
- projected `6 -> 7`: PASS

The real reports now expose `shape_invariant=true` on the two n=7 renewal SCCs.
