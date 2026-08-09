# RavelMath

RavelMath is an exact-mathematics research laboratory for symbolic dynamics,
Pisot substitutions, tiling questions, algebraic dynamics, and the proof
engineering needed to make computational mathematics auditable.

It is not a numerical-results scrapbook. The central design question is:

> When a program discovers a mathematical fact, what is the smallest typed
> certificate that lets an independent proof kernel check the fact from the
> data that actually produced it?

The repository combines a C++ exact-arithmetic library with Lean formalization
and a reflective proof pipeline. C++ performs the domain-specific finite work;
typed semantic payloads preserve the concrete observations; Lean checks the
general lemma and the mechanically rendered instance. The boundary between
those three layers is explicit, so “the program ran” and “the theorem was
kernel checked” are never treated as the same status.

## The research program

The main mathematical thread studies Pisot numbers and their canonical
beta-substitutions:

- greedy beta-expansions and canonical substitutions, including terminating
  and eventually periodic cases;
- strong coincidence and the geometric property (F), with exact prefix and
  contact automata for unit and non-unit Pisot substitutions;
- primitive generator structure, including when a fourth generator survives
  rather than collapsing to the Q/R/S basis;
- contact-boundary graphs, finite positive grammar majorants, and spectral
  consequences for tiling and domain-exchange questions;
- exact Pisot classification and Sturm root isolation, now connected to the
  reflective proof pipeline for a real `x^3 - x - 1` instance;
- the universal n-bonacci and Class-II boundary families, including exact
  characteristic-polynomial, occurrence, dominance, and matrix identities.

The project also contains deliberately parked or open directions. Among them
are the general strong-coincidence problem, the full complex-modulus certificate
for higher-degree Pisot classification, walk-realizability questions, and the
eventually-periodic generator formula. An experimental result, a finite exact
certificate, a paper proof, and a Lean theorem are recorded as different kinds
of result.

## What is formally checked

The strongest completed routes include:

- the universal n-bonacci characteristic-polynomial theorem;
- the universal n-bonacci boundary-dominance theorem, unconditional in `n`;
- reusable matrix, recurrence, contact, coincidence, and generator lemmas;
- the Barge–Diamond irrational-eigenvalue obstruction, rendered from a real
  reflection trace;
- exact Sturm/PRS root isolation for a concrete Pisot classifier run, using
  Mathlib’s `CertifiedSturmChain` bridge;
- many finite Class-II and substitution-table certificates whose concrete data
  is checked independently on the C++ and Lean sides.

See [`docs/THEOREM_STATUS.md`](docs/THEOREM_STATUS.md) for the authoritative
status vocabulary and theorem ledger, and
[`docs/FINDINGS_FOR_CITATION.md`](docs/FINDINGS_FOR_CITATION.md) for the
finding-by-finding research record.

## The proof architecture

The reflective route is intentionally theorem-neutral:

1. an application selects a mathematical task;
2. exact library code performs the relevant observation or derivation;
3. a typed payload records the concrete result and its bounded checks;
4. the reflection trace preserves provenance and dependencies;
5. a renderer emits a Lean definition or corollary from that payload;
6. the Lean kernel accepts or rejects the completed artifact.

Missing mathematics is implemented as a reusable derivation operation, not
as hand-written generated proof text. The governing contracts are in
[`docs/PROOF_SYSTEM_CONTRACTS.md`](docs/PROOF_SYSTEM_CONTRACTS.md), and the
current retrofit map is in
[`docs/REFLECTION_RETROFIT_PLAN.md`](docs/REFLECTION_RETROFIT_PLAN.md).

## Repository map

| Path | Role |
| --- | --- |
| `math/` | Exact arithmetic, polynomials, Sturm isolation, matrices, and algebraic data structures |
| `include/ravel/` | Domain operations and typed proof/certificate staging |
| `lean/` | General Lean lemmas and formal mathematical infrastructure |
| `lean/generated/` | Mechanically rendered Lean artifacts, each tied to concrete input data |
| `tests/` | C++ regression tests and reflection end-to-end tests |
| `app/` | Research applications and executable campaigns |
| `docs/` | The theorem ledger, research findings, reading routes, and design contracts |

Start with [`docs/DOCUMENTATION_INDEX.md`](docs/DOCUMENTATION_INDEX.md). For a
shorter mathematical status summary, read
[`docs/RESEARCH_STATUS.md`](docs/RESEARCH_STATUS.md).

## Building and checking

The project is developed on Linux with a C++20 compiler, GNU-style exact
integer/rational support, and a pinned Lean/Mathlib environment.

```sh
make math
make tests
make sturm_chain_reflection_test
```

The last target builds a real plastic-number classifier trace, writes its
Lean artifact, and checks the C++ side of the certificate. The generated Lean
module is then checked with the matching Mathlib transport environment. The
general Lean acceptance boundary and safe invocation are documented in
[`docs/SAFE_LEAN_CHECK.md`](docs/SAFE_LEAN_CHECK.md).

## How long it has taken

There are two useful clocks for this project. The public Git development line
begins with the repository’s first commit on 2026-07-29 and reaches this front
page on 2026-08-08: roughly ten calendar days. The oldest preserved research
material in the archive is dated 2026-08-02, so the archival research record
covers roughly six calendar days to this snapshot. These are elapsed-history
measures, not claims of uninterrupted wall-clock labor. In that short span the
project moved from exact C++ research code and hand-recorded findings to
reusable derivation operations, typed reflection payloads, and multiple
kernel-checked theorem routes.

## License and scope

RavelMath is released to the public domain under the Unlicense; see
[`LICENSE`](LICENSE). It is an active research repository rather than a frozen
software product. Interfaces, theorem names, and research conclusions can
change as proofs expose better formulations. The documentation is intentionally
explicit about what is proved, what is exactly computed for finite data, what
is experimental, and what remains open.
