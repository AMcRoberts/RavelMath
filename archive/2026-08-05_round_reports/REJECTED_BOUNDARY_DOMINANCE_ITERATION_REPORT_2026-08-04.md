# Rejected-boundary dominance iteration report — 2026-08-04

## Completed engineering

The rejected-boundary proof layer now exposes `derive_replayed_boundary_graph`.
It materializes the final `ProjectedCoronaTrace` graph only after replaying every
stored prefix witness against the canonical corona transition oracle.  The
exhaustion classifier and all later spectral operations consume this one replay
object; no parallel transition generator was introduced.

A second reusable operation, `derive_replayed_boundary_dominance`, now attempts
to close each replay-valid unknown recurrent SCC against the uniquely selected
predicted-core SCC.  It first computes coarsest equitable quotients, then tries:

1. reflective exact rank-one paired dominance;
2. finite path-count rank-one dominance;
3. exact rational Perron-bracket comparison.

The proof object records which methods were attempted, quotient sizes, and which
method succeeded.  Unknown SCCs remain unknown unless one operation returns
replayable proof evidence.

## n=7 result

The two 9-state alternating-zero SCCs reduce to 7-class equitable quotients.
The predicted-core SCC reduces to 181 classes.

For each theta SCC:

- reflective rank-one intertwiner: not derived;
- path-count rank-one intertwiner through horizon 160: not derived;
- exact rational Perron brackets through 160 iterations: inconclusive.

Therefore the boundary theorem remains honestly open.  The code does not absorb
or relabel these SCCs.

## Interpretation

The obstruction is no longer graph discovery or corona replay.  It is the
periodic/sparse spectral certificate for one fixed seven-class theta quotient.
The next operation should derive a block-step cone or exact recurrence for that
quotient.  Promising routes:

- use a common period/block power before Collatz comparison;
- derive the quotient characteristic polynomial and isolate its Perron root;
- derive a macro-renewal grammar from the two seven-cycles sharing a stem;
- compare that recurrence directly with the predicted-core recurrence.

The four lower SCCs reporting `NO_UPPER_FAMILY` remain a separate transport
engineering seam (sizes 6, 6, 6, and 2).  Their failed family requests feed the
same replay boundary, but individual adjacent-family transport is not yet
proved for them.

## Validation

- `make rejected_boundary_exhaustion_test` — PASS
- `make corona_projection_test` — PASS
- real projected `6 -> 7` run — replay valid; spectral obstruction retained
- log: `out/adjacent_competitor_transport_n6_round37_final.log`
