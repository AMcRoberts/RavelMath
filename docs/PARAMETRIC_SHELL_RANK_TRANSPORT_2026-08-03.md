# Parametric shell-rank transport

> **SUPERSEDED, 2026-08-08.** This describes an abandoned proof route. The theorem this file targets IS closed -- via a different route (the canonical Q/R split), Lean kernel-checked and independently reconfirmed. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md` for the current, authoritative status before reading further.

This checkpoint does not claim the arbitrary-shell theorem yet. It establishes
the next stronger certificate class.

Instead of topologically ranking each exact first-return DAG independently, the
miner maps shell states to a common phase:

    signs | exact absolute-value gaps | min-absolute-value mod (n+1)

and uses the rank form

    R(x) = sum_i |x_i| + h(phase(x)).

Every exact first-return edge contributes one difference constraint on `h`.
Constraints from several shell radii are merged and solved simultaneously. A
single offset vector satisfying all of them is evidence that the rank belongs
to the phase model rather than to one finite shell.

The C++ closure machinery:
- solves integer phase difference constraints;
- rejects positive cycles;
- replays every supplied inequality;
- records all covered `(n,M)` instances.

The Lean endpoint proves:
- phase inequalities imply strict rank increase;
- a finite shell with complete first returns and strict phase rank must be
  empty.

Current exact merged certificates:
- `n3_M2_6.json`: n=3, M=2..6, phases=1934, constraints=6684, raw edges=6756, checked=True
- `n4_M2_4.json`: n=4, M=2..4, phases=5672, constraints=25364, raw edges=25364, checked=True

This is finite transport evidence, not yet a uniform formula for all M. The
next seam is to derive closure of the phase transition relation under radial
translation by `n+1`, using the exact block-defect identity. If successful,
one merged offset certificate will extend from a finite set of residues to all
shell radii.
