# Direction and open threads

What this project is trying to do, and the concrete unfinished work.
Each item has a status and a pointer to where the work would start.
This document reflects the current state of the codebase and current
understanding only.

## The current direction: two items, complementary

The project is now pointed at **catching up to and going past
Nakaishi's 2016/2023-withdrawn claim** about the Pisot conjecture
for the unimodular case, and at the genuinely-open non-unimodular
Pisot problem that the unimodular case doesn't cover. These are
Item A and Item B respectively; they share tooling and inform
each other (the smooth-relaxation search is a generator for both;
the contact-boundary / BP-rho comparison machinery is the inference
engine for both).

The classical 2-letter case is already proven (Barge–Diamond 2002
+ Hollander–Solomyak 2003) and the β-substitution case is already
proven (Barge 2015/2018). The n-bonacci family — a strict subset
of the β-substitutions — has a complete structural proof here
(thread A's prior work). What remains is the general irreducible
unimodular Pisot case, plus the non-unimodular extension.

This document is about what's still open in those two items. The
closed-out work (4-letter `rnd13` TILES result, the non-unit
adelic bugs, the ZpInt precision refactor, the multi-prime bound)
is summarized inline below where it's relevant to Item A or Item B.

## Item A — Unimodular Pisot, the general case

**What's known (closed-out)**:

- The n-bonacci (simple Parry) family `σ(i) = 1(i+1) for i<n, σ(n) = 1`
  satisfies `ρ_nc = λ(G_B)` **PROVEN** for n=2..6 (Fibonacci,
  Tribonacci, Tetrabonacci, Pentanacci, Hexanacci), with a structural
  proof: a
  fixed-point-free involution φ on `G_B`'s dominant recurrent core,
  whose quotient's exact integer characteristic polynomial divides
  the balanced-pair automaton core's, with the cofactor's
  dominant root provably smaller than `λ(G_B)`. The four-step chain
  (free involution φ; exact polynomial divisibility; pure-`x^k`
  cofactor; ruler sequence explains why `E'` is nilpotent) is closed
  with zero exceptions n=2..7. See `docs/FINDINGS_FOR_CITATION.md`
  Finding 4 for the one-sentence proof.
- The spectral-separation half of the n-bonacci argument is proven
  in general (Zaslavsky 1982 signed-graph theorem + the classical
  strict Perron-Frobenius inequality for unbalanced signed graphs).
- The nilpotent-cofactor finding: `charpoly(Q_sym_GB) /
  charpoly(Q_sym_BP) = x^k` exactly, n=3,4,5. See
  `app/gb_bp_matrix_equality.cpp`.
- σ₁ (`σ(0)=(0,0,0,1)`, `σ(1)=(0,0,2)`, `σ(2)=(0)`) and σ₂ are
  bit-exactly PROVEN to be counterexamples to the conjecture at
  the unimodular-Pisot level (not just observed). Verified via
  `mathlib::compare_perron_roots_exact`.
- The structural reason the n-bonacci match works is now understood
  all the way to the *cofactor's nilpotence* (the ruler-sequence
  permutation-plus-correction mechanism, `E'` always nilpotent,
  index exactly `n-2` for n=3..7).
- The 2-letter Pisot case (Barge–Diamond 2002 + Hollander–Solomyak
  2003) and the β-substitution case (Barge 2015/2018, *Ergodic
  Theory Dynam. Systems*) are both **already proven in the
  literature**. The n-bonacci work is a structural explanation of
  a particular β-substitution case, not a new result against the
  classical Pisot conjecture.
- The dominant-core window target is now an exact finite certificate for
  n=3..7, not a floating survey. `make item1_per_pair_check` verifies exact
  left-covector heights, the sparse argmax witness in every face-pair group,
  every positive residual margin, the global worst `2/beta-1`, and exact
  transition-record agreement on core nodes. Core sizes are
  14/46/108/210/362; all failure counts are zero.

**Current general-n margin seam (sharpened 2026-08-02).** The algebraic half
is uniform: n-bonacci parent decompositions give prefix height 0 for letter 0
and 1 for every other letter, hence every forward edge obeys

`t' = (t + delta)/beta`, `delta=[j>=1]-[i>=1] in {-1,0,1}`.

The candidate global worst `e_0-e_{n-1}` already has exact positive margin
`2/beta_n-1`. The finite n=3..7 cores now also show, exactly, that every
face-pair maximum is the sparse row witness. What remains is not numerical
root or transition control: it is to classify the delta words of arbitrary-n
dominant recurrent cycles (or give an invariant trapping interval for them)
strongly enough to prove that no recurrent height exceeds its row witness.
Equivariance under `[i,x,j] -> [j,-x,i]` is already algebraic for all n; use it
to quotient the cycle classification before attempting raw-state exhaustion.

**What's open (Item A research targets) — SUPERSEDED, corrected
2026-08-01**: A1 and A2 below are answered, not open — this section
was never updated after the `Decomposition classification` section
further down (line ~348) resolved them. Read that section, and
`FINDINGS_FOR_CITATION.md` Finding 5, for the actual current state:
`app/probe_a1_a2_unimodular.cpp` + `app/tabulate_pisot_properties.cpp`
already ran A1 and A2 across 10 unimodular Pisot substitutions and
found the n-bonacci mechanism does NOT generalize — Pisot
substitutions split into 3 structural classes (constant-factor +
Arnoux-Rauzy complexity), and only AR-exact (n-bonacci) has the full
x^k cofactor + exact involution. The live open threads are named
**Thread A3** (find AR-partial's cofactor mechanism — the
small-group-action branch is already closed negatively, per Finding 6
and `app/class_ii_symmetry_probe.cpp`'s rigidity result), **Thread
A4** (non-AR structure — its originally proposed beta-expansion-
period invariant is refuted, see Finding 6.5; still open), and
**Thread A5** (extend the 10-candidate data set: 4-letter, β-substitutions,
a wider random survey) — kept below in their original numbering only
for historical/citation purposes:

**(A1, ANSWERED — see above) Generalize the n-bonacci nilpotent-cofactor
finding to all unimodular Pisot substitutions.** The pure-`x^k` cofactor
structure (`charpoly(Q_sym_GB) / charpoly(Q_sym_BP) = x^k`, exact) is
verified for n-bonacci at n=3,4,5, and does NOT persist beyond it —
specific to AR-exact only, per Finding 5.

**(A2, ANSWERED — see above) Apply the involution machinery to
non-n-bonacci unimodular Pisot substitutions.** The n-bonacci
involution φ is EXACT (14/14) for AR-exact only; PARTIAL (6/11 to
30/44) for AR-partial and non-AR, per Finding 5, and confirmed
independently rigid (automorphism group = identity only) for
σ_{1,1}/σ_{2,1}/σ_{3,1} via exact digraph-automorphism enumeration
(Finding 6).

**(A-relax, cross-listed as B2 below) Build a smooth-relaxation
search** — the natural generator of new candidates to test Thread A5
on, beyond the random-survey candidates already tested. A closed-form
differentiable objective (the closed-form `spectral_invariants_3x3`
Cardano solver) plus a Pisot-preserving gradient step on the matrix
entries.

**(A-settled) Check the classical Pisot conjecture via the project's
machinery on the same substitutions where the unimodular
`ρ_nc = λ(G_B)` conjecture fails (σ₁, σ₂, and 11 other random
unimodular mismatches in the project's 87-candidate batch).** Per
The non-unit survey found that every one of
the 13 confirmed-unequal cases (σ₁, σ₂, and 11 random
unimodular 3-letter) **does** satisfy the classical Pisot
conjecture (strong coincidence HOLDS and geometric property (F)
HOLDS in every case). The `ρ_nc ≠ λ(G_B)` discrepancy is a
*refinement* of the Pisot conjecture, not the Pisot conjecture
itself. This is a settled point; mentioned here for completeness.

## Item B — Non-unimodular Pisot, beyond the 24/24 survey

**What's known (closed-out)**:

- All three non-unit adelic bugs (the multi-prime crash in
  `make_combined_padic_bound`, the false-negative
  `qp_local_is_integral`, the geometric-mean `spectral_invariants_3x3`)
  are fixed:
  `make_combined_padic_bound`'s multi-prime AND,
  `qp_local_is_integral`'s cleared-denominator, and
  `spectral_invariants_3x3`'s max-modulus. The `ZpInt` precision
  refactor (precision == digits.size() enforced structurally,
  `set_precision`/`extend_to`/`truncate_to` methods) closed the
  heap-buffer-overflow that was intermittently crashing
  `make_combined_padic_bound`. ASan run clean.
- A historical non-unimodular Pisot survey (`app/sweep_nonunit_property_f.cpp`,
  24 candidates, seed=11) gave **24/24 ESTABLISHED, 0
  INCONCLUSIVE, 0 FAILED, 0 skipped** under the then-current local-order
  path. The current trust-aware classifier preserves those finite graph
  results but only promotes p-maximal, Dedekind-cross-checked orders; the
  non-maximal cases are now reported as exploratory/inconclusive until their
  prime-ideal data is transported into the maximal order.
  Corrected here (2026-08-01, re-ran the app directly rather than
  trusting this line): the batch is **3-letter** (cubic charpolys,
  explicit 3x3 determinant in the source), not 4x4 as previously
  stated, and `|det|` **varies** across candidates (2, 3, or both --
  e.g. `2`, `3`, `2 3` appear as "primes dividing det" in the actual
  output), not fixed at 2. `K_max=3`, `seed=11`, `alphabet_size=3` are
  the only fixed parameters, matching the source's own `main()`.
- The 39-/87-candidate non-unimodular survey's "11/87 differ by
  10-40%" row from Finding 1 was flagged as provisional
  (`FINDINGS_FOR_CITATION.md` line 199-207) because the pipeline
  used `extract_recurrent_core` (largest-by-node-count), not
  `extract_dominant_recurrent_core`. A re-audit driver
  (`app/reaudit_gb_dominant_scc.cpp`) re-runs the 4-letter
  non-unimodular survey with the dominant-SCC extractor and compares
  the three λ values (whole-matrix, largest-SCC, dominant-SCC). A
  correctness audit found a stale 3×3 determinant filter and an
  unassigned SCC-split flag. A further audit found the same stale 3×3
  determinant inside the candidate generator; both locations are now
  fixed. The full generator-corrected batch produced 10 clean candidates
  out of 20 determinant-qualified candidates, with exact λ agreement and
  no SCC membership splits; ten cap hits remain inconclusive. The
  historical row remains provisional because the original 11/87 matrices
  were not persisted and cannot be replayed exactly.

**What's open (Item B research targets, in priority order)**:

**(B1) Wider non-unimodular survey.** The 24/24 batch is **3-letter**
(corrected 2026-08-01, was wrongly stated as 4-letter -- see the
"What's known" correction above), `|det|` already varying (2, 3, or
both) rather than fixed, `K_max=3`, `seed=11`. The natural extensions
are: wider alphabet size (4-letter, 5-letter, 6-letter), wider `|det|`
beyond what random sampling already turns up, and wider `K_max` (5, 7).

**4-letter attempted (2026-08-01), and the "already handles all these"
assumption was wrong.** `app/sweep_nonunit_property_f_4letter.cpp`
generalizes the 3-letter driver (general `adelic::integer_determinant`
instead of a hand-expanded 3x3 formula; `adelic::
is_irreducible_over_q_via_small_primes` -- a real, rigorous,
any-degree replacement for the 3-letter driver's rational-root-only
check, which is correct at degree 3 but not degree 4). Running it (8
candidates, reduced certify/rho budgets for tractability): of 7
irreducible non-unit quartic candidates, only **1 reached a verdict**
(ESTABLISHED). The other 6 hit real exceptions -- one genuinely
upstream, one still uninvestigated:

- **4 cases, "secondary root modulus >= 1" in `check_property_f`.**
  This was NOT a `check_property_f` precision bug -- traced further
  (2026-08-01, same session) to the actual root cause, one layer up:
  `include/ravel/spectral.hpp`'s `spectral_invariants_general` (the
  `n>=4` matrix classifier `wide_random_pisot_survey` uses to decide
  which candidates are Pisot in the first place) computed the
  second-largest eigenvalue's modulus via Wielandt-deflation power
  iteration, which silently UNDERESTIMATED it when that eigenvalue was
  part of a genuinely dominant complex-conjugate pair (real-vector
  power iteration on a matrix with a dominant complex pair never
  settles to a fixed direction -- the norm-growth ratio oscillates with
  the pair's argument, so reading it off at a fixed iteration count can
  land on an arbitrary, wrong value). Confirmed directly for one such
  candidate (`rndW3_1`, matrix `[[2,1,2,1],[3,2,1,0],[2,1,0,0],
  [0,3,2,2]]`): the old code reported `beta2=0.926` (passing the `<1`
  Pisot filter), but an independent, precision-verified computation
  (`adelic::find_roots_durand_kerner`, identical results at 200 vs 2000
  iterations and 200 vs 500 bits) found the TRUE secondary pair has
  modulus `1.376`. So `check_property_f`'s exception was doing its job
  correctly, catching a non-Pisot matrix that should never have been
  admitted.

  **FIXED (2026-08-01, same session).** Replaced the naive power-
  iteration norm-ratio with a Rayleigh-Ritz step: still use real power
  iteration to steer the iterate `x` into `M'`'s dominant invariant
  subspace (this DOES converge in direction even for a complex pair,
  since that pair's combined real invariant subspace is 2-dimensional
  and genuinely attracting), then build an orthonormal basis for
  `span{x, M'x}`, project `M'` onto that 2D subspace as an explicit 2x2
  matrix, and solve ITS eigenvalues directly via the quadratic formula
  -- exact once the subspace has converged, regardless of any ongoing
  phase rotation within it, and correctly returns a complex-conjugate
  pair's modulus (`sqrt(det)`, since `det` = product of eigenvalues for
  any 2x2 matrix) when that's what's actually dominant. Verified: the
  fix reproduces `beta2=1.376` exactly for the `rndW3_1` matrix above
  (now correctly `pisot=false`), and reproduces every PREVIOUSLY
  correct case unchanged (Tribonacci, Tetrabonacci, `rnd13`, a 4-cycle
  permutation matrix) -- `tests/spectral_general_test.cpp`, 12/12,
  `make check` clean including the Lua survey suite. Since this
  function is core, widely-used infrastructure (the Pisot filter for
  every `n>=4` matrix this project's survey tools generate, not just
  the 4-letter experiment above), this is a real correctness fix to
  the project, not just an unblocking of one probe app.
- **The remaining `local_polynomial_cofactor` case is now closed
  (2026-08-01).** `rndW3_5` has two non-simple prime ideals above
  `p=3`, one `(e=1,f=2)` and one `(e=2,f=1)`. The old implementation
  bundled both after dividing out only simple factors and correctly
  threw on the resulting degree mismatch. The replacement identifies
  each target irreducible factor (with its multiplicity) mod `p` and
  Hensel-lifts it against the coprime product of all remaining factors.
  A first quadratic implementation exposed an invalid assumption:
  extended Euclid over `Z/(p^k)[x]` can encounter a nonunit leading
  coefficient because the coefficient ring is not a field. The final
  implementation uses the valid `F_p[x]` Bezout certificate and lifts
  one p-adic digit at a time. Focused tests cover the published
  correction assignment, two independent repeated lifts, and a
  synthetic quartic with the exact two-non-simple-factor shape.

So the contact-boundary pipeline, combined p-adic bound, and spectral
filter did NOT fully handle degree 4 when first tried -- not just a
parameter change as previously assumed here. Both root causes are now
closed. One (the
spectral filter's unsoundness for complex dominant secondary pairs at
`n>=4`) was a real, silent bug. The other was the bounded cofactor
construction above. **After both fixes, re-running the deterministic
4-letter batch gives 7/7 ESTABLISHED, zero skipped**; `rndW3_5` now
passes Property (F) with 421 nodes.
**A larger follow-up run confirms this holds at scale** (target=20,
same reduced certify/rho budgets, 2026-08-01): 17 candidates checked,
**14 ESTABLISHED, 2 INCONCLUSIVE (property-F budget exhausted -- a
resource cap, not a failure), 1 SKIPPED under the then-unfixed cofactor
limitation, zero unexplained exceptions.** That larger sample has not
yet been rerun after the Hensel fix; the deterministic regression is
the present ground-truth closure. The smooth-relaxation search (B2) is
a smarter candidate generator that targets Pisot-preserving
perturbations specifically now that the pipeline covers the known
multi-non-simple-ideal case.

**Wider rerun after both fixes (2026-08-09).** The 4-letter driver now
accepts explicit `--target`, `--K`, `--seed`, and bounded certify/rho/trial
arguments while retaining the historical defaults. Repeating the target-20,
`K_max=3`, seed-11 run produced 17 non-unit candidates. Before the trust
guard was tightened, all 17 completed the exploratory finite pipeline; two
reported non-maximal Dedekind-order diagnostics. The current classifier keeps
those two as **INCONCLUSIVE** until a maximal-order factorization is available,
leaving 15 trusted established cases, 2 inconclusive, 0 failed, and 0 skipped.
The largest finite property-(F) graph had 841,057 nodes; the corrected rerun's
peak observed RSS was about 0.65 GB, comfortably below the 10 GB machine
limit.

**Wider substitution-length probe (2026-08-09).** A bounded `K_max=5`
sample (target=8, seed=17, 1,000-pair/10,000-length certify and rho caps)
initially gave 7 established cases and one Property-F budget cutoff at the
default 1,000,000 nodes. The driver now exposes `--property-f-budget` and
`--only`; rerunning the deterministic outlier `rndW5_6` at 6,000,000 nodes
resolved its exploratory graph at 3,799,168 nodes. Two cases (`rndW5_5`,
`rndW5_6`) have non-maximal-order diagnostics, so the current trust-aware
classification records 6 trusted established and 2 inconclusive cases; the
finite graph result remains useful evidence but is not promoted to a theorem
until the order issue is resolved. Observed RSS was around 3.6 GB before the
hash-table optimization.

**(B2) Smooth-relaxation search.** The natural way to generate
new Item A and Item B candidates, replacing pure random sampling
with gradient-based Pisot-preserving mutation. The natural
starting point: a small `include/ravel/dual.hpp` (forward-mode
automatic-differentiation helper), use `spectral_invariants_3x3`
(closed-form Cardano) as the differentiable objective, and a
Pisot-preserving gradient step that perturbs the matrix in a way
that keeps the dominant eigenvalue > 1 and all others < 1. (A prior
version of this note pointed to `docs/RESEARCH_VECTORS.md` vector 2
for the full design; that file does not exist and has no git history
in this repository -- the full design was never written down beyond
this paragraph, checked 2026-07-31.)

**(B3) Apply the Item A machinery to the non-unimodular case as
it matures.** The involution φ, the `extract_dominant_recurrent_core`
selection, the nilpotent-cofactor finding — all of these are
written for the contact-boundary graph `G_B` specifically, and all
of them work on the non-unimodular case's `G_B` too. As Item A
matures, ask: do the n-bonacci involution results give structural
information about the non-unimodular tilers (only `rnd13` in the
24/24 batch)?

**(B4) Re-audit the historical 39-/87-candidate row.** Per
`FINDINGS_FOR_CITATION.md` line 199-207: the 11/87 differ by
10-40% row used `extract_recurrent_core` (largest-by-node-count),
not the dominant-SCC extractor. The pilot in
`app/reaudit_gb_dominant_scc.cpp` suggests the λ values are
unchanged for the surveyed batch — the original 11/87 stands.
If this is confirmed for the full 11, the 10/11 "λ > β"
non-tiler pattern in the original report holds. (Lower-priority
than the wider survey in (B1) because the original 11/87 row
is already published and the re-audit is essentially a
verification, not a new finding.)

## Cross-cutting tooling

Several pieces of machinery are shared between the two items.

**`ContactBoundaryLimits`** (this session's new struct,
`include/ravel/contact_boundary.hpp`) — the implementing layer's
knobs for the contact-boundary pipeline. Default-constructed
reproduces the historical 20000/60000/5000/50000/8 caps (the
`bp_rho_pairs`, `bp_rho_len`, `closure_cap`, `corona_cap`, and
`max_corona_rounds` respectively). For Item A's wider survey,
the defaults are fine. For Item B's smooth-relaxation search
(B2), the tightest viable caps (e.g. 4000/10000/1000/10000/8)
are appropriate — speed matters more than exactness at search
time. The report records `limits_used` in the `ContactBoundaryReport`
struct, so a re-audit driver can tell at a glance whether a
`closure_stopped_early` or `corona_capped` flag is the pipeline
hitting its cap or the run simply being at the cap without
hitting it.

**`extract_dominant_recurrent_core`** (the only correct
recurrent-SCC extractor) — the function that identifies the
SCC whose Perron root is the graph's dominant eigenvalue, NOT
the largest-by-node-count one. Use this whenever the graph has
more than one nontrivial recurrent SCC, which is "always" for
`G_B`-scale graphs (Hexanacci's `G_B` has 7 recurrent SCCs,
with the dominant one being the 210-node one, NOT the 221-node
largest). `extract_recurrent_core` is documented in its own
header comment to give wrong answers for graphs of comparable
size to Hexanacci's; the G_B re-audit driver
(`app/reaudit_gb_dominant_scc.cpp`) is the check for this distinction;
the withdrawn 10/15 run cannot serve as evidence; the fresh 10/20 run is
replacement for the full historical re-audit.

**ASan (AddressSanitizer)** — the safety net for the precision-
vs-digits.size() invariant that was the source of the recent
heap-buffer-overflow. The bug was a "non-deterministic function
detected" symptom (a function that should be deterministic was
returning different values across runs on the same input, caused
by reading past the end of a `digits` vector). Run all C++ test
binaries with `-fsanitize=address` at the start of the session
to confirm a clean baseline. Re-run whenever working in
`include/adelic/padic.hpp`, `local_field.hpp`, or
`ideal_arithmetic.hpp` (the three places where mismatched-array
indexing can still happen). **Live use**: when traversing
potentially mismatched arrays together, and an indeterministic
function is detected, run under ASan to localize the bad
access.

## Layout history

If a status in this document disagrees with the test count in
`make check`, the tests are the source of truth — update the
document to match.

## Decomposition classification (Item A2 / new Item A5)

The probe of σ_{a,b}-family, σ_1, σ_2, and Tribonacci at
`app/probe_a1_a2_unimodular.cpp` (with the per-candidate
tabulation at `app/tabulate_pisot_properties.cpp`) revealed that
Pisot substitutions fall into 3 structural classes, delineated
by (a) constant factor at position 0, and (b) the Arnoux-Rauzy
factor-complexity condition f(n) = 2n+1 for a 3-letter alphabet.

See `docs/FINDINGS_FOR_CITATION.md` Finding 5 for the full table.

The previous framing of "Item A (catching up to Nakaishi) and
Item B (non-unimodular)" is now bracketed under the decomposition
classification. The two new threads of work are:

### Thread A3 — find the AR-partial cofactor

σ_{a,b} a ≥ 1 has constant factor at position 0 and Arnoux-Rauzy
factor complexity (orbit of 0 is Sturmian-like), but the n-bonacci
involution [i,x,j]↔[j,-x,i] is only partial and the A1 nilpotent
cofactor is not x^k. The open question: is there a different
cofactor mechanism replaces the n-bonacci group quotient.

**Small-group-action branch closed negatively.** Exact weighted
digraph automorphism enumeration in
`app/class_ii_symmetry_probe.cpp` finds that the dominant cores for
σ_{1,1}, σ_{2,1}, and σ_{3,1} are rigid: their automorphism groups
contain only the identity. Directed weighted color refinement already
separates every core vertex into a singleton invariant class, giving
a certificate of rigidity independent of the backtracking enumerator.
Both the report adjacency and the direct-transition adjacency agree.
The Tribonacci control has exactly the expected two automorphisms
(identity plus seven transpositions).

Therefore the Class-II replacement cannot be a different Z/2, Z/3,
or other nontrivial automorphism group acting on the dominant core.
The natural non-invertible quotient branch is negative too: the
coarsest outgoing-equitable quotients of these three Class-II cores
have characteristic polynomials exactly coprime to their corresponding
balanced-pair quotients. The Tribonacci control instead gives two
four-dimensional quotients with a shared degree-four characteristic
polynomial. Thus a Class-II mechanism must use a different auxiliary
object (for example a return-word extension or induced substitution),
not an automorphism or the natural equitable quotient of `G_B`.

### Thread A4 — a non-AR structure

σ_{0,1} has no constant factor and not Arnoux-Rauzy (f(2) = 7, not
5). The orbit of 0 has a non-Sturmian factor complexity. The first
return word of letter 0 is empty — σ^k(0) never has "0, 0" as
consecutive characters, because σ(0) = (1, 2) and the orbit always
crosses through 1, 2 before returning. The A1 cofactor is not x^k
(not Arnoux-Rauzy), and A2 involution is partial (30/44).

**First step attempted and refuted (2026-08-01) — see
`FINDINGS_FOR_CITATION.md` Finding 6.5.** The original proposal here
("compute the Pisot continued fraction of 1/β_{0,1}... periodic for
all Pisot numbers, period length distinguishes AR-exact/AR-partial/non-AR") was
mathematically wrong as stated: by Lagrange's theorem, a classical
continued fraction is eventually periodic iff the number is a
quadratic irrational, and β_{0,1} is cubic. The real nearby theorem
(Bertrand/Schmidt) concerns the Rényi/Parry beta-expansion `d_β(1)`,
not a continued fraction. Computed `d_β(1)` exactly for 12
candidates: termination turns out NOT to distinguish the classes
(AR-exact, most of AR-partial, and the tested non-AR example all
terminate). Thread A4 is still open; this specific invariant doesn't
work. A different approach is needed.

**Positive next invariant (2026-08-01) — exact finite extension-graph
profile.** `app/thread_a4_extension_graph_probe.cpp` replaces the
aggregate factor count with each factor's left/right extension graph.
On orbit prefixes of at least 525456 symbols, a direct extension-graph
pass through factor length 64 and a suffix-automaton stability pass
through length 500000 (strengthened 2026-08-02 from an initial 60000)
on a separately enlarged, 35676949-symbol orbit prefix find that
`σ_{0,1}` has `p(n)=5n-5` for every
checked `n>=4`, with exactly five left-special and three right-special
factors. The AR-complexity controls all retain `p(n)=2n+1`, but now
split structurally: Tribonacci and `σ_2` use one ternary special factor
on each side; `σ_{a,1}` uses two binary left-special factors and one
ternary right-special factor; `σ_1` reverses that orientation. This is
the first positive Thread-A4 structure after the two refuted scalar
invariants: non-AR means several persistent local branching seams, not
merely a larger complexity number. Still open are an all-`n` proof and
the precise theorem identifying these symbolic extension branches with
local junction types of the stepped-hyperplane tiling. Citable as
`docs/FINDINGS_FOR_CITATION.md` Finding 6.7 (original result) and
Finding 14 (2026-08-02 range strengthening to `n=500000`).

Klouda--Pelantová (2009, `references.bib` key
`KloudaPelantova2009`, locally preserved and checksummed under the
Git-excluded `refs/FullText/` archive) gives a relevant route from
special-factor decompositions to infinite left-special branches. A
necessary caution exposed by inspecting that machinery is that five
left-special factors at each finite length do not by themselves imply
five infinite left-special branches: some may belong to recursively
growing finite maximal-special families. The next proof must establish
the actual predecessor/branch decomposition for this substitution.

#### A4 hypothesis sheet: guesses worth trying to break

These are deliberately conjectural. Each has a cheap falsifier and a useful
interpretation if it fails.

**H1 — Persistent seams form the missing base automaton.** The infinite
extension graph of a non-AR language should have a finite recurrent seam core;
the contact-boundary transport should be a skew product over that core, with
prefix/abelian labels as the fibre. A falsifier is an all-length predecessor
decomposition of `sigma_{0,1}` that produces an unbounded number of genuinely
new seam types. Then the missing base is a sofic/induced-substitution tower,
not a larger contact BFS.

**H2 — Complexity slope counts transport channels.** The increment
`p(n+1)-p(n)` is total right-branch excess, and after recurrent quotienting it
should equal the number of independent seam channels in the contact grammar.
This predicts five channels for `sigma_{0,1}` (slope `5`) and two aggregate
channels for the AR-complexity controls (slope `2`), even when left/right
orientation differs. Falsify it by constructing the exact seam grammar and
finding a different channel rank. Then geometric labels need return-word
displacement or eigenvector height, not extension valence alone.

**H3 — Large non-AR clouds are transient skew-product growth.** Once the seam
core is fixed, nonzero Property-F states should be transient fibres over it;
their size should be governed by contraction margins and fibre-word length,
while recurrent SCCs are controlled by the seam core. A nonzero recurrent SCC,
or a persistent growth mode absent from the seam core, would identify genuine
holonomy rather than harmless transport inflation.

**H4 — Predecessor depth supplies the rank proof.** The predecessor
decomposition of special factors should provide a well-founded height that
strictly decreases on every nonzero fibre return, except terminal permutation
channels. A cycle preserving every candidate height while Property F closes
would show that the rank must be signed or valuation-weighted, pointing
directly to the adelic holonomy potential needed for the non-unit theorem.

**H5 — Coincidence depth is seam diameter plus a bounded correction.** The
depth-13 resolution of `sigma_{0,1}` may be the diameter of its seam core plus
a short transport correction, rather than an unrelated word-search accident.
An unbounded discrepancy across a family would force separate structural
proofs for strong coincidence and Property-F recurrence.

The first experiment should attack H1 and H2 together: prove the all-length
predecessor decomposition for `sigma_{0,1}`, build its finite seam grammar,
and compare its channel rank with the existing contact-boundary generator
intertwiner. This is small enough to fail quickly and informative enough that
each failure selects the next representation layer.

**Initial break-it pass (2026-08-09).** A finite predecessor scan already
breaks the naive nested-branch model: for the stable `sigma_{0,1}` window,
none of the left-special factors at lengths 8, 16, 32, or 64 has a one-letter
left extension which is left-special at the next length. The five branches
therefore have to be organised by return words (or an induced/sofic tower),
not by five independent suffix chains. This is a useful failure of H1's
cheapest implementation, not evidence against a finite recurrent core.

The first small non-unit sweep gives a parallel warning. Among nine genuine
quartic non-unit candidates, five reached a validated Property-F closure and
four exhausted the deliberately tiny node budget. The latter four all had a
non-maximal local order or an untrusted order cross-check; their status is
therefore arithmetic certification failure, not a detected geometric
obstruction. This supports a refinement of H3/H4: separate the finite real
transport core from local-order/valuation bookkeeping before interpreting a
large cloud as holonomy. A counterexample would be a maximal-order,
cross-checked candidate whose nonzero recurrent SCC survives after the
valuation fibres are quotiented; that would be the first genuinely new
 non-unit obstruction target.

**Return-word probe (2026-08-09).** The existing return-substitution
constructor works on the minimal non-unit rule `0->001, 1->00`: it finds two
return words, three phase states, and one recurrent phase SCC. In contrast,
it rejects `sigma_{0,1}` at marker `0`, because an image of a return word does
not begin at the marker. Thus the bare marker is not recognizable enough for
the ordinary derived substitution. This falsifies the cheapest version of
H1, but points to the repair: collar the marker by phase/offset (or use a
two-sided return scheme) before quotienting. The non-unit experiment also
supports keeping valuation fibres separate from this collared seam core: the
real base closes in three phase states before any p-adic label is attached.

The expanded probe gives a concrete growth pattern. `sigma_{0,1}` needs collar
radius 2 and has 45 observed collared phase states (16 uncollared states,
with 6 transition conflicts). The left-proper controls `sigma_{a,1}` for
`1<=a<=5` are already single-valued at radius 0, with 6 phase states; their
radius-1 counts are 7. This suggests that the exceptional non-AR layer is a
finite recognizability-defect collar, not an intrinsically larger alphabet.
The next falsifier is a family whose minimal conflict-free collar radius is
unbounded; that would require a genuine sofic/S-adic tower rather than a
finite collared groupoid.

**Run-length hypothesis (revised 2026-08-09).** The first probe measured only
source-side collars: for `sigma_{0,b}` it found radii `1,2,3,4,5` for
`b=1,...,5`. The stronger endomorphism test collars both source and target
phase states, and therefore falsifies the claim that those radii already
close the groupoid. For example, `sigma_{0,1}` still has five transition
conflicts at radius 2; a larger finite collar is needed. The robust idea that
survives is weaker and more useful: the required radius is controlled by the
run-length recognizability defect in `sigma_{0,b}(0)=1^b2`, and appears to
grow only linearly in `b` in the tested range. A stable zero-conflict radius
must be established separately for each member (or bounded symbolically).

This is a productive failure: the missing information is not another return
letter, but the target-side collar transported through the substitution. A
family with no stable zero-conflict radius, or superlinear state growth after
both-side collaring, would falsify the harmless recognizability interpretation
and reopen H3 as a genuine holonomy problem.

The corrected probe now constructs the full directed phase graph. It finds
zero-conflict radii `5,9,9,11,13` for `sigma_{0,b}`, `b=1,...,5`; each has a
single large recurrent SCC (the small extra SCCs at `b=5` are boundary
artefacts in the finite prefix). The left-proper controls remain one SCC at
radius 0. Thus the current evidence is stronger than mere finite closure:
the collared layer itself is recurrent and finite, with no observed
nontrivial transient obstruction. The remaining proof task is to replace the
prefix-dependent radius list by a symbolic recognizability bound and to show
that the SCC is independent of the orbit margin.

**Margin and bound probe (2026-08-09).** Repeating the full two-sided test at
orbit prefixes of 100k, 200k, and 500k symbols gives identical conflict
profiles for `b=1,2,3`; the zero-conflict radii `5,9,9` are therefore not a
boundary artifact at this scale. Extending to `b=6,7,8` finds zero conflicts
at radii `17,19,21`, respectively. This supports the provisional uniform
bound `R_b <= 2*b + 5` for the full collared phase map (not a theorem yet).
It is the sort of bound expected from decoding the `1^b2` block on both sides
of a cut. A counterexample with `R_b > 2*b+5`, or a margin-dependent
zero-conflict radius, would force a different recognizability mechanism and
keep the non-AR plank open.

**Marker-power hypothesis (2026-08-09).** For every tested `sigma_{0,b}`, the
third power is marker-proper at `0`: `sigma_{0,b}^3(0)` begins with `0`. The
ordinary return-substitution construction therefore applies to the power and
produces a single recurrent phase SCC. The observed induced systems have
`(return words, phase states) = (5,16)` for `b=1`, then `(6,35),(6,54),
(6,77),(6,104)` for `b=2,...,5`. This gives a symbolic finite core behind
the larger two-sided collars: the latter should be a recognizability
refinement of the return system for `sigma^3`, not an independent dynamical
object. A falsifier would be a collared recurrent component not represented
in the marker-power phase system, or a member of the family requiring a
different power. If neither occurs, this is the natural closure mechanism
for the non-AR run-length family.

The power bound itself is algebraic for the entire family, not empirical:
`sigma_{a,b}(0)=0^a1^b2` starts with `0` when `a>=1`, while for `a=0`
one has `sigma_{0,b}^2(0)=2^b0` and
`sigma_{0,b}^3(0)=0^b1^b2`. Thus every member is marker-proper after at most
three powers. What remains numerical/structural is the finite collar bound
and its refinement map, not the existence of a marker-proper power.

**Refinement check (2026-08-09).** An exact finite composition script was run:
compose three successive full-collar transition maps,
then project each target state to `(return-word,offset)`. For `b=1,2,3` at
the stable radii `5,9,9`, every observed source state projects exactly to the
phase image of `sigma^3`; the failure count was zero in all three cases. This
is the first direct evidence that the large collared graph is a refinement of
the marker-power return system rather than a parallel coincidence. The
remaining falsifier is a collared edge whose three-step projection disagrees
with the return phase map at larger `b` or after increasing the orbit margin.

The marker-power mechanism is now packaged as
`marker_power_return_core.hpp` with a regression test. It checks the first
marker-proper power, return-word count, phase-state count, and single-SCC
condition for `sigma_{0,1}`, `sigma_{0,2}`, a left-proper control, and the
minimal non-unit rule and the `b<=5` run-length family. All twelve checks pass.
This is a reusable gate for the
third-plank lift: transport or p-adic fibres should be attached only after
this finite symbolic core certificate succeeds.

**Third-plank contact lift (2026-08-09).** The first actual lift over the
`sigma_{0,1}^3` return core closes completely: 110 powered contact nodes,
16 return phases, 3,306 lifted states, and 6,837 lifted edges. It projects
onto every powered contact node. However, it contains a recurrent nonzero
contact SCC of size 401. This falsifies the shortcut “all recurrent transport
already disappears at the contact layer.” It is not yet a Property-F
obstruction: the independent trusted Property-F automaton for the original
`sigma_{0,1}` closes at 205 nodes with `zero_nodes_beyond_frontier=0`.

The revised hypothesis is that contact holonomy is killed by the positive
prefix/zero-language quotient. The falsifier is a map from that 401-state
contact SCC to a recurrent nonzero Property-F component, or a future failure
of the zero-frontier certificate. This is now the concrete third-plank
question rather than a vague search for “more transport.”

The longer pass deliberately uses a separately enlarged orbit prefix. On the
525456-symbol prefix, extending the requested length too far produces a first
false loss of one left extension at `n=55406`; the 4-million-symbol prefix
restores all five left-special factors through `n=60000`, and the current
35-million-symbol prefix restores them through `n=500000`. Thus the probe also
records an explicit finite-boundary failure mode rather than mistaking it for
a structural transition.

### Thread A5 — extend the data

The current data set is 10 substitutions: Tribonacci + 6 σ_{a,1}
(a=0..5) + σ_{1,2} + σ_1 + σ_2. To validate the decomposition
hypothesis, more substitutions are needed:
- 4-letter Pisot (Tetrabonacci, Pentanacci, Hexanacci) to test
  d=4 Pisot decompositions
- β-substitutions (Barge 2015/2018 family) — well-studied
- Random 3-letter Pisot from a wider survey — to test whether
  AR-partial/non-AR partition is exhaustive

The tabulation driver handles 3-letter substitutions directly. For
4-letter we may need to template `compute_gb_sym_quotient<d>` on
alphabet size — the existing construction in
`gb_bp_matrix_equality.cpp` is already d-templated.

**Progress (2026-08-09):** `app/thread_a4_extension_graph_probe.cpp` now
also runs the direct extension-graph pass for Tetrabonacci, Pentanacci,
and Hexanacci. On 500k+ symbol prefixes through factor length 20, all four
n-bonacci cases (`n=3..6`) show the exact AR signature
`p(k)=(n-1)k+1`: one left-special and one right-special factor, each with
full alphabet valence, and zero bilateral sum at every checked length.
This closes the first finite-data slice of the 4-letter A5 target and
supports treating the AR-exact profile as dimension-stable. It does not
replace the requested β-substitution and wider random survey, nor an
all-length theorem.
