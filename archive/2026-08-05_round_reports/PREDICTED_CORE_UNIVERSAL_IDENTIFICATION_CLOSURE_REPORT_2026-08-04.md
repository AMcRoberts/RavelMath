# Universal predicted-core internal SCC identification closure

## Audit correction

The prior campaign node combined two logically distinct claims:

1. the formula-defined predicted-core graph is strongly connected for every
   dimension; and
2. that graph is the exact SCC of its anchor inside the full literal graph.

The first is a graph-grammar theorem.  The second additionally requires the
arithmetic recurrent-hull bound and a no-return-after-exit theorem.  Keeping
both under one open node made completed internal work appear unfinished and
made the literal theorem appear less dependent than it really is.

## New reusable operation

`derive_symbolic_shadow_paths(n, state)` works on one formula state at a time.
It does not enumerate the core or run Tarjan.  For every new state it uses the
closed distance formulas and the exact successor/predecessor grammar to derive:

- a path from the previous-alphabet shadow to the state of length at most 3;
- a path from the state back to the shadow of length at most 2.

Every edge replays through `predicted_core_successors`.  Shadow states receive
zero-length paths.  Malformed or non-core states return an obstruction.

Together with the finite n=3 base and the kernel-checked theorem
`predictedCore_stronglyConnected_induction`, this is the evidence-producing
operation needed for the universal internal SCC theorem.

## Validation

The symbolic operation was checked on every formula-defined core state for
n=4 through n=32:

- 410,176 states;
- all bounded paths derived;
- all edges replayed;
- all lengths equal the closed formulas.

The older whole-graph BFS/Tarjan certificate remains as an independent finite
regression, not as the universal proof operation.

## Campaign result

`core.universal_identification` is now an exact-certificate obligation with the
precise statement that the formula-defined predicted-core graph is strongly
connected for every n>=3.

A separate open node, `core.literal_exact_scc`, now states the full-literal-graph
claim.  It depends on both the completed internal theorem and
`hull.recurrent_bound`.  The generic Lean theorem `core_is_exact_scc` is already
available once that no-return premise is supplied.
