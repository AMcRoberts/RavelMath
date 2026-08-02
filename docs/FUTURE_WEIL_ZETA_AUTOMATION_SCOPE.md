# Future Weil/zeta spectral automation scope

## Boundary

This is a future cross-project research program, not a proposed method for the
n-bonacci margin theorem and not an RH claim.  The transferable object is the
audit strategy: freeze definitions, enumerate finite defects exactly, search
adversarially for failed uniform claims, and isolate the one analytic estimate
that a limiting theorem actually needs.

The source projects have mixed or restrictive licenses.  Reuse mathematical
facts and independently implement generic infrastructure; do not copy source,
research prose, or released artifacts unless their license affirmatively
permits it.

## Strategy inventory

1. **Frozen specification.** Record spaces, domains, normalizations,
   truncation parameters, prime-power endpoint conventions, and which objects
   live on a common Hilbert space before running experiments.
2. **Claim/dependency ledger.** Give every finite identity, asymptotic claim,
   conditional hypothesis, and target identification a machine-readable ID.
   A conclusion must list its dependencies and certification scope.
3. **Exact finite layer.** Use exact ranks and characteristic polynomials,
   rational or ball enclosures, certified root counts, and retained
   counterexamples.  Floating spectra remain diagnostics.
4. **Defect decomposition.** Split a proposed bridge into bottom cluster,
   middle band, high-energy tail, arithmetic tail, normalization defect, and
   cross-parameter defect.  Treat Feshbach reduction, SCC factorization, and
   relative determinants as alternative instances of the same principle:
   expose the active channel and ledger everything discarded.
5. **Cross-parameter audit.** Never infer a lambda-limit from fixed-lambda
   theorems.  Track normal-family bounds, spectral gaps, active coupling,
   determinant lower bounds, and tail estimates as separate obligations.
6. **Circularity audit.** Detect hypotheses equivalent to Weil positivity,
   global defect-kernel positivity, zero completeness, or the desired zeta
   identification.  Such a result is a reduction unless independent input
   proves the hypothesis.
7. **Schema and counterexample mining.** Infer candidate formulas across
   truncations, then actively search the smallest parameter pair that violates
   them.  Promote a pattern only after an exact verifier checks a separately
   generated instance.

## Work packages

### W0 — Source and claim map

- Pin source versions and checksums.
- Extract definitions, proved statements, conditional statements, explicit
  counterexamples, and open seams.
- Produce a dependency graph with no mathematical mutation.

**Exit:** every public claim has a source location, scope, and status.

### W1 — Common experiment schema

Define records for:

- model and frozen-specification version;
- parameters such as lambda, Fourier cutoff, smoothing packet, and contour;
- active/reference matrices or operators;
- exact finite invariants;
- numerical diagnostics;
- certificate method, precision, and validity domain;
- failed claim or unresolved obligation.

**Exit:** results from the operator, proxy, kernel, and positivity projects can
be compared without silently changing normalization.

### W2 — Reproducer adapters

Build read-only adapters for three initial tracks:

1. fixed-parameter semilocal operator identities and the certified failure of
   one-step interlacing;
2. bottom-cluster/Feshbach reduction and the unresolved active-coupling term;
3. finite Xi-defect positivity and the fixed cross-endpoint obstruction.

Adapters should reproduce upstream certificates first.  Independent code
comes only after agreement is established.

**Exit:** a clean run regenerates the stated finite result and its exact scope.

### W3 — Generic finite audit engine

Factor out reusable machinery for:

- exact rank and graded-rank bounds;
- characteristic and relative characteristic polynomials;
- rational/ball spectral separation;
- determinant and logarithmic-derivative identities;
- cluster/SCC decomposition;
- counterexample minimization;
- certificate serialization and replay.

**Exit:** each finite assertion is either certified, refuted by a retained
witness, or explicitly unsupported.

### W4 — Scale-defect tracker

For successive parameter values, compute a ledger rather than a single error
norm:

- bottom eigenvalue and gap;
- active bottom coupling;
- middle inverse moment;
- high-energy tail;
- prime-power tail;
- normalization ratio;
- neutral spectral-arithmetic defect;
- determinant zero/pole movement on safe compact sets.

Use the data to propose uniform bounds and to search deliberately for their
failure.  A fitted decay law is never a certificate.

**Exit:** one explicit list of estimates sufficient for the desired limiting
theorem, with finite evidence and counterevidence attached to each estimate.

### W5 — Conditional bridge theorem

Write and, where practical, formalize the implication:

> frozen construction + named uniform estimates + noncircular arithmetic
> identification => stated limiting determinant/trace conclusion.

Keep self-adjointness, determinant convergence, zero-divisor convergence, and
identification as separate hypotheses until proved.

**Exit:** the remaining mathematical burden is singular and inspectable, not
distributed among implementation assumptions.

### W6 — Analytic escalation

Only after W0–W5, choose the residual estimate with the best structural
evidence.  Develop new analysis for that estimate; do not spend theorem effort
improving finite truncation records that cannot affect the limit.

**Exit:** either an independent uniform theorem, an obstruction for the chosen
mechanism, or a documented equivalence to an RH-level statement.

## Reusable machinery from Ravel

- exact integer polynomial and divisibility checks;
- exact rational Perron/interval certificates;
- SCC and equitable-quotient ledgers;
- involution and signed-sector decomposition;
- generated-schema versus independent-ground-truth checks;
- adversarial finite searches with retained failures;
- Lean theorem skeletons that expose hypotheses rather than hiding them;
- regression enrollment, provenance, and theorem-status documentation.

The displacement grammar itself is problem-specific and should not be
exported.  The reusable pattern is the compression loop:

`finite catalogue -> symbolic cases -> independent exact cross-check ->
uniform proof obligation`.

## Suggested first build

Start with W0 and W1 only.  Then implement one narrow W2 adapter for the
finite interlacing counterexample and one W3 verifier for the corrected
rank-s bound.  That path is small, falsifiable, and exercises the complete
provenance/certificate stack before any work on cross-parameter convergence.

