# Dominance Theorems Around Twisted n-Bonacci Transfer Systems

Date: 2026-08-05

## Executive conclusion

The theorem now proved in Ravel sits at the intersection of several mature
subjects, but the particular synthesis appears unusual:

1. Perron--Frobenius comparison for nonnegative operators;
2. graph covers, voltage/gain graphs, and representation-sector spectra;
3. finite-to-one and sofic extensions in symbolic dynamics;
4. transfer operators twisted by vector-bundle or matrix cocycles;
5. noncommutative two-generator recurrences preserved by positive
   substitutions.

None of those ingredients is novel by itself.  The potentially novel result is
the identification of the canonical n-bonacci carry system as a twisted finite
extension of a universal `Q/R` parent pair, together with an all-dimension
noncommutative boundary substitution and a kernel-checked joint-comparison
induction.

## 1. Perron--Frobenius order dominance

The basic scalar principle is classical: for nonnegative matrices, entrywise
order implies spectral-radius order.  Strong monotonicity under irreducibility
is a sharpened version.  The Ravel joint `Q/R` theorem is a structured,
noncommutative use of this idea: generatorwise inequalities are propagated
through every positive word and every positive boundary polynomial.

Relevant source:

- D. W. Hadwin et al., *Strong Monotonicity of Spectral Radius of Positive
  Operators*, arXiv:1205.5583.
  https://arxiv.org/abs/1205.5583

The standard complex-matrix majorization used for phase twists is

\[
  \rho(T) \leq \rho(|T|) \leq \rho(B)
  \quad\text{when } |T|\leq B.
\]

This is the direct explanation for why unit-modulus phases cannot exceed the
positive untwisted sector.

## 2. Voltage graphs, gain graphs, and graph lifts

A finite graph lift is encoded by a base graph plus permutations or group
voltages on edges.  Representation theory decomposes the lifted adjacency into
represented base matrices.  This is exactly the framework underlying Ravel's
phase, orientation, sheet, and holonomy twists.

Primary references:

- C. Dalfó et al., *The spectra of lifted digraphs*, arXiv:1707.04463.
  https://arxiv.org/abs/1707.04463
- M. Cavaleri et al., *A group representation approach to balance of gain
  graphs*, arXiv:2001.08490.
  https://arxiv.org/abs/2001.08490
- C. Dalfó et al., *On factored lifts of graphs and their spectra*,
  arXiv:2404.02128.
  https://arxiv.org/abs/2404.02128

The Ravel theorem uses a particularly strong directed-cover property: every
base path has a unique lift from each initial fibre state.  Hence row path
counts agree at every length, so the lift and channel-count base have equal
spectral radius.

Two-lift literature emphasizes that twists may create substantial *new*
eigenvalues even though the Perron growth remains controlled by the positive
base sector:

- Y. Bilu and N. Linial, *Lifts, discrepancy and nearly optimal spectral gap*,
  Combinatorica 26 (2006).
  https://www.cs.huji.ac.il/~nati/PAPERS/raman_lift.pdf
- A. Marcus, D. Spielman, N. Srivastava, *Interlacing Families I: Bipartite
  Ramanujan Graphs of All Degrees*, Annals of Mathematics 182 (2015).
  https://annals.math.princeton.edu/wp-content/uploads/annals-v182-n1-p07-p.pdf

These papers concern control of nontrivial lift eigenvalues, whereas Ravel's
core theorem identifies the trivial positive sector as the universal Perron
sector for the n-bonacci transfer system.

## 3. Sofic and finite-to-one symbolic extensions

Finite-state presentations, right-resolving graphs, and finite-to-one factors
connect graph spectral radius to symbolic entropy.  A right-resolving finite
presentation has entropy equal to the logarithm of its Perron eigenvalue.

Relevant references:

- J. C. Lagarias and Y. Wang, *Path Sets in One-Sided Symbolic Dynamics*,
  arXiv:1207.5004.
  https://arxiv.org/abs/1207.5004
- J. Buzzi, *Subshifts of Quasi-Finite Type*, arXiv:math/0305164.
  https://arxiv.org/abs/math/0305164
- U. Jung and I.-J. Lee, *Bi-resolving graph homomorphisms and extensions of
  bi-closing codes*, arXiv:0904.3042.
  https://arxiv.org/abs/0904.3042

Ravel's Condition-F quotient is a sofic presentation, while finite permutation
fibres are zero-entropy extensions.  This is the symbolic-dynamics analogue of
the unique-path-lifting spectral equality.

## 4. Twisted transfer operators and matrix cocycles

For a transfer operator carrying a vector-bundle or matrix cocycle, growth is
controlled by base entropy together with fibre Lyapunov growth.  This is the
right large-scale analogue of the exotic principle derived in round 79.

References:

- V. M. Gundlach and Y. Latushkin, *A sharp formula for the essential spectral
  radius of the Ruelle transfer operator on smooth and Hölder spaces*,
  Ergodic Theory and Dynamical Systems 23 (2003), 175--191.
  https://doi.org/10.1017/S0143385702000962
- G. Froyland et al., *Coherent structures and isolated spectrum for
  Perron--Frobenius cocycles*, arXiv:0804.1426.
  https://arxiv.org/abs/0804.1426
- A. Blumenthal and I. D. Morris, *Characterization of dominated splittings for
  operator cocycles acting on Banach spaces*, arXiv:1512.07602.
  https://arxiv.org/abs/1512.07602

For unitary or contractive fibre maps, the fibre Lyapunov contribution is
nonpositive, so the untwisted positive operator remains dominant.  For
expansive fibre maps, the ordinary base need not dominate; a norm-weighted
transfer operator is required.

## 5. The new exotic boundary: norm-weighted augmentation

Let a base channel `e : u -> v` carry a fibre operator `U_e`.  Define

\[
  B_{\mathrm{norm}}(u,v)
   = \sum_{e:u\to v} m_e\,\lVert U_e\rVert.
\]

For every operator-valued path,

\[
  \lVert U_{e_k}\cdots U_{e_1}\rVert
  \leq \prod_i \lVert U_{e_i}\rVert.
\]

Therefore the twisted transfer operator is dominated by the scalar
norm-augmentation:

\[
  \rho(T_{\mathrm{twist}})
  \leq \rho(B_{\mathrm{norm}}).
\]

If every `||U_e|| <= 1`, then

\[
  B_{\mathrm{norm}}\leq B_{\mathrm{count}},
\]

and the original n-bonacci dominance theorem projects immediately.  If some
fibre operator is expansive, the unweighted base theorem is false in general.
The one-vertex example

\[
  B_{\mathrm{count}}=[1],\qquad U=[2]
\]

has twisted spectral radius `2`, not `1`.

This forces a new principle:

> **Norm-weighted twist dominance.**  An operator-valued n-bonacci twist is
> controlled by the canonical n-bonacci transfer operator weighted by fibre
> operator norms.  In thermodynamic language, the base entropy is corrected by
> the top fibre Lyapunov growth.

This is not a contradiction of the kernel-checked theorem.  It precisely marks
its natural hypothesis: finite permutation, unitary, or contractive twists.

## 6. Novelty assessment

### Standard ingredients

- Perron order comparison;
- voltage/gain graph representation sectors;
- spectral decomposition of finite lifts;
- entropy of right-resolving sofic presentations;
- transfer operators with matrix cocycles;
- norm/Lyapunov corrections for expanding fibres.

### Potentially novel synthesis

- the canonical n-bonacci prefix-defect split into a `Q/R` pair;
- the exact eleven-word terminal-parent boundary substitution;
- the proof that a joint `Q/R` comparison is preserved in every dimension;
- the identification of all phase, holonomy, and component twists as finite
  extensions of that pair;
- automatic generation of a Lean-kernel-checked universal dominance theorem.

A publication claim should be limited to the exact theorem and supported by a
formal comparison with the multinacci normalization, Bernoulli-convolution,
finite-type, graph-lift, and transfer-cocycle literatures.

## 7. Immediate corollaries now implemented

1. **Gauge/switching invariance.**  Vertexwise relabelling of fibres conjugates
   the lift and preserves the complete spectrum.
2. **Product twists.**  Independent finite twists combine by direct-product
   fibre action and retain unique path lifting and equal Perron growth.
3. **Unitary and contractive operator twists.**  Entrywise norm domination
   projects the original universal dominance theorem.
4. **Expansive operator twists.**  Ordinary dominance can fail; the correct
   comparator is the norm-weighted augmentation.
