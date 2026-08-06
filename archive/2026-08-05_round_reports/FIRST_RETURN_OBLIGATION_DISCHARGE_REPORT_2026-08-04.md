# First-return obligation discharge

The previous abstract interface exposed two obligations:

1. realized first-return language completeness;
2. accepted controller path soundness for strict shell pumping.

Concrete native replay now establishes the segment-level content of both on the complete stored witness corpus used by the current campaign.

## Native results

- n=3: 1,146 / 1,146 controller paths replay; endpoint monitors accept; affine transports replay; transported paths are strict outward first returns.
- n=4: 1,774 / 1,774 controller paths replay; endpoint monitors accept; affine transports replay; transported paths are strict outward first returns.
- Total: 2,920 / 2,920.

The C++ operation is `discharge_first_return_obligations` in
`include/ravel/proof/first_return_obligation_discharge.hpp`.

## Corrected universal seam

The stored objects are first-return **segments**, not closed carry cycles.  Feeding them directly to the closed-cycle `certify_strict_shell_pump` operation is invalid.  The remaining universal theorem is therefore:

> compatible controller/translation endpoints splice around every realized recurrent first-return cycle, so the transported source remains return-capable.

The standalone Lean file calls this `CyclicSpliceComplete`.  It proves that this single premise composes the now-discharged segment obligations into strict shell pumping and the radius-one recurrent bound.

No finite replay result is labeled as the universal cyclic splice theorem.
