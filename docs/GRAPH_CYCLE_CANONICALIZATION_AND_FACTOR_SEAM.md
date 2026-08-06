# Graph cycle canonicalization and characteristic-factor seam

The graph campaign now derives, rather than assumes:

1. the unique recurrent SCC;
2. one internal successor and one internal predecessor at every core vertex;
3. a canonical orbit starting at the least core vertex;
4. exhaustion and single closure of that orbit;
5. a canonical cycle order;
6. a feeder-first reindexing of the full adjacency matrix;
7. exact block form with a zero lower-left block;
8. a cycle permutation matrix as the recurrent block.

Mathlib's `Matrix.charpoly_fromBlocks_zero₂₁` then closes the transient
factor generically:

    charpoly(full) = X * charpoly(cycleCore).

The structural seam is now only:

    charpoly(cycleCore_n) = X^n - 1.

That obligation is intentionally left separate. Once kernel checked, it becomes
a reusable graph-to-spectrum closure operator for every certificate that
canonicalizes a recurrent SCC as a directed cycle.

## External API references

The block characteristic-polynomial closure uses
`Matrix.charpoly_fromBlocks_zero₂₁`; the planned canonical-index transport uses
Mathlib's matrix reindexing API.  See
`refs/references.bib` entries `MathlibMatrixCharpolyBasic2026`,
`MathlibMatrixReindex2026`, and `MathlibMatrixPermutation2026`, with usage notes
in `docs/WEB_AND_API_BIBLIOGRAPHY_USAGE_2026-08-03.md`.
