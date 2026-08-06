# Radial translation defect

The naive closure conjecture is false. For nonzero-coordinate states, outward
translation by `p=n+1` preserves the current phase encoding (signs, exact
absolute gaps, and minimum absolute value modulo `p`), but it does not preserve
the exact maximum-shell first-return relation.

Exact audits:

- `n=3, M=2`: 72 of 90 eligible edges transport; 18 fail.
- `n=3, M=3`: 190 of 302 transport; 112 fail.
- `n=4, M=2`: 0 of 320 transport; all fail.

A concrete counterexample is

    (2,-1,1) -> (1,1,-2)

at `n=3,M=2`, while the outward translates

    (6,-5,5), (5,5,-6)

are not a first-return edge at `M=6`.

The exact replacement theorem is affine translation transport. If one block is

    F(x)=B x + forcing,

then

    F(x+t)=F(x)+B t,

and the defect relative to reusing the same translation is

    F(x+t)-(F(x)+t)=B t-t.

For the established n-bonacci block identity `B=2A-I`, this is

    2(A t-t).

Therefore a true radial closure theorem must carry the orbit of the translation
direction under `A`, or an equivalent defect phase. The existing phase
`signs|exact gaps|min residue` is insufficient by itself.

The next target is **defect-corrected radial transport**:

1. classify radial translation directions `t` by the normalized defect
   `A t-t`;
2. add that defect class to the phase certificate;
3. prove block transition transport on the enlarged phase;
4. reduce all radii to finitely many defect/residue classes.
