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

**What's open (Item A research targets, in priority order)**:

**(A1) Generalize the n-bonacci nilpotent-cofactor finding to all
unimodular Pisot substitutions.** The pure-`x^k` cofactor structure
(`charpoly(Q_sym_GB) / charpoly(Q_sym_BP) = x^k`, exact) is currently
verified for n-bonacci at n=3,4,5. Does this structure persist for
all unimodular Pisot substitutions, or is it specific to the
n-bonacci family? If it persists, the conjecture is on its way to
being proven in general (the pure-`x^k` cofactor has only the
eigenvalue 0, and `BP-ρ_nc` is always strictly positive, so the
cofactor's eigenvalues can never be dominant — giving
`λ(G_B) = ρ_nc` directly, with no node bijection anywhere). If
it doesn't, the structural reason for the n-bonacci match is
specific to the n-bonacci family, and Item A becomes a
classification problem (which substitutions have the
nilpotent-cofactor structure, and which don't).

**(A2) Apply the involution machinery to non-n-bonacci unimodular
Pisot substitutions.** The n-bonacci involution φ is a Z/2
automorphism of `G_B`'s dominant recurrent core. Does the same
construction work for σ_{a,b}-style substitutions (constant-suffix
Jacobi–Perron / Brun)? For σ₁-style substitutions (the proven
counterexamples)? For random unimodular Pisot candidates from
`wide_random_pisot_survey`? The involution either exists (in which
case the same charpoly-divisibility argument should apply) or it
doesn't (in which case the n-bonacci match is genuinely
family-specific).

**(A3) Build a smooth-relaxation search** (cross-listed with Item B,
see (B2) below) — the natural generator of new candidates to
test (A1) and (A2) on, beyond the random-survey candidates already
tested. A closed-form differentiable objective (the closed-form
`spectral_invariants_3x3` Cardano solver) plus a Pisot-preserving
gradient step on the matrix entries.

**(A4) Check the classical Pisot conjecture via the project's
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
- A fresh non-unimodular Pisot survey (`app/sweep_nonunit_property_f.cpp`,
  24 candidates, seed=11) gives **24/24 ESTABLISHED, 0
  INCONCLUSIVE, 0 FAILED, 0 skipped** — the multi-prime skip is
  gone (multi-PRIME cases now go through the unified combined
  bound, which has been correct since the 3-bug close-out).
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
  (`app/reaudit_gb_dominant_scc.cpp`, started this session, not
  yet finished) re-runs the 4-letter non-unimodular survey with
  the dominant-SCC extractor and compares the three λ values
  (whole-matrix, largest-SCC, dominant-SCC). The pilot indicates
  the largest-SCC and dominant-SCC always agree on these
  candidates' G_B, so the original's λ values are NOT affected
  by the largest-vs-dominant distinction.

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
- **2 cases (1 after the spectral fix -- see below), "local_polynomial_
  cofactor: computed m_k has wrong degree"** in `include/adelic/
  local_field.hpp`. Understood precisely (2026-08-01), not a hidden
  bug: the remaining case (`rndW3_5`) has TWO non-simple prime ideals
  above the same rational prime (`p=3`: one `(e=1,f=2)`, one
  `(e=2,f=1)`, confirmed via the factoring diagnostic). `local_
  polynomial_cofactor`'s own header comment already documents this
  exact limitation -- its cofactor approach (divide the charpoly mod p
  by the product of all SIMPLE, degree-1 factors) only correctly
  isolates a single non-simple ideal's local polynomial; with two, the
  cofactor bundles both together and its degree no longer matches
  either ideal's `ef`. The function correctly THROWS rather than
  returning a silently-wrong polynomial -- this is safe, working-as-
  designed behavior hitting a real, pre-documented scope limitation,
  not a bug to chase down. A genuine fix would need a different,
  stronger construction (per-ideal Hensel lifting rather than a
  cofactor-of-simple-factors trick) -- real, separate work, not
  attempted here.

So the contact-boundary pipeline, combined p-adic bound, and spectral
filter did NOT fully handle degree 4 when first tried -- not just a
parameter change as previously assumed here. One root cause (the
spectral filter's unsoundness for complex dominant secondary pairs at
`n>=4`) was a real, silent bug, now fixed. **After the spectral fix,
re-running the same 4-letter batch: 6/7 candidates now reach a verdict
(all ESTABLISHED, tiles)** -- only one hits `local_polynomial_
cofactor`'s pre-documented (not silently broken) limitation: two
non-simple prime ideals over the same rational prime, which its
cofactor-of-simple-factors approach can't disentangle by construction.
Extending it to that case is real, separate work (a per-ideal Hensel-
lifting construction, not a bug fix) and is the actual remaining item
before a wider-alphabet survey can be trusted at full generality --
but it's a known, scoped feature gap now, not an unexplained failure.
The smooth-relaxation search (B2) is a smarter candidate generator that
targets Pisot-preserving perturbations specifically, once the pipeline
itself covers the multi-non-simple-ideal case too.

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
(`app/reaudit_gb_dominant_scc.cpp`) verifies this on a fresh
batch.

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

### Thread A3 — find the Class II cofactor

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

### Thread A4 — a Class III structure

σ_{0,1} has no constant factor and not Arnoux-Rauzy (f(2) = 7, not
5). The orbit of 0 has a non-Sturmian factor complexity. The first
return word of letter 0 is empty — σ^k(0) never has "0, 0" as
consecutive characters, because σ(0) = (1, 2) and the orbit always
crosses through 1, 2 before returning. The A1 cofactor is not x^k
(not Arnoux-Rauzy), and A2 involution is partial (30/44).

First step: compute the Pisot continued fraction of 1/β_{0,1}
where β_{0,1} is the unique real root of x^3 - x - 1. The Pisot
continued fraction is periodic for all Pisot numbers; the period
length is an invariant that distinguishes Class I/II/III.

### Thread A5 — extend the data

The current data set is 10 substitutions: Tribonacci + 6 σ_{a,1}
(a=0..5) + σ_{1,2} + σ_1 + σ_2. To validate the decomposition
hypothesis, more substitutions are needed:
- 4-letter Pisot (Tetrabonacci, Pentanacci, Hexanacci) to test
  d=4 Pisot decompositions
- β-substitutions (Barge 2015/2018 family) — well-studied
- Random 3-letter Pisot from a wider survey — to test whether
  Class II/III partition is exhaustive

The tabulation driver handles 3-letter substitutions directly. For
4-letter we may need to template `compute_gb_sym_quotient<d>` on
alphabet size — the existing construction in
`gb_bp_matrix_equality.cpp` is already d-templated.
