# Phase-aware relational grade simulation

The reusable grade module now derives three increasingly general transition
objects:

1. persistent active-atom compatibility;
2. greatest set-valued shell-to-core simulation;
3. recursive lower-grade and bounded-delay simulations.

The affine decomposition law was replayed on every SCC edge: the active part
carries the label defect and the remainder carries the homogeneous update.
A single active atom never forms a recurrent witness fiber.  Allowing the
active representative to hand off through the full face-labelled core does.

## Exact structural results through n=7

The greatest phase-aware simulation covers every vertex of every
non-permutation pure-grade SCC:

- n=5 grade-two SCCs: 48/48 and 18/18;
- n=6 grade-two SCCs: 236/236 and 80/80;
- n=7 grade-two SCCs: 766/766 and 250/250;
- n=7 pure grade-three SCC: 102/102.

The large n=6 component therefore no longer requires a component-specific
projection theorem.  Its complete surviving relation contains 3,176
shell/core pairs from 5,968 admissible decomposition pairs.

Full-support permutation components are separate rho=1 terminal cases.

## Remaining case

The mixed-grade n=7 SCCs (77, 77, and 22 vertices) have no surviving relation
under:

- direct shell-to-core simulation;
- recursive simulation into all strictly lower cyclic grades;
- bounded-delay matching with 0, 1, or 2 target steps.

Their obstruction is therefore not ordinary face phase or a one-edge timing
mismatch.  The next proof object must span a grade-changing source block,
permit a renewal/handoff with accumulated weight, or derive a positive
relational intertwiner rather than a Boolean simulation.
