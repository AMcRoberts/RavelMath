# Defect-corrected shell-rank transport result

The shell first-return miner has been ported into the native C++ proof library
and the enlarged phase

    signs | exact gaps | min residue | adjacent sign flux | signed imbalance

has been tested with one merged phase-rank certificate.

## Exact finite results

- n=3, M=2..8: phases=4382, constraints=14354, raw edges=19584, PASS
- n=4, M=2..6: phases=28480, constraints=122466, raw edges=156832, PASS

The larger phase counts include isolated source phases.  The merged constraint
counts are exactly the same as the previous phase model.

## Mathematical conclusion

The defect data does not refine the old phase.  If the sign word is
`s_0,...,s_{n-1}`, then

    adjacent_flux_i = s_{i+1}-s_i
    signed_imbalance = sum_{i=1}^{n-1} s_i
    terminal_flux = s_0-signed_imbalance-s_{n-1}.

Hence the full defect profile factors through the sign component already
present in `signs|gaps|min residue`.  It explains radial translation exactly,
but cannot separate any collision left by that phase abstraction.

## Closed branch and next pivot

This closes the defect-correction branch as a reusable transport identity and
rules it out as the missing phase separator.  The next automatic separator is
therefore the exact `(n+1)`-block forcing class.  If that still collides, add
the joint conjugate-height cell rather than independent height bounds.
