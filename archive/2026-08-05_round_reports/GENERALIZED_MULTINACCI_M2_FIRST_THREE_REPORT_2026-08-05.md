# Generalized multinacci unit family, m=2: first three indices

Author: **Ravel**. Coordinator and project architect: **AM**.

## Scope

This round reuses the existing n-bonacci balanced-pair/contact-boundary pipeline on

\[
\sigma_{D,m}(i)=0^m(i+1)\quad(i<D-1),\qquad \sigma_{D,m}(D-1)=0,
\]

with characteristic polynomial

\[
p_{D,m}(x)=x^D-m(x+x^2+\cdots+x^{D-1})-1.
\]

The requested indices `n=0,1,2` are represented as dimensions `D=n+2`, so the first tested non-n-bonacci slice is `(D,m)=(2,2),(3,2),(4,2)`.

## Reused pipeline

The new driver does not introduce a parallel graph construction. It uses the same:

- substitution and exact incidence machinery;
- D-continuation search;
- contact-boundary construction;
- balanced-pair transition graph;
- recurrent and dominant SCC extraction;
- involution audit;
- exact rational Collatz-Wielandt comparison;
- quotient and characteristic-polynomial checks.

The only generalized input layer is `include/ravel/generalized_multinacci.hpp`, which constructs the family rule, polynomial, and Perron root. At `m=1` it reproduces the old n-bonacci substitutions exactly.

## Results

### Index 0: D=2, m=2

\[
\beta=1+\sqrt2\approx2.4142135624.
\]

The balanced-pair recurrent core has spectral radius `1`, while the contact-boundary graph is empty. Thus the old equality theorem does not merely become a weighted equality; the boundary object itself degenerates in this base case.

### Index 1: D=3, m=2

The existing exact rational bracket engine proves

\[
\rho(G_B)<\rho(G_{BP}).
\]

Numerically:

\[
\rho(G_{BP})\approx1.937087,
\qquad
\rho(G_B)\approx1.787715.
\]

The old free involution is partial: `10/12` recurrent-core nodes pair under

\[
[i,x,j]\longleftrightarrow[j,-x,i].
\]

### Index 2: D=4, m=2

Again the exact bracket engine proves

\[
\rho(G_B)<\rho(G_{BP}).
\]

Numerically:

\[
\rho(G_{BP})\approx2.446268,
\qquad
\rho(G_B)\approx2.415332.
\]

The dominant recurrent SCC is correctly selected by the existing dominant-SCC machinery. The old involution is partial on the dominant core (`42/46`) and remains partial on the union of recurrent SCCs (`44/48`).

## Intuitive implication

The doubled prefix is not simply two parallel copies of the n-bonacci channel. The two occurrences of `0` have different prefix positions. They therefore carry a short positional phase.

The correct nearby interpretation is:

\[
\text{n-bonacci transport}
\quad+\quad
\text{a finite prefix-position twist}.
\]

Forgetting that phase merges distinct parent transports and destroys the old exact involution/equality mechanism. What survives in the first nontrivial cases is the directional claim

\[
\rho(G_B)<\rho(G_{BP}).
\]

This is favorable for the broader dominance program: the deformation appears to strengthen dominance rather than furnish a counterexample. But the proof grammar must retain the prefix phase, likely as either additional positive generators or a two-matrix base with a two-state fibre.

## Next exact derivation

The next reusable operation should split each `0^m` prefix by position and derive its parent-channel matrices rather than collapsing them into one symbol. For `m=2`, the raw phase alphabet is `{first-zero, second-zero, terminal-letter}`. The engine should determine whether this minimizes to:

1. more than two positive matrices; or
2. the old Q/R pair tensored with a two-state prefix-phase twist.

That is the precise wrench exposed by the first three cases.
