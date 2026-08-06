# Maximum-shell exclusion disjunction — 2026-08-04

## Result

The finite maximum-shell obligation is now complete for either outcome of the
role-lap residual tower.

For a hypothetical recurrent maximum-shell lap, the exact controller tower has
one of two outcomes.

1. **Nonempty periodic residual.**  The existing role-constrained lift produces
   a closed controller orbit, `StrictShellPump` constructs a larger recurrent
   shell cycle, and the maximality certificate derives contradiction.
2. **Empty residual.**  The controller route is unavailable, but the actual
   maximum-shell first-return relation may still be discharged by an exact
   strict topological rank.  A strict rank excludes a closed recurrent lap
   directly.

`certify_maximum_shell_exclusion` implements this logical disjunction.  It does
not treat controller extinction as failure and does not accept a cyclic shell
return graph as a rank certificate.

## Exact direct-rank stability already present

The project’s exact first-return generator and rank replay give:

- n=2, M=2..6: acyclic;
- n=3, M=2..4: acyclic;
- n=4, M=2..3: acyclic.

The broader repository corpus also contains the previously recorded n=3..5
and n=4-through-M=12 checks.  These are bounded evidence, not the universal
formula.

## Honest universal boundary

The universal obligation is not yet closed by this round.  It has been reduced
to one exact alternative theorem:

> For every n>=3 and M>=2, either the role-constrained tower has a nonempty
> cyclic kernel, or the maximum-shell first-return graph admits a symbolic
> strict rank.

All finite graph construction, rank replay, pump construction, and maximality
contradiction after that theorem are complete engine operations.
