# Reading list: Q/R/S Pisot continuation

## Bootstrap entry point — 2026-08-05 PC handoff

The universal n-bonacci determinant campaign is kernel checked, and the later transport work has moved beyond the n-bonacci family. The active campaign is now:

```text
finite positive grammar
→ exact observable prefix-defect alphabet
→ generatorwise parent-role intertwiner
→ positive noncommutative word closure
→ norm-weighted operator twists
→ exact cyclotomic-sector audit
→ next Pisot substitution
```

Do not restart from raw n-bonacci enumeration. Read the items below in order and continue from the named next target.

## Read first: operating contracts

1. `README.md`
2. `docs/DOCUMENTATION_INDEX.md`
3. `docs/MATHEMATICAL_API.md`, especially `Reflective proof engine`
4. `docs/PROOF_SYSTEM_MANUAL.md`
5. `docs/PROOF_SYSTEM_CONTRACTS.md`
6. `docs/PROOF_SYSTEM_EXTENSION_GUIDE.md`
7. `docs/THEOREM_STATUS.md`
8. `docs/RESEARCH_STATUS.md`
9. `CONTINUITY.md`
10. `ROUND102_QUALITATIVE_PC_HANDOFF_2026-08-05.md`

The governing engineering rule remains:

> When a derivation is missing, implement a reusable exact operation that returns evidence. Do not hard-code the desired theorem, generator count, factorization, or Lean proof text.

Keep `Closed`, `GeneratedUnchecked`, and `KernelChecked` distinct.

## Read next: theorem machinery now controlling decisions

1. `../archive/2026-08-05_round_reports/FINITE_POSITIVE_GRAMMAR_MAJORANT_REPORT_2026-08-05.md`
2. `../archive/2026-08-05_round_reports/NORM_WEIGHTED_QR_MAJORANT_REPORT_2026-08-05.md`
3. `../archive/2026-08-05_round_reports/TWIST_EXTENSION_THEOREMS_2026-08-05.md`
4. `../archive/2026-08-05_round_reports/CYCLOTOMIC_OBSTRUCTION_CERTIFICATE_REPORT_2026-08-05.md`
5. `../archive/2026-08-05_round_reports/DOMINANCE_THEOREM_LITERATURE_AUDIT_2026-08-05.md`

These establish that the theorem is not fundamentally a two-matrix theorem. It is a finite-positive-generator theorem, provided every concrete generator has a simultaneous comparison witness.

## Read next: how the n-bonacci corridor was closed

1. `../archive/2026-08-05_round_reports/CONDITION_F_CANONICAL_TWO_MATRIX_REPORT_2026-08-05.md`
2. `../archive/2026-08-05_round_reports/GENERALIZED_MULTINACCI_SYMBOLIC_ALL_D_CLOSURE_REPORT_2026-08-05.md`
3. `../archive/2026-08-05_round_reports/GENERALIZED_MULTINACCI_PRIMITIVE_INTERTWINER_REPORT_2026-08-05.md`
4. `../archive/2026-08-05_round_reports/GENERALIZED_MULTINACCI_ADMISSIBLE_SUBGRAMMAR_REPORT_2026-08-05.md`
5. `../archive/2026-08-05_round_reports/MONOTONE_PROFILE_CORRIDOR_CLOSURE_REPORT_2026-08-05.md`
6. `../archive/2026-08-05_round_reports/COEFFICIENT_PROFILE_OUTWARD_TWISTS_REPORT_2026-08-05.md`

The corridor `2^k 1^(D-k)` is closed as a parent-controlled renewal suspension of primitive `Q/R`. Powers such as `R^2`, selector copies, renewal phases, and finite character twists do not count as new primitive positive matrices.

## Read next: how plastic broke Q/R and produced Q/R/S

1. `../archive/2026-08-05_round_reports/NEXT_FAMILY_PLASTIC_NUMBER_REPORT_2026-08-05.md`
2. `../archive/2026-08-05_round_reports/PLASTIC_THREE_GENERATOR_GRAMMAR_REPORT_2026-08-05.md`
3. `../archive/2026-08-05_round_reports/PLASTIC_THREE_GENERATOR_INTERTWINER_REPORT_2026-08-05.md`
4. `../archive/2026-08-05_round_reports/SHIFT_BRANCH_PLASTIC_GENERAL_CLOSURE_REPORT_2026-08-05.md`

Plastic has three observable signed prefix-defect classes. The comparison map forgets displacement and retains ordered parent role. This is the first genuinely non-n-bonacci positive grammar captured by the general theorem.

## Read next: quartic and third-smallest Pisot audits

1. `../archive/2026-08-05_round_reports/QUARTIC_FOURTH_GENERATOR_AUDIT_REPORT_2026-08-05.md`
2. `../archive/2026-08-05_round_reports/DELAYED_SELF_BRANCH_CONTINUATION_REPORT_2026-08-05.md`
3. `../archive/2026-08-05_round_reports/THIRD_SMALLEST_PISOT_QRS_CLOSURE_REPORT_2026-08-05.md`
4. `../archive/2026-08-05_round_reports/ROUND100_QUARTIC_QRS_CLOSURE_VALIDATION_2026-08-05.txt`
5. `../archive/2026-08-05_round_reports/ROUND101_THIRD_SMALLEST_PISOT_VALIDATION_2026-08-05.txt`

The quartic raw four-colour prefix-pair split is not a fourth observable matrix: two colours lie in the neutral defect kernel. The third-smallest Pisot number also yields only `Q/R/S` in its canonical seven-state beta-substitution, while adding a `Phi_4` cyclotomic sector.

## Active next target: fourth-smallest Pisot number

The next target in increasing Pisot order is the supergolden number, the dominant root of

```text
x^3 - x^2 - 1.
```

Treat this as a new campaign, not as an assumed continuation of the prior `Q/R/S` families.

### Required order of work

1. Derive and exact-check the greedy beta expansion of `1`.
2. Construct the canonical beta-substitution from that expansion.
3. Exact-check the incidence characteristic/Parry polynomial and its factorization against `x^3-x^2-1`.
4. Enumerate the complete parent-prefix catalogue before constructing the boundary graph.
5. Form all ordered prefix differences and quotient them by the declared observable map.
6. Audit generator minimality:
   - do not count orientation partners that are globally reducible;
   - do not count neutral-kernel refinements;
   - do not count powers, renewal phases, selector copies, or character twists;
   - do count distinct defect classes that remain inequivalent under exact boundary recursion and minimization.
7. Only then enumerate the exact contact-boundary graph.
8. Construct the smallest ordered-parent-role comparison object.
9. Prove every generator inequality simultaneously.
10. Invoke finite-positive-word and norm-weighted-twist closure.
11. Run the exact cyclotomic obstruction detector on the substitution and every nontrivial character sector.
12. Emit Lean, but claim kernel acceptance only if the matching Lean/Mathlib environment actually checks it.

### Acceptance questions

- Is the canonical positive grammar `Q/R/S` again?
- Does the supergolden realization finally yield a genuine fourth generator?
- Is any apparent fourth class only a refinement in the kernel of the observable defect map?
- Does the canonical substitution introduce a cyclotomic lift distinct from the third-smallest Pisot case?
- Does the same displacement-forgetting parent-role intertwiner still work?

### Progress (2026-08-05, PC session)

Steps 1–6 of the required order of work are done, via a new reusable
operation rather than a hand-copied substitution:

- `include/ravel/canonical_beta_substitution.hpp` — exact greedy
  beta-expansion of 1 in `Q(beta)` and the canonical Parry/Dumont-Thomas
  substitution it drives.
- Supergolden's greedy expansion of 1 is `1 0 1` (finite, no correction
  needed); the derived substitution `0->01, 1->2, 2->0` was independently
  verified to have incidence characteristic polynomial exactly
  `x^3-x^2-1` — no cyclotomic cofactor, unlike third-smallest Pisot's
  `Phi_4` lift.
- Parent-prefix catalogue: 2 distinct prefixes (`{}`, `{0}`), 4 total parent
  decompositions, 3 defect classes (`Q/R/S`), `9` role states, `16` role
  edges split `10/3/3` — the same shape as plastic and third-smallest
  Pisot, no fourth class.
- See `include/ravel/proof/supergolden_qrs_closure.hpp` and
  `tests/supergolden_qrs_test.cpp`.

Steps 7–10 are now wired into the Q/R/S closure certificate rather than
remaining a parallel test: the exact contact-boundary graph has 20 states and
25 surviving edges, its smallest ordered-parent-role comparison object is the
9-state universal catalogue, all three generator inequalities pass, and the
finite-positive-grammar majorant is proved. The certificate now refuses to
claim closure unless those independent boundary/intertwiner checks succeed.
Steps 11–12 are now substantially discharged: cyclotomic-sector auditing for
boundary/twisted objects and Lean emission/actual kernel acceptance are all
covered by the finite certificate pipeline. Since there is no
cyclotomic cofactor at the substitution level, the raw substitution audit is
now explicit: the incidence characteristic polynomial is passed through the
exact cyclotomic factor certificate and has no cyclotomic factor. Boundary and
twisted sectors are now also checked pairwise: each of the three unordered
generator pairs is passed through the exact cyclic-voltage Fourier comparison,
and every nontrivial two-sheet sector is certified below its untwisted sector.
The regression now emits `lean/generated/supergolden_qrs_audit.lean`, whose
finite summary is accepted by the actual Lean/Mathlib environment. The
remaining work is to connect richer boundary matrices to a kernel-level
certificate rather than only their exact finite summary. `make lean-check`
now regenerates this artifact before checking the enrolled Lean files, so the
finite summary is not a stale hand-maintained snapshot. Its characteristic,
cyclotomic flag, and three pair-sector flags are serialized from the live
certificate rather than duplicated constants. The same artifact now serializes
the three exact rectangular matrix products and Lean checks every entry of all
three inequalities; a future generic matrix theorem would be a further
abstraction, not an unchecked gap in this finite instance.

## Parallel question kept in the wings

> How do we obtain a genuinely fourth primitive positive matrix?

A valid answer requires four irreducible observable transport classes after exact quotienting and minimization. Four raw labels are insufficient.

### 2026-08-05 update: answered negatively for every Pisot number below 2

`include/ravel/proof/beta_in_one_two_forces_qrs.hpp` proves that **no**
Pisot number with `1 < beta < 2` can ever yield a fourth generator through
the canonical beta-substitution construction: `1 < beta < 2` forces every
greedy digit into `{0,1}` and forces the first digit to be `1`, which
forces the parent-prefix set into `{[], [0]}`, which forces exactly three
defect classes. Checked against the golden ratio and all six smallest
Pisot numbers (`tests/beta_in_one_two_forces_qrs_test.cpp`) — all six pass,
including the fifth (`x^6-x^5-x^4+x^2-1`) and sixth (`x^5-x^3-x^2-x-1`)
smallest, neither of which has been folded into the concrete contact-
boundary/intertwiner machinery yet (only supergolden has, so far).

The runtime contact-boundary dispatcher now instantiates alphabet sizes through
9, so those fifth/sixth canonical substitutions are no longer rejected at the
type-dispatch boundary. This is only an API/compiler extension: a bounded
probe of the fifth case did not close within a 90-second exploratory budget,
so no contact-boundary theorem is claimed for it yet. The reproducible bounded
probe now records the first useful layer: with `search_bound=1` and explicit
caps it reaches `d_cont=84` and a partial 882-node boundary before both the
closure and corona caps fire; `search_bound=0` gives 28 candidates but no
boundary. The capped corona path now streams candidates before insertion;
the same probe completes in about 1.4 seconds with peak RSS about 18 MB,
instead of materializing the multi-gigabyte uncapped intermediate layer.

The adjacent-competitor truth-cache recipe also has an explicit
`--cache-only` mode. Cache construction is now independent of the separate
n=6→7 family-transport verdict, so a visible `NO_UPPER_FAMILY` classification
does not make the cache artifact itself fail.

The projected n=6→7 transport now closes as well. Boundary candidates carrying
a two-atom decomposition are filtered by their canonical catalogue grade before
entering the grade-two exhaustion proof; cancellation can produce a grade-one
atom with a two-atom witness, and treating that as grade two was the remaining
false obstruction. The projected run now reports `boundary=PASS result=PASS`.
The legacy materialized mode remains a compatibility/cache path and may still
report the older direct-family `NO_UPPER_FAMILY` diagnostic.

For every adjacent transport at dimension `n >= 5`, the driver now derives the
explicit twisted predicted-core extension first. The check records the
previous-alphabet shadow, orientation-sheet correction, path cocycle, and
substitution-length bounds; a raw shadow comparison is rejected if this twist
does not close. The n=5→6 and n=6→7 runs both report `proved=YES` (62 and 102
substituted edges respectively). The backward-closure membership table was
also changed to a structural hash set, retaining sorted output while reducing
tree-allocation overhead for larger exploratory frontiers.

The contact report now has an opt-in `retain_boundary_matrix` switch. The
theta5 growth probe disables the dense `|G_B|²` matrix/eigenvalue summary while
retaining all boundary nodes and cap flags; at the 5,000/50,000 exploratory
caps this reduced the measured peak from about 1.47 GB / 65 s to 0.81 GB /
22 s. This is an exploratory resource guard, not a spectral theorem result.

This reframes the parallel question: stop searching Pisot numbers below 2
for a fourth generator via this construction. The two remaining routes are
(a) `beta >= 2`, where a digit value of `2` opens a third prefix `[0,0]`
and a correspondingly richer defect classification, or (b) a substitution
not obtained by this canonical construction at all (e.g. plastic's own
hand-chosen 3-letter substitution, which is smaller than its canonical
7-letter Parry substitution but empirically shares the same `{[],[0]}`
prefix shape for a different, not-yet-written-down reason).

### 2026-08-06 update: route (a) answered -- found

`x^3-2x^2-2` (digits `(2,0,2)`) and `x^3-x^2-2x-2` (digits `(2,0,1,0,2)`)
each have a genuinely irreducible `+2`/`-2` defect class -- the first
confirmed examples of more than three primitive positive generators. See
`include/ravel/proof/first_genuine_fourth_generator.hpp`. Mechanism: an
interior `0` digit sandwiched between two nonzero digits isolates a
universal role state with no `+1`-generator outgoing edge at all, so the
`+2` jump cannot be decomposed into two `+1` steps -- there is no
alternative path, not just an undiscovered one. `x^2-2x-2` (digits
`(2,2)`, no interior zero) has zero such violations and stays Q/R/S,
confirming the interior-zero pattern is the actual signature.

Not yet done: constructing the CONCRETE contact-boundary graph for either
witness (the way plastic/supergolden's boundary graphs were built) to
confirm the extra generators survive at the geometric level too, not just
the abstract universal-role level -- this is exactly the gap that caught
quartic's spurious fourth colour, so it should be closed before this
result is treated as fully settled. Also not yet done: finding the
SMALLEST such example (both witnesses were found via unrelated searches,
not a targeted search for "interior zero between nonzero digits").

### 2026-08-06 (PC session, later) — the seam is "behaving in its own way"; strong coincidence and property (F) both formalized, one bug in core infrastructure found and fixed by reading the primary source

**The seam.** Applied the same "find the general pattern underlying what
looks case-specific" lens used to generalize the Spectre C++ port back
onto the eventually-periodic generator-seam question (Part 3 of
`general_generator_theorem.hpp`). A third closed-form hypothesis (sumset
of letter-0's occurrence set with the flexible junction letter's own set)
does better than the two earlier ones but is still refuted, by
`x^3-2x^2-x+1`. The precise reason: reachability lives on pairs
`(role, accumulated net defect)`, not on accumulated net alone — proven,
not just observed. See `include/ravel/proof/seam_reachability_is_the_
general_pattern.hpp`. Conclusion: the seam is not a second, murkier rule;
it is Part 1's general theorem in the regime that doesn't happen to
trivialize.

**Strong coincidence, closed for a real structural class.** If every
letter's image shares a leading letter ("constant factor"), every pair
resolves strong coincidence at depth 1, unconditionally, for ANY
substitution (no Pisot hypothesis needed) — proved from the coincidence
checker's own definition. `include/ravel/proof/constant_factor_forces_
depth1_coincidence.hpp`. Generalized further, same session: a PAIR
resolves at depth 1 if it merely shares a first OR last letter (not
requiring a global constant factor) — confirmed directly against
`sigma_{0,1}` (the non-AR control), where pair `(0,1)` resolves this way
and pairs `(0,2)`/`(1,2)` do not, matching their much higher actual
resolution depths (12, 13) exactly.

**Property (F): a real bug in `check_property_f`, found by reading the
primary source and fixed.** Minervino-Thuswaldner's actual criterion
(Lemma 9.8, full text now archived at `docs/bibliography/`) is a cycle
that is NOT entirely zero-nodes — this codebase's verdict logic instead
required a cycle to touch BOTH zero and nonzero nodes ("mixed"), which
`include/ravel/proof/property_f_unconditional.hpp` separately proves
(Lean-checked, `lean/generated/property_f_zero_walk.lean`) can never
happen — so the old check was checking a condition that could never
trigger, for this project's entire history. A first attempted fix
(enumerating cycles abstractly, `property_f_correct_cycle_criterion.hpp`,
kept in-tree marked SUPERSEDED) gave a false FAILS on Fibonacci and was
wrong. The real fix is a one-line change to `check_property_f` itself
(`include/adelic/coincidence_and_property_f.hpp`, 2026-08-06 comment):
drop the zero-touching requirement, flag any cycle touching a nonzero
node. Verified node-for-node identical against every ESTABLISHED case
this project has on record (Fibonacci 8 nodes, rnd13 33185 nodes, both
non-unit sweeps, the whole AR-partial/non-AR family) —
`tests/property_f_correct_verdict_test.cpp` locks this in. Nothing was
ever a false positive; the old check just could never have caught a real
failure. Full account: Findings 17-21 in `FINDINGS_FOR_CITATION.md`.

**Open target, explicitly handed forward: the Diophantine-synchronization
angle.** For non-constant-factor, non-boundary-matching pairs (e.g.
`sigma_{0,1}`'s pairs `(0,2)` and `(1,2)`), the eventual coincidence match
is NOT a boundary-letter trick — traced directly, both resolve via an
INTERIOR letter match (letter 2, via prefix, at depth 12 and 13
respectively), at a specific position where the two words' running
letter-count (abelianization) vectors happen to coincide exactly, despite
the words having different total lengths at that depth. This reframes the
open question: it is not a combinatorics-on-words problem past the
boundary case, it is a question about when two integer vector sequences
growing under the SAME linear recurrence (the incidence matrix) from
DIFFERENT starting letters land on the same point — structurally akin to
simultaneous Diophantine approximation / three-distance-theorem-type
synchronization, not word combinatorics. No work has been done yet on
this beyond locating and confirming the phenomenon in one example
(`/tmp/find_match.cpp`, not yet promoted into the tree — the next session
should start by turning that probe into a real `include/ravel/proof/`
header). This is the live next target for the coincidence question.

### 2026-08-06 (PC session, later still) — the Diophantine angle, actually reduced

Turned the "why does the interior match happen" observation into an exact
theorem: `abelianization_from_walk` (verified against direct word
materialization) shows any prefix's abelianization is
`sum_ell M^{ell-1} p_ell` over the Dumont-Thomas walk. For `sigma_{0,1}`
specifically, only letter 0 has a length->=2 image, so this collapses to
a sparse sum of a single fixed "landmark vector" under matrix powers.
Strong coincidence reduces exactly to a subset-sum collision question:
`M^11 v0 + v0 = M^10 v0 + M^5 v0 + M^3 v0` is the concrete relation behind
the depth-12 match found earlier. See Finding 22 and
`include/ravel/proof/coincidence_as_landmark_vector_cancellation.hpp`.
This is a real reduction (a well-posed linear-recurrence question), not
yet a closed form or a general bound -- the next session should try
either (a) a general theorem for which substitutions collapse to a
single-landmark-vector picture (any substitution with only one letter
having image length >= 2, generalizing beyond sigma_{0,1}), or (b) the
harder multi-landmark case where more than one letter branches.

### 2026-08-06 (PC session, very late) — walk-realizability closed for the single-junction class

Tried the obvious Cayley-Hamilton shortcut for the Diophantine reduction
(Finding 22) first: minpoly `x^3-x-1` has all {-1,0,1} coefficients, giving
`M^3=M+I` for free. Checked whether this gives a short coincidence witness
directly -- it does not; the predicted landmark pattern is combinatorially
unreachable given the substitution's forced-chain structure (a landmark at
depth 2 forces the walk straight past depth 1 to the leaf). This is the
"generalized with a twist" AM predicted: the linear algebra names which
relations CAN exist; a separate combinatorial layer (which depth-sets are
actually walk-realizable) decides which ones occur. That second layer is
now closed exactly for the single-junction class (one letter with image
length >= 2, everything else a deterministic chain back to it): achievable
landmark-sets are exactly compositions into the junction's own jump sizes,
verified zero-discrepancy against brute force (after fixing a real bug the
cross-check caught: dropped landmark events made right before running out
of depth). See Finding 23,
`include/ravel/proof/single_junction_coincidence_composition.hpp`.

Next: generalize past single-junction to substitutions with multiple
branching letters, where jumps from different junctions interleave.

### 2026-08-06 (PC session, still later) — reverse-engineering the minimal polynomial from pure counting data

AM proposed a genuinely useful methodological shortcut: instead of hand-
deriving a substitution's incidence matrix and characteristic polynomial,
count "which walks finish" (Dumont-Thomas walks that land exactly back on
a reference letter with zero depth left) and fit the minimal linear
recurrence that counting sequence satisfies. Verified exact on two
substitutions with different minimal polynomials (plastic number's
x^3-x-1 for sigma_{0,1}; x^3-2x^2-x+1 for its own substitution), zero
prior algebraic knowledge needed. See Finding 24,
`include/ravel/proof/reverse_engineer_minimal_polynomial_from_returns.hpp`.
This is a real, general, reusable tool -- not yet applied to actually
find a NEW multi-junction hard-coincidence example (the search for a
good test case for Finding 23's generalization is still open; several
candidates tried tonight were either constant-factor-trivial or not
genuinely Pisot).

### 2026-08-06 (PC session, very very late) — exact coincidence closure built, a real gcd-obstruction found with it

Built an exact, deduped memoized closure (ravel/proof/coincidence_closure.hpp,
on a new shared ravel/generic_memoized_dag_closure.hpp contract, matching
corona.hpp's own architectural pattern) replacing raw word materialization,
which had stalled past depth 30 on the sparser variants. Used it -- not more
raw searching -- to actually explain the sparse-chain behavior: reduced
every coincidence pair to a single "gap" number (difference in
deterministic run-in length), and found a clean, sharp pattern -- when the
junction's own jump sizes share a common factor g>1, pairs whose gap isn't
divisible by g NEVER coincide (checked exhaustively to depth 40, zero
exceptions across two structurally different substitutions), while
compatible-gap pairs resolve quickly. Proposed mechanism (residue-class
obstruction from jump-size divisibility) is strong and well-supported but
not yet a completed rigorous proof -- the mid-chain-cutoff case needs an
explicit argument. See Finding 25. This reframes "which pairs are hard" a
second time tonight: not about overall sparsity, but about gap-vs-gcd
compatibility for a SPECIFIC pair within a given substitution.

Next: (1) finish the rigorous proof of the gcd-obstruction, handling the
mid-chain-cutoff case explicitly; (2) extend beyond single-junction to see
whether an analogous obstruction exists for multi-junction substitutions.

### 2026-08-06 (PC session, final stretch) — gcd-obstruction proven, extends to multi-junction unchanged

Closed both open items from the previous entry in one proof: d + dist(terminal)
≡ 0 (mod g) holds by induction (base case, clean-edge case, and the
previously-missing mid-chain-cutoff case, all worked through explicitly),
where g = gcd of every jump size across the WHOLE junction graph. The
clean-edge induction step never depends on whether an edge stays at one
junction or crosses to a different one -- so the multi-junction case falls
out for free, no separate argument needed. Verified the invariant itself
(not just its consequence) against a genuine multi-junction gcd=2 example
under an explicit memory/time cap (10GB, 30s) -- small state counts by
design, since this checks an exact identity, not a search. See Finding 26,
include/ravel/proof/coincidence_gcd_obstruction_theorem.hpp.

What remains open, honestly: the CONVERSE (every compatible-gap pair
eventually coincides) is still empirical, not proven. That's the next
real target if this thread gets picked up again -- likely needs a
genuinely different argument (existence, not impossibility).

### 2026-08-06 (PC session, resumed) — a proved partial converse: the leftmost-loop corollary

Picked the converse back up (Finding 26's open item). Instead of guessing
at a general argument, drove the existing CoincidenceClosure at
compatible gaps it hadn't been asked about yet and looked at the MINIMAL
witnessing K, not just existence. Found real structure: on the Finding-26
multi-junction gcd=2 example, gap in {0,4,8,12} coincide at exactly
K=gap, while gap in {2,6,10} need K=gap+10. Traced the K=gap cases to an
exact mechanism: child_index=0 always carries the zero landmark vector
(no siblings before the first child), so the deterministic "always take
the leftmost branch" walk accumulates zero cost throughout; being
deterministic over a finite graph it's eventually periodic, and when the
start junction lies on its own cycle, that gives a genuine zero-landmark
closed loop of some length L (a multiple of g, but possibly a PROPER
multiple -- L=4 here, not 2). Repeating that loop m times is then an
exact, constructed witness for gap=mL at K=mL, no search needed. Proved
and verified (including a negative check that gap=2 is correctly NOT
covered). See Finding 27,
include/ravel/proof/coincidence_converse_leftmost_loop.hpp.

Honest scope: this only covers the sub-lattice of gaps that are
multiples of L, not every compatible gap (gap=2,6,10 still need the full
search, landing at K=gap+10 -- an unexplained but suggestively constant
offset). The full converse is still open; next angle if this gets picked
up again is probably explaining that +10 offset, which smells like a
second, nonzero-landmark loop whose contribution eventually cancels.

### 2026-08-06 (PC session, task 4 pass) — Finding 6.5's unimodularity hypothesis refuted

Tested the hedge from Finding 6.5 directly (did sigma_{0,2}'s
non-termination correlate with being non-unimodular?) with a fresh
6-vs-6 sample of small cubic Pisot candidates. Caught a real bug first
(a negative-real-root false positive in the Pisot filter -- fixed by
requiring the dominant root be real and >1, not just largest modulus)
then re-ran clean: 6/6 unimodular and 6/6 non-unimodular candidates all
terminate. Unimodularity is refuted as the distinguishing factor.
sigma_{0,2} itself, re-run to 2500 digits at 3000-dps, is still
UNRESOLVED -- no visible period, and pushed no further given rounding
risk starts to matter around that budget. See Finding 28,
python/beta_expansion_unimodularity_check.py. Task 4 (terminating vs.
eventually-periodic characterization) stays open; this narrows what
doesn't explain it rather than closing it.

### 2026-08-06 (PC session, task 3 pivot) — accumulation points, not a fixed-range search

AM correctly flagged that Task 3's premise was wrong: Pisot numbers
aren't confined to a finite range, so searching a fixed window doesn't
make sense. Asked instead whether there's a "mountain range" structure
to where they accumulate -- deep (dense) vs shallow (sparse) regions --
and whether an arbitrary target can be matched to a real nearby Pisot
number. Answered the constructive half exactly: the a-bonacci family
(x^n - a(x^{n-1}+...+1)) converges monotonically to a+1 from below for
every positive integer a, verified via the existing certified
pisot_classify_3x3/4x4 (no floating point in the classification step),
so every integer >=2 is a genuine, arbitrarily-deep accumulation point.
Built a practical (but honestly capped at n=4) nearest-Pisot tool from
this. Did NOT oversell the "valley" half -- a small dense cubic sweep
showed only a mild, not statistically compelling, lean toward more
hits near integers. See Finding 29,
app/probe_pisot_accumulation_structure.cpp. Task 3 closed as
re-scoped; the between-integer density question is a real open thread
if this gets picked up again, likely needs either a bigger sweep or an
actual read of Dufresnoy-Pisot's derived-set analysis rather than
rediscovering it computationally.

### 2026-08-06 (PC session, generic-degree classifier + exact sigma_{0,2} probe) — while waiting

Filled in the other half of AM's "combination of 1 and 3" answer for
the Task 3 pivot tool: exposed the existing degree-agnostic Pisot
classification logic (already inside exact_pisot.c, just never wired
past degree 4) as pisot_classify_degree_n, with an honest guard for
the one real limitation (multiple complex-conjugate pairs at degree
>=5 -- the modulus bound only certifies their combined product).
Verified the guard fires correctly on a-bonacci degree 5-8 and does
NOT over-fire on a hand-found single-pair degree-5 Pisot number. See
Finding 30.

Also started a genuinely exact (not floating-point) resolution attempt
for sigma_{0,2}'s open beta-expansion question, using pure integer
arithmetic in Z[beta] (beta is an algebraic integer, so the greedy
recurrence never leaves the integer lattice -- no precision wall at
all, unlike the earlier mpmath approach). Caught a real bug immediately
(qbeta_sign's endpoint-disagreement fallback can silently return the
wrong sign when the bracket isn't tight enough; fixed with a
robust_sign that refines until the endpoints actually agree). As of
this entry the search has run past 60,000 exact steps with zero
repeated states and no termination -- genuinely still open, running in
the background, not a precision artifact this time.

### 2026-08-06 (PC session, thread 4 closed) — sigma_{0,2} was never Pisot; Finding 32

AM asked whether closure/non-closure of sigma_{0,2}'s beta-expansion
could be predicted number-theoretically rather than by more search.
Checked the premise that should have been checked before any of this
started: is sigma_ab_matrix(0,2) actually Pisot? It is not --
pisot_classify_3x3 (this project's own certified tool) confirms its
complex-conjugate pair has modulus ~1.1466 > 1. Schmidt's
eventual-periodicity theorem requires Pisot-ness and simply doesn't
apply. Verified the mechanism quantitatively too: the greedy
recurrence's integer coefficient vector should grow like
1.1466^n (the expanding conjugate direction, the opposite of the
contraction Schmidt's proof relies on) instead of staying bounded --
confirmed directly, magnitude ~9.5e17 at step 300 against a predicted
~8e17. This retroactively explains Finding 31's 123,500-steps-zero-
repeats result completely: there was never a mechanism forcing a
repeat to exist. Traced the error to a probable typo in Finding 5's
original candidate table (sigma_{0,2} where sigma_{1,2}, the actual
tested candidate, was likely meant) that then got built into a real
matrix and used verbatim by Finding 6.5 without a fresh certified
check. Corrected the record in place (not deleted) in
python/beta_expansion_thread_a4.py, added a regression test
(tests/sigma02_not_pisot_test.cpp). Task 4 closed: the blocking case
is resolved, though the broader "why do some genuine Pisot numbers
terminate vs. not" question is still open if anyone wants it later.

Lesson for next time: this project's own discipline (verify premises
directly, don't trust inherited labels) is strong but didn't get
applied here -- a Pisot classification was trusted across three
findings and a 30-minute compute budget without ever being re-run
through the certified classifier, which would have taken under a
second.

### 2026-08-06 (PC session, harmonics/poles map) — Finding 33

AM asked for a map of where Pisot numbers actually appear, noting
"spottiness" near the smallest one, and framed the golden/silver
ratio, plastic number, etc. as "poles" in a Pisot-approximation
structure -- essentially asking about the classical derived-set theory
without necessarily knowing its name. Searched the literature (web
search, since this project's bibliography didn't have it): confirmed
phi is PROVEN to be the smallest limit point of S (Dufresnoy-Pisot
1955, who also completely classified every Pisot number below it --
a short, closed, finite list), and that Boyd's 1984-85 papers give a
genuine quantitative "harmonics" bound on successive derived sets:
sqrt(k) <= min S^(k) <= 2^(k/2). Combined this with a fresh, certified
201-point sweep (Finding 30's pisot_classify_degree_n) confirming the
predicted density transition quantitatively (4.85/unit below phi,
climbing to 99/unit by [3,4)) and surfacing an honest gap (theta_3, a
real literature Pisot number our classifier can't certify due to its
own disclosed two-complex-pair limitation). Published as an
interactive map artifact. See Finding 33,
app/probe_pisot_map.cpp.

Open thread if this comes back: the exact minima of S'', S''', etc.
(only the bracket is reproduced here, not Boyd's specific computed
values, which are paywalled) and whether this project's own tools
could independently re-derive them rather than cite Boyd.

### 2026-08-06 (PC session, converse revisited) — Finding 34: permanence phenomenon, still not a full proof

AM asked to work the converse of the gcd-obstruction theorem
specifically. Went past Finding 27's leftmost-loop corollary (which
only covers gaps that are multiples of a junction's own leftmost-cycle
length) by checking a sharper question: as a function of D, does
reachable(0,D) ever PERMANENTLY start intersecting reachable(0,D+g)?
Yes -- exhaustively empty for D<10, nonempty for every D in [10,40] on
the Finding-26 multi-junction gcd=2 example. Sharper still: the exact
same state recurs as the witness at D=10,12,14,16,18,20, a literal
fixed point under +g extension, not six separate coincidences.

Tried to close this into a full proof via an inductive "insertable
return-to-self loop of length g" argument -- found the obvious
candidate (the direct length-2 self-loop at junction 0) carries a
NONZERO landmark, so it doesn't trivially explain the fixed point the
way Finding 27's landmark-free loop did. The mechanism behind the
recurring fixed point is real but not yet understood structurally.

Named the most promising route to an actual proof: a Schmidt-style
pigeonhole argument using the SAME Pisot-contraction property of the
substitution's incidence matrix that makes Schmidt's own beta-
expansion periodicity theorem work (cited since Finding 6.5) -- not
attempted this session. See Finding 34,
tests/coincidence_converse_permanence_test.cpp.

Honest state of the thread: real, substantial new evidence, a named
and plausible proof strategy, still not a completed general theorem.
Next real step if this comes back: either execute the Schmidt-style
pigeonhole argument, or test the permanence phenomenon on a second,
structurally different multi-junction example before trusting it as
universal.

### 2026-08-06 (PC session, converse redirected) — Finding 35: g>1 provably can't be Pisot

AM's own question caught something important right as Finding 34 was
about to build further on it: is the non-Pisot-ness just found in the
gcd=2 test matrix a one-off, or does the WHOLE g>1 gcd-obstruction
scenario provably rule out Pisot-ness? Proved it in general: g>1
forces the Perron-Frobenius period of the incidence matrix to be a
multiple of g (every cycle in the full digraph passes through a
junction and decomposes into jump-size sums, all divisible by g), and
period>=2 forces multiple eigenvalues tied at the top modulus by
standard Perron-Frobenius theory -- directly contradicting Pisot's
unique-dominant-eigenvalue requirement. Verified exactly (Wielandt-
bound is_primitive, no floating point) on two independent g>1
examples (both imprimitive) plus the g=1 control (primitive). See
Finding 35, tests/gcd_obstruction_implies_nonpisot_test.cpp.

This redirects, not invalidates, Findings 25-27/34: the theorems are
correct but their nontrivial content only ever applies to substitutions
that can't be Pisot. For real Pisot substitutions g is always 1, so
the actually-relevant open question is whether coincidence witnesses
always exist for g=1 across genuine multi-junction Pisot substitutions
-- not yet tested on an actual Pisot example (every multi-junction
example built so far was for combinatorial testing and turned out
non-Pisot). That's the natural next step if this thread continues.

### 2026-08-06 (PC session, "light this place up") — Finding 36: real Pisot, real coincidence, immediate and permanent

Took the redirection from Finding 35 seriously and found the right
example: sigma_{1,1}, already certified Pisot in this project's much
earlier history, re-verified fresh (beta~2.14789904, primitive,
genuinely multi-junction -- letters 0 AND 1 both branch, unlike every
prior gcd-obstruction test case). g=1 confirmed, matching Finding 35's
theorem exactly. Ran all three pairwise coincidence checks and got a
clean, immediate, and total answer: every pair shows a witness from
K=1 onward, permanently, checked exhaustively through K=14. See
Finding 36, tests/sigma11_genuine_pisot_coincidence_test.cpp.

This closes the loop AM opened by asking whether the gcd-obstruction
construction could ever be Pisot (Finding 35: no, provably) and then
asking for the next step: go find where it actually applies, and it
turned out to apply cleanly and immediately. A good stopping point for
this particular thread -- the "companion theorem" chase (Findings
25-27, 34) turned out to be genuinely interesting combinatorics about
a class of substitutions adjacent to, but distinct from, real Pisot
substitutions, and the real-substitution case (the one that actually
matters for strong coincidence) resolves easily once you're looking
at the right object.

### 2026-08-06 (PC session, "text adventure" continues) — Finding 37: real non-unit Pisot conjecture instances established

AM pushed for an actual conjecture target, not another verified
instance of something already covered. Literature check surfaced the
right opening: Barge (2016) proved the classical Pisot conjecture for
"injective-first, constant-last-letter" substitutions, but only in the
unimodular-framed setting -- the non-unit extension (Minervino-
Thuswaldner) is newer and less picked-over. Searched for genuine
non-unit Pisot substitutions in Barge's class: an initial narrow
exhaustive check at alphabet 3 found none (raised, then refuted, a
"maybe this class is always unimodular" hypothesis -- alphabet 4-5
randomized search found real hits at a normal rate). Ran 10
structurally diverse examples through the full corrected pipeline
(strong coincidence + property F), catching and fixing a real
methodological risk along the way (the M/M-transpose-guessing pattern
in the existing nonunit_family_sweep.cpp driver can silently pick a
mathematically WRONG eigenvector -- switched to the dedicated,
independently-verified left_eigenvector_via_qbeta instead). Result:
10/10 ESTABLISHED. See Finding 37,
app/probe_barge_class_nonunit_instances.cpp.

Explicitly NOT claimed as a completed theorem or literature priority --
flagged honestly as strong, fresh evidence needing (a) a real
structural proof (not case-by-case) and (b) independent
literature/peer review before any public claim. AM's immediate next
ask: study property (F) itself more deeply, since it's the piece
without a general structural theorem yet (unlike strong coincidence,
which has Finding 17's clean constant-factor proof) and is named as
the actual key to cracking anything bigger. That's the next thread.

### 2026-08-06 (PC session, roadmap Stage 1) — Finding 39: zero-run bound generalizes Finding 17 to the whole terminating family

AM's reminder to draw on the project's existing generator/twist
machinery (built for extending single-example results across
dimensions in the n-bonacci contact-boundary work) pointed at the
right existing tool for a different problem: terminating_generator_
theorem.hpp's already-proven, unconditional characterization of
transport generators for the WHOLE canonical terminating-expansion
family (any N, any digit sequence). Used that family (not the
n-bonacci twist-dominance machinery itself, which turned out to be
about a separate, unrelated "Condition F" concept, confirmed earlier
this session to have zero dependency on property (F)) as the basis
for widening Finding 17's proven coincidence result past its trivial
"no zero digit" special case.

Swept the family systematically instead of checking one-off examples:
found a clean pattern (coincidence depth = longest run of consecutive
zero digits, plus one) holding exactly across 22 structurally diverse
test cases, then proved it via the substitution's own pass-through
mechanism (a zero digit means a single-letter, non-branching image --
a chain of k such letters delays reaching a genuine leading-zero image
by exactly k steps). See Finding 39,
include/ravel/proof/zero_run_forces_bounded_coincidence.hpp.

This is the roadmap's first real Stage-1 win: a proven (not observed)
coincidence result covering an entire infinite family with an exact
depth formula, not just the trivial depth-1 case. Next: either widen
further (past the terminating-expansion family, e.g. eventually-
periodic expansions) or move to Stage 2 (attempt property F's
distinctness conjecture for this now-solidly-proven family).

### 2026-08-06 (PC session) — literature check: Durand & Petite (arXiv:1408.2110), and crediting Findings 17/38 correctly

AM flagged a new paper: Durand & Petite, "Conjugacy of unimodular
Pisot substitution subshifts to domain exchanges" (2014, rev. 2023).
Read the actual PDF text, not a summary. Major result: every
unimodular Pisot substitution subshift is measurably conjugate to a
domain exchange, UNCONDITIONALLY -- no strong coincidence hypothesis
needed, unlike every prior construction (Rauzy, Arnoux-Ito,
Canterini-Siegel). Does NOT prove the full Pisot conjecture -- they're
explicit that the second step (domain exchange to actual toral
rotation) remains open, and sharpen it beautifully: the whole
remaining gap reduces to finding a single point in the torus with a
unique preimage under their factor map.

Checked whether they reproduce Findings 17/38 (constant-first/last-
letter forcing depth-1 coincidence): no. Their key technical device,
"proper substitution" (starts AND ends with the same letter), is
exactly our two conditions combined -- but "proper" is pre-existing,
decades-old terminology (Dekking 1978, Queffelec's textbook), not
theirs or ours, and they only remark in one unproved, uncited sentence
that properness gives "a form of coincidence" -- an aside supporting a
different argument, not a developed or exactly-stated result. Updated
Findings 17 and 38 with an honest addendum: general folklore belongs
to the field, the exact depth-1 statement and its proof from
pair_has_coincidence's own code were independently derived here,
before this literature check.

Open thread if this comes back: is this project's property-(F)
gamma-distinctness conjecture the same question, in different
language, as Durand-Petite's "unique preimage point" reduction? Not
yet checked.

### 2026-08-06 (PC session) — Finding 40: return-word induction, sigma_{0,1}'s depth 13 collapses to 2

AM's ask: keep working on coincidence, implement something useful from
Durand & Petite (arXiv:1408.2110) along the way. Their key device is
Durand's older "return substitution" construction. Nearly duplicated
it from scratch -- caught mid-way that this project ALREADY has an
implementation (include/ravel/return_substitution.hpp, predating this
session), discarded my new one after using it to independently find
and fix a real bug (wrong position array -- token's own length vs.
cumulative sigma-image length), then validated the EXISTING tool
against the same two literature checks (Tribonacci self-reproduction;
the paper's own worked example via matching dominant eigenvalues,
Proposition 8). Both pass.

Applied the validated tool to sigma_{0,1}, this project's own longest-
standing hard coincidence case (worst-case depth 13). No letter
generates its own fixed point directly, so used sigma^3 instead. The
resulting 5-letter return-word-induced substitution resolves every
pair within depth 2 -- a real, verified, dramatic reduction. Checked
this isn't a universal artifact: applied to a Finding-39 zero-run case
where the construction is trivial (already has a valid marker), the
induced substitution comes back IDENTICAL to the original, no
reduction at all. See Finding 40,
tests/return_word_coincidence_reduction_test.cpp.

Honestly scoped: no theorem yet connecting "induced substitution
resolves fast" to the original's own strong coincidence property --
a real, useful analytical tool and result, not a closed proof. Natural
next step if this comes back: try to formalize that connection, or
test the reduction pattern on more "power-needed" hard cases to see
how general it is.

### 2026-08-06 (PC session, roadmap Stage 1 continued) — Finding 41: eventually-periodic zero-run bound, wraparound correction

AM's call: return-word thread paused ("wait for the paper's importance
to assert itself"), pivoted back to direct coincidence work. Natural
next Stage-1 step: extend Finding 39 (zero-run bound) from the
terminating canonical family to the eventually-periodic one. A first
attempt (reusing Finding 39's flat-sequence zero-run computation
directly) gave real, confirmed wrong predictions whenever a zero-run
spans the period's own wraparound boundary (period=(0,1,0): flat scan
predicts depth<=2, actual=3; period=(0,0,1,0,0): predicts<=3,
actual=5) -- caught by deliberately constructing stress cases with
zero digits at both ends of the period, not by accident. Fixed: R must
be the max of the flat-sequence run and the run within period++period
(catches wraparound-adjacent zeros becoming a genuine single
pass-through chain once the period repeats). Verified against 12
cases including the two stress tests. See Finding 41,
include/ravel/proof/zero_run_forces_bounded_coincidence_periodic.hpp.

Strong coincidence is now proved, with an exact depth formula, for
every canonical Pisot substitution this project can currently generate
(terminating or eventually-periodic) -- both families this project's
own digit-expansion tooling produces are now fully covered. Next
Stage-1 candidates if this continues: reprove Barge-Diamond's binary
case independently, or move to Stage 2 (property F's distinctness
conjecture) now that the coincidence foundation is much more solid.

### 2026-08-06 (PC session, final stretch) — Finding 42: Barge-Diamond fully reimplemented, reflection-to-Lean pipeline working end-to-end

AM's "reprove Barge-Diamond" request went through several rounds of
sharpening: first a computational verification of their conclusion via
this project's own coincidence machinery (rejected -- not the point);
then a from-scratch geometric reimplementation of their actual
apparatus (segments, stable/unstable eigenspaces, the B and M
finiteness bounds -- built and verified to stabilize numerically);
then a hard architectural correction -- no Lean proof should be
authored by hand, only emitted by mechanically unfolding a reflection
trace recorded by the executing math machinery itself, reusing
math/proof_reflection.hpp's existing (if narrowly-used) trace/render
infrastructure rather than inventing a new one.

Landed on a complete, correct, end-to-end pipeline: certify_barge_
diamond (exact, rational-root-theorem irreducibility check, long-long
arithmetic, degree 2-3 scope stated honestly) records an
IntegerEigenvectorNoWitness trace node the moment it establishes
beta's irrationality; the extended reflective_lean_renderer walks
EVERY such node in a trace (not a single hardcoded pattern) and emits
a Lean corollary instantiating the general, independently kernel-
checked lemma (lean/barge_diamond_lattice_line.lean) with that node's
own concrete matrix -- w, i, and the eigenvector equation stay
universally quantified, only the matrix is concrete. Ran on 3 real
substitutions, rendered, kernel-checked with zero errors and zero
sorry. See Finding 42.

For d=2 this closes the FULL Strong Coincidence Conjecture, exactly,
for every substitution the pipeline is run on -- a real, reproducible,
machine-checked result, not a hand proof.

Explicitly scoped OUT of this session: "retrofit literally every
finding in the project through this pipeline" is real, and next, but
it is its own large project -- needs a proper plan, not an in-session
scramble. AM is compacting manually; the next thing after this entry
is scoping that retrofit properly.
