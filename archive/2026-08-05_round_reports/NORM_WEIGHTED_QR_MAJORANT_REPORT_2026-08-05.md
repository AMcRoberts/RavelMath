# Norm-weighted Q/R majorant theorem — implementation report

Date: 2026-08-05

## Result

The former `derive_norm_weighted_twist_dominance` assertion recorder has been
replaced underneath by an exact proof-certificate operation:

```cpp
derive_norm_weighted_qr_majorant(base_vertices, channels, boundary_words)
```

The operation constructs four exact nonnegative rational matrices:

- ordinary Q channel count,
- ordinary R channel count,
- norm-weighted Q majorant,
- norm-weighted R majorant.

It then replays arbitrary finite Q/R words and finite nonnegative boundary
polynomials (multiplicity represented by repeated words) using exact rational
matrix arithmetic.

## The theorem schema certified

For each Q- or R-channel `e`, the input carries a certified bound

`||U_e|| <= a_e`.

Parallel channel assembly uses the triangle inequality and path composition
uses submultiplicativity. Consequently every operator-valued Q/R word is
bounded by the corresponding scalar norm-majorant word, and every finite
nonnegative sum of words is bounded by the corresponding scalar boundary
polynomial.

If every `a_e <= 1`, the norm-weighted Q/R pair is entrywise bounded by the
ordinary channel-count Q/R pair. Thus the existing unweighted joint-Q/R
comparison applies without change.

If some `a_e > 1`, ordinary count dominance is not asserted. The norm-weighted
word and polynomial theorem remains available. The one-vertex scalar loop of
bound 2 is derived as the minimal exact counterexample to unweighted unit
majorization.

## Files

- `include/ravel/proof/norm_weighted_qr_majorant.hpp`
- `tests/norm_weighted_qr_majorant_test.cpp`
- `lean/generated/norm_weighted_qr_majorant.lean`
- `include/ravel/proof/twist_dominance_extensions.hpp` now delegates to the
  exact certificate.

## Validation

Passed:

```text
norm-weighted Q/R majorant PASS
twist dominance extensions PASS
```

The generated Lean file contains:

- `norm_qr_word_majorant`,
- `norm_qr_polynomial_majorant`,
- `contractive_qr_word_majorant`,
- `scalar_two_loop_counterexample`.

Kernel checking was attempted through `scripts/safe_lean_check.sh`, but the
uploaded project did not contain the configured Mathlib ridealong/toolchain.
The precise obstruction was:

```text
safe_lean_check: Mathlib ridealong not found under the configured local dependency directory
```

Therefore the Lean artifact is emitted and reviewable but is not marked
kernel-checked in this round.

## Remaining theorem bridge

The present theorem begins at certified generator/channel norm bounds. The
next reusable operation should derive those bounds from concrete operator
matrices or linear maps and a selected fibre norm. After that, the spectral
radius corollary should be emitted explicitly through a Gelfand-formula or
finite-dimensional matrix-majorization theorem, rather than recorded only as
an available mathematical consequence.
