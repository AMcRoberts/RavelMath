# Research status

This is the concise entry point for the mathematical and engineering
state of the project. Detailed claim strength is recorded in
`THEOREM_STATUS.md`; the exact active Class-II seam is recorded in
`GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`.

## Verified engineering boundary

The latest complete validation passed `make check` and
`make lean-check`; enrolled Lean files were sorry-free. A fresh checkout
does not include generated build outputs and should rerun those commands.

## Class-II family

The balanced-pair side has an explicit recurrent eight-state family for
every `sigma_{a,1}`, `a>=2`, with exact reductions, strong connectivity,
and characteristic polynomial.

The contact side has an explicit rigid dominant core, a twenty-role
affine interior shell, exact non-base catalogues, universal window and
ranked Red certificates on their stated domains, and a kernel-checked
partition of every legal round for `a>=7` into five phases.

The global occurrence theorem is not closed. Its remaining state-level
seam consists of four exceptional base transitions:

1. signed-contact reverse inclusion at the neighbor base;
2. round-two raw reverse inclusion and Red exclusion;
3. round-three raw reverse inclusion and Red exclusion;
4. the round-four reverse/exclusion bridge into the stable theorem.

Positive witnesses establish inclusion and survival but do not establish
reverse inclusion or exclusion. Cardinality agreement is discovery
evidence, not set equality.

After layer occurrence is closed, recurrent-SCC exhaustion and dominance
remain separate. Layer equality alone does not prove that a displayed
block is exactly a recurrent SCC.

## Exact and formal machinery

The native math library provides arbitrary-precision integer and
rational arithmetic, polynomial and matrix arithmetic, `Q(beta)`
arithmetic and linear algebra, Sturm sequencing and root isolation,
exact characteristic polynomials, Perron certificates, rational
intervals, tunable binary big floats, and FFT implementations.

Lean modules formalize the free-involution descent mechanism, return/
contact projection, the conditional determinant reduction, Class-II
affine shells and margins, endpoint catalogues, support identities, and
the global round partition. The explicitly named existence draft is not
an enrolled theorem.

## Adelic and CSY boundary

The adelic classifier includes number-field factorization, ideal
arithmetic cross-checks, local-field arithmetic, prefix automata,
coincidence, and property-(F) components.

The CSY power machinery has exact recurrence and companion-matrix routes
with bounded memo residency. This does not prove globally bounded
memory: exact outputs, arithmetic temporaries, and live polynomial
contexts remain separate resource dimensions.

The current `CSYAutomaton` is a bounded absolute-position prefix object,
not yet the finite local carry quotient of the cited construction.
Depth exhaustion is reported as truncation.

## Research directions

The direct next theorem work is the four-transition base seam, followed
by recurrent-SCC exhaustion/dominance. Parallel directions include:

- explaining the sparse Class-II boundary-layer slope structurally;
- extending contact-side family proofs to neighboring substitutions;
- constructing finite local carry quotients and bounded corrections;
- comparing geometric windows, labelled dynamics, twisted operators,
  zeta/L-function packages, and spectral embeddings with an explicit
  preservation/forgetting ledger;
- retiring Python prototypes only after bounded native parity or a
  documented correction.

## Validation commands

```sh
make class_ii_corona_literature_probe
make class_ii_symmetry_probe
make class_ii_bp_family_probe
make lean-check
make check
```

The detailed terminal transport audit is opt-in:

```sh
make class_ii_terminal_transport_probe
```
