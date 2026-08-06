# Universal Rejected-Boundary Partition — Round 47

## Result

A new replay-backed symbolic partition gate classifies every recurrent component of the rejected-boundary corona graph before any spectral dominance operation is allowed.

For canonical n-bonacci, each recurrent replay component must be exactly one of:

1. **transported/core** — every concrete state satisfies the independently supplied requested-family or predicted-core predicate;
2. **terminal permutation** — every concrete state has internal weighted indegree and outdegree one;
3. **alternating-zero renewal** — every state has the alternating-zero displacement descriptor and the exact labelled state set equals one of the closed-form schedules derived from the parent grammar.

Anything else leaves the partition certificate unsupported.

## Implementation

`derive_symbolic_rejected_boundary_partition` consumes:

- the canonical substitution;
- `ReplayedBoundaryGraph`, whose edges retain and validate corona prefix witnesses;
- the structural transported/core predicate.

Tarjan SCC is used only to identify which replayed pieces are recurrent. It is not used to discover the alternating-zero schedule. That schedule is generated beforehand from the closed endpoint-label formulas.

`derive_replayed_boundary_symbolic_partition` binds the partition result into each recurrent witness. The real adjacent-dimension campaign now runs this gate before dominance.

## Validation

Focused tests include:

- an exact closed-form renewal schedule, accepted as alternating-zero;
- the same recurrent graph with a malformed displacement, rejected as unclassified.

Projected campaign results:

- `4 -> 5`: PASS;
- `5 -> 6`: PASS;
- `6 -> 7`: PASS.

At `6 -> 7`, the five recurrent rejected-boundary components partition as:

- transported/core: 3;
- terminal permutation: 0;
- alternating-zero renewal: 2;
- unclassified: 0.

The two renewal components are then independently discharged by the existing prefix phase-twist, state-level block-power, and exact dominance certificates.

## Remaining universal seam

This is a symbolic partition of every **recurrent component present in the replayed rejected-boundary graph**, not yet a proof that every possible recurrent rejected state in arbitrary dimension must enter one of the three syntactic classes.

The remaining theorem obligation is therefore upstream:

> Derive from the general rejected-boundary state equations that any recurrent state not satisfying the transported/core predicate and not having permutation dynamics must have grade two with one zero and cyclically alternating `+1/-1` remainder.

That requires a displacement-shape invariant or defect rank on general rejected states. Once that invariant is derived, the endpoint-parent grammar completed in rounds 42–46 supplies the rest of the alternating-zero branch uniformly.
