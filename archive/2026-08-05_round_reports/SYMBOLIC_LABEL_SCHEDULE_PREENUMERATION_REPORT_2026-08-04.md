# Symbolic Label-Schedule Pre-enumeration — 2026-08-04

## Result

The label-pair schedule for the alternating-zero renewal family is now derived before full rejected-boundary SCC enumeration.

The new operation `derive_nbonacci_symbolic_label_schedule` constructs only the bounded symbolic schema

- orientation in `{ -1, +1 }`;
- zero phase `z in {0,...,D-1}`;
- endpoint labels `(i,j) in {0,...,D-1}^2`;
- displacement fixed by the alternating-zero formula.

This is at most `2 D^3` candidates. Candidate validity is delegated to the canonical `is_valid_simple_node` predicate. Edges and multiplicities are delegated to `simple_forward_targets_exact`, so there is no second transition generator.

For `D=7`, the symbolic schema contains:

- 686 valid labelled candidates;
- 674 exact parent-prefix witnesses;
- exactly two recurrent renewal schedules;
- 9 states in each schedule;
- one schedule for each orientation;
- the same unique doubled phase `z=2` in both schedules.

## Derived label schedules

Orientation `-1`:

- `z=0`: `(0,0)`, `(1,2)`
- `z=1`: `(1,0)`, `(2,0)`
- `z=2`: `(0,1)` with weight 2
- `z=3`: `(1,0)`
- `z=4`: `(0,1)`
- `z=5`: `(1,0)`
- `z=6`: `(0,1)`

Orientation `+1`:

- `z=0`: `(0,0)`, `(2,1)`
- `z=1`: `(0,1)`, `(0,2)`
- `z=2`: `(1,0)` with weight 2
- `z=3`: `(0,1)`
- `z=4`: `(1,0)`
- `z=5`: `(0,1)`
- `z=6`: `(1,0)`

## Verification boundary

`replay_component_matches_symbolic_label_schedule` now requires exact equality between a replayed recurrent component's labelled state set and one of the pre-enumerated symbolic schedules. Graph isomorphism or agreement only at the quotient level is insufficient.

`derive_replayed_boundary_phase_twist` now derives the symbolic schedule once and grants `symbolic_label_schedule=true` only after this exact match. Prefix phase-twist evidence is accepted only when both the pre-enumeration match and the replay-level arithmetic proof succeed.

Thus the full corona SCC computation is now verification of a previously predicted family, not the mechanism that discovers its label schedule.

## Tests

Passing:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`

The focused test also mutates one predicted endpoint label and confirms that exact schedule matching rejects it.

## Remaining seam

The bounded symbolic schema still performs SCC decomposition internally. The next universal step is to replace that finite decomposition with a closed-form recurrence for the two label schedules, then prove that all other symbolic candidates are transient by a label-rank or parent-depth argument.
