# Universal n-bonacci theorem — kernel-checked checkpoint

Date: 2026-08-03

## Status

**Lean kernel checked.**

The generated module accepted by Lean is:

- `out/nbonacci_reflective_trace.lean`
- canonical copy: `out/GeneratedCampaign.lean`
- archival source copy: `lean/generated/nbonacci_universal_n.kernel_checked.lean`
- SHA-256: `adf0554e32cc1734badbada637c6888e16f0e2ea0f007c75ed24754e195846fe`

## Public theorem

`RavelGenerated.nbonacci_universal_n` proves, for every natural number
`n` with `2 ≤ n`, that the determinant of the generated
`(n+1) × (n+1)` n-bonacci characteristic-matrix family equals the
corresponding polynomial

\[
X + X^2 + \cdots + X^{n+1} - 1.
\]

The checked chain includes the lower-triangular residual determinant,
erased-index minor transports, residual cofactor determinant, the q-matrix
determinant recurrence and closed form, sparse characteristic cofactor split,
polynomial normalization, and final theorem composition.

## Scope boundary

This proves the determinant identity for the generated matrix family. Any
claim identifying that family with another independently defined recurrence,
substitution, or operator must cite a separate bridge theorem or executable
source-equality certificate.
