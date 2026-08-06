# Persistent boundary atom-witness propagation — round 54

## Result

Canonical two-atom evidence is now created at corona candidate generation and
preserved through projected closure, rejected-boundary recording, `Red`, final
trace replay, SCC analysis, and the rejected-state shape theorem.

The proof layer no longer reconstructs a grade-two decomposition after SCC
extraction.  For every unresolved recurrent state it consumes the node-aligned
witness retained by `ProjectedCoronaTrace`.  Missing evidence is an explicit
obstruction.

## Engine changes

- Added `nbonacci_margin::GradeTwoAtomWitness`.
- Added `derive_grade_two_atom_witness(target)`, using canonical forced
  complement over the root/triple grammar.
- Added atom-witness maps to `ProjectedCoronaLayer`, `ProjectedCoronaTrace`, and
  `DerivedCoronaImage`.
- Candidate admission records evidence for accepted, rejected, pre-Red, pruned,
  and surviving nodes.
- `ReplayedBoundaryGraph` carries node-aligned optional witnesses and validates
  their preservation.
- `derive_rejected_state_shape_invariant` consumes persistent witnesses and
  replays their vector sum and catalogue grade; it does not search atom pairs.
- The adjacent boundary request now selects grade-two candidates from the
  canonical atom witness.  `DisplacementSumCatalogue::grade` is only a mismatch
  check.

## Failure discipline

A focused negative test deletes one survivor witness after replay.  The shape
operation rejects the component with:

```
an unresolved recurrent state lost its boundary atom witness through projection or Red
```

No fallback reconstruction is permitted.

## Validation

Passing:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`
- projected adjacent transport `4 -> 5`
- projected adjacent transport `5 -> 6`
- projected adjacent transport `6 -> 7`

At `6 -> 7`, the boundary theorem remains fully proved: three recurrent
components are transported/core and the two nine-state renewal components have
persistent atom witnesses, symbolic shape/label/phase certificates, exact
`A^7 = 2I`, and strict rational dominance.

## Honest completion boundary

This completes the finite adjacent-dimensional rejected-boundary product through
the `n=7` complexity transition.  It does not complete the repository's full
universal-`n` dominance theorem.  `universal_dominance_campaign_test` still
reports four independent open seams:

1. parametric maximum-shell rank;
2. universal predicted-core SCC identification;
3. uniform recurrent non-core grade exhaustion;
4. symbolic uniform spectral dominance.

The work in rounds 36–54 materially advances seam 3 for the grade-two rejected
boundary and proves that its exceptional renewal mechanism ends at dimension
seven.  It does not by itself prove exhaustion of every grade in the entire
arithmetic hull.
