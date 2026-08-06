# Prefix-Level Phase-Twist Derivation — 2026-08-04

## Result

The periodic grade-two boundary phase law is now derived from the canonical n-bonacci substitution images and the prefix witnesses already stored by the corona projection trace.

For the canonical family

- `sigma(c) = 0(c+1)` for `c+1 < D`,
- `sigma(D-1) = 0`,

every parent prefix witnessing inner letter `0` is empty, and every prefix witnessing an inner letter `k>0` is `[0]` with parent `k-1`.

For a witnessed edge with prefixes `p,q`, the exact simple-edge equation is

`M x' = x + (|q|-|p|) e_0`.

The n-bonacci incidence rows therefore give

- `x'_r = x_{r+1}` for `0 <= r < D-1`,
- `x'_{D-1} = 2 x_0 + |q|-|p|`.

For an alternating-zero state, cyclic left shift occurs exactly when

`|q|-|p| = -x_0`.

The engine now checks this identity for every internal replay witness. It follows directly that the unique zero phase moves by

`z -> z-1 mod D`

and orientation is preserved.

## Parent-role multiplicity

The operation also checks that each prefix word agrees with its source inner label and target parent label under the n-bonacci image formula. Parallel prefix witnesses are counted state-by-state and phase-by-phase. Every state in a phase must agree on multiplicity.

For both real `n=7` theta SCCs, the prefix-parent count derives:

- weight `1` at six phases;
- weight `2` at phase `z=2`;
- no other nonunit phase.

Thus the unique doubled phase is derived from corona prefix roles rather than read from the quotient matrix.

## Engineering changes

- `ReplayedBoundaryGraph` now retains every validated prefix witness, not only aggregated graph weights.
- Added `derive_nbonacci_phase_twist`.
- Added parent-role validation for empty and `[0]` prefixes.
- Added exact displacement reconstruction from source state and prefix imbalance.
- Added `prefix_phase_twist` to recurrent boundary witnesses and reports.
- Added a post-dominance phase-twist obligation: symbolic renewal components lose closure if this stronger certificate fails.

## Validation

Focused test:

`rejected_boundary_exhaustion_test PASS`

Full projected adjacent transport:

- both 9-state theta SCCs: `phase_twist=true`, `branch_phase=2`;
- `6 -> 7`: `PASS`;
- rejected-boundary exhaustion: `proved=true`.

## Remaining seam

The derivation still begins with an enumerated replay SCC. The next universal step is to derive the admissible label schedule and the unique doubled phase from a symbolic alternating-zero family schema before finite state enumeration. The arithmetic phase twist itself is now universal in `D` for the canonical n-bonacci substitution.
