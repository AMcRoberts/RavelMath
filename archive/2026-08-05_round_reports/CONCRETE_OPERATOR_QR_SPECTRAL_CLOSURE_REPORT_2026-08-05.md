# Concrete operator Q/R majorant and spectral closure

## Result

Round 81 removes the remaining supplied-norm gap in the norm-weighted Q/R
majorant layer.  A channel now carries an exact rational fibre matrix.  The
engine computes its induced infinity norm exactly, constructs the weighted
Q/R pair from those derived norms, assembles the full twisted block transfer,
and derives the block-power majorant

\[
  \| (T^k)_{vu} \|_\infty \le (A^k)_{vu}\qquad(k\ge 0).
\]

Here

\[
 A=Q_{\mathrm{norm}}+R_{\mathrm{norm}},\qquad
 (Q_{\mathrm{norm}})_{uv}
 =\sum_{e:u\to v,\,e\in Q}m_e\|U_e\|_\infty,
\]

with the analogous definition for `R`.

## Constructive proof path

1. Validate a common finite fibre dimension and exact rational entries.
2. Compute each edge bound by
   \(\|U\|_\infty=\max_i\sum_j|U_{ij}|\).
3. Pass the computed bounds—not caller assertions—to
   `derive_norm_weighted_qr_majorant`.
4. Assemble `T` as an exact signed-rational block matrix.
5. Verify the first-power block inequality directly.
6. Derive all powers by block multiplication, the triangle inequality, and
   submultiplicativity of the induced infinity norm.
7. Replay finitely many powers exactly as independent executable witnesses.
8. Apply the Gelfand spectral-radius formula to obtain
   \(\rho(T)\le\rho(A)\).

The replay is diagnostic evidence and is not substituted for the universal
induction.

## Expansive counterexample

For the one-loop fibre matrix `[2]`, the operation computes

\[
 \|[2]\|_\infty=2,
 \quad A=[2],
 \quad T^k=A^k=[2^k].
\]

Thus the norm-weighted theorem is sharp, while the ordinary count matrix
`[1]` fails at `k=1`.

## Formal artifact

`lean/generated/concrete_operator_qr_spectral_closure.lean` contains:

- universal induction from the emitted composition step;
- passage from pointwise Gelfand-root domination to limit domination;
- the exact scalar expansive witness.

The current archive has no Lean/Mathlib toolchain, so this artifact is emitted
but not marked kernel-checked.

## Scope

The implemented concrete layer presently uses:

- a common finite fibre dimension;
- exact rational matrices;
- the induced infinity norm;
- nonnegative integer channel multiplicities.

The theorem itself is broader: any certified submultiplicative operator norm
and varying finite fibres can feed the same positive-majorant construction.
