# Web and API bibliography usage — 2026-08-03

This record lists external web sources consulted during the universal-n and
subsequent graph-certificate conversion work.  Canonical BibTeX entries are in
`refs/references.bib`.

## Mathlib matrix characteristic-polynomial API

**Citation key:** `MathlibMatrixCharpolyBasic2026`

Consulted for `Matrix.charpoly_fromBlocks_zero₂₁`.  This theorem is the
published closure step converting an exact feeder/core block certificate into

```text
charpoly(full adjacency) = X * charpoly(recurrent core).
```

The graph engine is responsible for deriving the exact block decomposition;
Mathlib supplies the kernel theorem for the characteristic-polynomial factor.

## Mathlib matrix reindexing API

**Citation key:** `MathlibMatrixReindex2026`

Consulted for the reusable conversion seam

```text
canonical graph ordering
→ index equivalence
→ matrix reindexing
→ determinant/characteristic-polynomial transport.
```

The API includes `Matrix.reindex`, ring/algebra equivalences for reindexing,
and determinant preservation.  Characteristic-polynomial transport may be
closed either through the corresponding linear-map conjugacy theorem or by
unfolding the characteristic matrix and using determinant reindexing.

## Mathlib permutation-matrix API

**Citation key:** `MathlibMatrixPermutation2026`

Consulted when choosing a canonical representation for a directed cycle.  The
API provides `Equiv.Perm.permMatrix` and basic determinant/trace facts.  The
project does not treat these facts alone as a proof of the cycle
characteristic polynomial; the certificate converter must still produce the
exact equivalence between the recovered graph core and the chosen cyclic
permutation.

## Mathlib finite geometric sums

**Citation key:** `MathlibGeomSum2026`

Consulted for `geom_sum_succ` while extracting recurrence closure from the
universal-n prover.  The project already contains a stronger purpose-specific
polynomial identity,
`Ravel.Polynomial.one_add_X_mul_sum_range_pow`, which is preferred by the
capability machine when its hypotheses match.

## Classical generalized Fibonacci matrices

**Citation key:** `Miles1960GeneralizedFibonacci`

Consulted during novelty assessment.  Miles's 1960 paper establishes the
classical status of generalized Fibonacci numbers and associated matrices.
Accordingly, project documentation must not claim novelty for the bare
universal-n characteristic-polynomial identity.  The research claim under
development concerns the structural proof, kernel-checked formalization, and
reflected proof-generation machinery.

## Documentation rule

When a future web search materially influences a theorem, implementation
choice, or novelty claim:

1. add a canonical entry to `refs/references.bib`;
2. record the exact declaration, identity, or claim used;
3. identify whether the source supports mathematics, implementation, or
   historical/novelty context;
4. record access date for mutable API documentation;
5. do not replace project-local stronger theorems merely because a generic
   external theorem exists.
