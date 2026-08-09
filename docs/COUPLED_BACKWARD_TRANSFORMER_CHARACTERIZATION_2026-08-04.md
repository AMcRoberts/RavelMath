# Coupled backward-transformer characterization

> **SUPERSEDED, 2026-08-08.** This describes an abandoned proof route. The theorem this file targets IS closed -- via a different route (the canonical Q/R split), Lean kernel-checked and independently reconfirmed. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md` for the current, authoritative status before reading further.

Local joint features are not exact: the learned decision trees reach about 98.3% accuracy because the same base/controller/face/length signature can have opposite winning labels under different future digit suffixes.

The exact symbolic remaining-word state is the backward transformer

    W([], targetFace) = targetFace
    W(d :: suffix, targetFace) = Pre_d(W(suffix, targetFace)).

Here `Pre_d` is the symbolic controller predecessor operator. Since the radius-one controller state space is finite for fixed `n`, every `W` is a finite bitset and equivalent suffixes collapse automatically to the same canonical transformer class.

Corpus minimization results:
- n=3: 379 canonical transformer classes across 4924 suffix occurrences; 555 distinct class transitions; all 1146/1146 source obligations accept.
- n=4: 1424 canonical transformer classes across 7984 suffix occurrences; 1473 distinct class transitions; all 1774/1774 source obligations accept.

This is now an exact, collision-free characterization. It is not yet dimension-uniform because the bitset has `3^n-1` controller positions. The next seam is to synthesize a compact formula for these transformer classes, or prove that only a bounded family of center/face profiles is reachable from exact first-return suffixes.
