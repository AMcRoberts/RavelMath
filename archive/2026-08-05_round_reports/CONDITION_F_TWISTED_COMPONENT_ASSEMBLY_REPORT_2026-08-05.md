# Condition-F twisted component assembly — 2026-08-05

## Result

The finite Condition-F quotient must be treated as a directed multigraph with
permutation-sheet transport.  Distinct concrete component channels between the
same pair of quotient roles are parallel quotient edges, not ambiguous copies
of one edge.

The new operation

```cpp
derive_condition_f_twisted_component_quotient
```

constructs the exact channelized quotient from a complete role-by-sheet table.
For each ordered role pair, its concrete sheet relation is checked to be a
regular bipartite multigraph and decomposed into perfect matchings.  Every
matching becomes one parallel quotient edge carrying a sheet permutation.
The resulting permutation skew product reconstructs the concrete edge multiset
exactly and preserves every row path count, hence Perron radius.

The quotient proof also computes SCCs before and after lifting.  Nontrivial
holonomy can glue several untwisted sheet SCCs into one concrete SCC.  This is
precisely the mechanism by which components that are separately pure-grade can
assemble into one mixed-grade recurrent component.

The second operation

```cpp
derive_twisted_grade_component_partition
```

therefore classifies terminal/pure/mixed components only after holonomy gluing
has been resolved.  A holonomy-glued mixed component is routed to the existing
renewal/reflective-weight branch; it is never incorrectly sent through
one-stage pure-grade descent.

## Exact regression

A two-role, two-sheet example has identity and swap channels in both
directions.  A flat Z/2 quotient would report conflicting transport on the
same role edge.  The channelized quotient instead derives four parallel
permutation edges, reconstructs all eight concrete edges, and proves equal path
growth.  The swap holonomy glues the two sheet SCCs into one branching SCC.
When the sheets carry grades one and two, the assembled SCC is correctly
classified as mixed-grade.

Removing one concrete lift makes the sheet relation irregular and the proof
rejects it rather than silently losing a component or branch multiplicity.

## Consequence for universal dominance

The remaining finite quotient theorem now has the correct exhaustive routing:

1. terminal permutation SCCs: `rho = 1`;
2. holonomy-resolved pure-grade SCCs: relational grade descent;
3. holonomy-resolved mixed-grade SCCs: renewal/reflective-weight proof.

Twists do not introduce a fourth spectral case.  They determine which concrete
components are assembled before one of the existing three cases is selected.
