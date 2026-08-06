# Cyclic splice completion

The one-lap fixed-point requirement was stronger than necessary.

For one complete traversal of a recurrent base cycle, restrict the radius-one
controller plant to the face-aligned invariant fiber and write `R(c,c')` when a
controller path traverses that lap from `c` to `c'`.

If the fiber is finite, nonempty, and `R` is serial, choose one successor after
each lap.  The resulting infinite orbit repeats a controller state.  The
portion between repetitions is a closed controller run over a positive number
of repeated base-cycle laps.  Repeating the source carry cycle by the same
factor therefore supplies the closed affine controller run required by the
strict-shell-pump constructor.

This removes the need to prove a one-lap controller fixed point.

Implemented:

- `include/ravel/proof/cyclic_splice_completion.hpp`
- `tests/cyclic_splice_completion_test.cpp`
- `lean/generated/cyclic_splice_completion.lean`

The C++ operation derives and replays the periodic orbit from a finite serial
relation.  The Lean theorem proves the finite-pigeonhole step universally and
composes it with the shell-pumping boundedness argument.

The remaining concrete system work is now local rather than cyclic:

1. construct, for every outer recurrent source cycle, a nonempty face-aligned
   controller fiber on which the one-lap relation is serial;
2. instantiate `repeatedLapPumps` from the existing accepted-path affine replay
   and repetition/concatenation operations.

The former global `CyclicSpliceComplete` axiom is no longer needed.
