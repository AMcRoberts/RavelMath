# Sign-Flux Chamber Fit Report — 2026-08-04

## Goal

Replace the 39 sign-symmetric chamber-orbit coefficient vectors for the exact
`n=4`, `M=2..6` first-return rank with one role-relative affine potential whose
coefficients are generated from local sign-flux statistics.

The fit was posed on the rank itself, not as a regression against the old
coefficient table.  For a state `x`, the candidate potential has the form

`R(x) = sum_{a,j} w[a,j] Flux_a(sign(x)) Observable_j(x)`

where the state is first normalized modulo global sign negation.

## Exact corpus

- dimension: `n=4`;
- shell bounds: `M=2..6`;
- exact first-return edges: `156,832`;
- state observables: shell level, block-forcing support/zeros/gcd, Krylov
  variation/energy, adjacent energy, signed sum, maximum, minimum, intercept.

## Adjacent sign-flux grammar

The first grammar contains:

- sign populations;
- first- and last-coordinate signs;
- oriented adjacent sign-pair counts;
- oriented cyclic sign-pair counts.

This gives 28 sign features and 308 rank variables after tensoring with the
11 state observables.

A counterexample-guided passive-aggressive fit ran for 30 full exact replay
rounds.  It did not close:

- remaining nonpositive-gain edges: `1,540`;
- minimum fitted margin: approximately `-21.4059`.

The failures are concentrated in a small sign-chamber transition family.  The
largest exact groups are:

- `-++- -> ++--`: 391;
- `+--+ -> --++`: 391;
- `--+- -> -+--`: 234;
- `++-+ -> +-++`: 234.

The paired counts reflect the already-proved global sign symmetry.

## Bounded three-sign flux pivot

The second grammar adds all 27 cyclic length-three sign-factor counts.  This
raises the model to 55 sign features and 605 variables.

A memory-fenced 15-round run completed with about 2.99 GB peak resident memory.
It also did not close:

- remaining nonpositive-gain edges: `1,828`;
- minimum fitted margin: approximately `-18.1236`.

A longer run continued reducing online violations but did not reach exact
closure before the time fence.  This is not an infeasibility proof for the
three-sign grammar; it is a failed fit under the current affine optimizer.

## Mathematical diagnosis

Pure local sign-flux coefficients are not yet sufficient to replace the
chamber table.  The dominant collisions exchange sign blocks across the
boundary while preserving most low-order sign counts.  The next prepared
abstractions are:

1. add boundary-position-sensitive flux rather than only aggregate factor
   counts;
2. add a bounded relational correction indexed by the source/target flux
   event;
3. fit a piecewise-affine potential on a small sign-flux transition grammar;
4. preserve the exact sign-symmetric chamber rank as a fallback proof while
   proving that the correction family is finite and role-parametric.

The run does establish a useful negative result: merely tensoring the current
block/height observables with aggregate adjacent or length-three sign-factor
counts does not immediately reproduce the exact strict rank.

## Native code

Added:

- `app/sign_flux_chamber_fit.cpp`;
- `app/sign_flux_chamber_fit_triplet.cpp`.

Both tools enumerate and replay the exact C++ first-return graph.  They return
nonzero unless every exact edge has positive fitted gain.
