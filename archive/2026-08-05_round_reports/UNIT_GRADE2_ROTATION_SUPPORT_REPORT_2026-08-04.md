# Unit Grade-Two Rotation Support Report — 2026-08-04

## Result

The quadratic defect from round 49 proves that every unresolved recurrent
n-bonacci boundary edge is a pure cyclic rotation and that every coordinate is
in `{-1,0,1}`.  This round adds an independent atom-decomposition certificate.

A grade-two displacement is replayed explicitly as the sum of two canonical
margin atoms.  A root atom has support two and an alternating-triple atom has
support three.  Therefore every grade-two displacement satisfies

\[
|\operatorname{supp}(x)|\le 3+3=6.
\]

If an orbit has exactly one zero and all remaining coordinates are nonzero,
then its support is `D-1`.  Consequently

\[
D-1\le6,\qquad D\le7.
\]

Thus the alternating-zero renewal mechanism seen at dimension seven cannot
continue to dimensions nine, eleven, and beyond.  Dimension seven is the
terminal dimension at which this grade-two mechanism can occur.

## Engineering changes

`RejectedStateShapeInvariantProof` now records:

- `grade_two_atom_decomposition_derived`;
- `support_bound_derived`;
- `renewal_dimension_bound_derived`;
- `maximum_grade_two_support`.

Each unresolved recurrent state receives an explicit two-atom replay witness.
The shape proof rejects any alleged grade-two state with support above six.

The closed-form symbolic schedule generator was corrected.  It previously
announced schedules for every odd `D >= 5`; that was a label-grammar formula
without the grade-two support condition.  It now announces renewal schedules
only for `D=5` and `D=7`.  For `D>=8`, the support theorem excludes the schedule
before any corona or SCC work.

## Validation

- `rejected_boundary_exhaustion_test`: PASS
- `recurrent_family_exhaustion_real_test`: PASS
- The direct symbolic schedule checks now treat `D=8,9` as fully transient/no
  grade-two renewal schedule.

## Remaining seam

The support theorem reduces the unresolved unit-grade-two orbit classification
to dimensions at most seven.  The next operation should classify the finite
root/root, root/triple, and triple/triple cancellation types under cyclic
rotation and the parent-label cases.  Its output should derive the remaining
`unique_zero_derived` and `cyclic_alternation_derived` fields rather than merely
replaying those coordinate properties on the already-discovered SCC.
