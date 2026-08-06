# Parametric radius quasi-polynomial rank transport — round 63

## Result

The radius parameter is no longer treated as an unbounded list of shell
instances.  Along an affine stepped-hyperplane ray

    x(q) = b + q d,

all linear shell, block-forcing, and Krylov coordinates are affine in `q`.
After a computable threshold, every sign and zero predicate used by the
block-height chamber rank is fixed.  Absolute-value summaries are then affine
and energy summaries are quadratic.

The remaining non-polynomial feature is the gcd of the block-forcing
coefficients.  For affine coefficients `a_i q+b_i`, every common divisor also
divides every determinant

    a_i b_j - a_j b_i.

When the determinant gcd `P` is nonzero, the forcing gcd is exactly periodic
modulo `P`.  Consequently the chamber-rank gain is an exact quadratic on each
of finitely many residue classes after a finite preperiod.

The new operations are:

- `derive_parametric_radial_rank` for genuinely quadratic rank operations;
- `derive_parametric_block_height_rank` for the full eventual
  quasi-quadratic block/height grammar;
- exact integer positivity checking on every nonnegative radius quotient.

No floating-point root or sampling-only claim is used.  Values at additional
points replay the derived polynomial, while the structural affine/periodic
grammar is the reason degree and period are finite.

## Audit against the existing n=4 rank

On the 1,774 exact `n=4, M=2` first-return edges, 346 have nonzero source and
target coordinates and therefore define immediate stable radial chambers.
Using the existing generated chamber rank and the natural `(n+1) sign`
directions:

- 253 edge rays are closed for every `q>=0` by the new quasi-polynomial
  certificate;
- 24 have a degenerate collinear forcing-gcd ray and need a separate affine-gcd
  case;
- 69 have a genuine nonpositive residue polynomial, confirming that naive
  same-sign radial copying is not the universal transition schema.

This is useful separation.  Infinite-radius positivity is now solved whenever
a symbolic affine transition schema is supplied; the remaining theorem is no
longer “check all M”.  It is:

> derive a finite complete list of stepped-hyperplane affine first-return
> schemas, including the defect-corrected translation directions, and certify
> each schema by an eventual quasi-polynomial rank or by the controller pump
> branch.

The 69 failures are expected evidence that the correct schema must transport
the translation defect, not reuse independent source and target sign rays.

## Next proof seam

Extend the quasi-polynomial operation to the defect-corrected affine splice
certificate already emitted by `covering_translation_tube`.  That certificate
contains the exact source ray, transported target ray, and forcing-coordinate
defects.  Feeding those derived rays into the all-radius rank operation should
turn the remaining parametric alternative into a finite schema coverage
problem.
