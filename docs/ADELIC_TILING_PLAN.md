# Adelic tiling classification: what to build, and how

Goal: implement a finite, terminating, fully rigorous check for whether a
given Pisot substitution's Rauzy fractal actually tiles — replacing the
double-precision-sampled "does the picture look laminated" evidence
(`RAUZY_FRACTAL_RESULTS.md`, and the density-heatmap visualization)
with a real yes/no answer, and — in the interesting case
where the answer is "yes, but not in the space you were looking at" —
an explanation of exactly what extra structure was missing.

This document is self-contained: every definition and theorem below is
paraphrased in full from the source papers, in plain text, specifically
so a future session without image/OCR access can work from this file
alone rather than needing to re-read PDF figures. Citations are given
in prose (§7) rather than assumed to be re-fetchable.

## 0. One-paragraph summary of the plan

For a Pisot substitution whose incidence matrix has $|\det| > 1$ (a
"non-unit" Pisot substitution, in this literature's terminology — this
is the same thing this project's other docs call "non-unimodular"),
the Rauzy fractal does not live in $\mathbb{R}^{d-1}$ alone. It lives in
a larger space that also includes one $p$-adic factor for every prime
dividing $\det M$. A theorem (Minervino & Thuswaldner, full citation in
§7.1) proves this extended-space tiling *always* exists as at least a
*multiple* tiling (possibly overlapping); a second, finite, checkable
condition determines whether it's a genuine non-overlapping tiling.
Most of the pieces needed already exist in this project in a reusable
form — eigenvector data, prefix-decomposition combinatorics, exact
Sturm-based root isolation, and the `ModPolyRing` number-ring
representation (§3.0) — and the two genuinely new components (ideal
factorization, §3.1, and $p$-adic arithmetic, §3.2) are built and
cross-checked entirely from scratch inside this codebase, deliberately
not outsourced to an external CAS (§3.1b) — consistent with this
project's existing "roll your own, FREE alternative" convention
(`CPP_DESIGN_PHILOSOPHY.md` §2) and more useful for an LLM-driven
session to actually run and verify than a tool built for interactive
human use. Full breakdown of what's reused vs. new is §3.0; the new
pieces themselves are §3.1-§3.4, all designed to sit inside
`lua/src/` following this project's existing conventions (header +
paired `_test.cpp`, reuse `mini-gmp` rather than adding a new
dependency).

## 1. The question this closes

`RAUZY_FRACTAL_RESULTS.md` concluded rnd13 "does not tile" from a
striped/laminated real-space point cloud. `FINDINGS_FOR_CITATION.md`
Finding 1/2 treats this as suggestive-but-not-conclusive. The theorem
in §2.6 below gives an actual finite procedure that either (a) confirms
rnd13 tiles a specific, explicitly-describable larger space (in which
case the stripes are fully explained — see §2.7 for a worked example of
exactly this outcome) or (b) confirms it genuinely fails to tile
anything, which is the real Barge-Bruin-Jones-Sadun non-tiling
verdict, not an artifact of only having looked at $\mathbb{R}^{d-1}$.

## 2. Math background, condensed

### 2.1 Number field setup

Let $\beta$ be the Pisot root of a substitution's characteristic
polynomial (this project already computes this exactly — the char
polys and Sturm-based root isolation apply
directly). Let $K = \mathbb{Q}(\beta)$, a number field of degree $n$
(the alphabet size, assuming the char poly is irreducible — check this
first; the project's exact Pisot classifier already tests
squarefreeness, but full irreducibility over $\mathbb{Q}$ is a
separate, additional check, needed here and not previously needed).
Let $\mathcal{O}$ be the ring of integers of $K$ (not necessarily
$\mathbb{Z}[\beta]$ — see §3.1). A *place* of $K$ is an equivalence
class of absolute values; the *infinite* (Archimedean) places come from
the real and complex embeddings of $K$; a *finite* (non-Archimedean)
place $\mathfrak{p}$ corresponds to a prime ideal of $\mathcal{O}$, with
an associated $\mathfrak{p}$-adic valuation $v_\mathfrak{p}$ and
completion $K_\mathfrak{p}$ (isomorphic to a finite extension of
$\mathbb{Q}_p$, where $p$ is the rational prime below $\mathfrak{p}$).

### 2.2 The representation space for a non-unit substitution

For a *unit* (unimodular, $|\det M| = 1$) Pisot substitution, the
classical Rauzy fractal lives in $\mathbb{R}^{n-1}$ (the hyperplane
orthogonal to the left Perron eigenvector) — this is what
`rauzy_fractal.hpp` already builds via the natural-coding method.

For a *non-unit* substitution, $(\beta)$ is a proper ideal of
$\mathcal{O}$, so it factors into a product of prime ideals
$\mathfrak{p}_1^{e_1}\cdots\mathfrak{p}_k^{e_k}$. The correct
representation space is

$$K_\sigma = (\text{Archimedean factor, }\mathbb{R}^r\times\mathbb{C}^s\text{, minus the }\beta\text{ direction}) \;\times\; \prod_{i=1}^{k} K_{\mathfrak{p}_i}$$

— the usual real/complex Rauzy-fractal space, times one $p$-adic
completion per prime dividing $(\beta)$. Multiplication by $\beta$ acts
as a genuine contraction on all of $K_\sigma$ (it contracts in the real
directions the usual way, and in each $p$-adic factor because $\beta$
has $\mathfrak{p}$-adic valuation $\geq 1$ there by construction — this
is exactly why the $p$-adic factor has to be included at all: it's the
factor of $K_\alpha$ where $\beta$ is *not* already a contraction from
the ordinary real Perron-Frobenius fact alone).

### 2.3 Prefix automaton and Dumont-Thomas digits

Already close to what `ambient_graph.hpp`'s `parent_decompositions`
computes. For each letter $b$, for every way to write $\sigma(b) = p\,a\,s$
(a prefix $p$, a single letter $a$, a suffix $s$), record a labeled
edge $b \xrightarrow{p} a$ in the *prefix automaton*. Define
$\delta(p) = \langle \mathbf{P}(p), \mathbf{v}\rangle$ where
$\mathbf{P}(p)$ is the abelianization (letter-count vector) of $p$ and
$\mathbf{v}$ is the left Perron eigenvector, scaled so its entries lie
in $\mathbb{Q}(\beta)$. The finite set $\mathcal{D}$ of all such
$\delta(p)$ values (one per prefix that occurs in the automaton) is the
*digit set*.

*Worked mini-example* (in place of a figure): for
$\sigma(1) = 1\,1\,1\,3$, $\sigma(2) = 1\,1$, $\sigma(3) = 2$ (a real
example from the source paper, degree 3, $|\det|=2$ — deliberately
close in shape to rnd13), the prefix automaton has edges
$1\xrightarrow{\epsilon}1$ (prefix empty, since $\sigma(1)$ starts with
1), $1\xrightarrow{1}1$, $1\xrightarrow{11}1$, $1\xrightarrow{111}3$,
$2\xrightarrow{\epsilon}1$, $2\xrightarrow{1}1$, $3\xrightarrow{\epsilon}2$.
This kind of table (letter, prefix, resulting-letter triples) is
exactly what a text-based implementation needs; no figure is required
to specify it.

### 2.4 Strong coincidence condition

A substitution satisfies the *strong coincidence condition* if: for
every pair of letters $(b_1, b_2)$, there exists some power $k$ and
some letter $a$ such that $\sigma^k(b_1) = p_1\,a\,s_1$ and
$\sigma^k(b_2) = p_2\,a\,s_2$ (i.e. $\sigma^k(b_1)$ and $\sigma^k(b_2)$
both contain letter $a$), with either the abelianizations of the two
prefixes equal ($\mathbf{P}(p_1)=\mathbf{P}(p_2)$) or the abelianizations
of the two suffixes equal. This is a finite condition to search for
(bounded alphabet size squared pairs, increasing $k$) but is not a
priori bounded in $k$ — see §5 for the practical search strategy.

### 2.5 Geometric property (F) and the zero-expansion graph

Define $\mathcal{U}$ as the set of $(0, a)$ pairs, one per letter $a$.
Define a "desubstitution" map $T^{-1}_{\mathrm{ext}}$ on
(translation, letter) pairs by: $(\gamma', b) \in T^{-1}_{\mathrm{ext}}(\gamma, a)$
iff there is a prefix-automaton edge $b\xrightarrow{p}a$ with
$\gamma' = \beta^{-1}(\gamma + \delta(p))$. A substitution satisfies
*geometric property (F)* if repeatedly applying $T^{-1}_{\mathrm{ext}}$
to $\mathcal{U}$ eventually reaches every translation that can ever
occur (formally: the translation set $\Gamma$, defined as all
$(\gamma, a)$ reachable this way from any starting point, equals the
union of finitely many iterates of $T^{-1}_{\mathrm{ext}}$ starting
from $\mathcal{U}$ alone).

This has a genuinely finite, terminating check: build the
*zero-expansion graph* $\mathcal{G}^{(0)}$ — nodes are pairs
$(\gamma, a)$ with $\|\gamma\|$ below an explicit, computable bound $M$
(given in the source paper as
$M = \max_{\delta(p)\in\mathcal{D}}\|\delta(p)\| \,/\, (1-\|\beta\|)$,
using the norm on the non-Archimedean-factor coordinates), with an
edge from $(\gamma_1,a_1)$ to $(\gamma_2,a_2)$ exactly when
$(\gamma_2,a_2) \in T^{-1}_{\mathrm{ext}}(\gamma_1,a_1)$. This graph is
finite because $\Gamma$ is a Delone set (uniformly discrete) intersected
with a bounded ball. **Property (F) holds if and only if the only cycle
in $\mathcal{G}^{(0)}$ passing through $(\mathbf{0},a)$-type nodes is
the trivial one** — equivalently, $\mathbf{0}$ is not reachable from any
nonzero node via a cycle. This is a plain graph algorithm (cycle
detection), not an unbounded search — the finiteness of
$\mathcal{G}^{(0)}$ is what makes property (F) actually decidable in
finite time, unlike the strong coincidence condition in §2.4.

### 2.6 The classification theorem

Putting the pieces together (this combines two results from the source
paper): (1) every irreducible Pisot substitution — unit or not —
induces at least a *multiple* tiling of $K_\sigma$ (some points covered
more than once, but never uncovered) — this direction is unconditional,
always true, no check needed. (2) If the substitution satisfies *both*
the strong coincidence condition (§2.4) *and* geometric property (F)
(§2.5), that multiple tiling is a genuine simple tiling — multiplicity
exactly 1 almost everywhere.

So: run the property-(F) check (finite, always terminates) and search
for strong coincidence (finite per depth, unbounded in principle — see
§5 for the practical stopping rule). Both hold → rnd13 (or whichever
substitution) tiles $K_\sigma$ cleanly, and the specific $p$-adic
factor computed in §3.1 tells you exactly what the "hidden coordinate"
behind the stripes is. Either check fails (with enough search depth to
trust the failure) → genuine non-tiling, no adelic rescue available.

### 2.7 A worked example structurally identical to rnd13

The source paper's own §10.2 example is $\sigma(1)=1113$, $\sigma(2)=11$,
$\sigma(3)=2$ — 3 letters, incidence matrix
$\begin{pmatrix}3&2&0\\0&0&1\\1&0&0\end{pmatrix}$, characteristic
polynomial $x^3-3x^2-2$, irreducible, Pisot root $\beta\approx3.196$,
complex conjugate pair $\approx -0.098\pm0.785i$ (modulus $<1$, so
genuinely Pisot), $\det = -2$ so $|\det|=2$ — the same determinant
magnitude as rnd13. Worked factorization: $(2) = (\beta)^2\cdot(\beta^2-1)$
in $\mathcal{O}$, i.e. the ideal $(2)$ splits as $\mathfrak{p}_1^2\cdot\mathfrak{p}_2$
with $\mathfrak{p}_1=(\beta)$ and $\mathfrak{p}_2=(\beta^2-1)$ (using
$|\beta|_{\mathfrak{p}_1}=1/2$, $|\beta|_{\mathfrak{p}_2}=1$, so only
$\mathfrak{p}_1$ actually needs to be adjoined — $\mathfrak{p}_2$'s
completion doesn't contract and drops out). The resulting non-Archimedean
factor is a degree-2 extension of $\mathbb{Q}_2$. The paper verifies
this specific example satisfies both §2.4 and §2.5, and shows the
resulting tiling explicitly. **This is the template to mirror for
rnd13** — same alphabet size range, same $|\det|=2$, same overall shape
of problem; the only unknowns are rnd13's specific ideal factorization
and whether its coincidence/property-(F) checks pass.

## 3. What to build, mapped onto this codebase

Four new pieces, in priority/dependency order. All as header-only
`.hpp` + paired `_test.cpp`, per `CPP_DESIGN_PHILOSOPHY.md` convention;
all using `mini-gmp` for exact integer/rational arithmetic (already
vendored, already linked into several targets) rather
than introducing a new dependency. Cross-checking (§3.1b) is also
rolled from scratch, for the same reason — see there for why.

### 3.0 Leverage, extend, or deprecate: the existing Rauzy-fractal / contact-boundary code

Before adding new files, here's what happens to the ones already in
`lua/src/` and `lua/scripts/`.

**Directly leverage, no changes needed:**
- `rauzy_fractal.hpp`'s eigenvector machinery (`M`, `beta()`, `v()`,
  `u()`, `M_inv()`) is exactly the Perron-Frobenius data §2.1-§2.3
  need. No need to recompute any of it.
- `ambient_graph.hpp`'s `parent_decompositions` and `abelianization`
  are, almost verbatim, the machinery §3.3's prefix automaton needs —
  `parent_decompositions` already enumerates every $(p, a, s)$ with
  $\sigma(c) = p\,a\,s$ for a *given* target letter `a`; §3.3 just
  needs to call it once per letter to get the *whole* automaton
  instead of one inner-letter slice at a time. Same function, wider
  loop around it.
- `exact_pisot.c`'s Sturm chain and `isolate_real_root_generic_mpz`
  directly produce the certified minimal
  polynomial data §3.1 starts from.
- `algebraic.hpp`'s `ModPolyRing<Derived, d, Coeff>` CRTP pattern is
  exactly the representation needed for elements of $\mathbb{Z}[\beta]$
  in §3.1/§3.1b's ideal arithmetic — multiplication with reduction mod
  the minimal polynomial is already implemented there, correctly.

**Needs extending, not replacing — and this closes an item already on
the books:** `algebraic.hpp`'s own header comment says it only
supports 8 hand-picked named Pisot rings and explicitly defers
"NumberField constructor" — i.e. building the ring from an arbitrary
computed minimal polynomial rather than a hardcoded list — as future
work (this is also `DIRECTION_AND_OPEN_THREADS.md` thread C). This
plan is the concrete reason to finally do that: §3.1/§3.1b need
`ModPolyRing` instantiated on rnd13's actual (currently unnamed)
quartic minimal polynomial, not one of the 8 named rings. Generalizing
the CRTP base to take a runtime- or template-parameter-supplied
`minpoly()` instead of requiring a hand-written subclass per Pisot
number is a small, mechanical change — the arithmetic itself doesn't
change at all.

**Genuinely complementary, not redundant — keep both, with one open
question flagged:** `contact_boundary.hpp`/`corona.hpp`'s $\lambda(G_B)$
vs. $\beta$ computation and the new adelic classifier are different
tools. $\lambda(G_B) \le \beta$ is a *necessary* condition for the
real-space geometric realization (Barge–Gambaudo); the adelic
strong-coincidence + property-(F) check (§2.6) is *necessary and
sufficient* for a simple tiling, but of the extended space $K_\sigma$,
not of $\mathbb{R}^{d-1}$ alone. **Open question, worth resolving
before treating a disagreement as a bug**: does the November-2025
paper's $G_B$ construction already implicitly encode the $p$-adic
factor for non-unit substitutions, or is it real-space-only and
therefore could in principle report an obstruction for a substitution
that the adelic method correctly says tiles $K_\sigma$? Don't assume
either answer — if rnd13 comes back TILES from §3.5's driver while
$\lambda(G_B) = 8.48 > \beta = 5.62$ still holds, that disagreement is
itself the thing to chase down (it would mean either $G_B$ needs its
own $p$-adic generalization, or the two constructions are legitimately
answering different questions and both facts stand). Keep computing
both, on purpose, as a built-in cross-check between two independently-
built tools — same spirit as §3.1b below.

**Resolved engineering issue:** `rauzy_fractal_natural.cpp` sampled
letters i.i.d. rather than from substitution dynamics and has been
removed. The tested native graph-directed generator is exposed as
`spectre.rauzy_fractal.generate` and driven by
`lua/scripts/rnd13_rauzy.lua`. A future genuine Dumont-Thomas coding
could still color the real-space point cloud by p-adic residue class,
turning the striped picture from suggestive into a labeled
visualization of the algebraic answer. Any future doc referencing
the density heatmap says so too).

### 3.1 `dedekind_factorization.hpp` — ring of integers and factoring $(\det M)$

Input: the exact monic integer minimal polynomial of $\beta$ (already
producible via this project's char-poly + Sturm-irreducibility-check
machinery). Output: for each rational prime $p$ dividing $|\det M|$,
the factorization of $(p)$ into prime ideals of $\mathcal{O}$, with
each factor's ramification index $e$ and residue degree $f$ (giving the
degree of the resulting $p$-adic extension, $ef$).

**This is the one piece where I'd flag real implementation risk.** The
standard method is Dedekind's criterion (factor the minimal polynomial
mod $p$; check whether $\mathbb{Z}[\beta]$ is $p$-maximal via a
specific gcd computation on the factors and their lift; if not
$p$-maximal, a more involved algorithm — the Round 2 or Round 4 method
— is needed to find the actual ring of integers at $p$). The precise
formula for Dedekind's criterion is the kind of thing worth verifying
against a real reference (Cohen, *A Course in Computational Algebraic
Number Theory*, §6.1.4, is the standard one) rather than reconstructing
from memory alone. **Practical recommendation**: implement Dedekind's
criterion for the (very likely, for these small-degree, small-
coefficient polynomials) case where $\mathbb{Z}[\beta]$ is already
$p$-maximal — this is the easy 90% case and is what the worked example
in §2.7 needed. Treat the non-maximal case (Round 2/4) as a follow-up
only if the easy case doesn't apply to the substitutions actually being
tested. Rather than trusting this single implementation, or reaching
for an external CAS, cross-check it against a second, independently-
built implementation — §3.1b.

### 3.1b `ideal_arithmetic.hpp` — an independent, from-scratch cross-checker

The plan's first draft suggested checking §3.1's output against
PARI/GP or SageMath. Fair alternative, on reflection, given this
project's own stated preference (`CPP_DESIGN_PHILOSOPHY.md` §2, the
"FREE alternative" pattern) and the practical point that neither tool
is set up for an LLM-driven session to call and parse cleanly the way
a small owned C++ header is: **build a second, algorithmically
different implementation that checks the same claim by direct
computation, rather than reaching for an external system.** The two
implementations don't need to be equally sophisticated — the
cross-checker's job is to *verify a specific claimed factorization*,
which is a much easier problem than *finding* the factorization from
scratch (§3.1's actual job).

**The method: ideals as Hermite Normal Form lattice bases, verified by
direct multiplication.** Represent $\mathbb{Z}[\beta]$ as $\mathbb{Z}^n$
via the power basis $1, \beta, \ldots, \beta^{n-1}$ (already how
`algebraic.hpp`'s `ModPolyRing` stores elements). An ideal of
$\mathbb{Z}[\beta]$ is then a rank-$n$ sublattice of $\mathbb{Z}^n$,
representable by an $n\times n$ integer matrix in Hermite Normal Form
(HNF) — the lattice-arithmetic analogue of row-reduced echelon form,
upper triangular with canonical pivot/reduction rules, unique for a
given lattice (this uniqueness is exactly what makes it usable as an
equality test — see below). Needed pieces:

1. **HNF reduction**: given a set of generator vectors (columns) of a
   full-rank sublattice of $\mathbb{Z}^n$, reduce to the canonical
   $n\times n$ HNF basis. Standard algorithm: repeated GCD/Bézout
   combination of column pairs sharing a nonzero entry in the current
   pivot row (this is essentially the extended Euclidean algorithm
   applied to eliminate entries, column by column, top row to bottom),
   producing an upper-triangular matrix, then reducing entries above
   each pivot modulo that pivot for canonical form. Well-documented,
   standard algorithm (again, Cohen §2.4.5 is the reference to pin
   exact index/sign conventions against) — implementable directly with
   `mini-gmp` integers (`mpz_gcdext` gives the Bézout coefficients
   needed for each column combination step).
2. **Ideal from generators**: given a set of ring elements
   $\gamma_1,\ldots,\gamma_k$ (e.g. $p$ and a lift of an irreducible
   factor $g_i(\beta)$, for the claimed prime $\mathfrak{p}_i$), the
   ideal they generate as a $\mathbb{Z}$-module is spanned by
   $\{\beta^j \gamma_i : 0\le j<n,\ 1\le i\le k\}$ (multiply every
   generator by every power of $\beta$ up to degree $n-1$, reducing
   mod the minimal polynomial via `ModPolyRing`'s existing
   multiplication) — then HNF-reduce that spanning set down to $n$
   basis vectors.
3. **Ideal multiplication**: given HNF bases for ideals $I$ and $J$
   (each $n$ vectors), form all $n^2$ pairwise products of basis
   vectors (again via `ModPolyRing` multiplication), then HNF-reduce
   the resulting $n^2$ vectors down to the $n$-vector HNF basis of
   $IJ$.
4. **Ideal power**: $\mathfrak{p}^e$ via repeated multiplication (§3).
5. **Ideal equality**: two ideals are equal exactly when their HNF
   bases are identical, entry for entry — a direct, exact, ambiguity-
   free check, no floating point or heuristic involved anywhere.

**The actual cross-check**: take §3.1's claimed factorization
$(p) = \mathfrak{p}_1^{e_1}\cdots\mathfrak{p}_k^{e_k}$, where Dedekind's
criterion gives each $\mathfrak{p}_i$ as $(p,\, g_i(\beta))$ for the
lifted irreducible factor $g_i$. Build each $\mathfrak{p}_i$'s HNF
basis via step 2, raise to the $e_i$ via step 4, multiply all $k$
results together via step 3, and check the result equals the HNF basis
of $(p) = p\cdot\mathbb{Z}[\beta]$ (trivially $p$ times the identity
matrix) via step 5. **Agreement between the two independently-built
pieces of code (Dedekind's criterion, and direct ideal-lattice
arithmetic) is the actual evidence the factorization is right** — no
external tool, no unverifiable "I'm pretty sure this is correct."

**Required tests, in order of increasing difficulty, before trusting
this on rnd13:**

1. **HNF sanity check, no number theory involved**: reduce a small
   hand-picked set of $\mathbb{Z}^2$ or $\mathbb{Z}^3$ vectors to HNF
   and check the result by hand (e.g. generators $(4,0),(0,6)$ should
   HNF-reduce to exactly themselves; generators $(2,3),(4,5)$ should
   reduce to a basis with determinant $|2\cdot5-3\cdot4|=2$). Pure
   linear algebra, no substitution or Pisot number involved yet.
2. **$\mathbb{Z}[i]$ warm-up** (Gaussian integers, $x^2+1$, degree 2):
   classical, hand-checkable answers — $2$ ramifies as $(1+i)^2$ (up to
   a unit), $5$ splits as $(2+i)(2-i)$, $3$ stays inert (prime in
   $\mathbb{Z}[i]$ too). Confirms both §3.1 and §3.1b agree with
   textbook facts before touching anything Pisot-specific, at a
   complexity level simple enough to check the arithmetic by hand if
   something disagrees.
3. **The paper's own worked example** (§2.7): $\sigma(1)=1113$,
   $\sigma(2)=11$, $\sigma(3)=2$, minimal polynomial $x^3-3x^2-2$,
   claimed factorization $(2)=\mathfrak{p}_1^2\mathfrak{p}_2$ with
   $\mathfrak{p}_1=(\beta)$. This is the primary oracle for the actual
   Pisot use case — a published, checked-by-someone-else answer to
   compare against, structurally the closest available case to rnd13.
4. **The hard required case — Dedekind's own classical non-monogenic
   example**: the cubic field defined by $\theta^3-\theta^2-2\theta-8=0$
   is the standard textbook example where $\mathbb{Z}[\theta]$ is *not*
   the full ring of integers at the prime 2 (this is the example
   Dedekind himself used to show naive factor-the-minimal-polynomial-
   mod-$p$ reasoning can go wrong). **Test that §3.1 correctly detects
   non-maximality and reports "Round 2/4 needed" rather than silently
   returning a wrong factorization** — this is the single most
   important test in the whole list, since it's specifically testing
   whether the code recognizes the case it's not built to solve,
   rather than testing whether it gets the easy cases right. Getting
   only the easy cases right and silently mishandling the hard one is
   exactly the failure mode worth a dedicated test for.

Only after all four pass — especially #4 — should rnd13's own
(currently unknown) factorization be trusted from either
implementation.

### 3.2 `padic.hpp` — $p$-adic (or unramified/ramified extension) arithmetic

Represent an element as a digit sequence base $p$ (or base a
uniformizer, for a ramified extension) with a starting valuation and a
fixed working precision (truncate after enough digits — this project's
existing "double precision by default, exact only when demonstrated
necessary" philosophy applies directly here: work to, say, 40-60 digits
of $p$-adic precision by default, which is more than enough to
distinguish the finitely many translation classes involved in §2.5's
graph, and only push higher if a specific check turns out to need it).
Needed operations: addition/subtraction with carrying, multiplication,
and the valuation function $v_\mathfrak{p}$. This is genuinely simpler
to get right than §3.1 — it's mechanical carrying arithmetic, not
number-theoretic structure-finding — and can be built and tested purely
against known identities (e.g. verify $1/(1-p) = 1+p+p^2+\cdots$ digit
by digit) without needing any substitution-specific input.

### 3.3 `prefix_automaton.hpp` — build $\mathcal{P}$, $\mathcal{D}$, and the automaton from a `Substitution<d>`

Directly computable from data this project already has:
`Substitution<d>::images` gives $\sigma$; `ambient_graph.hpp`'s
`parent_decompositions` already computes almost exactly the
$(p, a, s)$ triples needed (it currently searches for a specific
*inner* letter; generalize slightly to enumerate *all* triples for
every letter at once). Combine with the left eigenvector (already
computed in `core.hpp`/`spectral.hpp`) to get $\delta(p)$ for each
prefix, expressed as an element of $\mathbb{Q}(\beta)$ (a rational
vector in the power basis $1,\beta,\ldots,\beta^{n-1}$ — this reuses
`algebraic.hpp`'s `ModPolyRing` representation directly, generalized
from its current fixed named rings to an arbitrary computed minimal
polynomial).

### 3.4 `coincidence_and_property_f.hpp` — the two finite checks

Given the prefix automaton (§3.3) and $\delta$, implement:
- Strong coincidence search (§2.4): breadth-first over increasing $k$,
  for each pair of letters, checking whether $\sigma^k$ produces a
  shared letter with matching prefix or suffix abelianization. See §5
  for the stopping rule.
- Zero-expansion graph (§2.5): build $\mathcal{G}^{(0)}$ (finite, bound
  $M$ computable directly from $\mathcal{D}$ and $\beta$ as given in
  §2.5), then run ordinary cycle detection (e.g. Tarjan's SCC algorithm)
  to check whether any nonzero node has a path back to a
  zero-translation node.

### 3.5 Driver: `classify_adelic_tiling.cpp`

Ties it together: given a `Substitution<d>`, (1) get the exact char
poly and verify irreducibility (new check — see §3.1's prerequisite),
(2) factor $|\det M|$'s prime divisors via §3.1, (3) build the prefix
automaton and digit set via §3.3, (4) run both checks via §3.4, (5)
report: TILES ($K_\sigma$, with the specific $p$-adic factors named) /
DOES NOT TILE (either check failed) / INCONCLUSIVE (coincidence search
hit the depth cutoff without resolving — see §5). Follow the existing
`gkw_sturm_certify.cpp` / `verify_exploded_12.cpp` pattern: a small,
single-purpose `.cpp` in `scripts/`, printing a clear labeled report,
not yet wired into the Makefile until it's been run and checked by
hand at least once (same lesson as `cylinder_measure.cpp`'s history —
don't add to `make all` until it's actually been exercised).

## 4. Applying this to rnd13 specifically

rnd13: $\sigma(0)=(0,0,1,2,3,3)$, $\sigma(1)=(0,0,2,3,3)$,
$\sigma(2)=(0,0,3,3)$, $\sigma(3)=(0,0,0,2,3,3)$, $\beta\approx5.6236$,
$|\det|=2$, degree 4. Concrete first steps, in order:
1. Confirm the characteristic polynomial is irreducible over
   $\mathbb{Q}$ (not yet checked anywhere in this project — do this
   before anything else, since the whole $K=\mathbb{Q}(\beta)$
   construction assumes it).
2. Factor $(2)$ in $\mathcal{O}$ via §3.1. Expect (by analogy with
   §2.7's degree-3 example) either one ramified prime of degree
   dividing 4, or a product of two, giving one or two $2$-adic factors.
3. Build the prefix automaton (§3.3) — rnd13's automaton is somewhat
   larger than the degree-3 worked example (4 letters, longer images)
   but still small and fully explicit; write it out in the same
   table format as §2.3's mini-example when documenting the result.
4. Run both checks (§3.4). Report the verdict.
5. If TILES: the number of distinct residue classes in the relevant
   $p$-adic factor (computable directly from $e,f$ from §3.1) predicts
   how many "stripes" should appear in a real-space projection —
   compare against the density heatmap already generated
   (`rf_rnd13.txt` data, or a fresh higher-resolution run) as a sanity
   cross-check between the rigorous algebraic answer and the empirical
   picture.

## 5. Practical stopping rules (honesty about the one non-terminating piece)

Property (F) (§2.5) is unconditionally decidable in finite time — no
caveat needed there. Strong coincidence (§2.4) is not a priori bounded.
In practice: primitive Pisot substitutions satisfy strong coincidence
extremely often (it's known to hold for all Pisot substitutions on a
2-letter alphabet, per Barge & Kwapisz's result cited in §7.3, and
empirically holds quickly — within a handful of iterations — for most
larger examples in the literature). Practical rule: search up to some
generous depth (start with $k \le 20$; this is cheap, exact integer
arithmetic on small alphabets) and report INCONCLUSIVE rather than
DOES-NOT-TILE if no coincidence is found by the cutoff — do not
silently treat a search timeout as a negative result. If rnd13
specifically doesn't resolve by depth 20, that's itself an interesting,
reportable fact (worth escalating depth further before concluding
anything), not a proof of failure.

## 6. Regression-testing this like everything else in the project

Per `CPP_DESIGN_PHILOSOPHY.md` §3: every new header needs a paired
`_test.cpp`, checked against a case with a known answer before it's
trusted on rnd13. §3.1b's four-case ladder (HNF sanity check →
$\mathbb{Z}[i]$ → the paper's worked example → Dedekind's own
non-monogenic cubic) is exactly that for the factorization layer —
`dedekind_factorization_test.cpp` and `ideal_arithmetic_test.cpp`
should both run all four and agree with each other on the ones with a
known textbook answer, and on each other for rnd13, where the answer
is actually unknown. Same principle extends to §3.3-§3.4: build
`prefix_automaton_test.cpp` and `coincidence_and_property_f_test.cpp`
against $\sigma(1)=1113,\sigma(2)=11,\sigma(3)=2$ first (expected:
strong coincidence holds, property (F) holds, verdict TILES, matching
the paper's own stated conclusion for that example), and only then run
the full §3.5 pipeline on rnd13.

## 7. Condensed references (text, no figures needed)

### 7.1 Primary reference

Minervino, M. & Thuswaldner, J. M., "The geometry of non-unit Pisot
substitutions" (Annales de l'Institut Fourier, and an extended version
on arXiv as 1402.2002). This is the source of every definition and
theorem in §2 above. Structure of the paper, for anyone going back to
it directly: §2 sets up prefix automata and the strong coincidence
condition; §3 defines the adelic representation space $K_\sigma$ and
proves it's well-defined (a locally compact ring with a lattice acting
on it); §4 develops Dumont-Thomas numeration (the digit-expansion
viewpoint used in §2.3 above) and defines the Dumont-Thomas subtiles;
§5 gives an equivalent "dual geometric realization" construction (the
$E_1^*$-subtiles, matching what `rauzy_fractal.hpp`'s IFS-style
construction already does in the unit case, generalized); §6 gives a
third equivalent construction via cut-and-project model sets; §7 proves
all three constructions agree; §8 proves basic topological facts
(subtiles are closures of their interiors, boundaries have measure
zero); §9 is the key section — proves the always-a-multiple-tiling
result (Theorem 9.2, unconditional) and the simple-tiling criterion
(Theorem 9.9, conditional on strong coincidence + property (F)); §10
gives two fully worked examples, the second of which (§2.7 above) is
the template to mirror.

### 7.2 Where the non-unit Rauzy fractal concept originates

Siegel, A. — first defined Rauzy fractals for Pisot substitutions that
aren't units (cited in the primary reference's introduction as
reference [29]), building on the idea (already conjectured by Rauzy
himself) that the representation space needs $p$-adic factors for the
non-unit case. Sing, B. — PhD thesis (cited as [31]), developed the
cut-and-project/model-set viewpoint on these fractals in detail; §6 of
the primary reference is essentially a self-contained recap of Sing's
approach. Arnoux, P. & Ito, S. — the original (2001) paper associating
Rauzy fractals to unit Pisot substitutions via the geometric
realization / dual-map construction that §5 of the primary reference
generalizes.

### 7.3 Strong coincidence for small alphabets

Barge, M. & Kwapisz, J., "Geometric theory of unimodular Pisot
substitutions" — proves (among other things) that the strong
coincidence condition (called GCC, geometric coincidence condition, in
a closely related form there) holds unconditionally for every
unimodular Pisot substitution on a two-letter alphabet. Relevant here
only as background evidence that strong coincidence is a "usually
true, occasionally fails" condition rather than a rare accident —
supports the search-with-generous-cutoff strategy in §5 rather than
expecting pathological non-termination.

### 7.4 What happens when tiling genuinely fails

Barge, M., Bruin, H., Jones, L. & Sadun, L. (2010) — already the
primary citation in this project's `FINDINGS_FOR_CITATION.md` Finding
1. Introduces the Exact Regularity Property (ERP) and proves specific
non-unimodular Pisot substitutions fail to tile even in the fully
extended sense — i.e., this is the reference for what a genuine,
no-rescue-available DOES-NOT-TILE verdict from §3.5 would mean
mathematically, as opposed to a merely-incomplete real-space picture.
