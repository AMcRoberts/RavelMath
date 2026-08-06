# Generalized multinacci prefix-phase reduction

Author: **Ravel**. Coordinator and project architect: **AM**.

## Result

For the generalized multinacci substitution

\[
\sigma_{D,m}(i)=0^m(i+1),\qquad \sigma_{D,m}(D-1)=0,
\]

every parent occurrence has prefix length in \(\{0,\ldots,m\}\).  A paired parent channel therefore has signed prefix defect

\[
\delta=q-p\in\{-m,\ldots,m\}.
\]

At the raw macro level this appears to produce \(m+1\) positive transport matrices \(G_0,\ldots,G_m\), indexed by \(|\delta|\).  The exact prefix-position certificate proves that they are not primitive generators:

\[
G_0=Q,\qquad G_d=R^d\quad(d>0),
\]

while \(\operatorname{sign}(\delta)\) remains a separate side/orientation twist coordinate.

Thus the generalized family is handled by the already-built free positive word grammar on \(Q,R\).  Multiplicity \(m\) enlarges the finite positional scheduler and permits residual words up to length \(m\); it does not require a new primitive matrix alphabet.

## The m=2 slice

There are three prefix positions and nine ordered prefix pairs.  Their exact defect-magnitude multiplicities are

\[
|\delta|=0:3,\qquad |\delta|=1:4,\qquad |\delta|=2:2.
\]

The corresponding macro words are

\[
G_0=Q,\qquad G_1=R,\qquad G_2=RR.
\]

This identifies the wrench exposed in round 85: the doubled zero prefix is a finite positional phase whose long jump is a two-step residual word.  Collapsing the two zero positions destroys parent information; retaining them feeds directly into the old word-polynomial pipeline.

## Implementation

Added:

- `include/ravel/proof/generalized_multinacci_prefix_phase.hpp`
- `tests/generalized_multinacci_prefix_phase_test.cpp`

The derivation enumerates every ordered prefix-position pair, checks the closed-form channel multiplicities, retains signed defect separately, and emits the exact primitive Q/R word for every macro defect class.

## Validation

The following pass together:

```text
generalized multinacci prefix phase PASS
finite positive grammar majorant PASS
generalized multinacci family PASS
```

The certificate also checks the exact \(m=1\) specialization and a general \(m=3\) case, where \(G_3=R^3\).

## Next proof-engineering step

Replace the collapsed parent-prefix treatment inside the balanced-pair/contact-boundary bridge with the emitted positional scheduler.  Each concrete boundary transition should carry its two prefix positions, signed side twist, and primitive Q/R word.  The existing positive-word and norm-weighted theorem can then replay the concrete \(m=2\) boundary recursion without introducing a third primitive generator.
