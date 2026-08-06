# Shared Polynomial and Recurrence Closure

Date: 2026-08-03

## Purpose

This layer migrates two closure operations out of the theorem-specific universal-n engine:

1. polynomial expression normalization by collecting coefficients at equal exponents;
2. first-order polynomial recurrence closure by induction against a canonical recursive normal form.

The implementation is in:

- `include/ravel/proof/shared_polynomial_closure.hpp`
- `tests/shared_polynomial_closure_test.cpp`

## Common evidence

`PolynomialExpressionEvidence` is a compartment-neutral multiset of integer coefficient/exponent terms.
`PolynomialNormalFormCertificate` stores its unique ascending coefficient vector.

`FirstOrderPolynomialRecurrenceEvidence` describes the currently installed schema

```text
f(0) = 1
f(n+1) = X * f(n) + 1.
```

`RecurrenceClosureCertificate` records that the evidence matches this schema and that the shared induction backend emitted a universal closure theorem.

## Trust boundary

The C++ operation does not claim that a compartment has proved its recurrence. It consumes recurrence evidence already closed by that compartment. It proves the reusable implication:

```text
base certificate + step certificate
=> equality with the canonical recursive normal form for every n.
```

The emitted Lean theorem is quantified over an arbitrary `f : ℕ → Polynomial ℤ`; it is not tied to n-bonacci or to matrices.

The conversion from the canonical recursive normal form to

```text
∑ k ∈ Finset.range (n + 1), X^k
```

is intentionally not folded into this pass. That finite-sum shift identity is the next shared normalization theorem and must receive its own kernel-checked backend.

## Cross-compartment exercise

The regression plan sends both matrix and spectral recurrence evidence through the same recurrence operation. It also normalizes two differently ordered polynomial term presentations from matrix and spectral compartments and closes their equality through the same coefficient normal form.

## Validation

The following native targets pass:

- `shared_polynomial_closure_test`
- `multi_compartment_closure_test`
- `generalized_campaign_test`
- `legacy_campaign_bridge_test`
- `proof_campaign_engine_test`
- `family_closed_forms_test`
- `nbonacci_charmpoly_proof_general`

The original universal-n generated Lean artifact remains unchanged.
