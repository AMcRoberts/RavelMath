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

Not yet done: steps 7–12 (full contact-boundary enumeration, the smallest
ordered-parent-role comparison object, the simultaneous generator
inequalities, finite-positive-word/norm-weighted closure invocation, and
Lean emission). Since there is no cyclotomic cofactor at the substitution
level, step 11 (cyclotomic sector audit) is currently vacuous for the raw
substitution; it may still matter once boundary/twisted objects are built.

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
