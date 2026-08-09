# Universal shell-pumping proof

> **SUPERSEDED, 2026-08-08.** This describes an abandoned proof route. The theorem this file targets IS closed -- via a different route (the canonical Q/R split), Lean kernel-checked and independently reconfirmed. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md` for the current, authoritative status before reading further.

## Architectural correction

The continuation-controller family is not itself the final universal proof.
It supplies one local pumping lemma.

Let `ReturnCapable(x)` mean that `x` belongs to the recurrent or indefinitely
continuable exact carry system, and let `radius(x)` be its maximum-coordinate
shell radius.

The local controller obligation is:

    radius(x) >= 2 and ReturnCapable(x)
      => exists y,
           ReturnCapable(y)
           and radius(y) > radius(x).

This is `StrictShellPump`.

## High-level universal proof

Assume the exact admissibility/window theory supplies a finite radius bound
`B` for every return-capable state.

Suppose an outer recurrent state exists at radius at least two.  Apply the
strict shell pump repeatedly.  After `B+1` applications, the resulting
return-capable state has radius at least

    radius(x) + B + 1 > B,

contradicting admissibility.

Therefore every recurrent carry state has radius at most one; equivalently,
every recurrent coefficient vector is ternary.

This argument is independent of:
- the shape of the finite controller family;
- the number of residual controllers;
- shell-specific rank tables;
- a symbolic presentation of the fixed-dimension family algebra.

Those are possible implementations of `StrictShellPump`, not components of the
outer contradiction.

## Exact remaining local theorem

The controller-family work must now prove that a maximum-shell first-return
segment can be transported outward in a way that leaves its target
`ReturnCapable`, so the pumping step can be applied again.

Endpoint alignment alone is insufficient unless return capability is preserved.
The formal obligation must therefore carry one of:

1. a transported recurrent cycle;
2. an infinite compatible continuation;
3. a first-return edge inside a relation already proved total on the recurrent
   subsystem.

This is the final local seam in the carry-bound part of the high-level proof.

## Downstream universal dominance proof

Once the ternary recurrent bound is available:

1. all recurrent carry states lie in the finite ternary layer;
2. universal core identification classifies the grade-one recurrent component;
3. grade exhaustion classifies every remaining recurrent SCC;
4. uniform spectral comparison proves every competing grade has smaller
   spectral radius;
5. hence `rho(G_B(n)) = rho(predicted_core(n))` for every `n >= 3`.

The shell-pumping theorem closes the first of those four campaign seams.  It
does not by itself prove core identification, grade exhaustion, or spectral
dominance.
