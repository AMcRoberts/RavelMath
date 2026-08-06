# Stepped-face residual seriality — round 57

## Abstraction correction

The attempted common literal controller fiber is too strong.  A first-return
word does not preserve one fixed coordinate face, and neither the raw
face-aligned radius-one fiber nor the exact signed `H_sigma` subfiber is serial
for every stored segment.

The stepped-hyperplane geometry instead supplies a varying linear fiber.  The
terminal condition is the finite face monitor

    T_y(t) := exists (i,s) active at y, t_i = s.

For a word `d_0 ... d_(L-1)`, define residual fibers backward by

    R_L = T_y,
    R_k = Pre_(d_k)(R_(k+1)).

Because `Pre_d` is an existential linear inverse image of the radius-one
controller transition, every state of `R_k` has a `d_k` successor in
`R_(k+1)`.  Thus the controller relation is serial at every step and over the
whole first-return segment.  The fiber is not guessed, enumerated by a shell
phase, or strengthened by unrelated Krylov data; it is derived from the
stepped face and the exact linear predecessor grammar.

## Engine operation

`include/ravel/proof/stepped_face_residual_seriality.hpp` adds

    derive_stepped_face_residual_seriality(product, witness)

The certificate records the exact residual chain, all fiber sizes, stepwise
seriality, and an independent whole-word replay.

The operation consumes the already closed residual family and predecessor
table.  It does not trust the stored translation path as evidence of totality.

## Validation

All stored first-return words pass:

- n=3, M=2 and M=3: 1,146 / 1,146;
- n=4, M=2: 1,774 / 1,774;
- total: 2,920 / 2,920.

The predecessor semantics used here is already represented by the generic
`ResidualFormula.pre` constructor and its Lean theorem, so the seriality
argument is dimension-independent once the terminal stepped-face monitor and
word are supplied.

## Exact remaining obligation

The parametric shell theorem is not yet completely closed.  Segment seriality
is no longer the blocker.  The remaining theorem is cyclic junction
compatibility:

> For a hypothetical recurrent shell lap with consecutive words `w_k`, the
> terminal face fiber of `w_k` and the initial residual fiber of `w_(k+1)`
> admit a nonempty cyclic choice around the whole lap.

Equivalently, the composed one-lap relation on the finite controller contains
a positive directed cycle.  `cyclic_splice_compactness.hpp` already closes all
subsequent steps once that cycle is derived.

This is the irreducible point at which stepped-hyperplane face intersections,
not additional linear radial features, must be used.
