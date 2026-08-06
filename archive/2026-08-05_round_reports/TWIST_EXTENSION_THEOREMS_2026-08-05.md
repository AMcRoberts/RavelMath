# Continued Twist Theorems and the First Exotic Dominance Principle

Date: 2026-08-05

The kernel-checked n-bonacci theorem is stable under a broad family of twists.
This report states the directly continued proofs and the first twist that
changes the theorem.

## Theorem A — gauge-equivalent twists

For a finite permutation lift, let `p_e` be the fibre permutation on an edge
`e : u -> v`.  Choose a fibre relabelling `h_u` at every base vertex and define

\[
  p'_e=h_v p_e h_u^{-1}.
\]

The lifted adjacency matrices are conjugate by the block-diagonal permutation
`diag(h_u)`.  Therefore they have identical path counts, characteristic
polynomials, spectra, spectral radii, and dominance conclusions.

Engine operation:

```cpp
derive_gauge_twist_invariance(...)
```

## Theorem B — products of finite twists

If two independent finite twists act by permutations `p_e` on `F` and `q_e`
on `E`, their simultaneous twist acts on `F x E` by

\[
  (a,b)\mapsto(p_e(a),q_e(b)).
\]

This is again a finite permutation skew product.  Every base path has one lift
from every initial product-fibre point, so its spectral radius equals the
untwisted base.  Examples include phase plus orientation, orientation plus
finite memory, and several simultaneous holonomy coordinates.

Engine operation:

```cpp
derive_product_twist_dominance(...)
```

## Theorem C — unitary and contractive operator twists

Suppose edge `e` carries a finite-dimensional fibre operator `U_e` with

\[
  \lVert U_e\rVert\leq1.
\]

The norm of every lifted path is at most the scalar count of its base path.
Thus the operator-valued transfer system is dominated by the untwisted
channel-count system.  The kernel-checked n-bonacci core dominance theorem
therefore applies unchanged.

This includes:

- scalar signs;
- complex phases;
- finite-dimensional unitary representations;
- orthogonal twists;
- strict contractions;
- finite products and higher-block forms of these twists.

Engine operation:

```cpp
derive_norm_weighted_twist_dominance(...)
```

with `ordinary_augmentation_dominates=true`.

## Theorem D — the exotic expansive case

If some edge operator satisfies

\[
  \lVert U_e\rVert>1,
\]

ordinary channel counts no longer dominate.  The exact minimal counterexample
is one base vertex, one loop, and fibre action `[2]`:

\[
  \rho(B_{\mathrm{count}})=1,
  \qquad
  \rho(T_{\mathrm{twist}})=2.
\]

The replacement is the norm-weighted augmentation

\[
  B_{\mathrm{norm}}(u,v)
   =\sum_{e:u\to v}m_e\lVert U_e\rVert,
\]

for which

\[
  \rho(T_{\mathrm{twist}})
  \leq\rho(B_{\mathrm{norm}}).
\]

This is the first genuinely new dominance principle forced by an exotic twist.
It is the finite-state form of the entropy-plus-Lyapunov rule for twisted
transfer operators.

The resulting research question is no longer whether the original core is
maximal, but which weighted `Q/R` core is maximal for a given cocycle norm or
potential.  The existing pair-word induction should continue after replacing
`Q` and `R` by their norm-weighted versions, provided the boundary substitution
uses the same multiplicative weights.

## Test coverage

`tests/twist_dominance_extensions_test.cpp` checks:

- nontrivial gauge switching;
- a six-state product fibre (orientation times phase memory);
- unitary/contractive projection to ordinary n-bonacci dominance;
- the expansive one-loop counterexample;
- rejection of malformed channels.
