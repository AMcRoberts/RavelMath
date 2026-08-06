# Exact cyclotomic-obstruction certificates

This round replaces numerical unit-circle root detection with three agreeing exact mechanisms.

1. **Polynomial factor certificate.** `cyclotomic_polynomial(n)` constructs `Phi_n` over `Z`; `derive_cyclotomic_obstruction_certificate` divides out every detected `Phi_n` exactly and reconstructs the original polynomial.
2. **Controller character sectors.** A `Z/2` group-ring transfer `A_0 + A_1 g` is evaluated at the trivial and alternating characters as `A_0 + A_1` and `A_0 - A_1`. Their characteristic polynomials are computed by the existing exact Faddeev--LeVerrier implementation and passed through the same factor certificate.
3. **Graph periods.** `derive_weighted_cycle_period_certificate` proves the gcd of weighted closed-cycle lengths in a strongly connected directed graph using integer vertex potentials and cycle defects.

The nearest-left profile parity result now delegates to the general detector.  Its `x+1` flag is no longer a bespoke assertion: it is an exact `Phi_2` factor certificate checked against direct evaluation at `-1`.

The generated Lean file contains the generic `x+1` divisibility/evaluation theorem and contains no `sorry`. It was emitted but not kernel-run because this archive has no configured Lean/Mathlib environment.
