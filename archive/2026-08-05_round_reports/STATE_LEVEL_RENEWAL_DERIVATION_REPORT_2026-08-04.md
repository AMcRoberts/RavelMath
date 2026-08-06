# State-Level Renewal Derivation — 2026-08-04

## Result

The rejected-boundary macro-renewal proof no longer begins from an unexplained quotient matrix.

`derive_state_level_weighted_cycle` now consumes the replay-validated concrete boundary graph and one recurrent component. It computes the equitable fibers, but derives the renewal grammar by checking every concrete state and every stored replay edge:

1. every state in a fiber has internal transitions into exactly one successor fiber;
2. all states in that fiber agree on the successor fiber;
3. all states in that fiber agree on the total witnessed multiplicity;
4. the resulting successor map has indegree one at every fiber;
5. the successor permutation is one cycle covering every fiber.

If the cycle has fibers `C_0,...,C_{p-1}` and state-derived edge weights `w_0,...,w_{p-1}`, unique path composition gives

`A^p = (w_0 ... w_{p-1}) I`.

The independently constructed quotient matrix is used only afterward to replay-check the state certificate.

## n=7 replay

The complete projected `6 -> 7` run closed both unknown nine-state theta SCCs with:

- concrete states: 9;
- replay edges: 10;
- equitable fibers: 7;
- state-derived successor cycle: one cycle;
- period: 7;
- cycle multiplicity: 2;
- derived identity: `A^7 = 2 I`;
- exact rational separation from the predicted core: PASS.

Both witnesses report:

`macro_renewal=true state_level_renewal=true renewal=7/2`

The complete adjacent-dimension result remains PASS.

## Trust boundary

The derivation depends on `ReplayedBoundaryGraph`, whose edges are accepted only after each stored corona prefix pair is replayed against the canonical simple-transition oracle. The state-level renewal operation therefore does not regenerate transitions and does not infer graph edges from state patterns.

## Remaining universal seam

The finite state-level seam is closed. The next universal step is to derive, from the symbolic corona/state grammar under dimension transport, that any newly born periodic grade-two boundary component has these fiber properties uniformly, rather than discovering the equitable fibers by finite color refinement.
