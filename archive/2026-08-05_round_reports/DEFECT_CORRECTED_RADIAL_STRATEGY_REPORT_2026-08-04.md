# Defect-corrected radial strategy round

The multi-strategy campaign retained four proof styles: exact carry-cycle
exclusion, merged shell ranks, defect-corrected radial transport, and
controller/height fallbacks.

## New exact result

For the n-bonacci linear carry step

    A(t_0,...,t_{n-1}) = (t_1,...,t_{n-1}, t_0-sum_{i>0}t_i)

and radial translation `t_i = p sign(x_i)`, the defect `A t - t` is

    p(sign(x_{i+1})-sign(x_i))                 for i<n-1,
    p(sign(x_0)-sum_{i>0}sign(x_i)-sign(x_{n-1})) at the final coordinate.

Thus the extra radial phase required by the failed naive transport is
parametric: adjacent sign flux plus one global signed imbalance.  It is not an
arbitrary dimension-sized vector.

The C++ proof operation is
`include/ravel/proof/defect_corrected_radial_transport.hpp`.  Exhaustive replay
covered all sign configurations over representatives `{-2,0,3}` through n=10:

    defect-corrected radial transport PASS checks=88560

The standalone Lean file `lean/defect_corrected_radial_transport.lean` proves
the corresponding dimension-uniform coordinate equations from the exact
n-bonacci shift recurrence.

## Parallel branch results

- Exact carry-cycle exclusion: n=8, bound=2, 390625 states, 1947 cyclic states,
  zero nonternary cyclic states.
- Merged shell rank: n=3, M=2..7, 2822 phases, 10064 constraints, PASS.
- Merged shell rank: n=4, M=2..5, 12468 phases, 61054 constraints, PASS.
- Merged shell rank: n=5, M=2..3, 14080 phases, 75846 constraints, PASS.
- Naive radial translation remains false, as expected: n=3,M=2 transports
  72/90 eligible edges and fails on 18.

## Next proof move

Enlarge the shell phase by the proved defect profile:

    signs | exact gaps | min residue | adjacent sign flux | signed imbalance.

Then derive the block transition on this phase and test whether every translated
first-return edge is represented relationally.  If deterministic transport
fails, use the already-established relational/game rank proof.  If the phase
still collides, the next automatic separator is the exact block-forcing class,
followed by conjugate height.
