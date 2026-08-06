# Quadratic Rejected-State Shape Defect — 2026-08-04

## Result

The unit-coordinate clause of the rejected-state shape invariant is no longer
obtained by inspecting every coordinate of the finite recurrent SCC.

For a canonical n-bonacci prefix edge, write

    delta = |right_prefix| - |left_prefix| in {-1,0,1}.

The inverse-incidence equation gives

    x' = (x_1,...,x_{D-1}, 2*x_0 + delta).

Therefore the exact quadratic defect is

    ||x'||^2 - ||x||^2
      = (2*x_0 + delta)^2 - x_0^2
      = 3*x_0^2 + 4*delta*x_0 + delta^2.

For integral x_0 and delta in {-1,0,1}, this quantity is nonnegative.  It is
zero exactly when

    x_0 in {-1,0,1},  delta = -x_0.

Every internal edge of a strongly connected component lies on a closed walk.
A strict increase of the quadratic norm on any internal edge is therefore
impossible: all internal edges must have zero defect.  Each such edge is the
pure cyclic left rotation

    x' = (x_1,...,x_{D-1},x_0).

Iterating internal edges moves every source coordinate through coordinate zero,
so every coordinate lies in {-1,0,1}.  This is dimension-independent and uses
only the n-bonacci incidence equation, prefix roles, and recurrence.

## Engine changes

`RejectedStateShapeInvariantProof` now records:

- `quadratic_norm_defect_derived`
- `pure_rotation_derived`

`derive_rejected_state_shape_invariant` validates every internal stored prefix
witness, derives the exact norm difference, requires equality on recurrent
edges, and verifies the resulting cyclic rotation equation.  The direct unit
coordinate scan remains only as a replay check of the symbolic conclusion.

## Remaining seam

The norm defect does not alone imply that a unit grade-two vector has exactly
one zero or cyclically alternating signs.  The remaining theorem is a symbolic
classification of unit grade-two rotation orbits under the same-letter parent
grammar:

- transported/core orbit;
- terminal permutation orbit; or
- exactly-one-zero alternating orbit.

This classification should use the two-atom displacement descriptors and label
parent cases, not another SCC search.

## Validation

Passing:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`
