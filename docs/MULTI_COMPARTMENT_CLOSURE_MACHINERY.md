# Multi-compartment closure machinery

## Purpose

The universal-n campaign demonstrated that typed derivations can be scheduled,
closed, rendered to Lean, and accepted by the kernel. The generalized campaign
substrate originally shared only the scheduler and evidence ledger. This layer
shares the closure machinery itself across project compartments.

## Components

`include/ravel/proof/multi_compartment_closure.hpp` provides:

- canonical scalar projections from word, graph, and matrix evidence;
- primitive normalization of descending integer coefficient lists;
- artifact-shape validation and Lean declaration uniqueness checks;
- scalar-equality closure;
- polynomial-equality closure;
- closed-premise theorem composition;
- a pluggable Lean emitter registry;
- module-level Lean emission with declaration deduplication.

The closure operations are installed into any `OperationRegistry` through
`install_multi_compartment_closure_operations`. They are therefore available
to word-language, affine-catalogue, graph, matrix, spectral, cross-compartment,
and imported legacy campaigns without adding branches to the executor.

## Trusted-boundary rule

Generic theorem composition does not fabricate a mathematical theorem. It
emits a typed manifest recording the number of closed premise artifacts. A real
Lean theorem is emitted only by a closure backend that owns a concrete
proposition and proof-producing normalization. The standard scalar and
polynomial equality backends emit literal reflexive equalities of the computed
normal forms.

This separation is deliberate:

1. evidence operations establish typed mathematical facts;
2. closure operations reduce both sides to canonical forms or combine closed
   premise ledgers;
3. compartment backends supply theorem-specific propositions only where the
   generic normal form is sufficient to justify them;
4. Lean remains the final authority.

## Current cross-compartment validation

`tests/multi_compartment_closure_test.cpp` drives the same closure service over:

- Class-II literal swap sites and Parikh-preserving swaps;
- affine catalogue evaluation and transport;
- graph edge count versus matrix trace;
- recurrence versus matrix-model characteristic coefficients;
- a closed universal-n legacy artifact imported into the generalized ledger.

Every plan closes under the common executor. The emitted validation module is
`lean/generated/multi_compartment_closure_validation.lean`.

## Migration path from universal-n

The next extraction order is:

1. move polynomial-expression normalization from the legacy renderer into a
   typed polynomial normal-form service;
2. move recurrence closure into a generic recurrence certificate with explicit
   base cases and induction step;
3. move determinant/cofactor composition into matrix-specific closure backends
   registered through the same interface;
4. reuse the same polynomial and theorem-composition services from graph and
   spectral campaigns;
5. replace the legacy bridge one operation at a time after emitted Lean agrees
   and kernel checks.
