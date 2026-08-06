# Rejected-boundary replay report — 2026-08-04

## Implemented

The recurrent-family exhaustion seam now consumes the final edge layer of
`ProjectedCoronaTrace` directly.  It does not regenerate an approximate graph.
Every retained edge is checked by replaying its stored pair of prefix witnesses
against the canonical simple-transition oracle for the selected arithmetic
mode.  SCC classification proceeds only after the trace is converged, cap-free,
and witness-valid.

New reusable engine operation:

- `derive_rejected_boundary_exhaustion_from_trace`
- source: `include/ravel/proof/rejected_boundary_exhaustion.hpp`

It classifies every recurrent SCC as:

1. an already transported family or predicted core;
2. a terminal one-in/one-out permutation component; or
3. an explicit unknown obstruction.

The operation returns typed evidence and never silently absorbs an unknown SCC
into the family catalogue.

## Validation

- `make rejected_boundary_exhaustion_test` — PASS
- `make corona_projection_test` — PASS
- projected adjacent transport `4 -> 5` — PASS
- projected adjacent transport `5 -> 6` — PASS
- projected adjacent transport `6 -> 7` — expected proof obstruction, replay valid

Logs:

- `out/adjacent_competitor_transport_n4.log`
- `out/adjacent_competitor_transport_n5.log`
- `out/adjacent_competitor_transport_n6.log`

## Exact n=7 seam

The replay proof isolates two previously unclassified recurrent SCCs, each with
9 states and 10 witnessed internal edges.  Every state has catalogue grade two.
They are sign-reversed alternating-vector families with a moving zero, plus
label-phase closure states.  They are not predicted-core states and were not
included in the transported-family set.

Representative displacement patterns:

- `[0,1,-1,1,-1,1,-1]` and cyclic zero shifts;
- `[0,-1,1,-1,1,-1,1]` and cyclic zero shifts.

This is a much narrower seam than a general rank search.  The next proof should
first test whether these are the two orientations of one uniform alternating
terminal family.  They are not presently classified as permutation SCCs because
each 9-state SCC has 10 internal witnessed edges, so one phase has a branch or
parallel continuation.  The exact edge and prefix witness causing the tenth
edge should be inspected before selecting a theorem:

- if the branch merges after a bounded phase, derive a macro-step permutation
  certificate;
- if it is a transported lower-dimensional family omitted by the current
  family matcher, repair the matcher and return its transport evidence;
- if neither, derive its explicit recurrence and spectral comparison as a new
  uniform terminal family.

## Other n=7 engineering obstruction

Four small lower recurrent SCCs (`6, 6, 6, 2` states; grades 3, 2, 2, 2) report
`NO_UPPER_FAMILY` in the per-family transport loop.  They do not invalidate the
boundary replay operation, but universal family transport remains incomplete
until they are either recognized as terminal/permutation artifacts or matched
through a macro-family relation.
