# Generalized proof-campaign architecture

## Purpose

The kernel-checked universal-n campaign proved that reflected mathematical
execution can be converted into a dependency-ordered collection of typed
proof operations and then into Lean accepted by the kernel. The next step is
not another theorem-specific generator. It is a reusable campaign substrate
that can attack the project's matrix, word-language, graph, affine-catalogue,
and spectral compartments through one execution model.

## New common layer

`include/ravel/proof/generalized_campaign.hpp` provides:

- a compartment-neutral `Plan` and dependency scheduler;
- a typed `Evidence` variant rather than raw theorem strings;
- a registry of reusable operations identified independently of theorem names;
- closed `ProofArtifact` values with evidence, Lean declarations, and an
  established-fact ledger;
- exact failure propagation when an operation or dependency does not close;
- cross-compartment comparison tasks.

The standard registry currently contains operations for:

- exhaustive enumeration of adjacent unequal sites in literal words;
- certification that every generated adjacent swap preserves Parikh data;
- exact evaluation of affine families;
- two-parameter affine difference-law certification;
- directed/undirected graph degree-edge accounting;
- exact matrix trace;
- recurrence-to-characteristic-polynomial transport;
- cross-compartment scalar-invariant agreement.

These are small foundation operations, not the final family theorems. Their
role is to establish the stable interface and execution semantics before more
powerful operations are moved in.

## Relationship to universal-n

The original `proof_campaign_engine.hpp` remains the authoritative,
kernel-proven matrix implementation. It is not rewritten or weakened.

`include/ravel/proof/legacy_campaign_bridge.hpp` imports every closed legacy
artifact into the generalized artifact model without losing its definitions,
theorems, or established-fact ledger. Thus universal-n is now the reference
matrix backend of the generalized system rather than an isolated pipeline.

Migration can proceed operation by operation:

1. retain the checked legacy operation and its regression tests;
2. define its compartment-neutral evidence input/output;
3. install the operation in the generalized registry;
4. compare generalized and legacy artifacts;
5. switch new campaigns to the generalized implementation only after Lean
   output agrees and kernel checks.

## Target operation spectra

### Matrix

- structural support classification;
- erased-index transport;
- triangular/block-triangular determinant closure;
- sparse cofactor decomposition;
- recurrence extraction and closure;
- polynomial normalization.

### Word-language

- occurrence witness construction;
- exact adjacent-site enumeration;
- desubstitution;
- parent-case classification;
- reverse exhaustion;
- branch distinctness and language/conjugacy transport.

### Graph/contact

- reachable and recurrent core extraction;
- SCC decomposition and transport;
- contact-edge generation/exhaustion;
- component characteristic polynomial;
- dominance comparison.

### Affine catalogues

- affine evaluation and transport;
- symbolic branch normalization;
- collision classification;
- cardinality closure;
- parameter chamber/wall certification.

### Spectral

- recurrence/transfer operator construction;
- characteristic/Fredholm polynomial generation;
- factor transport;
- root-radius and dominance obligations;
- comparison of spectral probes across presentations.

### Cross-compartment

- incidence equality from word operations;
- graph/operator consistency;
- catalogue-to-matrix agreement;
- recurrence-to-matrix characteristic-polynomial agreement;
- preservation/forgetfulness ledgers for presentation changes.

## Validation checkpoint

The same generalized executor currently closes four independent plans:

- literal Class-II word sites and Parikh-preserving swaps;
- affine-family evaluation and difference transport;
- graph edge count versus matrix trace agreement;
- tribonacci recurrence to characteristic polynomial.

The legacy bridge also accepts a closed universal-n-style matrix artifact.
All corresponding native tests pass.

## Next implementation order

1. Extract the universal-n polynomial-normalization and composition operations
   into the generalized registry.
2. Add typed Lean emitters separate from mathematical derivations.
3. Add a word-language reverse-exhaustion operation and drive both Class-II
   and n-bonacci adjacent-site theorems through it.
4. Add affine branch distinctness/collision classification.
5. Add graph SCC and component-polynomial operations.
6. Build the first cross-compartment theorem campaign combining catalogue,
   graph, matrix, and spectral artifacts.


## Multi-compartment closure checkpoint

The generalized layer now shares closure, not only scheduling. See
`docs/MULTI_COMPARTMENT_CLOSURE_MACHINERY.md`. Scalar and polynomial normal
forms, artifact validation, theorem composition, and Lean backend dispatch are
installed as ordinary registry operations and exercised by every active
compartment plus the universal-n legacy bridge.
