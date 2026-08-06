# Recurrent-family exhaustion theorem boundary — 2026-08-04

## Reusable proof operation

`include/ravel/proof/recurrent_family_exhaustion.hpp` now separates two claims
that earlier reports blurred together.

1. `derive_recurrent_family_exhaustion` proves a finite graph is exhausted by
   uniquely replayed structural family predicates.  Each recurrent SCC must
   carry closure or no-return evidence and a corresponding dominance theorem.
2. `derive_inductive_family_exhaustion` states the exact uniform induction
   boundary: transported successors must be total, newly born terminal families
   must be classified, rejected corona boundary states must be unable to return,
   and the upper family predicates must remain disjoint.

Neither operation accepts a flat list of names as proof.  It returns the first
missing structural premise.

## Independent n=6 truth-graph result

The canonical materialized n=6 truth graph was used only as an independent
oracle.  Tarjan extracted seven recurrent SCCs containing 531 vertices.  Every
component received exactly one family witness:

- predicted core: 210 vertices;
- transported grade-two competitor: 221 vertices;
- transported grade-two competitor: 80 vertices;
- terminal permutation families: sizes 6, 6, 6, and 2.

Every terminal component is one-in/one-out and therefore has spectral radius 1.
The two nonterminal competitors already carry adjacent twisted-renewal
certificates.  The finite exhaustion certificate passes.

## Inductive status

The 6->7 adjacent transport is total for the two nonterminal competitor
families.  The remaining all-n premise is now sharply isolated:

> Prove that every state rejected by the union of transported-family and
> formula-core corona requests cannot return to any recurrent upper component,
> except for uniformly generated terminal permutation families.

That is the no-return boundary theorem needed to turn the current conditional
induction into an unconditional family-exhaustion induction.
