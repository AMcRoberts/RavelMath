# Role-constrained controller lift and shell-pump bridge

## Closed engineering/theorem operations

The stepped-face role cycle is now lifted to the literal radius-one controller
state space by `derive_role_constrained_controller_lift`.

For a closed source lap word `w` and a positive cyclic role sequence
`F_0,...,F_{r-1}`, the operation constructs the exact relation

```
q R q'  iff  q in F_0 and q --w--> F_1 --w--> ... --w--> F_0 contains q'.
```

It then:

1. computes the role-restricted finite relation;
2. prunes nothing implicitly and searches the literal relation for a positive cycle;
3. reconstructs every digit-level controller transition;
4. checks every intermediate stepped-face junction;
5. emits a closed `CyclicRunCertificate`.

`certify_role_constrained_shell_pump` repeats the source carry cycle the exact
number of laps required by that controller orbit, verifies that the selected
role is an active source-shell face, and feeds both witnesses into the existing
`StrictShellPump` operation.

The whole path is replayable and fails explicitly if the restricted relation is
acyclic or loses a role constraint.

## Validation

Passing:

- `stepped_face_residual_seriality_test` — 2,920 first-return witnesses;
- `stepped_face_role_junction_test` — 2,920 witnesses;
- `role_constrained_controller_lift_test`;
- `role_constrained_shell_pump_test`;
- `cyclic_splice_completion_test`;
- `periodic_word_controller_cycle_test` — all ternary words through the enrolled bounds;
- `strict_shell_pump_test`;
- `universal_dominance_campaign_test`.

## Honest remaining shell theorem

Role-cycle existence alone does not prove that the restricted controller
relation has a cyclic kernel. The new operation makes that exact missing fact
observable:

> Every role cycle arising from a hypothetical maximum-shell source lap has a
> nonempty cyclic kernel in the role-constrained controller relation.

Once this statement is derived from the stepped-hyperplane linear grammar, the
strict shell pump is automatic and the maximum-shell obligation closes.

## Next primary obligation started

The recurrent-hull-bound audit is now started at the correct dependency
boundary. It should consume the shell exclusion theorem plus the existing weak
conjugate-height bound to produce a finite coefficient region. No hull claim is
promoted until the cyclic-kernel statement above is closed.
