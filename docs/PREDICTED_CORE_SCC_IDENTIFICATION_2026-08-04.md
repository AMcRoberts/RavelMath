# Predicted-core SCC identification

## Recovered implementation

The earlier reporting obstruction has already been substantially removed by
`nbonacci_margin_invariant.hpp` and `nbonacci_margin_core_graph_test.cpp`.
The efficient direct-core path is:

1. enumerate formula-defined `CoreState` values;
2. generate edges with `predicted_core_successors`;
3. invert them with `predicted_core_predecessors`;
4. identify the previous-alphabet shadow;
5. run one forward and one reverse multi-source BFS from that shadow;
6. compare the distances with the closed formulas
   `predicted_distance_from_previous_shadow` and
   `predicted_distance_to_previous_shadow`;
7. run Tarjan once as an independent direct check.

The existing regression checks dimensions 3 through 32.  Its current run takes
about 11 seconds and 46 MB and reports zero failures.  The new proof certificate
does not replace or slow that regression.

## New witness-emitting certificate

`proof/predicted_core_scc_identification.hpp` retains parent pointers in the
same two multi-source BFS passes.  It can therefore reconstruct:

- a path from the previous-alphabet shadow to every new core state;
- a path from every new core state back to the shadow.

The paths replay against the independently generated edge table.  Their lengths
must equal the existing closed formulas:

- outward distance: 1, 2, or 3;
- inward distance: 1 or 2.

The dedicated test emits/replays paths through dimension 12, where it checks
4,416 directed witness paths in about 0.3 seconds and 5 MB.  The large n=32
regression remains distance-only for efficiency.

## Universal proof shape

Base case: the n=3 formula-defined core is strongly connected.

Induction step: assume the previous core is strongly connected.  Its embedded
previous-alphabet shadow is therefore strongly connected.  Every new state is
reachable from that shadow and returns to it by the emitted bounded paths.
The generic `stronglyConnected_of_shadow` theorem then proves the entire next
core strongly connected.

This closes the *internal* SCC theorem for the formula-defined graph once the
shadow embedding is formalized in Lean.

## Remaining literal-graph identification seam

Strong connectivity and transition closure show that the formula-defined core
lies inside one SCC of the literal corona graph.  To identify that SCC exactly,
one additional exhaustion statement is required:

> any literal graph vertex mutually reachable with a fixed predicted-core
> anchor satisfies `predicted_core_member`.

The generated Lean theorem `core_is_exact_scc` isolates precisely this final
occurrence/exhaustion premise.  It prevents the earlier error of choosing an SCC
by node count or spectral estimate rather than proving formula equality.
