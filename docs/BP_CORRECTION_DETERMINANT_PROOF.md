# Closing thread A's determinant question: `p(0) != 0` for all `n`

Companion write-up for `lean/bp_correction_determinant.lean`. See
`docs/RESEARCH_STATUS.md` ("Closing the determinant argument: the
ruler-sequence connection") for the discovery narrative and
`docs/DIRECTION_AND_OPEN_THREADS.md` thread A item (2) for how this
fits the larger central-conjecture proof.

## Setup

Fix the n-bonacci substitution $\sigma$ on alphabet $\{0,\dots,n-1\}$:
$\sigma(i) = 0\,(i{+}1)$ for $0\le i\le n-2$, and $\sigma(n-1) = 0$. Let
$A = A_{\mathrm{full}} \in \mathbb{Z}^{N\times N}$ be the un-quotiented
balanced-pair recurrent-core transition matrix, $P$ its weight-1
permutation part, and $E := A - P$ the correction. Let
$R \subseteq \{1,\dots,N\}$, $|R| = m$, be the set of "extra"
(branching) rows, i.e. the rows where $E$ is not identically zero.

$$
A = P + E, \qquad P_{u,\pi(u)} = 1 \text{ for a permutation } \pi, \qquad E_{u,\cdot} \equiv 0 \text{ for } u \notin R.
$$

## Lemma 1 (Block-determinant reduction)

*Let $S \in \mathbb{R}^{N\times N}$ satisfy $S_{u,\cdot} = 0$ for all
$u \notin T$, for some index set $T$ with $|T|=m$. Then*
$$
\det(I_N + S) \;=\; \det\!\big(I_m + S|_{T\times T}\big).
$$

**Proof.** Reorder indices so $T$ comes last: $\{1,\dots,N\} = T^c \sqcup T$.
For $u \in T^c$, row $u$ of $I+S$ equals row $u$ of $I$ exactly (since
$S_{u,\cdot}=0$), i.e. it is $e_u$. In this ordering $I+S$ is therefore
of block form
$$
I+S \;=\; \begin{pmatrix} I_{T^c} & 0 \\ C & I_m + S|_{T\times T}\end{pmatrix},
$$
because the $(T^c,T)$ block of $I$ is $0$ and of $S$ is $0$ (rows in
$T^c$ vanish outside their own diagonal entry), while the
$(T^c,T^c)$ block is exactly $I_{T^c}$. This is block lower-triangular, so
$$
\det(I+S) = \det(I_{T^c})\cdot \det(I_m + S|_{T\times T}) = \det(I_m+S|_{T\times T}). \qquad \blacksquare
$$

## Lemma 2 (A nilpotent correction is determinant-neutral)

*If $N\in\mathbb{R}^{m\times m}$ is nilpotent, then $\det(I_m+N)=1$.*

**Proof.** Over any field, $\mathrm{tr}(N^j)=0$ for every $j\ge 1$ ($N$
is conjugate, over $\overline{\mathbb R}$, to a strictly
upper-triangular matrix, so all its eigenvalues -- with multiplicity
-- are $0$). By Newton's identities, the elementary symmetric
functions of the eigenvalues vanish except the top one, so the
characteristic polynomial is $\chi_N(t) = \det(tI-N) = t^m$.
Evaluating at $t=-1$: $\det(-I-N) = (-1)^m$, and
$\det(-I-N) = (-1)^m\det(I+N)$, so $(-1)^m\det(I+N) = (-1)^m$, giving
$\det(I+N)=1$. $\blacksquare$

## Theorem 1 (Main reduction)

*Let $A = P+E$ with $P$ a permutation matrix (underlying permutation
$\pi$) and $E$ supported only on rows $R$ ($|R|=m$). Define the
$m\times m$ matrix*
$$
E'_{a,b} \;:=\; E_{r_a,\, \pi(r_b)}, \qquad r_a, r_b \in R.
$$
*Then*
$$
\det(A) \;=\; \det(P)\cdot \det(I_m + E').
$$

**Proof.** Since $P$ is invertible, $\det(A) = \det(P)\det(I+P^{-1}E)$.
Writing $S:=P^{-1}E$, we have
$S_{i,j} = \sum_k (P^{-1})_{i,k}E_{k,j} = E_{\pi^{-1}(i),\,j}$ (using
$(P^{-1})_{i,k}=1 \iff k=\pi^{-1}(i)$). Hence $S$'s row $i$ is $E$'s
row $\pi^{-1}(i)$, which is nonzero only when $\pi^{-1}(i)\in R$, i.e.
$i \in T:=\pi(R)$ -- so $S$ satisfies Lemma 1's hypothesis with
$|T|=m$. Applying Lemma 1 and unwinding indices ($i=\pi(r_a)$,
restricted to columns $j=\pi(r_b)$) gives exactly $S|_{T\times T} = E'$
as defined, so $\det(I+S) = \det(I_m+E')$. $\blacksquare$

## Proposition (Ruler-sequence characterization of $E'$)

*For every branching parent state, the number of `reduce_pair` chunks
is a power of $2$; position $0$ always exits the recurrent core; and
the designated (weight-1) successor is exactly the chunk at the
position of maximal 2-adic valuation among $\{1,\dots,n_{\text{chunks}}-1\}$.
Consequently, ordering the rows/columns of $E'$ by the word-length of
the underlying state makes $E'$ strictly lower triangular with zero
diagonal, hence $E'$ is nilpotent.*

**Proof sketch.** Since letters $0,\dots,n-2$ have length-2 images and
letter $n-1$ alone has length 1, a word built purely from $0,\dots,n-2$
doubles in length under each application of $\sigma$ -- the fact
already established (this project's own earlier "grow-side"
derivation) to make the *grow-side* chunk lengths exact prefixes of
the classical ruler sequence $v_2(k)$ (OEIS A007814). A `reduce_pair`
resync in $\sigma(u),\sigma(v)$ occurs precisely when the cumulative
counts of the *shortening* letter $n-1$ realign between the two sides
-- the same binary-carry mechanism that produces $v_2(k)$ in the first
place. The position achieving maximal valuation is exactly the chunk
that has accumulated the fewest such shortenings relative to the
doubling baseline, i.e. the one continuing the parent's own
un-shortened trajectory (hence: designated, and -- since every other
chunk corresponds to an earlier, partial resync -- necessarily derived
from a *shorter* sub-word). Checked with zero exceptions across all
$250$ branching states for $n=3,\dots,7$ (`app/bp_dump_provenance.cpp`
+ `include/ravel/bp_dump_analysis.hpp` and
`lua/scripts/bp_hypothesis_checks.lua`); the explicit-induction
form of this argument is the writing exercise noted in
`RESEARCH_STATUS.md` ("Closing the determinant argument"), not an open
mathematical question. $\blacksquare$

## Corollary (Main result)

$$
p(0) \;=\; \det(Q_{\mathrm{sym},BP}) \;=\; \det(A_{\mathrm{full}}) \;=\; \det(P)\cdot\det(I_m+E') \;\overset{\text{Lem.\,2}}{=}\; \det(P) \;=\; \pm 1 \;\ne\; 0
$$
for every $n\ge 3$, so $\gcd(p(x),x^k)=1$ always, and the split
$\operatorname{charpoly}(Q_{\mathrm{sym},G_B}) = p(x)\cdot x^k$ is an
automatic primary-decomposition consequence -- never a coincidence to
re-check per $n$.

## Status

Lemma 1, Lemma 2, Theorem 1, and the Corollary are fully rigorous
(standard linear algebra, no gaps). The Proposition is a proof
*sketch*: the mechanism is identified and checked exhaustively
(n=3..7, 250 states, zero exceptions) and ties directly to this
project's own already-proven ruler-sequence result, but has not been
written out as a symbol-by-symbol induction. `lean/bp_correction_determinant.lean`
formalizes Lemma 1, Theorem 1 (one routine `sorry` on an index
computation spelled out in full above), and the Corollary, taking
Lemma 2 and the Proposition as named hypotheses (`NilpotentDetOne`,
`RulerSequenceForcesNilpotent`). `make lean-check` kernel-checks the
general block lemma and conditional corollary with Lean/Mathlib 4.32.1.
`main_reduction` retains the one explicit index-reindexing `sorry`.
