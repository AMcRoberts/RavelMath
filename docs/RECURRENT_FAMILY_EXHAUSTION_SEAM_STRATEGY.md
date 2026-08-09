# Strategy: close the recurrent-family exhaustion seam

> **Correction, 2026-08-08.** The finite result below (n=6, resp. n=3..7) stands and is now the basis of a kernel-checked reflection wiring (`recurrent_family_exhaustion_reflection_test.cpp` / `predicted_core_scc_exhaustion_reflection_test.cpp`). Its stated dependency on an "all-n induction" / "universal carry-bound theorem" is moot: the universal n-bonacci dominance theorem closed via a different route (the canonical Q/R split) that never needed this hull/carry-bound machinery. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md`.

## Exact seam

The finite classifier is already complete through `n = 7`. Grade descent closes
all pure-grade competitors, positive Collatz/paired weights close the mixed-grade
`n = 7` competitors, and full-support permutation SCCs are handled separately.
The remaining theorem is therefore not another grade catalogue and not another
finite SCC sweep.

For the induction step `n -> n + 1`, let

- `K_{n+1}` be the formula-defined predicted core;
- `T(F_n)` be the union of request-driven images of the nonterminal recurrent
  families already classified at dimension `n`;
- `P_{n+1}` be the uniformly generated terminal permutation families; and
- `R_{n+1}` be the corona boundary rejected by the predicates defining
  `K_{n+1} ∪ T(F_n)`.

The target is:

> Every infinite path beginning in `R_{n+1}` eventually enters
> `K_{n+1} ∪ T(F_n) ∪ P_{n+1}`.

Equivalently, the induced graph on
`R_{n+1} \ P_{n+1}` has no recurrent SCC disjoint from the requested families.

The current finite implementation in `adjacent_competitor_transport.cpp`
closes the rejected boundary, finds any recurrent births, and adds them to the
catalogue. That is a valid discovery procedure but not an exhaustion proof.
The proof implementation must replace this catalogue absorption with replayed
no-return evidence.

## Main proof shape

The preferred proof is a lexicographic first-return rank, derived from the
existing executable queue grammar.

For each rejected state `s`, derive

```
rank(s) = (grade_excess(s), chamber_defect(s), boundary_queue_rank(s), phase_rank(s))
```

with a lexicographic order. The components have distinct jobs:

1. `grade_excess` records distance from a request-supported grade/family.
   It is not expected to decrease on every edge; it prevents hidden creation
   of arbitrarily high catalogue grade.
2. `chamber_defect` measures failure of the face-relative sign/chamber pattern
   required by a transported family or the formula core.
3. `boundary_queue_rank` is the strict first-return rank obtained from the
   left/right queue transport laws already derived by
   `derive_boundary_queue_grammar` and `derive_queue_weight_recurrence`.
4. `phase_rank` resolves neutral queue moves and identifies the finite terminal
   permutation cases.

The edge theorem should not require one-step descent. Use first return to the
maximal shell or to the rejected predicate boundary:

> If `s` is rejected, nonterminal, and does not enter a requested family before
> its next boundary return, then the returned state `s'` satisfies
> `rank(s') < rank(s)`.

This is enough: an infinite rejected path would produce an infinite descending
chain in a well-founded lexicographic order. Neutral cycles are permitted only
when the derivation recognizes the uniform one-in/one-out terminal grammar.

## Why grade-two dynamics still matter

Grade is an outer invariant, not the final decreasing measure. The finite data
show:

- genuine pure grade-two recurrence begins at `n = 5`;
- pure grade-three recurrence first appears at `n = 7`;
- pure grade-three simulates completely into grade two;
- mixed-grade `n = 7` recurrence requires positive weights, not Boolean grade
  simulation.

Thus grade-two dynamics provide the induction base for family transport and
for decoding which rejected states are already represented by a lower family.
They should be used to prove `grade_excess = 0` means “requested or terminal,”
not to claim that raw grade decreases on every transition.

## Required reusable operations

### 1. Rejected-state normal form

Add an operation, tentatively

```
derive_rejected_boundary_normal_form(...)
```

It must derive from a state and the request predicates:

- catalogue grade and decomposition witnesses;
- source/target face positions;
- sign chamber relative to those faces;
- left/right queue words under the boundary grammar;
- whether the state is formula-core, transported-family, terminal-permutation,
  or genuinely rejected;
- the exact failed predicate(s), not merely a Boolean rejection.

The operation must replay every field against the executable transition and
catalogue code.

### 2. First-return transducer

Add

```
derive_rejected_boundary_first_return(...)
```

This should symbolically execute the queue grammar until one of four outcomes:

- entry into the formula core;
- entry into a transported family request;
- recognition of a terminal permutation orbit;
- return to the rejected boundary with a derived rank delta.

The result is a proof object, not a search log. It should include the branch
word, queue recurrence, phase transport, and exact rank comparison.

### 3. Parametric rank coefficient derivation

Complete the seam already named after the reflective boundary grammar:

```
derive_first_return_rank_coefficients(...)
```

Inputs should be the first-return normal form and queue transport laws. Outputs
should be symbolic coefficients indexed by face position/chamber class, with a
proof that every nonterminal rejected return has negative total delta.

Do not fit coefficients from quotient matrices. Finite dimensions may be used
to conjecture the closed form, but the operation must derive and replay the
formula from the transition grammar.

### 4. Terminal permutation grammar

Add

```
derive_terminal_permutation_family(...)
```

It should prove uniformly that the neutral-rank cases are exactly the terminal
one-in/one-out sign-block permutations, and emit:

- their parametric state formula;
- closure;
- internal indegree/outdegree one;
- spectral radius one;
- disjointness from core and transported nonterminal families.

This prevents the no-return theorem from silently excluding legitimate neutral
cycles.

### 5. No-return composition

Add

```
derive_rejected_boundary_no_return(...)
```

It composes the normal form, first-return transducer, strict rank theorem, and
terminal grammar into the exact field currently represented by the Boolean
`rejected_boundary_no_return` in `InductiveFamilyExhaustionProof`.

The operation should fail with the first named missing premise. It must never
accept a caller-supplied `true` in production proof paths.

## Theorem decomposition

The all-dimensions exhaustion theorem should be assembled from five lemmas.

### A. Request completeness

Every formula-core state and every transported successor of a lower recurrent
family is accepted by its corresponding request, and all induced transition
witnesses are preserved.

### B. Grade/support ceiling

Every state reachable from the rejected corona boundary has a catalogue grade
within the finite range handled by the normal-form grammar, or immediately
enters a requested family. This is where grade-two/grade-three decomposition is
used structurally.

### C. Strict rejected first return

Every nonterminal rejected first return strictly lowers the lexicographic rank.

### D. Neutral-case classification

Every rejected return with zero rank delta belongs to a uniform terminal
permutation family.

### E. Well-founded exhaustion

A finite or well-founded descent argument rules out a nonterminal recurrent SCC
inside the rejected boundary. Combined with A and D, every upper recurrent SCC
is core, transported, or terminal.

## Execution order

### Stage 0: preserve the finite oracle

Keep the current rejected-boundary SCC closure as a diagnostic oracle. Change
its status label to `finite_oracle`, and ensure it is not consumed as universal
proof evidence.

Run and record `n = 4, 5, 6, 7` rejected-boundary traces with, for every edge:
normal form, first-return block, grade, chamber, queue coordinates, and terminal
classification. This supplies counterexamples quickly while deriving the rank.

### Stage 1: isolate the `n = 6 -> 7` complexity change

Compare rejected first-return words for `5 -> 6` and `6 -> 7`, grouped by:

- pure grade two;
- pure grade three;
- mixed grade two/three;
- terminal permutation.

The expected result is that `n = 7` introduces new chamber/phase classes but
not a new queue recurrence. Verify this rather than assuming it. If true, the
rank coefficients should depend on face-relative chamber type, not on raw
`n`-dimensional sign vectors.

### Stage 2: derive a minimal chamber quotient

Start with the full face-relative sign chamber, then partition-refine by
first-return behavior. Merge two chamber states only when they have identical:

- request acceptance;
- terminal status;
- queue update;
- successor quotient classes;
- rank delta expression.

This creates the smallest behaviorally exact chamber grammar and avoids the
failed label-pair-only Bellman abstractions.

### Stage 3: solve and prove the rank inequalities

From the quotient grammar, generate symbolic strict inequalities for the queue
and phase coefficients. Search may propose a solution, but convert it into a
closed recurrence formula and replay all inequalities algebraically.

The preferred coefficient family is telescoping and local in face position.
If strict one-return descent is impossible, expand the return section before
changing proof paradigms.

### Stage 4: prove terminal exactness

Extract all zero-delta quotient cycles. Prove each matches the uniform terminal
permutation grammar. Any zero-delta non-permutation cycle is a genuine
counterexample to the proposed rank and must be reported verbosely.

### Stage 5: integrate with inductive exhaustion

Replace the Boolean production call to `derive_inductive_family_exhaustion`
with a typed `RejectedBoundaryNoReturnProof`. The final constructor should
accept the proof object and independently check `proof.proved` plus dimension
and request identities.

### Stage 6: Lean boundary

Emit Lean theorems for:

1. lexicographic well-foundedness;
2. strict first-return descent excludes recurrent nonterminal rejected paths;
3. neutral paths are terminal;
4. exhaustion promotion from dimension `n` to `n + 1`.

C++ remains responsible for deriving the symbolic branch grammar and
coefficients; Lean checks the general logical composition and the emitted
algebraic identities.

## Failure pivots

### Pivot 1: no scalar lexicographic rank exists

Use a finite-dimensional vector rank ordered by a pointed cone. Derive a matrix
`Q` for first-return classes and a positive covector `w` with
`w Q < w` outside terminal classes. This is the parametric analogue of the
successful Collatz branch, but on the small chamber grammar rather than the
full SCC quotient.

### Pivot 2: first-return behavior needs memory

Enrich the normal form with the shortest suffix that restores determinism.
Partition-refine until the transducer is exact. Do not return to raw full-state
enumeration; the added memory must be a bounded grammar field with a uniform
meaning.

### Pivot 3: grade ceiling fails

Generalize catalogue-sum decomposition recursively and prove that every added
atom either maps into a transported lower family or increases a support defect
that the queue rank subsequently decreases. This becomes induction on
`(grade, first-return rank)`.

### Pivot 4: terminal families are not the only neutral cycles

Classify the new neutral grammar as an additional transported family only if a
uniform seed/formula and closure theorem can be derived. Do not absorb finite
births without such a theorem.

### Pivot 5: `n = 7` changes the queue recurrence itself

Split by a structural threshold predicate—likely the first availability of a
third disjoint catalogue atom—and prove two uniform regimes: low-grade and
higher-grade. The theorem may be nested induction on grade, but the transition
between regimes must be stated explicitly rather than hidden in dimension.

## Acceptance gates

The seam is closed only when all of the following hold:

- no full rejected-boundary SCC enumeration is required by the proof consumer;
- every rejected state receives a replayed normal form;
- every nonterminal first return has an exact strict descent certificate;
- every neutral return has a uniform terminal-family certificate;
- the `n = 6 -> 7` replay passes using the same symbolic theorem used for all
  dimensions;
- the inductive exhaustion constructor consumes typed evidence, not Booleans;
- generated Lean checks the well-founded no-return composition;
- finite SCC closure remains only an independent regression oracle.

## Immediate implementation target

Implement `rejected_boundary_rank.hpp` with data structures and diagnostics for
normal forms and first-return traces, then instrument the existing
`n = 5 -> 6` and `n = 6 -> 7` request-driven runs. The first concrete question
is:

> Which face-relative chamber/phase field distinguishes the `n = 7`
> first-return branches while leaving the queue recurrence unchanged?

Answering that from exact traces is the shortest path to the parametric rank.
