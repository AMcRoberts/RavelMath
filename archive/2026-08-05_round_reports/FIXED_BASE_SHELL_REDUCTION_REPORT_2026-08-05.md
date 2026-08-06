# Fixed-Base Maximum-Shell Reduction — 2026-08-05

## Purpose

Replace the radius-by-radius shell-rank search with the same proof pattern that
closed the universal determinant and predicted-core connectivity theorems:
reduce every parameter instance to one canonical base family, then transport a
checked certificate through the reduction.

## New canonical graph operation

`derive_shell_first_return_graph(n,M)` is now the single exact constructor for
the maximum-shell first-return relation.  It enumerates the shell, explores
bounded carry paths through the interior, records only first returns to the
shell, and emits replay metadata.  Simulation and rank derivation therefore no
longer depend on separately implemented graph probes.

## Fixed-base theorem operation

`derive_fixed_base_shell_reduction(n,Mmax)` performs the exact finite theorem:

1. derive the canonical shell-2 first-return graph;
2. derive its strict topological rank;
3. for each announced shell `M>=3`, derive the greatest shell-covering forward
   simulation directly into shell 2;
4. transport the shell-2 rank by

   `h_M(x)=max { h_2(r) : R_M(x,r) }`;
5. replay strict increase on every upper first-return edge.

It deliberately does not chain `M -> M-1`, because exact audits showed that an
intermediate one-step simulation can fail even when direct reduction to shell
2 succeeds.

## Exact validation

The complete reduction passes for:

- `n=2`, shells `M=2..8`;
- `n=3`, shells `M=2..8`;
- `n=4`, shells `M=2..4`.

The test constructs every shell state and every first-return edge in those
ranges, computes the greatest relation, transports the rank, and replays every
strict inequality.

## Universal composition boundary

`certify_parametric_maximum_shell` is the final composition operation.  It
accepts only typed, replay-proven universal evidence for:

1. strict rank of shell 2 in every dimension;
2. total fixed-base simulation `M -> 2` for every `M>=3`;
3. common transition-semantics provenance.

Once those two derivation operations are supplied, rank transport gives a
strict rank on every maximum-shell first-return relation, contradicting any
closed recurrent maximum shell.

This round builds the missing reusable support and closes the finite theorem
architecture.  It does not pretend that finite validation alone proves the two
remaining dimension/radius-uniform premises.

## Next derivation targets

The remaining mathematics is now exactly two local grammar theorems, not an
open-ended global rank search:

- derive the greatest fixed-base relation symbolically from stepped-hyperplane
  shell-covering inequalities and the carry digit grammar;
- derive shell-2 acyclicity by a dimension-shadow grammar with finitely many
  boundary roles.

Both should return executable evidence consumed by
`certify_parametric_maximum_shell`; no new theorem-composition code is needed.
