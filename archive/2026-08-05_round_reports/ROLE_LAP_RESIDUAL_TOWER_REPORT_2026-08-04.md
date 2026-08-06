# Role-Lap Residual Tower Report — 2026-08-04

## Result

The empty cyclic-kernel branch is now expressed in the existing continuation-
controller predecessor algebra rather than in a parallel graph-height system.
One complete selected stepped-face role lap is treated as a single macro digit.
Starting from the full controller residual, repeated exact predecessor produces
an extensional finite tower

\[
U,\;\operatorname{Pre}_L(U),\;\operatorname{Pre}_L^2(U),\ldots.
\]

The tower has an exact finite dichotomy:

* reaching the empty residual is equivalent to an empty cyclic kernel and gives
  an extinction height at most the number of radius-one controller states;
* repeating a nonempty residual is equivalent to a positive controller cycle.

This is implemented by `derive_role_lap_residual_tower`, reusing
`ResidualController`, `predecessor_controller`, and
`close_controller_family`.

## Bounded section theorem

`certify_role_lap_tower_section` proves that a concrete section of `N` complete
role-lap edges on an `N`-state controller contains a repeated state and hence a
positive controller cycle.  Thus the remaining stepped-hyperplane obligation
is finite:

> construct a compatible role-constrained controller section of length
> `3^D` complete laps.

No infinite compactness assumption is needed.  Once such a section is derived,
the already-complete controller lift, strict shell pump, and maximality
contradiction close `carry.parametric_shell_rank`.

## Validation

* positive and empty-kernel residual towers replay;
* extinction height is bounded by state count;
* bounded tower sections reconstruct and replay their forced cycles;
* existing kernel dichotomy, controller lift, strict pump, and maximality
  contradiction tests remain passing.

## Next mathematical target

Use the symbolic residual-formula/return-phase tower machinery to derive a
stepped-hyperplane section through all `3^D` macro levels.  Equivalently, prove
that the macro predecessor orbit of the active-face terminal formula never
reaches `False`.  This is now a formula nonemptiness theorem, not a graph search.
