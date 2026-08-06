# Maximum-shell cyclic-kernel dichotomy and maximality contradiction

## Completed branch

The successful role-constrained branch is now closed end to end.

For the exact one-lap controller relation, `derive_role_constrained_kernel_dichotomy`
computes the greatest serial survivor by repeatedly deleting states with no
successor in the current set.

- If the survivor is nonempty, finiteness gives a positive controller cycle.
- `derive_role_constrained_controller_lift` reconstructs every digit transition.
- `certify_role_constrained_shell_pump` repeats the source cycle and constructs
  a closed recurrent carry cycle at strictly larger radius.
- `certify_maximum_shell_maximality_contradiction` checks that this contradicts
  selection of the source as a recurrent cycle of maximum radius.

The final logical implication is also recorded in
`lean/maximum_shell_maximality_contradiction.lean`.

## Exact negative branch

If the cyclic kernel is empty, the same operation emits the deletion round of
every controller state.  Every relation edge strictly descends this finite
elimination rank.  Thus acyclicity is now proof evidence, not an opaque failed
search.

## Honest remaining universal seam

The parametric maximum-shell obligation is not yet fully closed.  The only
remaining implication is to rule out the empty-kernel branch for every role
lap arising from a hypothetical maximum-shell recurrent source cycle, or to
transport its elimination rank back to a strict rank on the source shell
first-return relation.

Everything after nonempty cyclic kernel is complete and replayable, including
the contradiction with maximality.
