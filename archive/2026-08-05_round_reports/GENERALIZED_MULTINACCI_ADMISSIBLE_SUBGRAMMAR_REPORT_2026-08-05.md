# Generalized Multinacci Admissible-Subgrammar Closure

## Result

The remaining symbolic boundary-state catalogue is not required for the
dominance theorem.  For the generalized multinacci substitution

\[
\sigma_{D,m}(i)=0^m(i+1),\qquad \sigma_{D,m}(D-1)=0,
\]

every exact parent-prefix witness has cuts `p,q in {0,...,m}` and therefore
emits `Q` when `p=q`, otherwise `R^|q-p|`.  The contact-boundary restriction
retains only witnesses whose destination remains in the exact boundary node
set.  Hence admissibility is channel deletion from the universal positive
renewal grammar; it introduces no subtraction, relabelling, or new generator.

If

\[
Q_C P\le P Q_K,\qquad R_C P\le P R_K,
\]

then every witnessed edge word intertwines, and every finite admissible sum of
such edge words intertwines by positivity.  A closed-form catalogue of the
surviving states is useful for enumeration and sharp counts, but is not a
logical prerequisite for dominance.

## Implementation

Added

- `include/ravel/proof/generalized_multinacci_admissible_subgrammar.hpp`
- `tests/generalized_multinacci_admissible_subgrammar_test.cpp`
- `lean/generated/generalized_multinacci_admissible_subgrammar.lean`

The C++ certificate consumes the exact boundary-word lift generated from
`simple_forward_targets_exact`; it checks every prefix cut, every roof bound,
every concrete word inequality, and exact projection back to `G_B`.

## Exact validation

Checked all cases

\[
D=2,3,4,\qquad 1\le m\le6,
\]

plus

\[
D=5,\qquad m=1,2,3.
\]

The largest checked case had 1179 boundary states and 4310 witnessed macro
edges.  Every edge was an admissible `Q` or `R^d` channel, and every positive
subgrammar comparison closed.

## Theorem boundary

This closes the `m`-direction conditionally on the primitive simultaneous
`Q/R` intertwiner.  It does not derive that primitive intertwiner for every
new concrete core/competitor pair; that remains the family-comparison input.
It does show that no uniform symbolic catalogue of concrete boundary states is
needed merely to transport the comparison through generalized-multinacci
admissibility.
