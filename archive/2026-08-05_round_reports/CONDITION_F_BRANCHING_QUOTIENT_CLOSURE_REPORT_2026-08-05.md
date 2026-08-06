# Condition-F branching quotient closure — 2026-08-05

## Result

The false universal maximum-shell exclusion and the equally false finite
absolute coefficient-hull requirement have been removed from the universal
dominance dependency graph.

For every n-bonacci dimension `n>=2`, a recurrent carry SCC is treated by the
following exact dichotomy.

1. If every vertex has one internal incoming and outgoing edge, it is a
   terminal permutation component and has spectral radius one.
2. Otherwise, two distinct recurrent branches can be completed to closed loops
   at a common basepoint and repeated to a common length. Their difference is
   an exact finite zero expansion over `{-2,-1,0,1,2}`. Multinacci Condition
   (F), together with Carton--Sudbery--Yassawi Theorem 3, gives a finite
   continuation quotient for those branch differences.

Absolute outer-shell motion survives only as a deterministic terminal phase
fibre. The new finite permutation-skew-product theorem proves unique lifting of
every quotient path from every initial fibre point. Therefore quotient and lift
have identical row path counts at every length and the same Perron radius.

Consequently all non-permutation spectral growth factors through a finite
Condition-F branching quotient. No finite absolute coefficient bound is
needed.

## New reusable operations

- `derive_balanced_unit_step_terminal_shell`
- `derive_synchronized_zero_expansion`
- `derive_synchronized_loop_difference`
- `derive_multinacci_zero_language_theorem`
- `derive_multinacci_branching_terminal_reduction`
- `derive_permutation_skew_product`
- `compose_universal_dominance_from_finite_quotient`
- `derive_path_injective_simulation`
- `compose_graded_path_injections`

## Exact checks

- generalized balanced terminal words, including a non-triangular plateau;
- nontrivial synchronized Fibonacci zero expansion;
- Condition-F theorem application for dimensions `2..32`;
- permutation skew-product path counts through depth 20;
- seven-phase `A^7=2I` renewal path injection into a two-state branching core;
- existing triangular terminal families (`32` parameter instances);
- rejected-boundary focused proof test;
- universal campaign dependency test.

## Setbacks retained

- Universal shell acyclicity is false (`n=9,M=2` triangular wave).
- Coordinatewise clipping is not a universal normalization theorem.
- Direct multiplicity-preserving injection into the core succeeds for all
  terminal components and several low/mixed components, but fails for the
  largest pure grade-two families from `n=6` onward. It is therefore only one
  branch of the quotient-dominance proof, not the universal mechanism.
- The existing bounded `CSYZeroAutomaton` probes remain truncated and are not
  used as proof of regularity. Regularity is a theorem application from
  multinacci Condition (F) and CSY Theorem 3; the bounded automata remain replay
  and diagnostics.

## Correct remaining theorem

The universal dominance theorem now reduces to one statement:

> Every recurrent SCC of the finite Condition-F branching quotient has Perron
> root at most that of the predicted core.

Literal uniqueness of the core SCC is unnecessary. Terminal translations of a
core quotient, if present, are permutation skew lifts with the same spectral
radius and do not invalidate `rho(G_B)=rho(core)`.

The current finite evidence decomposes this last theorem as follows:

- pure grade-two and grade-three SCCs have complete relational descent to lower
  grade through `n=7`;
- terminal components have rho one;
- the exceptional low-dimensional renewal SCCs have exact block-power
  certificates;
- mixed-grade SCCs have exact reflective Collatz weights, with the higher-grade
  stratum acyclic;
- a new path-injective simulation operation gives a compositional spectral
  proof whenever edge multiplicities inject.

The remaining promotion is to derive the mixed/pure family weight or
multiplicity-preserving descent uniformly from the finite quotient grammar.
