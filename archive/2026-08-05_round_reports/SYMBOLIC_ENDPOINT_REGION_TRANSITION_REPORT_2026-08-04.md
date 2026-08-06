# Symbolic Endpoint Regions and Parent-Grammar Transitions

## Result

The alternating-zero renewal proof no longer enumerates the `2 D^3` labelled candidate family.

For canonical n-bonacci,

- `parent(0) = {(c, empty) : 0 <= c < D}`;
- `parent(a) = {(a-1, [0])}` for `a > 0`.

The prefix equation for an alternating-zero source requires

`|q| - |p| = -x_0`.

This gives exactly three active endpoint regions.

### Case x_0 = 0

Admissible sources are either `(i,j)=(0,0)` or `i>0, j>0`.

- `(0,0)` maps to the full parent rectangle `[0,D-1]^2`;
- `(i,j)` with both positive maps uniquely to `(i-1,j-1)`.

### Case x_0 = +1

Admissible sources satisfy `i>0, j=0` and map to

`(i-1, b)`, for every parent label `0 <= b < D`.

### Case x_0 = -1

Admissible sources satisfy `i=0, j>0` and map to

`(a, j-1)`, for every parent label `0 <= a < D`.

In every case the phase is `z -> z-1 mod D`.

## Engineering change

`derive_nbonacci_symbolic_label_schedule` now:

1. recognizes the n-bonacci parent grammar;
2. derives these endpoint regions and transition cases;
3. constructs only the `2(D+2)` announced odd-dimensional renewal states;
4. replays those states through `simple_forward_targets_exact`;
5. derives transient rank from the case inequalities.

It does not build the bounded candidate graph, perform candidate closure, run SCC decomposition, topologically eliminate candidates, or scan `2D^3` concrete `SNode` values.

The exhaustive mechanisms remain only in lower-level canonical code and tests used to verify the formula, not in the proof operation.

## Validation

Passing:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`

The real dimension-six recurrent-family exhaustion remains proved.

## Next seam

The remaining symbolic seam is to lift these endpoint-region cases from the alternating-zero family into the universal rejected-boundary partition: prove that every recurrent rejected state is either in the known transported/core families or enters one of the symbolic regions governed by this grammar.
