# Literature audit: Class-II balanced pairs and contact boundaries

Status: targeted primary-source review, 2026-07-27. This is a citation
aid, not an exhaustive novelty certification.

The literal project family is

```text
sigma(0)=0^a 1 2,  sigma(1)=0^a 2,  sigma(2)=0,
```

with incidence polynomial `x^3-a x^2-(a+1)x-1`. A shared incidence
polynomial does not imply a shared language, balanced-pair automaton,
Rauzy fractal, or contact graph.

## Closest sources

### Balanced pairs

- V. F. Sirvent and B. Solomyak, *Pure discrete spectrum for
  one-dimensional substitution systems of Pisot type*, Canadian
  Mathematical Bulletin 45 (2002), 697--710,
  <https://doi.org/10.4153/CMB-2002-062-3>. Theorem 5.1 relates the
  balanced-pair and overlap algorithms; Corollary 5.3 gives the
  pure-discrete criterion for the tiling flow.
- J. Bernat, *Study of irreducible balanced pairs for substitutive
  languages*, RAIRO 41 (2007), 663--678,
  <https://doi.org/10.1051/ita:2007062>. This treats the incidence
  matrix of a terminating balanced-pair algorithm and records the
  criterion comparing its Perron root with the substitution root.
- S. Akiyama, M. Barge, V. Berthé, J.-Y. Lee, and A. Siegel,
  *On the Pisot substitution conjecture*, in *Mathematics of
  Aperiodic Order* (2015), 33--72,
  <https://www.irif.fr/~berthe/Articles/AkiyamaBargeBertheLeeSiegel.pdf>.
  This is the best broad map of balanced, strong, geometric, and
  overlap coincidence conditions.

These works establish the method and its dynamical meaning. The
review did not find the project's complete states
`A,A*,B,B*,C,C*,D,D*`, their literal transition identities, or the
two quotient cubics. The safe claim is “derived here,” not “first
discovered here.”

### Contact and boundary graphs

The closest family calculation is:

- J. M. Thuswaldner, *Unimodular Pisot substitutions and their
  associated tiles*, Journal de Théorie des Nombres de Bordeaux 18
  (2006), 487--536, <https://doi.org/10.5802/jtnb.556>.

Thuswaldner treats

```text
tau(1)=1^b 2,  tau(2)=1^c 3,  tau(3)=1,  b>=c>=1.
```

Its contact graph has 17--19 states. The factor carrying its Perron
root is

`x^4+(1-c)x^3+(c-b)x^2-(b+1)x-1`.

This is a close precedent for an explicit parametric contact graph,
but not the present family. Matching incidence polynomials would
require `(b,c)=(a,a+1)`, outside `b>=c`; the incidence matrices and
words also differ. Its quartic is not the project's quintic
`x^5-a^2x^3-a(a+1)`.

The graph construction used by this project is:

- B. Loridant, J. M. Thuswaldner, and S.-Q. Zhang, *Neighbors of
  self-affine tiles and Rauzy Fractals*, arXiv:2511.16442, version
  dated 2026-03-22, <https://arxiv.org/abs/2511.16442>.

That paper distinguishes the smaller contact graph from the
self-replicating boundary graph produced by corona and reduction.
Therefore closure of the displayed eleven-state component does not
prove universal dominance in the full boundary graph.

**Errata found in this paper (archaeology note, 2026-07-31/08-01):** an
earlier, independent reimplementation branch of this project
(predecessor tree `PisotProjectOrig`, before the `RavelMathPub`
namespace existed) found and documented two genuine transcription
errors in the paper's own published worked-example
`D_cont` tables (`sigma_1`/`sigma_2`) -- found by computing the sets
from the geometric definition rather than trusting the tables, and
confirmed because each replacement restores an otherwise-missing
structural symmetry -- plus a real notational ambiguity (the source
overloads `-` for two different operations: triple-level swap-and-
negate for node validity versus pair-level negate-only for the
connection relation and hence the C-corona) that caused a genuine
implementation bug (corona iteration diverging to 139 nodes for
`sigma_1` instead of converging to 26) before being caught and fixed.
Full details, restated independently in this project's own words (not
a copy of the source), are in `refs/loridant_thuswaldner_zhang_2026_
corrected_reference.tex`. This project's own `search_D_cont` computes
`D_cont` from the geometric definition directly, never from the
published tables, so it was never exposed to the transcription errors;
`same_letter_H` in `include/ravel/corona.hpp` already implements the
corrected pair-level negation the source's C-corona definition needs
(see that file's own header comment, "the reference's 'Tale of Pair-
vs Triple-level Negation'") -- independently re-derived rather than
inherited, but landing on the identical fix the predecessor tree found.
Round 1's literature check earlier this session (see
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`) relied on this same
paper's `±K:=K∪(-K)` notation and Algorithm 2, unaffected by either
erratum (neither touches those specific passages).

For graph-directed boundaries and dimension, also cite:

- A. Siegel and J. M. Thuswaldner, *Topological properties of Rauzy
  fractals*, Mém. SMF 118 (2009),
  <https://www.numdam.org/item/MSMF_2009_2_118__1_0/>.
- R. D. Mauldin and S. C. Williams, *Hausdorff dimension in graph
  directed constructions*, Trans. AMS 309 (1988), 811--829,
  <https://doi.org/10.2307/2000925>.

A boundary-dimension conclusion needs the graph-directed geometric
and separation hypotheses; a matrix spectral radius alone is not a
Hausdorff-dimension theorem.

### Pisot geometry

- P. Arnoux and S. Ito, *Pisot substitutions and Rauzy fractals*,
  Bull. Belgian Math. Soc. 8 (2001), 181--207,
  <https://doi.org/10.36045/bbms/1102714169>.
- M. Minervino and J. M. Thuswaldner, *The geometry of non-unit Pisot
  substitutions*, Ann. Inst. Fourier 64 (2014), 1373--1417,
  <https://doi.org/10.5802/aif.2884>. This is essential when finite
  places make Euclidean internal space incomplete.
- M. Barge, *The Pisot conjecture for beta-substitutions*, ETDS 38
  (2018), 1--26, <https://doi.org/10.1017/etds.2016.29>. Do not apply
  this merely from Pisot incidence: this project family has not been
  identified here as the canonical beta-substitution.

## Conclusions

1. The balanced-pair framework is classical; the explicit eight-state
   family and its two cubics are project-derived and were not found in
   this targeted review.
2. The eleven-state contact matrix and quintic were not found, but
   Thuswaldner (2006) is a strong methodological precedent and must be
   cited.
3. No source reviewed equates the noncoincidence balanced-pair
   spectral radius with the Rauzy contact or boundary spectral radius.
   Treat that equality as this project's research question.
4. The endpoint-phase projection is structurally a finite graph
   morphism/fibre construction. The “only nilpotent modes on five
   inputs” observation remains a local exact certificate.
5. Use “not found in the reviewed literature,” not “novel,” until a
   MathSciNet/zbMATH and forward-citation search is completed.

## Search boundary

The review used exact-polynomial and literal-substitution searches,
balanced-pair spectral-radius searches, and contact/boundary/Pisot
queries, followed by the primary sources above. It was not an
exhaustive bibliographic-database search.

