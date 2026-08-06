# Condition-F canonical two-matrix derivation

## Result

The binary channel split is forced by the canonical n-bonacci parent-prefix grammar.
Every parent prefix is either `[]` or `[0]`.  For a transport witness `(p,q)`, set

```
delta = |q| - |p| in {-1,0,+1}.
```

The two matrix generators are

```
Q : delta = 0       (balanced / bulk transport)
R : |delta| = 1     (one-sided / residual transport).
```

The sign of delta is retained as fibre voltage/holonomy.  It is not a third
matrix.  Thus the Condition-F problem is the earlier qMatrix/rMatrix bulk-
residual split with a signed graph-lift twist.

## Why this split is canonical

Alternative local splits were rejected:

- carry digit sign is not invariant under synchronized branch subtraction;
- grade is assigned only after holonomy assembles SCCs, so it is not edge-local;
- permutation parity is not defined for general local fibres;
- branch/transport naming is semantic rather than derived.

Prefix defect is dimension-independent, edge-local, preserved under quotient
matching, and already appears in the exact displacement identity

```
M x' = x + delta e_0.
```

## Upstream preservation

`derive_condition_f_prefix_twisted_quotient` groups concrete sheet edges by

```
(source role, target role, |delta|, delta)
```

before perfect-matching decomposition.  Consequently matching extraction
cannot mix Q and R channels or the two signed residual voltages.

## Two-sheet Fourier form

When the fibre is the orientation double cover, balanced channels preserve the
sheet and residual channels flip it.  The lift adjacency is

```
[ Q R ]
[ R Q ]
```

and the even/odd sheet transform gives `Q+R` and `Q-R`.  Since
`|Q-R| <= Q+R` entrywise, the Perron sector is the untwisted `Q+R` sector.
This is the standard signed 2-lift / voltage-graph decomposition.

For larger fibres the same statement is representation-theoretic: the trivial
fibre representation gives the channel-count pair; nontrivial representations
carry the holonomy twists.  Word-by-word unique lifting is preserved before
summing the pair.

## New operations

- `derive_condition_f_prefix_two_matrix`
- `derive_condition_f_prefix_twisted_quotient`
- `derive_condition_f_two_sheet_fourier`

## Validation

Passing focused tests:

- `condition_f_prefix_two_matrix_test` (8191 colored words)
- `condition_f_prefix_twisted_quotient_test` (typed matching reconstruction)
- `condition_f_two_sheet_fourier_test`

Passing regressions:

- `condition_f_component_maximality_test`
- `finite_quotient_core_maximality_test`
- `branching_terminal_reduction_test`
- `condition_f_twisted_component_quotient_test`

## Next theorem target

The general-n recurrence should now be derived for the canonical pair `(Q_n,R_n)`.
The signed defect is carried as a voltage cocycle and should not appear in the
bulk recurrence except through the residual transport/reindexing step.  This is
the direct analogue of the earlier universal qMatrix/rMatrix proof: derive the
Q recurrence, identify the R residual block after the appropriate twisted
index/fibre transport, normalize, and compose.
