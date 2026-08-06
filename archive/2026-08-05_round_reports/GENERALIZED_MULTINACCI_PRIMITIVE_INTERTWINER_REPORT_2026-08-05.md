# Generalized Multinacci Primitive Q/R Intertwiner

## Result

The missing primitive comparison is obtained by retaining the renewal phase in
the core.  The unsuspended role quotient `(i,x,j) -> (i,j)` is too small for
`m>1`: exact tests reject it because distinct prefix positions have different
return times and multiplicities.

For dimension `D` and multiplicity `m`, form the universal parent-role macro
graph on ordered roles `(i,j)`.  A parent-prefix witness with cuts `(p,q)` is
labelled `Q` when `p=q` and `R^|q-p|` otherwise.  Expand every `R^d` edge into
a primitive path with `d-1` private renewal states.

Expand the concrete contact-boundary word graph in the same way.  Define `P`
by

* `(i,x,j) -> (i,j)` on original boundary states;
* each private phase state of a concrete macro edge maps to the corresponding
  private phase state of its unique universal parent-prefix witness.

Then `P` is a total one-hot state map and satisfies exactly

    Q_B P <= P Q_U,
    R_B P <= P R_U.

The inequalities follow by deletion-only admissibility: every concrete edge is
one of the universal parent-prefix witnesses, and no concrete phase path is
relabeled or merged.

## Interpretation

The generalized-multinacci core is not merely the old `D^2` parent-role graph.
It is its bounded renewal suspension.  The extra structure required by `m` is
therefore finite timing state, not a new primitive transport generator.

Combining this simultaneous primitive intertwiner with the existing positive
word theorem, signed renewal reduction, admissible-subgrammar theorem, and
norm-weighted twist majorant closes the general-m transport comparison for each
exactly generated boundary system.

## Validation

Exact executable validation passed for:

* `D=2,3,4`, `m=1,...,6`;
* `D=5`, `m=1,2,3`.

The largest tested concrete expanded boundary had 5690 states (`D=4,m=6`).
The `D=5,m=3` case had 3467 concrete expanded states and 177 universal
suspension states.

The empty `D=2,m>=2` boundary is handled as the zero competitor and is
trivially dominated.
