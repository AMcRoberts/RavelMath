# Boundary Request Atom Derivation — Round 53

## Result

The unresolved rejected-state shape proof no longer assumes
`DisplacementSumCatalogue::grade(x) == 2`.

For each state it first derives an explicit canonical two-atom witness using the
forced-complement operation:

1. choose one canonical root/triple atom `a`;
2. compute the forced complement `b = x - a`;
3. recognize `b` as a canonical root or alternating triple;
4. emit the pair descriptors, overlap, cancellation sites, zero count, and
   cyclic-alternation evidence.

Only after this evidence exists does the displacement catalogue replay-check
that the state has grade two. Catalogue grade is now validation, not a premise.

## Four linked changes

1. **Request-derived atom evidence**
   `request_atom_decomposition_derived` records that every unresolved recurrent
   state has an explicit two-atom witness from the canonical atom grammar.

2. **Catalogue demoted to replay**
   `catalogue_grade_replayed` records agreement between the explicit witness and
   `DisplacementSumCatalogue`. A disagreement is an engine obstruction.

3. **Partition consumes the shape certificate**
   The universal rejected-boundary partition no longer independently calls the
   raw alternating-zero recognizer. It accepts the renewal branch only when the
   recurrent component ID occurs in the typed shape certificate and its exact
   labeled state set matches the closed schedule.

4. **High-dimensional renewal bypass**
   For `D >= 8`, the partition does not construct symbolic renewal schedules.
   The support theorem must eliminate the renewal branch before label or
   spectral machinery is considered. Only support-permitted dimensions use the
   finite schedule proof.

## Validation

Passing targets:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`

## Next seam

The remaining upstream premise is component selection: the shape operation is
still invoked only after SCC decomposition identifies an unresolved recurrent
component. The next useful theorem is a request-level forward invariant that
attaches the explicit atom-pair witness to every admitted/rejected boundary
state as it is generated, allowing recurrence analysis to consume persistent
state evidence rather than derive decomposition after SCC discovery.
