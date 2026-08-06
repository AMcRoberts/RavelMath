# Unit Grade-Two Cancellation Classification — 2026-08-04

## Result

The rejected-state shape gate no longer derives `unique_zero` and
`cyclic_alternation` by directly inspecting those predicates on the replayed
state.  It now invokes the reusable operation

`derive_unit_grade_two_cancellation<D>`

which searches the canonical descriptor grammar for a proof-producing atom
pair and classifies that pair as one of:

- root + root;
- root + alternating triple;
- alternating triple + alternating triple.

For a candidate pair the operation constructs the sum from the two descriptor
supports and signs, verifies unit cancellation, derives the zero count, and
walks cyclically from the derived zero to prove sign alternation.  The returned
certificate includes both descriptors and the atom-pair kind.  Failure returns
an obstruction rather than allowing the caller to insert a raw shape claim.

## Integration

`derive_rejected_state_shape_invariant` now requires one cancellation
certificate for every unresolved recurrent state.  Its report records counts
of the three certificate kinds and requires their sum to equal the number of
checked states.  The old direct calls that counted zero coordinates and invoked
`describe_alternating_zero_fiber` as the source of the theorem have been
removed from this gate.

The earlier quadratic argument remains responsible for unit coordinates and
pure rotation.  The cancellation classifier is responsible for the remaining
one-zero and cyclic-alternation clauses.

## Dimension cutoff

Each atom has support at most three, hence a certified grade-two sum has support
at most six.  A one-zero alternating displacement has support `D-1`.  Therefore
this renewal branch is impossible for `D >= 8` before label schedules, corona
closure, SCC decomposition, or Perron comparison are considered.

The only possible dimensions for the currently observed grade-two renewal
mechanism are `D=5` and `D=7`.

## Validation

Passing targets:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`

The Makefile emits a non-fatal `pkg-config` warning because Lua 5.4 metadata is
not installed in this environment; the relevant C++ targets compile and pass.

## Remaining seam

The classifier currently enumerates the finite canonical descriptor pairs in
`D <= 7` to find the proof-producing pair.  The next strengthening is to derive
closed index-overlap formulae for the three pair kinds, so the certificate can
be selected from cancellation geometry without descriptor-pair search.  After
that, the universal rejected-boundary theorem can use the dimension cutoff to
remove the renewal branch entirely above seven.
