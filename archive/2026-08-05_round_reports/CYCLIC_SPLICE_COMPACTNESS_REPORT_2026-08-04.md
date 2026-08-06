# Cyclic splice compactness

The prior `Serial Fiber Theorem` was stronger than the shell-pump composition
actually needs. For one complete source-cycle lap, form the exact finite
controller relation `R`. A single positive directed cycle in `R` already
supplies a closed controller run after repeating the source lap by the cycle
length. No seriality assertion on unrelated controller states is required.

Implemented:

- `include/ravel/proof/cyclic_splice_compactness.hpp`
- `tests/cyclic_splice_compactness_test.cpp`
- `lean/generated/cyclic_splice_compactness.lean`

The C++ operation searches the exact one-lap relation for a directed cycle,
reconstructs it, and independently replays every arc. It also rejects an
acyclic relation. The standalone Lean endpoint consumes the replayable closed
orbit and composes it with the strict-shell-pump boundedness argument.

This is a genuine reduction, but not a claim that the concrete universal
n-bonacci one-lap relation has already been shown cyclic. The remaining
system-specific statement is now irreducible and stable:

> For every hypothetical outer recurrent n-bonacci source cycle, the exact
> face-aligned radius-one controller relation for one complete source lap
> contains a positive directed cycle whose repeated affine lift is admissible
> and strictly outward.

The audited maximum-shell first-return graphs at the currently stored finite
shells are DAGs, so they contain no recurrent source cycle from which to build
this relation; the finite data therefore verify the theorem vacuously there.
The universal proof must derive the one-lap controller cycle from the algebra
of a hypothetical recurrent source cycle (or eliminate that source cycle
number-theoretically). Segment certificates alone cannot prove this by
concatenation because their controller endpoints are not pairwise total.
