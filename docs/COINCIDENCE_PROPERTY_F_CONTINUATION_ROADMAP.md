# Coincidence / property (F) continuation roadmap

Written 2026-08-07, as a bridge document for resuming this thread once
the reflection-retrofit work (currently active: Sturm's theorem port
in `lean/sturm_sequence_root_counting.lean`, then wiring it and the
remaining ~25 findings into the `mathlib::reflection` C++ emitters,
see `docs/REFLECTION_RETROFIT_PLAN.md`) is done. Written so a future
session — Claude Code or the ChatGPT chain — can pick this up without
re-deriving where things stand from `DIRECTION_AND_OPEN_THREADS.md`'s
full length.

## Where this sits relative to the two live threads

The project currently has two active research threads and one active
engineering thread:

1. **Item A / Item B** (`docs/DIRECTION_AND_OPEN_THREADS.md`) — the
   general unimodular and non-unimodular Pisot conjecture work. This
   is the mathematical frontier.
2. **The reflection retrofit** (`docs/REFLECTION_RETROFIT_PLAN.md`) —
   making every existing finding's Lean proof a mechanical emission
   from concrete C++ evidence rather than hand-authored text. This is
   the trust/engineering frontier, currently consuming session time.
3. **This document** — the specific sub-question of strong coincidence
   and property (F), which is where Item A and Item B's open cases
   actually bottom out once the easier obstructions are ruled out.

## What's already closed (as of 2026-08-06/07, verify against
`docs/PROPERTY_F_UNCONDITIONAL_KERNEL_CHECKED_2026-08-06.md` and
`docs/DIRECTION_AND_OPEN_THREADS.md` before trusting this as current)

- The earlier “property (F) is now unconditionally established”
  headline is withdrawn. `RavelGenerated.zeroWalk_eq_zero_iff`
  (`lean/generated/property_f_zero_walk.lean`, kernel-checked, zero
  `sorry`) proves the real-analysis lemma that a nonnegative walk from
  zero cannot return to zero after a nonempty prefix. It is the
  load-bearing lemma that rules out mixed zero/nonzero SCCs; it does
  **not** rule out cycles consisting entirely of nonzero nodes, which
  are the paper's actual obstruction (Lemma 9.8). The corrected
  `adelic::check_property_f` therefore remains a meaningful finite
  check, conditional on its certified closure and bounds.
- The 13 confirmed unimodular `ρ_nc ≠ λ(G_B)` mismatches (σ₁, σ₂, +11
  random) all separately satisfy the classical Pisot conjecture
  (strong coincidence holds, property (F) holds) — that discrepancy is
  a refinement finding, not a counterexample to the classical
  conjecture. Settled, not open.
- Non-unimodular survey: 24/24 three-letter candidates ESTABLISHED
  (`app/sweep_nonunit_property_f.cpp`), all three known adelic bugs
  fixed. Four-letter survey (`sweep_nonunit_property_f_4letter.cpp`)
  found a real upstream bug in `spectral_invariants_general`'s
  Wielandt-deflation power iteration (underestimates the second
  eigenvalue's modulus when it's part of a dominant complex-conjugate
  pair) — check `docs/DIRECTION_AND_OPEN_THREADS.md` Item B for
  whether that's been fixed since 2026-08-01, since this document does
  not re-verify it.

## What's still open — the actual next targets

**Strong coincidence itself.** With property (F) closed off as an
obstruction, strong coincidence is now the *entire* remaining content
of the Pisot conjecture question in this framework. There is currently
no unconditional closure for it analogous to `zeroWalk_eq_zero_iff` —
check whether one has been attempted since this document was written
before assuming it's still open.

**Item A (unimodular, general case).** The n-bonacci structural proof
(free involution + exact polynomial divisibility + nilpotent cofactor)
is closed for n=2..7 but does NOT generalize past AR-exact substitutions
(Finding 5/6). Thread A3/A4 (AR-partial and non-AR cofactor mechanisms)
remain open with no known invariant — the originally proposed
beta-expansion-period invariant for non-AR was refuted. The
smooth-relaxation search (Thread A-relax/B2) is the standing plan for
generating new test candidates beyond random survey.

**Item B (non-unimodular).** Wider survey (4/5/6-letter, wider `|det|`,
wider `K_max`) is the concrete next step once the 4-letter blocker
above is confirmed fixed.

## Concrete pointers for resuming

- Full open-thread detail: `docs/DIRECTION_AND_OPEN_THREADS.md`
  (Item A starts ~line 30, Item B ~line 142 as of this writing — line
  numbers will drift, search for the header).
- Property (F) closure: `docs/PROPERTY_F_UNCONDITIONAL_KERNEL_CHECKED_2026-08-06.md`.
- Findings ledger (cite-by-number): `docs/FINDINGS_FOR_CITATION.md`.
- Lean coincidence infrastructure already in the library tier:
  `lean/constant_first_letter_forces_prefix_coincidence.lean`,
  `lean/first_letter_orbit_coincidence.lean`,
  `lean/zero_run_same_chain_coincidence.lean`,
  `lean/last_letter_orbit_coincidence.lean`,
  `lean/constant_last_letter_forces_suffix_coincidence.lean` — these
  are about symbolic-dynamics coincidence in the substitution/orbit
  sense used by the n-bonacci corridor's own proofs, not yet connected
  (as far as this document's author checked) to the strong-coincidence
  question above; worth a deliberate check for overlap before treating
  strong coincidence as needing entirely new machinery.

## Why this waits on the reflection retrofit

Per `docs/REFLECTION_RETROFIT_PLAN.md`'s corrected discipline (the
"citation pattern" mistake caught 2026-08-07): every new finding,
including whatever comes out of resuming this thread, needs to be
staged as concrete C++ evidence threaded into a typed trace, not a
hand-written Lean proof with a name-matched citation. Picking this
thread back up before the retrofit's patterns and tooling are solid
risks producing more of the exact citation-pattern debt that was just
caught and removed once already (property F's `zeroWalk` connection).
Finish the retrofit's emitter patterns first; resume this thread with
those patterns already load-bearing rather than retrofitting again
later. The first post-retrofit property-(F) task is now concrete:
retain and reflect the per-run finite graph evidence (node classes,
SCC count, and nonzero-cycle count) instead of citing the zero-walk
lemma by name. `PropertyFResult` now exposes those counters, and the
historical Fibonacci/rnd13/x²−2x−2 regression covers them. This is
evidence for a particular closed run, not an unconditional theorem.
The next increment now exports the actual bounded adjacency and canonical
Q(beta) node keys through `PropertyFGraph`; the Fibonacci graph topology is
rendered and kernel-checked as a finite Lean artifact. The Lean bridge still
checks topology/partition data rather than pretending string keys prove
algebraic equality; a future increment can replace those opaque keys with a
typed Q(beta) representation.
The graph certificate now also carries each node's exact rational coefficient
vector in the power basis, and the Fibonacci artifact renders those vectors
as Lean `List (Int × Nat)` data. The remaining algebraic bridge is to export
the edge digit/prefix data and characteristic polynomial so Lean can check
the recurrence on every serialized edge.
The export now carries each edge's exact digit coefficient vector as well;
the remaining step is to include the characteristic polynomial and a generic
Lean Q(beta) recurrence checker.
The exact characteristic polynomial is now exported too; only the generic
recurrence checker remains on this sub-route.
The graph export now also serializes the per-source successor rows and emits a
kernel-checked topology theorem for every digit edge. Thus each Q(beta)
recurrence corollary is tied to the actual graph target, rather than only to a
target coefficient literal selected by the renderer.
The recurrence bridge now uses the serialized successor lookup directly: each
edge theorem applies the generic Q(β) step to the source node and equates it to
the typed destination node selected from the graph row. This makes topology
and algebra one kernel-checked statement rather than two adjacent facts.
Successful Property-F graph staging now also rejects out-of-range SCC labels,
and the reflected graph theorem proves the nonzero-cycle component count is
exactly zero. The no-cycle conclusion is therefore carried as checked data,
not just a nonnegative counter invariant.
SCC labels are now checked against the component-size vector in both C++
staging and Lean: every component receives a kernel-checked filtered-label
count theorem. The finite SCC partition is therefore independently audited,
not inferred from its total size alone.
The graph node-letter map is now serialized too; staging rejects out-of-range
letters and Lean proves every exported letter index lies in the finite
substitution-alphabet domain.
The flattened edge list is independently checked for endpoint range as well,
so both the row-wise successor relation and its compact edge view are bounded
to the same finite node set.
The zero-state index list is now tied to the per-node zero predicate by a
kernel-checked membership theorem for every state, eliminating a second
unconnected representation of the zero/nonzero partition.
The per-instance Q(β) step now consumes the serialized beta-inverse matrix
through a typed rational lookup, rather than duplicating those entries as
unrelated literals in the recurrence definition.
The reflected matrix payload also carries kernel-checked row and row-width
theorems, preventing the rational lookup's defensive defaults from masking a
dimension mismatch.
The graph shape theorem now also ties the characteristic-polynomial length to
the beta-inverse matrix dimension, making the degree used by the Q(β) state
type explicit in the kernel artifact.
Every serialized beta-inverse denominator is also checked positive in Lean, so
the rational step cannot silently rely on division-by-zero behavior.

**2026-08-08 finite-classifier increment.** `check_strong_coincidence` now
returns a lexicographically indexed resolution profile for every unordered
letter pair: the exact first resolving depth, or `-1` when the bounded run
hits its depth/word-length cutoff. The reflection staging path carries this
profile into `StrongCoincidenceRunCertificate`, checks it against the exact
per-pair word witness before recording anything, and renders the two profiles
as equal finite Lean lists. This closes a trust-layer gap in the bounded
classifier: the aggregate `holds` bit is no longer the only exported result,
and a second search cannot silently disagree about which pair resolved when.
It remains finite evidence, not an unconditional strong-coincidence theorem.

**2026-08-08 closure-backed prefix classifier.** The existing exact
`CoincidenceClosure` is now exposed as
`check_prefix_coincidence_closure`: it deduplicates reachable
`(terminal-letter, landmark-vector)` states and compares them across every
letter pair without constructing `sigma^K`. On the non-AR control
`sigma(0)=12, sigma(1)=2, sigma(2)=0`, its finite prefix verdict agrees with
the word checker; a disjoint deterministic-cycle control remains explicitly
inconclusive. Matrix powers, landmark additions, and products now use checked
integer arithmetic and reject overflow. This is a prefix-half classifier,
not yet the full suffix-inclusive theorem, but it removes exponential word
materialization from the next strong-coincidence search path.

The live closure invocation now searches to depth 64 (with a one-million
outcome budget) instead of inheriting the legacy word check's depth-20 cap.
The worked example and `rnd13` remain at depths 4 and 1 respectively, with a
measured driver peak of only about 38 MB RSS; the bounded word fallback stays
at its original limits.

`TilingClassification` now exports the complete lexicographic per-pair
resolution-depth profile as well as the aggregate depth and engine flag. The
worked-example and `rnd13` shared-classifier tests assert those profiles
directly, so production classification no longer discards the finite evidence
needed to audit a strong-coincidence verdict.

Closure certificates now also export the concrete common terminal letter and
landmark vector for every resolved pair, plus a suffix-route bit for the full
reversed-substitution classifier. The generated Lean summaries check the
lengths of these state payloads alongside the depth and matrix data; this
removes the last depth-only gap in the closure reflection boundary.

Staging now independently materializes each resolved pair at its recorded
depth and rejects the certificate unless the exported terminal/vector state
occurs in both words (using prefix or suffix orientation as recorded). Thus
the efficient closure remains the producer, but reflected closure artifacts
are accepted only after an exact occurrence-level cross-check.

The prefix closure now also has a typed reflection certificate and renderer.
The staging operation derives the incidence matrix from the supplied images,
rejects invalid shapes, inconclusive cutoffs, and arithmetic overflow, and
records the exact pair profile only after closure succeeds. Its generated
finite Lean summary kernel-checks independently; it is explicitly a prefix
certificate and does not get promoted to a full strong-coincidence claim.

The same closure is now paired with the reversed substitution, using
`rev(σ^k(b)) = rev(σ)^k(b)` to turn its prefix outcomes into suffix outcomes.
`check_strong_coincidence_closure` merges the two exact per-pair profiles and
therefore gives a finite full strong-coincidence verdict for the
deterministic-chain class. Its typed reflection certificate and generated
Lean summary are separate from the prefix certificate; a cutoff in both
halves remains inconclusive.

The closure regression now covers the non-AR control, the certified genuine
multi-junction Pisot substitution `sigma_{1,1}`, and the non-unit `rnd13`
instance; in each case its exact pair-depth profile agrees with the direct
checker. This makes the deterministic-chain closure route the active engine
for the project's concrete Pisot corpus, while the general theorem remains
open.

The closure precondition is now exact rather than timeout-based: a
deterministic non-branching cycle, an empty non-junction image, or a
substitution with no branching letter is rejected as unsupported. There is
no arbitrary 10,000-step traversal guard, so a primitive expanding input can
be audited against the real finite-shape condition directly.

A fixed-seed cross-check now compares the full closure against direct word
materialization on 20 additional primitive three-letter substitutions; every
accepted sample agrees on the complete per-pair resolution-depth profile, and
none violates the closure shape precondition. This is regression evidence,
not a probabilistic theorem, but it materially broadens confidence in the
finite invariant.

The shared adelic `classify_tiling` entry point now uses the full closure as
its first strong-coincidence engine (with the existing bounded word checker
as an explicit fallback for unsupported or inconclusive shapes). The worked
example and `rnd13` drivers report `engine=exact closure`; the classification
regression asserts that both use this route. This turns the closure from a
parallel research certificate into the live finite classifier path.

The staging operation now reconstructs every exported source/target/digit
triple in the supplied `QBetaRing` and rejects any edge whose exact identity
`gamma' = beta⁻¹ (gamma + delta)` fails. Fibonacci includes a tampering
regression proving that invalid coefficient data is rejected before reflection.
For the quadratic Fibonacci characteristic polynomial, the renderer now also
emits a small generic `PropertyFQ2` arithmetic model and one `norm_num`
theorem per serialized edge; all eight edge identities kernel-check. Higher
degree fields remain an explicit follow-up rather than hidden unsupported
text generation.
The exact C++ recurrence validator is already degree-independent and now runs
on a Tribonacci degree-3 graph as well; only the Lean arithmetic renderer is
currently specialized to the quadratic case.
That specialization is now removed: the renderer generates a coefficient
structure and linear beta-inverse operator from the exported transition matrix
for any finite degree. A Tribonacci degree-3 artifact, including every edge
recurrence, kernel-checks successfully.
The graph certificate now also exports SCC labels, SCC sizes, and the
nonzero-cycle count. The generated finite Lean artifact checks that the SCC
size list sums to the node count, while the C++ staging validator checks the
partition and exact recurrence data before serialization.
The Lean artifact now additionally checks the finite obstruction predicate that
no edge between two nonzero nodes remains inside one SCC. Since every directed
cycle lies inside an SCC, this is the concrete decidable form of the corrected
property-(F) cycle criterion for the exported run.
The staging API now refuses inconclusive or failing runs on this proof path;
definitive failures remain reserved for a future explicit cycle-witness
certificate rather than being serialized as false theorems.
That witness path is now present: a failing closed graph reconstructs an
explicit directed cycle containing a nonzero node, validates every edge
against the actual adjacency, and renders a separate Lean counterexample
artifact. A synthetic self-loop exercises and kernel-checks the path; no
historical case is being mislabeled as a failure.
The witness is reconstructed from the real Tarjan SCC graph, not from a
reimplemented abstract automaton, so the failure branch preserves the same
instance-data discipline as the successful certificate path.

The production tiling classifier now records closure provenance explicitly:
whether the exact closure was attempted, whether it completed successfully,
and whether a fallback was caused by an inconclusive budget/overflow or an
unsupported substitution shape. The worked example and `rnd13` regression
cases assert the completed supported state, so downstream reports can
distinguish an exact closure verdict from the legacy bounded-word fallback.

The reflected prefix/full closure artifacts now also serialize the concrete
substitution images, rather than retaining them only in the in-memory typed
payload. Their Lean summaries check the image-list cardinality alongside the
pair profiles, terminal letters, landmark vectors, orientation flags, and
incidence matrix. This keeps the source substitution visible at the kernel
artifact boundary and prevents a summary from silently becoming detached from
the input instance.

The closure API itself now reconstructs and checks the incidence matrix from
the supplied images before exploring any states. A deliberately tampered
matrix is rejected by the closure regression, so this input-consistency guard
does not rely only on the production caller's construction discipline.

Closure witness staging now records the first and second occurrence positions
for every validated terminal/vector witness. Those positions are emitted in
the typed prefix/full payloads and their Lean summaries, so the reflected
artifact retains an occurrence-level witness rather than only an aggregate
state match.

The closure producer now also reconstructs one recursive junction-edge path
for each side of every resolved pair. Each path is checked against the same
memoized DAG outcome recurrence that produced the terminal/vector state, then
serialized in the typed prefix/full certificates. This keeps the next Lean
bridge structural: it can check edge choices and remaining-depth landmark
recurrences without expanding the exponentially growing substitution words.
The certificate now includes the corresponding finite junction-edge table
(source/target, jump size, child index, landmark, and forced chain), so path
indices in the Lean payload are meaningful structural references rather than
opaque integers.
The payload also records each side's post-deterministic-chain junction and
remaining depth, so a Lean recurrence checker has all of the state needed to
interpret a path without reconstructing the skipped singleton chain.
The generated artifact now has a small structural `sc_checkPath` interpreter
and a `by decide` theorem for each nontrivial exported path, checking edge
indices, junction continuity, jump-budget consumption, and terminal-chain
cutoffs. Full certificates serialize a separate reversed-edge table for
suffix witnesses, so the suffix route is not accidentally checked against the
original orientation.
Each path now also exports the exact per-edge weighted landmark vectors that
the C++ closure computed. Lean's generated `sc_sumVectors` theorem checks that
their finite sum is the recorded common landmark vector, giving the first
kernel-checked recurrence content beyond path shape.
The next bridge derives those same per-edge vectors in Lean from the serialized
incidence matrix: a compact row-major matrix/vector arithmetic interpreter
implements matrix powers and applies the power dictated by each edge's
remaining depth. A `sc_pathWeights` theorem now checks the entire structural
path against the recorded weighted vectors, so the certificate no longer needs
to treat the C++-computed edge weights as primitive data.
The companion `sc_sumVectors (sc_pathWeights ...)` theorem now derives the
common landmark vector itself from the matrix and path semantics. The recorded
edge vectors remain available for diagnostics, while the kernel-visible
coincidence equation is independently reconstructed.
Finally, each resolved pair now receives a direct theorem equating the two
matrix-derived path sums. This removes even the recorded common vector from
the logical dependency of the coincidence claim; the vector remains only as a
cross-check and readable diagnostic payload.
The edge table is now independently validated in Lean as well: for each edge,
the kernel reconstructs the prefix landmark, follows the forced singleton
chain, checks its jump length, and checks the terminal junction. Suffix tables
are checked against the reversed serialized images, so orientation is explicit
rather than an unchecked convention.
The closure metadata is also explicit in the Lean payload now: unresolved-pair
count, outcome budget, aggregate `holds`, and `inconclusive` status are emitted
and checked in the finite summary. A bounded positive result therefore cannot
be mistaken for an unqualified infinite theorem at the artifact boundary.
The incidence matrix is independently reconstructed from the serialized image
lists in Lean and compared elementwise with the matrix consumed by the path
weight recurrence. This closes the remaining input-consistency gap at the
kernel boundary.
The edge validator also requires both serialized endpoints to be actual
branching junctions, so deterministic letters cannot enter the junction graph
silently through malformed metadata.
The regression now renders and kernel-checks the genuine two-junction
`sigma_{1,1}` certificate as well as the single-junction control, exercising the
forced-letter route and suffix orientation in the same artifact path.
