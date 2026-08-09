# Cyclic continuation-controller pumping

> **SUPERSEDED, 2026-08-08.** This describes an abandoned proof route. The theorem this file targets IS closed -- via a different route (the canonical Q/R split), Lean kernel-checked and independently reconfirmed. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md` for the current, authoritative status before reading further.

The outer universal shell-pumping proof requires preservation of recurrence.
Endpoint-to-endpoint transport is not sufficient.

This checkpoint introduces cyclic continuation-controller runs.  Given a base
cycle with digit word `w`, the controller must produce a translation path whose
state after reading `w` equals its initial state.  When the base cycle also
closes, affine transport produces a transported cycle.

The fixed-point search is exact for a finite controller plant:
- compute the image relation of each controller state under the whole word;
- retain states that can return to themselves;
- reconstruct and replay a closing controller path.

The generated Lean theorem proves the abstract closure composition:
a closed base path plus a closed compatible translation path yields a closed
transported path.

Bounded all-word characterization results:
- n=3: 3279/3279 words through the configured length bound admit a cyclic radius-one controller run.
- n=4: 1092/1092 words through the configured length bound admit a cyclic radius-one controller run.

These bounded all-word results are characterization only.  They do not prove
that every recurrent carry cycle admits a strictly outward cyclic transport.
The concrete remaining obligations are:

1. restrict the input language to digit words realized by non-ternary recurrent
   carry cycles;
2. require endpoint-face alignment that strictly increases shell radius;
3. prove the transported adjusted digit word remains admissible;
4. prove a closing controller state exists for every such realized cycle word.

The parallel next-axis audit recommends universal predicted-core SCC
identification as the next downstream theorem once cyclic pumping closes the
ternary recurrent bound.
