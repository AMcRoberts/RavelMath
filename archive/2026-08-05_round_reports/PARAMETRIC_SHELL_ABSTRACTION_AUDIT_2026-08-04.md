# Parametric shell-rank abstraction audit — round 56

## Selected obligation

The parametric maximum-shell theorem remains the highest-leverage open node:
it unlocks the recurrent hull, literal core SCC identification, and materially
shrinks full-grade exhaustion.

## Abstractions tested

The proof was examined as:

1. a direct shell phase rank;
2. an exact `(n+1)`-block forcing class;
3. a joint integer Krylov/conjugate-height cell;
4. an affine radial-transport problem;
5. a finite controller relation and cyclic splice problem.

## Closed negative branch: all linear radial cells

For a shell state `x`, let `s=sign(x)`, let `p=min_i |x_i|`, and write

    p = q(n+1)+r,
    x = g + r s + q(n+1)s,

where `g=x-p s` is the exact gap base.  Both the block-forcing dual map and the
Krylov moment map are linear.  Therefore

    BF(x) = BF(g+r s) + q(n+1) BF(s),
    K(x)  = K(g+r s)  + q(n+1) K(s).

Consequently every radius-normalized exact joint cell made from these linear
features is already determined by the existing

    signs | exact gaps | minimum residue

phase.  It cannot separate a collision left by that phase.  This closes not
only the previously tested summary features, but the entire class of linear
block/Krylov refinements.

The replayable operation is
`derive_radial_linear_feature_factorization` in
`include/ravel/proof/radial_linear_feature_factorization.hpp`.

Regression coverage: 3,004,485 exact states through dimensions 2..8 and box
radius 5.

## Completed positive branch: generic cyclic splice

The controller abstraction separates the remaining shell theorem into a local
and a generic part.

Given exact controller relations for consecutive first-return segments, if
each relation is serial and preserves a common finite face-aligned fiber, then:

1. relational composition around the source lap is serial on that fiber;
2. finiteness gives a repeated controller state;
3. the repeated segment is a positive periodic controller orbit;
4. repeating the source lap by the same period supplies the cyclic splice
   required by strict shell pumping.

This is implemented by `derive_recurrent_lap_splice` in
`include/ravel/proof/segment_relation_splice.hpp`.  It rejects nonserial and
fiber-exiting relations and independently replays the periodic orbit.

## Exact remaining local theorem

The parametric shell theorem is not yet closed.  Its irreducible concrete
obligation is now:

> For every realized maximum-shell first-return segment, derive an exact
> face-aligned controller relation that is serial on a common invariant fiber.

All later composition and cyclic-closure steps are now reusable engine
operations.  Future work should target segment seriality directly, rather than
adding more linear shell features.
