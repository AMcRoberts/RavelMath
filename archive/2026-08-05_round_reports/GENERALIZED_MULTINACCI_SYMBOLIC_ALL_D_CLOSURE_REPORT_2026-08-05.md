# Generalized Multinacci Symbolic All-D Closure

## Result

The remaining dimension-enumeration dependency has been removed from the
parent-prefix and embedding argument.

For

    sigma_{D,m}(a) = 0^m(a+1), a < D-1,
    sigma_{D,m}(D-1) = 0,

with `D >= 2` and `m >= 1`, the complete parent catalogue follows directly
from the two literal image forms:

* inner letter `0` has `m` occurrences in each of the `D-1` nonterminal
  images, at cuts `0,...,m-1`, and one occurrence in the terminal image at
  cut `0`; hence `(D-1)m+1` parent occurrences;
* every inner letter `r>0` occurs exactly once, at cut `m` in image `r-1`.

The total number of parent occurrences over all inner letters is therefore

    (D-1)(m+1)+1,

and the universal ordered parent-pair macro catalogue has the square of that
cardinality.

Every cut lies in `{0,...,m}`.  A pair of cuts `(p,q)` consequently emits
exactly

    p=q      -> Q,
    p!=q     -> R^|q-p|,

with roof at most `m` and sign retained as the independent side voltage.

## Why this proves the all-D embedding

The actual `simple_forward_targets_exact` implementation obtains both loops
solely from `parent_decompositions`.  It then performs an exact linear solve
and a node-validity check.  Those operations can only delete parent pairs; they
cannot introduce a parent, prefix cut, target role, or transport label absent
from the catalogue above.

The boundary-word lift subsequently retains an exact forward witness only
when its destination belongs to the exact contact-boundary map.  This is a
second deletion-only filter and likewise performs no relabelling.

Therefore, for every `D>=2` and `m>=1`, every concrete contact-boundary
primitive graph is a deletion-only subgraph of the universal parent-role
renewal suspension.  Its base-state map forgets displacement, and each private
renewal state maps to the corresponding state of its unique parent-prefix witness.
Hence

    Q_B P <= P Q_U,
    R_B P <= P R_U

for any exactly generated boundary system, without a dimension-specific state
catalogue or finite-D induction.

Combined with the existing positive-word theorem, arbitrary-m scheduler,
admissible-subgrammar closure, signed side voltage, and norm-weighted operator
majorant, this closes the structural generalized-multinacci dominance theorem
for all dimensions and multiplicities, conditional only on the exact boundary
object to which the comparison is applied.

## Engineering changes

Added:

* `include/ravel/proof/generalized_multinacci_symbolic_embedding.hpp`
* `tests/generalized_multinacci_symbolic_embedding_test.cpp`
* `lean/generated/generalized_multinacci_symbolic_embedding.lean`

The arbitrary implementation ceiling `m <= 120` was removed from
`generalized_multinacci_rule`; the constructor now enforces only `m>=1`.

## Validation

The symbolic arithmetic identities were checked for `2<=D<=256` and
`1<=m<=256`.  The explicit schema was independently cross-checked against the
actual generic `parent_decompositions` source for `D=2,...,8` and
`m=1,...,32`.

The existing exact concrete regressions remain clean through the previously
reported ranges, including `D=4,m=6` and `D=5,m=3`.

The Lean artifact was emitted but not kernel-run because this archive contains
no Lean/Mathlib environment.
