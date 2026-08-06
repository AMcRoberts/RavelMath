# Condition-F component maximality — round 69

## Completed operation

`derive_condition_f_component_maximality` now consumes the assembled concrete
lift of a Condition-F quotient and derives all recurrent SCC data itself.
It accepts either:

- the channelized global role-by-sheet quotient; or
- the more general locally-fibered quotient with twisted subelements.

No caller-supplied SCC matrices, grade classifications, or dominance Booleans
are accepted.

The operation performs, in order:

1. exact quotient/lift replay and unique-path-growth validation;
2. SCC decomposition on the assembled lift;
3. grade collection only after holonomy has glued the concrete sheets;
4. exact terminal-permutation recognition;
5. pure-grade multiplicity-preserving path injection, followed by renewal;
6. mixed-grade renewal/reflective transfer;
7. exact paired-matrix fallback if the structural branches do not close.

The catalogue is proved only if every recurrent non-core SCC is discharged.
An unsupported SCC remains a typed obstruction.

## Exact arithmetic quotient replay

The coefficient-one arithmetic quotient was replayed through the first
complexity transition:

| n | non-core SCCs | terminal | pure/path injection | renewal | paired | result |
|---:|---:|---:|---:|---:|---:|:---:|
| 3 | 0 | 0 | 0 | 0 | 0 | PASS |
| 4 | 2 | 2 | 0 | 0 | 0 | PASS |
| 5 | 2 | 0 | 2 | 0 | 0 | PASS |
| 6 | 6 | 4 | 0 | 2 | 0 | PASS |
| 7 | 6 | 0 | 3 | 3 | 0 | PASS |

No component through `n=7` uses the generic paired fallback.  The `n=7`
mixed-grade components are closed by the renewal branch after component
assembly; the pure components close by multiplicity-preserving descent.

## Twist correction

Grades are never assigned to quotient roles before component assembly.
Parallel quotient channels retain their distinct sheet permutations, and
holonomy determines the actual lifted SCCs.  Identity-plus-swap regression
cases verify that two separately pure sheets can glue into one mixed-grade SCC.
The glued component is routed to renewal machinery rather than incorrectly
sent through pure-grade descent.

The locally-fibered overload does not assume a global product `base × fibre`.
It consumes each quotient class's own representative set after the covering
certificate proves unique path lifting and spectral-radius preservation.

## Universal composition

`compose_universal_dominance_from_finite_quotient` now accepts the assembled
component-maximality proof directly.  Combined with:

- core occurrence;
- core spectral radius strictly above one; and
- the Condition-F terminal/fiber reduction,

it produces the final spectral sandwich for any quotient catalogue closed by
the operation.

## Remaining universal promotion

The finite proof operation and the complete exact catalogue through `n=7` are
closed.  The remaining dimension-independent theorem is the grammar-level
exhaustion statement that the Condition-F quotient generated at arbitrary `n`
always makes one of the same structural branches succeed.  It is no longer a
quotient/twist engineering gap or a missing spectral-composition operation.
