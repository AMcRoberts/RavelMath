# Forced-Complement Overlap Classification

## Result

The grade-two cancellation operation no longer searches unordered pairs of
canonical root/triple descriptors.  For each possible first atom `a`, it forms
the forced residual

\[
b=x-a.
\]

The existing canonical sparse descriptor recognizer proves whether `b` is a
root or alternating triple.  Thus the derivation is a single pass through the
atom grammar rather than a pair search.

The certificate records:

- both canonical descriptors;
- pair kind: root/root, root/triple, or triple/triple;
- support-overlap size;
- number of opposite-sign cancellation sites;
- unique-zero and cyclic-alternation conclusions.

## Closed overlap cases

For a one-zero alternating grade-two displacement:

- In dimension 7, support is six.  Both atoms must therefore be triples, and
  their supports are disjoint.  Every real renewal state is certified by this
  triple/triple case.
- In dimension 5, the four nonzero coordinates admit a root/root cover.  Some
  vectors also possess alternative overlapping triple/triple decompositions,
  but the forced-complement operation returns a canonical root/root witness.
- In dimensions at least 8, support `D-1` exceeds the grade-two maximum of six,
  so the renewal branch is impossible before schedule or spectral analysis.

## Engineering changes

- Replaced nested descriptor-pair enumeration in
  `derive_unit_grade_two_cancellation`.
- Added `forced_complement_derived`, `overlap_size`, and
  `cancellation_sites` evidence.
- Added shape-proof fields for forced-complement and closed-overlap derivation.
- Added regression checks for D=5, D=7, and the D=9 support obstruction.
- Removed a duplicated `internal_witnesses` declaration discovered in the
  shape proof.

## Validation

Passing targets:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`

## Next seam

The shape theorem still receives grade two from `DisplacementSumCatalogue`.
The next reusable derivation should produce the two-atom decomposition directly
from the boundary request and transported-family grammar, making the catalogue
lookup a replay check rather than a premise.
