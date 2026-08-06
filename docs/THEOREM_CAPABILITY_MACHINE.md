# Theorem Capability Machine

## Purpose

The theorem capability machine is the first question-to-proof-planning layer over the Ravel theorem corpus.
It does not treat text similarity as proof. It separates:

1. theorem discovery;
2. hypothesis and compartment matching;
3. derivation-chain selection;
4. proof-plan emission;
5. Lean generation and kernel validation.

This checkpoint implements the first three and exposes the fourth as a precise list of required hypotheses and theorem applications.

## Corpus inventory

`tools/build_theorem_capability_index.py` scans every Lean source under `lean/` and writes
`config/theorem_capabilities.tsv`. Each record includes declaration name, source location,
compartment tags, signature, and trust status.

Trust statuses are deliberately conservative:

- `kernel-checked-export`: returned kernel-checked generated artifact;
- `lean-declaration`: theorem declaration present in the project corpus;
- `draft`: declaration in a draft or proof-attempt file.

A nearby kernel-checked theorem does not upgrade another chain's trust status.

## Query interface

Build:

```sh
make ravel_truth_machine
```

Ask:

```sh
./out/ravel_truth_machine --question='Can the nbonacci charpoly reduce high matrix powers?'
```

The answer contains:

- classification;
- selected theorem chain;
- exact hypotheses that must be instantiated;
- exact theorem declarations used;
- resulting conclusion;
- ranked supporting capabilities;
- explicit unclosed obligations.

## Curated cross-compartment chains

The first registry includes:

- universal n-bonacci determinant: matrix -> polynomial;
- n-bonacci charpoly to matrix-power reduction: matrix -> number theory;
- Perron column differences to coordinate-gap ordering: spectral -> number theory;
- Class-II Perron inequalities to window bounds: spectral -> window;
- affine edge catalogues to boundary support: affine catalogue -> graph;
- shell propagation to injectivity and window validity: affine catalogue -> window.

The theorem index is generated automatically. Cross-compartment chains are curated because a chain is a mathematical claim about theorem composition, not a keyword coincidence.

## Current boundary

The machine currently answers: “which existing theorems can close this question, under which hypotheses?”
It does not yet synthesize arbitrary Lean terms. The next layer should:

1. parse supplied facts into typed evidence;
2. unify those facts against theorem hypotheses;
3. instantiate a selected chain;
4. emit a standalone Lean file;
5. accept returned kernel output as authoritative evidence.
