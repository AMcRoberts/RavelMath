# Reflective mixed-grade Collatz weight

## Target

Close the mixed-grade n=7 components that reject synchronous, bounded-delay,
and Boolean renewal simulations.

## Block/renewal branch

The reusable renewal operation treats minimum-grade vertices as a section and
compresses every excursion through the higher grade into one macro edge.
For the mixed components the higher stratum is acyclic and the maximum block
lengths are 5, 5, and 2.  Nevertheless, the greatest Boolean renewal relation
to the predicted core is empty in all three cases.  This branch is therefore
falsified cleanly.

## Positive-weight branch

`include/ravel/proof/reflective_collatz_weight.hpp` lets a nonnegative adjacency
matrix derive its own exact positive weight:

1. begin with the all-ones integer vector;
2. repeatedly apply the adjacency matrix;
3. divide out the exact common gcd;
4. derive the exact coordinate ratios `(A v)_i / v_i`;
5. replay the stored Collatz--Wielandt interval from the derived vector.

For a competitor and core, strict dominance follows when the competitor's
exact upper ratio is below the core's exact lower ratio.

## Results

The operation replays and separates every cyclic competitor for n=3..7.
For n=7 specifically:

- mixed SCC 77: rho approximately 1.185144556205, strict weight PASS;
- mirror mixed SCC 77: rho approximately 1.185144556205, strict weight PASS;
- mixed SCC 22: rho approximately 1.162004394495, strict weight PASS;
- predicted core: rho approximately 1.886782118062.

The pre-existing independent exact Perron comparison agrees in every case.

## Remaining universal task

The weights are derived, not supplied, but they are still component-sized.
The next theorem-discovery target is to decode them into a symbolic family
indexed by grade, face phase, and local support roles, or to derive a weighted
renewal relation whose transfer law generates the same subeigenvector.
