# Residual signature transfer

The native C++ proof layer now composes the local shift-register update laws
with the continuation-residual predecessor operation.

For a controller-state signature map `sigma`, the derived symbolic successor
relation is

```
SigmaStep(q,d,q') iff some concrete s,t satisfy
  sigma(s)=q, s -d-> t, and sigma(t)=q'.
```

For a residual `R`, membership is proved constant on every signature class,
so `R` descends to a Boolean predicate `Rbar` on signatures.  The predecessor
then obeys the exact existential inverse-image equation

```
Pre_d(R)bar(q) = exists q', SigmaStep(q,d,q') and Rbar(q').
```

The C++ operation replays this equation against every concrete state, every
residual in the witness-conditioned continuation family, and all three digits.
It also replays the local boundary-flux equations used to compute the
signatures.

## Results

| dimension | controller states | residuals | signature classes | local window |
|---:|---:|---:|---:|---:|
| 3 | 26 | 728 | 26 | 1 |
| 4 | 80 | 47,838 | 80 | 2 |

For both dimensions:

- residual membership is constant on signatures;
- the signature successor relation is exact;
- symbolic predecessor membership exactly equals the concrete predecessor;
- all local update equations replay.

The current signatures separate every controller state in these dimensions.
Thus this closes the residual-transfer equation but does not yet compress the
controller state space.  The next universal task is to prove that the selected
finite-window signature grammar remains sufficient parametrically, or replace
state separation with a smaller role-relative signature family whose transfer
commutation square can be proved for arbitrary dimension.
