# Smallest known non-unit Pisot number, and three classifier bugs found finding it

Date: 2026-08-05

## Result

**x^3 - x^2 - 2x - 2**, beta ~ 2.269530842..., |norm| = 2 (non-unit).

Found by a systematic companion-matrix search (degree 3: |c2|,|c1| <= 8,
|c0| in [2,10]; degree 4: |c3|,|c2|,|c1| <= 6, |c0| in [2,8]) over
`pisot_classify_3x3`/`pisot_classify_4x4`, then independently
cross-checked with `mathlib::sturm_chain` directly (exactly one real
root, isolated to the same value) and by hand (product of all three
roots = 2, so the complex-conjugate pair has modulus ~0.9386 < 1).

**Every Pisot number below 2 is provably a unit**: `|norm| = beta *
prod(|conjugates|) < beta` since each conjugate has modulus strictly
less than 1, so `beta < 2` forces the nonzero integer norm to be
exactly 1. Non-unimodularity can only start at beta >= 2, and does not
start immediately: the silver ratio (beta = 1+sqrt(2) ~ 2.41421, root
of x^2-2x-1) is already past 2 but is still a unit. x^3-x^2-2x-2 is
smaller than the silver ratio and is the smallest non-unit example
found; a smaller one has not been ruled out (the search above is wide
but not exhaustive over all degrees/coefficient ranges).

## Three real bugs in `math/src/exact_pisot.c`, found in the process

All three were pre-existing (not introduced this session) and were
never caught by the existing test suite because it only ever exercised
a handful of hand-picked, already-known-good matrices.

1. **Missing real-root accounting.** `pisot_classify_poly` isolated
   roots in `(-1,1)` and `(1,bound]` but never checked that those two
   counts actually summed to the total real-root count. A real root
   `<= -1` (modulus >= 1, violating Pisot-ness) fell in neither bucket
   and was silently never checked. Confirmed via AddressSanitizer-free
   direct construction: `x^3+4x^2-3x-3` (real roots near -4.6, -0.6,
   1.1) previously classified as Pisot with beta~1.11.

2. **Heap-use-after-free in `poly_is_squarefree`.** `poly_t a = *f, b =
   fp;` was a shallow struct copy of a type that (at the time) held
   heap-pointer coefficients, so `a`/`b` could alias `f`'s/`fp`'s
   storage. Depending on how many Euclidean-algorithm steps the gcd
   loop took, `a` could still alias `fp`'s pointers at loop exit, and
   `poly_clear(&a); poly_clear(&fp);` then freed the same memory twice.
   Found via AddressSanitizer during a systematic 3x3 search (a
   degree-3 case whose gcd loop happened to take one step). Fixed by
   deep-copying (`poly_copy`) instead of aliasing, and by converting
   `poly_t`/`iv_t`'s coefficient/bound storage from arrays of
   individually-`malloc`'d wrapper pointers to embedded `mpq_t`/`mpz_t`
   value arrays (bounded degree/known count, so this is free
   pre-allocation, not a new allocator) -- which also eliminates
   several other latent instances of the same aliasing risk (a second,
   independent double-free surfaced in the degree-4 path before this
   change, and is gone after it).

3. **`is_pisot` never actually gated on the complex-secondary-pair
   modulus check, and that check's own bound arithmetic was unsound.**
   `out->is_pisot = 1;` was set unconditionally as soon as the real-root
   structure was valid, *before* the complex-modulus check even ran;
   `is_complex_modulus_lt_1` was computed but only ever written to an
   informational field. Separately, that computation itself was broken:
   it seeded the running bound with `beta_lo_num * beta_lo_den`
   (numerator times denominator -- not the value `beta_lo` at all, and
   enormous after 40 bisection refinements) and used the *upper* bound
   of each real secondary root's magnitude where a *lower* bound was
   required for a valid `>` argument, then collapsed the running
   fraction with `mpz_divexact`, which is only defined for exact
   division. Net effect: the complex-modulus check was very close to
   vacuously true. Confirmed via the same false-positive example above
   (`x^3+3x^2+x-6`: real root ~1.0945, complex-pair modulus ~2.34,
   `is_complex_modulus_lt_1` still reported `1`). Fixed by properly
   gating `is_pisot` on the outcome, and rewriting the bound as an
   exact running numerator/denominator pair seeded at `beta_lo` and
   multiplied by each secondary root's *lower*-bound magnitude.

All three fixes are covered by new regression tests in
`tests/exact_pisot_test.cpp` (`smallest_known_nonunit_pisot`,
`false_positive_regression_x3_3x2_x_m6`); the full suite (46 checks) and
a fresh AddressSanitizer pass over a ~10,770-case systematic 3x3 search
and a 6,561-case degree-4 search are both clean after the fix.

## What this means for the project

Unimodularity was not actually the load-bearing condition behind the
"beta in (1,2) forces Q/R/S" theorem (`beta_in_one_two_forces_qrs.hpp`)
-- that theorem is purely about digit range. This non-unit boundary case
has not yet been folded into the canonical-beta-substitution /
parent-prefix machinery the way supergolden was; that is the natural
next step if this direction is continued.
