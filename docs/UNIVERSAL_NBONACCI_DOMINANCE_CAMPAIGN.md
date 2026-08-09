# Universal n-bonacci boundary-dominance campaign

> **SUPERSEDED, 2026-08-08.** This describes an abandoned proof route. The theorem this file targets IS closed -- via a different route (the canonical Q/R split), Lean kernel-checked and independently reconfirmed. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md` for the current, authoritative status before reading further.

## Target

For every `n >= 3`:

    rho(G_B(n)) = rho(predicted_core(n)).

This is the highest-value next theorem because it simultaneously crosses:

- matrix characteristic identities;
- integer carry dynamics;
- shell-return escape;
- recurrent SCC classification;
- exact Perron comparison;
- graph-to-spectral theorem transport.

## Structural decomposition

The campaign records four genuinely open universal seams:

1. **Parametric shell rank.**
   Derive, for every `n` and every shell `M >= 2`, a strict integer rank on
   the maximum-shell first-return relation. This replaces infeasible box
   enumeration and would give a universal recurrent carry bound.

2. **Universal core identification.**
   Promote `predicted_core_member`, successor/predecessor formulas, size, and
   edge-count closed forms into an arbitrary-`n` SCC theorem.

3. **Grade exhaustion.**
   Prove every recurrent non-core SCC lies in a finite symbolic grade family.

4. **Uniform spectral dominance.**
   Replace per-dimension exact Collatz-Wielandt runs with a symbolic positive
   vector or polynomial comparison valid for every competing grade.

The kernel-checked universal-n characteristic polynomial and the exact
`n+1`-block identity are upstream machinery, not the endpoint.

## First new conversion layer

`universal_dominance_shell_return_validation.lean` formalizes the generic
certificate interface:

- a finite shell;
- a first-return relation;
- an outgoing return from every shell state;
- a strict integer rank on every return edge.

Lean proves such a nonempty shell is impossible. C++/certificate generators
therefore need only emit and validate the finite relation, rank, and totality
facts; the contradiction is shared closure machinery.

## Research direction

The finite shell certificates are attractors to a parametric rank formula.
The next discovery loop should mine:

- rank differences under the exact `n+1` block map;
- maximum-coordinate position and sign;
- boundary-mask transitions;
- affine chamber coordinates;
- stable rank transport from `(n,M)` to `(n+1,M)` and `(n,M+1)`.

The target is not another table of successful shells. It is the symbolic
operator that generates all such ranks.
