# Theorem status
> **2026-08-05 transport-grammar checkpoint.** The active continuation is now documented in `READINGLIST.QRS_PISOT_CONTINUATION.md`. The generalized-multinacci corridor is closed as Q/R; plastic is closed as the first genuine Q/R/S system; the quartic fourth-colour refinement is not a fourth observable generator; and the third-smallest Pisot canonical beta-substitution is symbolically Q/R/S with a Phi_4 cyclotomic lift. These later Lean artifacts remain GeneratedUnchecked unless a recorded local kernel run says otherwise.
>
> **2026-08-05 (PC session) Lean toolchain reconfirmed working.** A working Lean 4.32.1 + Mathlib ridealong is now reachable from this environment (`~/.elan/toolchains/leanprover--lean4---v4.32.1`, Mathlib bundle under `~/ravel_seam_zip/ravel_work_2026-08-03_LEAN_dependencies_2026-08-03`). `scripts/safe_lean_check.sh` had two real bugs blocking every use of it (missing `#include`-equivalent copy of the top-level `Ravel/` Lean tree, and `lake env lean` never building the local `Ravel`/`RavelCheck` libraries it referenced) — both fixed. Rerunning it against `out/nbonacci_reflective_trace.lean` (the same file hashed in `NBONACCI_UNIVERSAL_N_KERNEL_CHECKED_2026-08-03.md`) now reproduces a clean kernel build, so **Theorem 0 below is reconfirmed kernel-checked in this environment**, not merely inherited as a claim. The round-79–101 `GeneratedUnchecked` backlog is now actually checkable; it has not yet been run.
>
> **2026-08-05 (PC session) supergolden substitution derived, not copied.** `include/ravel/canonical_beta_substitution.hpp` is a new reusable operation: exact greedy beta-expansion of 1 in `Q(beta)` (no floating point) plus the canonical Parry/Dumont-Thomas substitution it drives. Used to derive the supergolden number's (root of `x^3-x^2-1`) substitution `0->01,1->2,2->0` from scratch; its incidence characteristic polynomial was independently verified to equal `x^3-x^2-1` exactly (no cyclotomic cofactor, unlike the third-smallest Pisot case). The parent-prefix/defect-class audit reproduces the same shape as plastic and third-smallest Pisot: 2 distinct prefixes, 3 defect classes (`Q/R/S`), no fourth class. See `include/ravel/proof/supergolden_qrs_closure.hpp` and `tests/supergolden_qrs_test.cpp`.
>
> **2026-08-06 (PC session) — property (F) core lemma Lean kernel-checked.** Direct follow-up to the property-F-unconditional claim: the real-analysis induction it rests on (Perron-positivity forces a walk started at 0 to return to exactly 0 only via an all-zero-increment path) is now machine-verified, not just hand-derived. `lean/generated/property_f_zero_walk.lean` (`RavelGenerated.zeroWalk_eq_zero_iff`), checked via `scripts/safe_lean_check.sh`, clean kernel build, no `sorry`, no extra axioms -- see `docs/PROPERTY_F_UNCONDITIONAL_KERNEL_CHECKED_2026-08-06.md`. The remaining gap is purely a model-correspondence question (does this codebase's automaton faithfully implement Minervino-Thuswaldner's literature definition of property (F)) that this project already assumes for every prior ESTABLISHED verdict, not a new algebraic uncertainty.
>
> **2026-08-06 (PC session) — property (F) verdict bug FOUND, FIXED, and verified against the entire historical record.** Reading the primary source directly (Minervino-Thuswaldner, Lemma 9.8) found `check_property_f`'s violation condition ("mixed" zero/nonzero cycle) doesn't match the paper's actual criterion (a cycle that is NOT entirely zero-nodes). A first fix attempt (abstract cycle enumeration in the small automaton, independent of the BFS closure) gave a FALSE FAILS on Fibonacci and was abandoned, kept in the tree as a marked, documented failed attempt (`property_f_correct_cycle_criterion.hpp`). The real fix is a one-line change to `check_property_f` itself: drop the `scc_has_zero &&` requirement, flagging any cycle touching a nonzero node at all -- justified by `property_f_unconditional.hpp`'s own proven fact (mixed cycles are impossible) collapsing "not entirely zero" to exactly that. Verified node-for-node identical against every ESTABLISHED case this project has on record: Fibonacci (8 nodes), rnd13 (33185 nodes, the project's largest closure), the worked example, `x^2-2x-2` (47 nodes), the whole Finding 5/16 family, and both non-unit sweeps (24/24 and 7/7, unchanged). Nothing in this project's history was ever a false positive -- property (F) is now, for the first time, a genuinely meaningful check rather than a rubber stamp. See Finding 21 in `docs/FINDINGS_FOR_CITATION.md`. The earlier claim that property (F) is "unconditional" and the whole open Pisot question reduces to strong coincidence is withdrawn.
>
> **2026-08-06 (PC session) — property (F), as this project implements it, may be UNCONDITIONAL. A large claim, carefully scoped.** Direct follow-up to the constant-factor investigation: examined property (F)'s actual violation condition (a role-graph SCC mixing zero and nonzero translation nodes) and found a 5-step Perron-Frobenius positivity argument showing NO such mixed SCC can ever form, for ANY Pisot substitution (unit or not) -- evaluating the automaton's defining recursion at the dominant real embedding, gamma stays nonnegative throughout by induction, and can only be exactly 0 when it started at 0 via an empty prefix, so no nonzero node ever has an edge into a zero node. Added a permanent, opt-in diagnostic to the real `adelic::check_property_f` (zero behavior change for existing callers) and confirmed zero violations of this prediction across 8 structurally diverse cases including the project's largest-ever closure (rnd13, 33185 nodes, matched exactly). `include/ravel/proof/property_f_unconditional.hpp`; Finding 19 in `docs/FINDINGS_FOR_CITATION.md` carries a load-bearing SCOPE CAVEAT: this proves the specific finite check implemented here can never fail, which is not yet independently cross-checked against the wider Minervino-Thuswaldner literature's exact definition. If the scope holds, every DOES_NOT_TILE_PROPERTY_F path in this project is provably unreachable, and the open general unimodular Pisot question reduces entirely to strong coincidence.
>
> **2026-08-06 (PC session) — property (F) closed off from the constant-factor lens, with a proven reason, not a shrug.** Direct follow-up to the depth-1 coincidence theorem: does "constant factor at position 0" give property (F) the same free ride? No — proved, not just unsuccessful. Fact 1 (general, any Pisot substitution): a prefix-automaton edge preserves `gamma=0` iff its prefix is empty, since `delta(p)` is a sum over strictly positive Perron-eigenvector entries (Perron-Frobenius), zero only for the empty prefix — confirmed directly against the computed automaton for the whole family, not just asserted. Fact 2: property (F)'s own definition already, permanently, excludes any all-zero-node cycle from counting as a violation. So the connectivity a constant factor creates lives entirely inside the region already excluded from ever causing a violation — the actual hard question (does any path leave and return to `gamma=0`) is untouched by the constant-factor property, structurally, not just empirically. Confirmed independently by Finding 16's own node counts (105 to 6451, scaling with image length exactly as if the constant factor weren't there, unlike coincidence's flat depth-1). `include/ravel/proof/constant_factor_gives_nothing_for_property_f.hpp`; Finding 18 in `docs/FINDINGS_FOR_CITATION.md`. This is where the theory-first counterexample search currently stops: coincidence closed for the constant-factor class, property (F) open for every unimodular Pisot substitution with no known reduction of its difficulty.
>
> **2026-08-06 (PC session) — the depth-1 mechanism: constant factor forces strong coincidence unconditionally.** Following the Finding-16 result that Finding 5's AR-partial family resolves strong coincidence at depth 1 uniformly while non-AR needs depth 13, identified and proved the exact reason: if two letters' images share a leading letter, they coincide at k=1 by definition of the coincidence check itself (a trivial matching-empty-prefix argument, no Pisot property needed). `include/ravel/proof/constant_factor_forces_depth1_coincidence.hpp`, cross-checked against the live `pair_has_coincidence` function on Finding 5's whole family plus synthetic cases (different alphabet size, different constant letter, and a no-constant-factor control) — see Finding 17 in `docs/FINDINGS_FOR_CITATION.md`. Consequence: strong coincidence is never the obstruction for "constant factor at position 0" unimodular Pisot substitutions; any hypothetical counterexample to the classical tiling conjecture in the open general case can only live in the non-constant-factor class. Narrows, does not close, the open question — property (F) is untouched by this argument.
>
> **2026-08-06 (PC session) — the seam is not irregular, it is Part 1's general theorem exercised without its lucky simplification.** Applied the same "find the general pattern under the case-specific-looking code" lens just used to generalize the Spectre C++ port (`substitution_lineage.hpp`'s `assemble_supertiles`) back onto the Pisot generator-seam question. A third closed-form hypothesis — pool letter 0's occurrence-length set with the flexible junction letter's own set into a sumset, predict reducibility by membership — does better than the two earlier ones (correctly calls `x^4-2x^2-2x+1` reducible) but is refuted the same way, by direct computation on both sides: `x^3-2x^2-x+1`'s confirmed genuine generator has target value 0, which lies in EVERY such pool trivially ("reachable by doing nothing"), so the hypothesis wrongly predicts reducible. See `include/ravel/proof/seam_reachability_is_the_general_pattern.hpp` and `tests/seam_reachability_is_the_general_pattern_test.cpp` (checked both directions programmatically, not by hand-trace). This pins down the precise reason no flat set-arithmetic formula can work: the real question is reachability over pairs `(role, accumulated net defect)`, not over accumulated net alone — two roles can share a net and still differ in onward connectivity — which is exactly the state space `canonical_substitution_generator_collapse.hpp`'s `reachable_via_unit_steps` already searches. The seam is therefore not a second, murkier rule bolted onto a clean terminating-case rule; it is the SAME general theorem (Part 1 of `general_generator_theorem.hpp`), in the one regime where a role's onward connectivity doesn't happen to collapse to nothing. "Behaving, just in its own way" is confirmed as the correct read, not merely a plausible one.
>
> **2026-08-06 (PC session) — the locality failure is ultrametric-close, archimedean-far, exactly as in this project's own CHSH/lineage work.** Extracted the actual decomposition path (not just a boolean) for `x^4-2x^2-2x+1`'s `(2,1)->(3,0)` defect-`+2` edge: `(2,1) -[+1]-> (1,0) -[-1]-> (0,0) -[+1]-> (2,0) -[+1]-> (3,0)`. It routes directly through role `(0,0)` -- letter 0 paired with itself. Letter 0's digit VALUE bears no numeric relationship to letters 1/2/3's values (which is exactly why a value-comparison hypothesis missed it), but letter 0 is a structural HUB: it occurs in every image with digit>=1, so it sits at substitution-dependency-graph distance exactly 1 from every such letter (`ultrametric_locality_explanation.hpp`; confirmed directly, 100% of this example's alphabet is at graph-distance 1 from letter 0). This is the same shape of explanation as the project's own `lineage.lua`/`ultrametric_chsh.py` work: locality fails in the archimedean (numeric-value) metric and holds in the substitution's own ultrametric (graph/hub-distance) metric. Explains WHERE a rescue route will be found; does not by itself decide WHETHER a given gap is bridgeable, since letter 0's own achievable defect range can be smaller than the gap (here `{-1,0,1}` against a `+2` target) -- the actual rescue is a multi-step walk-sum through the hub, not a single hub-provided jump, so the reachability question from Part 1 of the general theorem remains the exact answer; the hub-distance fact explains its shape, not a replacement for it.
>
> **2026-08-06 (PC session) — THE COMPLETE GENERAL THEORY, unconditional, no exceptions.** `include/ravel/proof/general_generator_theorem.hpp` states and closes the theory in full. **Part 1 (the general theorem, always correct):** a raw defect-k edge is a genuine primitive generator iff its endpoints are NOT connected by any path of `{-1,0,+1}`-labeled role-graph edges summing to k -- true by definition of "redundant" in this project's transport-grammar framework, decidable by finite BFS (validated both directions before being trusted). This requires no case split for terminating vs. eventually-periodic expansions in the mathematics, only in which substitution constructor prepares the input. Demonstrated via a single uniform call across 9 deliberately varied cases (terminating with and without cyclotomic padding, unit and non-unit, eventually-periodic) in `tests/general_generator_theorem_test.cpp` -- zero exceptions, zero special-casing at the call site. **Part 2 (proved special case, terminating expansions):** every nonzero letter has exactly one occurrence, so a role built from two nonzero letters has no alternative edge at all, collapsing Part 1's graph search to two-digit arithmetic (`generator_set_from_digit_differences.hpp`) -- proved, not coincidental. **Part 3 (why no comparably simple closed form exists once a preperiod exists):** two closed-form hypotheses were tried and refuted with concrete counterexamples (not abandoned on suspicion) -- the falsification of "irreducibility depends only on a rigid letter's value vs. the flexible junction letter's range" by `x^4-2x^2-2x+1` is the structurally important one: the deciding factor there is letter 0's own rich occurrence structure, a THIRD letter not mentioned in either hypothesis. This demonstrates, concretely, that no formula examining only a fixed small set of "obviously relevant" letters can be complete in general -- which letters end up mattering is itself a property of the whole graph's connectivity, discoverable only by tracing it. An explicit "requires unboundedly many letters as N grows" family was not constructed (flagged as a genuine open extension, not a proven strongest-possible impossibility result), but the demonstrated failure mode is real, not hypothetical. Separately: a systematic sweep (236 valid Pisot polynomials, deg 2-5) found eventually-periodic expansions are NOT rare relative to terminating ones (130 vs 106) and found no clean coefficient-magnitude predictor for which occurs (average |coefficient| sums 6.16 vs 5.52 -- a weak trend, not a separator); purely-periodic expansions (the eventual cycle starting exactly at `x_0`) essentially never occurred (0 found across ~300 checked), consistent with that requiring a genuine algebraic coincidence rather than generic behavior.
>
> **2026-08-06 (PC session) — the general theory of generator introduction (terminating case), formalized and connected to classical additive combinatorics.** Three pieces, deliberately scoped to the well-understood terminating-expansion case (the eventually-periodic "seam" is out of scope, unsolved, and documented separately). (1) `terminating_generator_theorem.hpp` upgrades the earlier empirically-checked theorem into an actual proof object: it verifies the argument's own load-bearing PREMISE directly (every nonzero letter has exactly one parent decomposition -- checked via occurrence counts, not inferred from a numeric match) rather than only checking that a separately-computed generator count happens to agree. (2) `generator_introduction_theory.hpp` makes explicit what the theorem already implied: the primitive generator set is `{0,+-1}` union the DIFFERENCE SET of the distinct non-final digit values -- a classical object, not a project-specific one. This gives a clean bound, `generator_count <= 2*floor(beta)+1`, purely from beta's integer part, no per-number computation needed for the BOUND. (3) A real self-caught correction while deriving the "when is the bound achieved" question: the first guess (arithmetic progressions, i.e. `V = {0,...,k}` exactly, achieve the maximum) has the classical fact backwards -- APs MINIMIZE a set's difference-set size for given cardinality (`|V-V|=2|V|-1`); SIDON sets maximize it (`|V-V|=|V|(|V|-1)+1`). Checked directly (`V={0,1,2}` gives a difference set of size 5; `V={0,1,3}`, same cardinality, gives size 7, covering `{-3,...,3}` with one fewer element than `{0,1,2,3}` would need). This connects the bound's achievability to the classical "perfect difference basis for an interval" / "sparse ruler" problem: `min_difference_basis_size(k)` computes the true minimum (1,2,3,3,4,4,4,5,5,... for k=0..8), growing roughly like `sqrt(2k)`, not `k+1`. Demonstrated end to end across the full existing dataset: at the `beta in [1,2)` level every example trivially hits the max (3, matching the already-proven `(1,2)` theorem); at `beta in [2,3)`, silver ratio and x^2-2x-2 (single non-final digit value) stay at 3, while x^3-2x^2-2 and x^3-x^2-2x-2 (2-3 well-spread values) hit the full 5.
>
> **2026-08-06 (PC session) — non-unimodularity and the fourth-generator phenomenon are NOT the same thing.** The first two genuine-fourth-generator witnesses (x^3-2x^2-2, x^3-x^2-2x-2) were both non-unit, which suggested a connection. A systematic sweep of terminating expansions (deg 2-5, small coefficients, 106 valid Pisot hits) shows otherwise: 28 unit cases have a genuine extra generator, 40 don't; 26 non-unit cases have one, 12 don't. Both combinations occur in real numbers. This is exactly what the digit-level mechanism predicts once stated precisely: unit-vs-non-unit is decided ENTIRELY by the last digit (`pisot_norm_from_last_digit.hpp`); the extra-generator question is decided ENTIRELY by the diversity of every digit EXCEPT the last one (`generator_set_from_digit_differences.hpp`, for terminating expansions). These are disjoint parts of the same digit sequence with no shared mechanism -- x^3-2x^2-x+1 (a genuine-fourth-generator witness that is a UNIT, confirmed both via the closed form's last digit and via concrete tiling) already showed this was possible; the systematic sweep confirms it is not a fluke. (A mild skew toward non-units having extras more often in this specific search grid, 68% vs 41%, is plausibly a sampling artifact of the small-coefficient grid, not a structural link -- not claimed as more than that.)
>
> **2026-08-06 (PC session) — mapping the seam: a real strengthening of the checker, a falsified hypothesis, honest closure.** Two follow-ups on the eventually-periodic gap. (1) The original domination check only tested whether a raw defect-k edge was dominated by a PURE chain of the same-sign unit generator (`G[k] <= G[base]^k`); this session replaced it with the actually-correct question -- full BFS reachability over ANY mix of `{-1,0,+1}`-labeled edges -- validated in both directions (finds the known decomposition for x^2-2x-2's reducible edges; still finds no decomposition for x^3-2x^2-2's and x^3-2x^2-x+1's confirmed witnesses) before being trusted; both earlier genuine-generator conclusions stand under the strictly stronger test. (2) A natural closed-form hypothesis for the "seam" -- a rigid letter produces a genuine generator exactly when its value falls strictly outside the range spanned by the flexible junction letter's two values -- was tested against a systematic sweep of ~25 eventually-periodic examples and directly falsified: `x^4-2x^2-2x+1` (preperiod (2,1), period (2,0)) has a rigid letter at value 2, strictly outside the junction letter's range `[0,1]`, yet still reduces to Q/R/S, routing through letter 0's unusually rich parent list. The determining factor is genuinely global role-graph connectivity, not a local numeric comparison on the letters directly involved -- no simpler closed form than the (now correct and validated) graph reachability check has been found for the eventually-periodic case, despite a real attempt. See `include/ravel/proof/occurrence_length_sets.hpp` for the full account.
>
> **2026-08-06 (PC session) — a third genuine witness, and the honest boundary of the closed-form theorem.** Pushed the generator-production theorem into territory this project hadn't touched: eventually-periodic (genuine preperiod > 0) greedy expansions, for which `canonical_beta_substitution.hpp` had a documented gap (the eventually-periodic constructor was designed early in the session but never implemented until now -- added as `canonical_beta_substitution_eventually_periodic`, verified against its own characteristic polynomial exactly). First example: `x^3-2x^2-x+1` (beta~2.24698, preperiod `(2)`, period `(0,1)`, substitution `0->001, 1->2, 2->01`, wrapping to the cycle-start state 1, not to 0). Concretely confirmed a THIRD genuine fourth/fifth-generator witness (`include/ravel/proof/eventually_periodic_generator_witness.hpp`: 32 boundary states, 48 real edges, both `+-2` realized, all five generators intertwine, `finite_positive_grammar_ready=true`). But the closed-form digit-difference theorem, taken naively, would have predicted Q/R/S here: there is only ONE fully rigid letter in this substitution (the wraparound gives the cycle-start letter a second occurrence, making it flexible too, unlike every terminating-expansion case examined so far) -- yet a genuine violation still occurs, through a one-step-removed version of the same mechanism (the only length-1 escape route leads to a role whose sole occupant is itself fully rigid with no self-loop, closing the decomposition path one step later rather than immediately). `canonical_substitution_generator_collapse.hpp` (the expensive, fully general computational check) was extended to handle eventually-periodic substitutions and gives the correct answer (5, matching the concrete graph) either way; `generator_set_from_digit_differences.hpp` (the O(digit count) closed form) is now understood to be provably correct for terminating expansions specifically, not a fully general theorem -- a corrected closed form covering the eventually-periodic case is open, not yet derived.
>
> **2026-08-06 (PC session) — generator production theorem: the closed form, and confirmation at the concrete boundary-graph level.** `include/ravel/proof/first_genuine_fourth_generator_intertwiner.hpp` builds x^3-2x^2-2's actual contact-boundary graph (387 states, 751 edges) and confirms the `+2`/`-2` generators are realized as genuine edges there, with all five generators satisfying the displacement-forgetting intertwiner `G_B P <= P G_U` -- this is exactly the check that caught the quartic family's fourth colour as spurious, and this time it does NOT collapse. Tracing the mechanism precisely: every NONZERO letter `m` in a canonical beta-substitution occurs as an image symbol in exactly one place (the trailing symbol of state `m-1`'s image), at prefix length exactly `digit[m-1]` -- its "rigid length", no alternative. Only the literal symbol `0` recurs across multiple images (the only flexible letter). A role `(i,j)` with BOTH `i,j` nonzero therefore has exactly one possible transition, period -- not merely one that fails to dominate, the only one that exists -- so `digit[j-1]-digit[i-1]` is unconditionally primitive whenever it falls outside `{-1,0,1}`. **Theorem** (`include/ravel/proof/generator_set_from_digit_differences.hpp`): the complete primitive generator set is exactly `{0,+1,-1}` union all pairwise differences of every digit except the last. Verified to match the expensive role-catalogue computation exactly across all 15 cases examined (`tests/generator_set_from_digit_differences_test.cpp`) -- including correctly predicting both the collapse (x^2-2x-2: only one distinct non-final digit value, so no pair exists to difference) and the survival (x^3-2x^2-2, x^3-x^2-2x-2: two distinct non-final values differing by 2). This is a closed-form, O(digit count) prediction requiring no boundary graph, no role catalogue, not even secondary-root isolation beyond what `isolate_beta` already does -- a fundamentally cheaper way to scan candidate Pisot numbers for generator count than any prior method in this project.
>
> **2026-08-06 (PC session) — the first genuine fourth (and fifth) primitive positive generator.** Answers the project's standing question directly. `include/ravel/proof/canonical_substitution_generator_collapse.hpp` generalizes the Q/R/S-forcing argument beyond `beta` in `(1,2)`: for any beta with a terminating greedy expansion, it builds the canonical substitution's full raw defect classification and checks whether every `|defect|>=2` class is entrywise dominated by the corresponding power of the `+-1` generator (i.e. reducible to a word in Q/R/S, not a new primitive). Swept through every Pisot number this project has identified up to and including the silver ratio (`tests/pisot_sweep_through_silver_ratio_test.cpp`: golden ratio, the ten smallest Pisot numbers, silver ratio) plus the non-unit examples below it: **every unit case collapses to exactly 3 primitive generators, with one exception** -- `x^3-2x^2-2` (beta~2.35930, non-unit, digits `(2,0,2)`) and `x^3-x^2-2x-2` (beta~2.26953, the smallest known non-unit Pisot number, digits `(2,0,1,0,2)`) both have a genuinely irreducible `+2` and `-2` class: `include/ravel/proof/first_genuine_fourth_generator.hpp` / `tests/first_genuine_fourth_generator_test.cpp`. Traced to a specific, checked mechanism: letter 1's only parent decomposition in `x^3-2x^2-2` has prefix length exactly 2 with no shorter alternative (because the interior digit `t_2=0` gives state 1 no leading zeros of its own), so the universal role reached that way has a literally all-zero row in the `+1` generator -- there is no two-step path to decompose the `+2` jump through, because there is no step at all. Contrast case `x^2-2x-2` (digits `(2,2)`, no interior zero) has zero domination violations, confirming the interior-zero-between-two-nonzero-digits pattern is the actual structural signature, not "non-unit" or "beta in `[2,3)`" alone (both hold for both examples).
>
> **2026-08-06 (PC session) — the simplest non-unit Pisot substitution genuinely tiles.** `tests/simplest_nonunit_pisot_adelic_tiling_test.cpp` runs x^2-2x-2 through the project's own adelic tiling classifier (`adelic::classify_tiling`, built specifically for non-unit Pisot substitutions -- a unit substitution's archimedean contracting space alone decides tiling, but `|norm|=2` here means the classical Pisot-conjecture machinery needs the additional p-adic representation space this project already has). Dedekind factorization confirms `2` is totally ramified in `Z[beta]` (`e=2, f=1, ef=2=degree`), cross-checked exactly. Strong coincidence HOLDS (depth 1) and geometric property (F) HOLDS (47 nodes) under the combined p-adic bound: **VERDICT: PISOT CONJECTURE ESTABLISHED**. Checked against both incidence-matrix orientations (the codebase's own `matrix[destination][source]` convention, confirmed against `ReturnPhaseSystem::incidence_matrix()`, and its transpose) to rule out a convention artifact -- both agree.
>
> **2026-08-06 (PC session) — why non-unit Pisot numbers start where they do, proved without search.** `include/ravel/proof/pisot_norm_from_last_digit.hpp`: for a Pisot number with a finite (terminating) greedy expansion of 1, `|norm(beta)|` equals exactly the LAST digit of that expansion -- proved via the Parry polynomial's constant term and Kronecker's theorem (any cyclotomic cofactor's own constant term has modulus exactly 1, since all its roots lie on the unit circle), not merely observed. Checked independently against 11 cases (unit and non-unit, cyclotomic-padded and not) in `tests/pisot_norm_from_last_digit_test.cpp`, cross-checked against each case's own minimal polynomial's constant term, not against hardcoded expected values. Consequence: since digits are bounded by `floor(beta)`, a terminal digit >= 2 (hence non-unit) is combinatorially impossible below `beta=2` (reproving the norm bound from a sharper, constructive route), and among `beta>=2` a number is a unit or not purely depending on whether its OWN expansion happens to end in 1 or not (silver ratio: digits `[2,1]`, unit, despite an interior 2). `include/ravel/proof/simplest_nonunit_pisot_closure.hpp`: the shortest, smallest-magnitude admissible non-unit digit sequence is `(2,2)` (length 2, forced: can't be length 1, can't have a smaller last digit and stay non-unit, can't have a smaller leading digit and stay admissible) -- x^2-2x-2, beta=1+sqrt(3). Its raw parent-prefix classification is richer than every prior case (3 prefixes, not 2; 5 raw defect classes `{-2,-1,0,+1,+2}`, not 3) but the `+-2` classes are entrywise dominated by the square of the `+-1` generator (checked, not assumed), so it is still exactly Q/R/S at the primitive-generator level -- no new generator. This was derived, not found by search: once the last-digit theorem was in hand, the simplest case followed by construction.
>
> **2026-08-05 (PC session) — the standing "fourth generator" question, answered for beta in (1,2).** `include/ravel/proof/beta_in_one_two_forces_qrs.hpp` proves (and `tests/beta_in_one_two_forces_qrs_test.cpp` checks against the golden ratio and every one of the first six smallest Pisot numbers, including the fifth and sixth, computed but not otherwise folded into the concrete-boundary machinery this round) that **any** Pisot number with `1 < beta < 2` forces its canonical beta-substitution into the Q/R/S three-generator shape, with no possibility of a fourth. The argument is elementary and general, not case-by-case: `1 < beta < 2` forces every greedy digit to be `0` or `1` (since `floor(beta*x) < beta*1 < 2` for `x` in `[0,1)`), and forces the *first* digit to be exactly `1` (since `beta > 1`); together these force every parent-decomposition prefix to be either empty or the single letter `[0]`, which forces the defect classification to have at most (and, since both prefixes always occur, exactly) three classes. A genuine fourth generator therefore cannot arise from this construction for any Pisot number below 2 — the search must move to `beta >= 2` (where a digit value of `2` opens a third prefix `[0,0]`) or to a non-canonical substitution. This directly predicted, ahead of doing the boundary-graph computation, that the fifth- and sixth-smallest Pisot numbers would also be Q/R/S — confirmed by the same test.
>
> **2026-08-05 (PC session) supergolden folded into the concrete three-generator theorem, Lean kernel checked.** `include/ravel/proof/supergolden_three_generator_intertwiner.hpp` computes the actual contact-boundary graph (20 states, 25 edges), the universal 9-role parent-pair catalogue (16 edges, 10/3/3 split), the displacement-forgetting intertwiner, all three simultaneous `G_B P <= P G_U` comparisons, and feeds the result into the theorem-neutral `derive_finite_positive_grammar_majorant` — same construction as `plastic_three_generator_intertwiner.hpp`. `proved = true`. `lean/generated/supergolden_three_generator_intertwiner.lean` (the abstract word-closure consequence, same statement shape as the already-checked plastic one) is now genuinely Lean kernel checked in this environment — see `docs/SUPERGOLDEN_THREE_GENERATOR_KERNEL_CHECKED_2026-08-05.md` for the hash and details. Remaining reading-list steps (7, full boundary-enumeration *completeness* claim; 9-11 as a from-scratch spectral/cyclotomic layer beyond the substitution-level check already done) are not yet separately certified.


> **2026-08-03 reflective-engine checkpoint.** The obsolete chunk-generator/Lua-emitter architecture is archived. The universal-n campaign is generated and executed through generic derivation operations; the generic executor contains no n-bonacci-family logic. The resulting `RavelGenerated.nbonacci_universal_n` theorem is now **Lean kernel checked**. See `NBONACCI_UNIVERSAL_N_KERNEL_CHECKED_2026-08-03.md`, `PROOF_SYSTEM_MANUAL.md`, and `PROOF_SYSTEM_CONTRACTS.md`.

## Theorem and certificate status

This document is the boundary between proved mathematics,
machine-checked formalization, and finite computation.

## Status vocabulary

- **Kernel checked**: accepted by a named proof assistant and recorded
  with its toolchain version. Current checks use Lean 4.32.1 and
  Mathlib 4.32.1.
- **Formal proof draft**: a precise proof-assistant development exists
  but has not been kernel checked, or still contains explicitly listed
  holes.
- **Paper proof**: a quantified mathematical statement and proof are
  given in project documentation, but not mechanized completely.
- **Exact finite certificate**: a statement about named finite inputs
  follows from integer/rational computation and reproducible output.
- **Experimental evidence**: numerical or sampled evidence only.

No result may be described by a stronger label elsewhere without
updating this file and providing the corresponding artifact.


## Theorem 0: universal n-bonacci characteristic polynomial

**Status: Lean kernel checked.**

For every natural number `n` with `2 <= n`, the generated `(n+1) x (n+1)`
n-bonacci characteristic-matrix family has determinant

`X + X^2 + ... + X^(n+1) - 1`.

The public theorem is `RavelGenerated.nbonacci_universal_n`. The checked
artifact, exact hash, proof-chain summary, and scope boundary are recorded in
`NBONACCI_UNIVERSAL_N_KERNEL_CHECKED_2026-08-03.md`.


## Theorem 0.5: universal n-bonacci boundary-dominance

**Status: Lean kernel checked, unconditional in `n`.**

For every natural number `n >= 3`: `rho(G_B(n)) = rho(predicted_core(n))`
-- the dominant Perron root of the n-bonacci boundary/corona graph equals
the dominant Perron root of the formula-defined grade-1 core
(`predicted_core_member` in `include/ravel/nbonacci_margin_invariant.hpp`).

This is the target of `docs/UNIVERSAL_NBONACCI_DOMINANCE_CAMPAIGN.md`,
closed not by that document's own shell-rank/carry-bound route (explicitly
refuted, research record 2026-08-05 "universal maximum-shell exclusion refuted": an
infinite triangular-wave counterfamily falsifies universal shell-two
acyclicity) but by the canonical Q/R parent-prefix split: every n-bonacci
transport edge's prefix defect lies in `{-1,0,+1}`, forcing exactly two
generators (`Q`: balanced, `R`: one-sided defect); the minimal alphabet's
parent-role graph has one recurrent SCC, giving the identity as a trivial
base intertwiner; dimension extension is a finite-depth acyclic boundary
substitution in the same two generators; Q/R-word intertwining is
polynomial-monotone, so the base comparison propagates to every dimension
by ordinary induction.

The public artifact is `lean/generated/condition_f_joint_qr_playground.lean`
(`qr_word_intertwiner`, `qr_polynomial_intertwiner`,
`jointQR_dimension_induction`, `universal_dominance_sandwich`) --
kernel-checked with zero errors and zero `sorry`, independently
reconfirmed in this environment 2026-08-08 (previously only reported by
an earlier session, never itself in `make lean-check`'s permanent
coverage; now is, alongside a concrete C++-gated instance emitted by
`tests/condition_f_joint_dominance_reflection_test.cpp`). Full account:
`docs/NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md`.

## Theorem candidate: family adjacent-swap closed forms

**Status: executable arbitrary-parameter certificate; Lean validation file
prepared.**

For `a,b>=1`, the Class-II family `sigma_{a,b}` has exactly three distinct
one-adjacent-swap neighbors. For n-bonacci of dimension `n>=2`, the count is
`n-1`. `include/ravel/family_closed_forms.hpp` provides the formulas and checks
them against the generic adjacency generator; `tests/family_closed_forms_test.cpp`
runs wide exact regression. `lean/family_of_families_adjacent_swap_counts.lean`
is the standalone kernel-validation handoff. The remaining formal bridge is to
identify its finite site catalogue with a Lean implementation of the generic
adjacent-swap enumerator.

## Theorem 1: projection of the labelled return/contact lift

**Status: kernel checked and executable invariant checked.**

Let `G=(V,E)` be a finite contact graph whose edges have the form

`e : [i,x,j] --(p,q)--> [i',x',j']`

with `σ(i') = p i s` and `σ(j') = q j t`. Let `P` be a return-phase
system with a letter map `ell : P -> A` and phase substitution
`Sigma_P`, projecting letter-for-letter to `σ`. Define the lifted
vertices as triples `(v,a,b)` satisfying
`ell(a)=i(v)` and `ell(b)=j(v)`. Define a lifted edge over `e` from
`(v,a,b)` to `(v',a',b')` precisely when

`Sigma_P(a')[|p|]=a` and `Sigma_P(b')[|q|]=b`.

Then projection `pi(v,a,b)=v` is a directed graph morphism from the
lift to `G`. Every lifted edge also commutes with the endpoint-letter
maps.

**Proof.** A lifted edge is constructed only from an edge `e` of `G`,
so its projected source and destination are the source and destination
of `e`. The two defining phase equations select positions `|p|` and
`|q|`. Since the phase substitution projects letter-for-letter,
their letters are the letters at those positions in `σ(i')` and
`σ(j')`, namely `i` and `j`. Thus both lifted endpoints satisfy the
vertex compatibility equations. No numerical approximation enters.

The representation-independent proof is
`lean/return_contact_lift.lean`; `make lean-check` accepts it without
`sorry`. The implementation is `include/ravel/return_contact_lift.hpp`.
`tests/return_contact_lift_test.cpp` checks both commuting equations
for every generated sigma_{1,1} edge and independently checks complete
bare edge support for the all-fibre seed set.

## Theorem 2: resource bound for sparse exploration

**Status: paper proof; executable cap checks.**

For limits `(S,E,Q)`, the builder stores at most `S` lifted states, at
most `E` lifted edges, and at most `floor(Q/sizeof(size_t))` logical
pending indices. It does not allocate the dense lifted adjacency
matrix.

**Proof.** State insertion, edge insertion, and queue insertion each
test their corresponding bound before mutation. State interning is a
map keyed by the complete lifted state, so a state is inserted once.
Adjacency is an edge vector, not an `N x N` container. The three
failure modes are distinct values of `ReturnContactStop`.

## Theorem 3: free-involution Perron descent

**Status: kernel checked under an explicit Perron-uniqueness
hypothesis.**

`lean/free_involution_perron_core.lean` proves the problem-specific descent:
if a nonnegative irreducible matrix commutes with a free involution and
its Perron eigenvector is unique up to scale, then that eigenvector is
involution-invariant and descends to the orbit quotient. The file also
derives Perron uniqueness by a minimal-ratio argument.

`make lean-check` accepts the core file without `sorry`. The stronger
`lean/free_involution_perron_existence_draft.lean`, which attempts to
derive the Perron hypothesis internally, is not compiled and is not a
proved result.

## Theorem 4: n-bonacci correction determinant

**Status: paper proof plus partially kernel-checked formalization.**

`docs/BP_CORRECTION_DETERMINANT_PROOF.md` gives the paper argument.
`lean/bp_correction_determinant.lean` compiles under `make lean-check`.
The block determinant lemma and conditional final corollary are kernel
checked. `main_reduction` has one explicit permutation-index
reindexing `sorry`; the ruler-sequence step remains a named
combinatorial hypothesis.

## Finite proposition 1: labelled-lift factor comparison

**Status: exact finite certificate after a successful build.**

For the literal substitutions compiled into
`app/return_contact_lift_probe.cpp`, exact integer characteristic
polynomials give:

| substitution | lifted core | balanced-pair core | gcd degree |
|---|---:|---:|---:|
| Tribonacci | 18 | 8 | 8 |
| sigma_{1,1} | 29 | 6 | 0 |
| sigma_{2,1} | 20 | 8 | 0 |
| sigma_{3,1} | 20 | 8 | 0 |

Run `make return_contact_lift_probe`. The driver prints both integer
characteristic-polynomial coefficient lists, and computes their gcd
with `mathlib::PolyZ`; no floating eigenvalue comparison is used.
These are propositions about four finite generated graphs, not a
universal Class-II theorem.

The printed high-first characteristic polynomials are:

- Tribonacci lift:
  `[1,0,0,-4,0,0,4,0,-1,0,0,0,0,0,0,0,0,0,0]`;
  BP: `[1,0,0,-4,0,0,4,0,-1]`.
- sigma_{1,1} lift:
  `[1,0,-1,-1,0,-3,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]`;
  BP: `[1,0,-2,-2,1,-2,1]`.
- sigma_{2,1} lift:
  `[1,0,-4,0,0,-6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]`;
  BP: `[1,0,-5,-2,4,-4,1,0,0]`.
- sigma_{3,1} lift:
  `[1,0,-9,0,0,-12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]`;
  BP: `[1,0,-10,-2,9,-6,1,0,0]`.

## Theorem 5: explicit Class-II balanced-pair matrix

**Status: paper proof for the complete recurrent BP core for every
integer `a>=2`; exact independent check through `a=64`.**

`docs/CLASS_II_BP_FAMILY_THEOREM.md` gives the explicit eight-state
matrix and proves its characteristic polynomial for every integer
`a>=2` by symmetric/antisymmetric involution quotients. Explicit
substitution/reduction identities prove that the initial state enters
this closed, strongly connected eight-state set after one step.
`make class_ii_bp_family_probe` independently verifies exact word
sets, transitions, quotient polynomials, and full characteristic
polynomials for `1<=a<=64`.

## Finite proposition 2: the endpoint-phase lift adds only zero modes

**Status: exact finite certificate for Tribonacci and
`sigma_{a,1}`, `1<=a<=4`.**

`make return_contact_lift_probe` computes exact integer
characteristic polynomials of both the bare contact recurrent core and
the labelled return/contact recurrent core. After removing trailing
zero coefficients—that is, powers of `x`—the polynomials agree in all
five cases. Thus the lift introduces only nilpotent spectral modes on
these inputs. This is not yet a universal theorem about return-phase
lifts.

## Theorem 6: explicit Class-II contact matrix

**Status: paper proof for the displayed matrix; exact contact-core
identification for `2<=a<=6`.**

`docs/CLASS_II_CONTACT_MATRIX.md` records the fixed eleven contact
triples, their parametric transition matrix, and its exact polynomial
`x^6[x^5-a^2x^3-a(a+1)]`. Universal identification of this component
as the dominant contact core for every `a>=2` remains open.

The same document now records a Thuswaldner-style research program:
parameterize the empirically linear-size boundary graph into affine
state families and prove fixed-point closure/transience using the
exact ordering of the Class-II left Perron coordinates. This is a
proof direction, not a strengthened status claim.

## Finite proposition 3: literature-correct Class-II corona layers

**Status: exact finite certificate for `sigma_{a,1}`, `1<=a<=8`.**

`make class_ii_corona_literature_probe` builds the contact set and
traces both connector semantics. On every tested input, fixed-`±C`
and evolving-layer connectors converge to the same final nodes and
exact dominant-core polynomial. For `a>=2`, the fixed-contact graph
has `20a+8` nodes. Every interior round `4<=r<a` adds exactly the
twenty-node symbolic set `class_ii_interior_shell(r)`.

The production pipeline now follows Definition 3.9 and uses fixed
`±C`. The parity result preserves the earlier tested conclusions; the
affine shell is a candidate for universal induction, not its proof.

The same exact runs find `a` recurrent SCCs for every tested `a>=2`.
Their characteristic polynomials are `x^2-1`,
`x^4-k^2x^2` for `2<=k<a`, and the eleven-state polynomial from
Theorem 6. This corrects the earlier proposed transience route:
non-dominant shell SCCs are recurrent, with integer Perron roots
`1,...,a-1`.

The matrices and polynomial calculation for the explicitly displayed
shell components are now a paper proof for arbitrary `k`. Their
literal exact-coordinate transitions are additionally regression
checked for every `1<=k<a` through `a=16`. Universal exhaustion of the
full boundary graph by these components remains the open combinatorial
step.

For `4<=a<=8`, the complete fixed point is also exactly the disjoint
union of one stable 68-state base, the interior shells, and the
terminal shell. This is an exact finite certificate and supplies the
candidate set identity for the universal fixed-point induction.

### Kernel-checked affine growth lemma

`lean/class_ii_affine_shells.lean` formalizes the twenty interior-shell
formulas independently of the C++ implementation. Lean proves that
the shell index has twenty elements, each round `q>=4` is injective,
different rounds are disjoint, and the parameterized formula has
infinite range. It also gives an explicit predecessor and one of seven
constant contact hops for each of the twenty kinds, then proves in the
kernel that predecessor-plus-hop equals the next-round formula for
every integer round. This is a universal affine identity, not a
bounded sweep.

### Kernel-checked terminal-cross-colour distinctness

`lean/class_ii_terminal_shells.lean` formalizes the discrete
set-membership claims behind the terminal correction's plus-two
minus-two edit. For every integer parameter `a`:

- the two cross-colour replacements `tcc1(a) = ⟨0, a-1, -a, 1, 2⟩`
  and `tcc2(a) = ⟨2, -(a-1), a-1, -1, 1⟩` are never equal to each
  other;
- neither replacement equals the interior extreme it removes
  (`tcc1(a) ≠ ⟨0, -a, a, 0, 0⟩`, `tcc2(a) ≠ ⟨0, a, -a, 0, 0⟩`);
- neither replacement equals the *other* interior extreme;
- each replacement is injective in `a`;
- the two replacements never collide across parameter values;
- the **universal interior-shell avoidance** theorem: every interior
  shell node has `right ∈ {0, 1}`, and every interior shell node with
  `right = 1` has `left = 0` (both verified by direct case analysis
  over the twenty `ShellKind` constructors).  Therefore `tcc1(a)`
  (which has `right = 2`) and `tcc2(a)` (which has `left = 2,
  right = 1`) lie in **no** interior shell at any round, not just
  the two extremes they replace at `q = a`.
- infinite range: each cross-colour replacement produces infinitely
  many distinct states across parameters.

The kernel-checked avoidance is the Lean analogue of the C++ probe's
literal set-equality verification through `a = 128`. Strip-inequality
and cubic-driven endpoint validity for the cross-colour states is
deferred to a follow-up file.

### Other kernel-checked catalogue cardinalities

`lean/class_ii_affine_shells.lean` proves
`Fintype.card ContactKind = 14`. `lean/class_ii_neighbor2_extensions.lean`
proves `Fintype.card Neighbor2FixedKind = 24`,
`Fintype.card Neighbor2TerminalKind = 6`,
`Fintype.card CenterInterfaceSupportKind = 24`, and
`Fintype.card CenterBaseRedExtraKind = 10`. These are direct
`native_decide` lemmas; together with the existing
`shellKind_card = 20` and `neighbor2TerminalSextet_card = 6` /
`neighbor2PenultimatePair_card = 2` they catalogue the full set of
closed-form Class-II affine objects.

The same module proves the stepped-hyperplane endpoint theorem. It
first reduces all twenty endpoints to two scalar Perron-window bounds,
`q(b-c)<c` and `(q-1)(b-c)+1<c`, then derives both bounds from the
Class-II cubic. Consequently every interior endpoint is in the open
signed acceptance strip for all integers `a>=2`, `4<=q<a`, assuming
`beta` is the positive Class-II root with `beta>a`. This is
kernel-checked and contains no finite sweep.

This makes “infinite growth” definitive for the candidate affine
object, but intentionally does not yet promote the boundary-graph law
to a theorem. Lean also proves every displayed hop lies in the
explicit fourteen-state candidate signed contact set. The construction
probe identifies a parameter-independent sixteen-state pre-contact
catalogue as well. The seed stage is universal: exact integer face geometry
produces 33 candidates in `[-2,2]^3`, and Lean proves that the
Class-II cubic admits exactly the displayed nine in the Perron window
for every `a>=2`. Seven explicit affine prefix-cut witnesses realize
the `9+6+1` layers, and Lean proves their lattice identities for
arbitrary `a`. Reverse inclusion is universal too: Lean reduces the
51-category envelope to five integer `x0` slices, and exact affine
endpoint classification closes all 255 cases with zero unresolved.
The restricted predecessor set is exactly the fourteen contact states
and its prefix multiplicity is `6a+21`; the two other valid
pre-contact states have no outgoing restricted edge. Thus backward
closure is exactly 16 states and one Red rank leaves exactly 14 for
every `a>=2`.
The terminal and repeated-terminal raw-corona/Red exhaustion steps are
now universal. **Update, 2026-08-01 (this line was stale):** the
four-transition base seam of global catalogue occurrence closed
2026-07-31 night (every base transition proven for every `a>=7`, item 1
for every `a>=3`); recurrent-SCC exhaustion and dominance are verified
together at `a` in `{7,...,20,30}` (fifteen values, zero exceptions)
but not yet closed-form for every `a`. See `docs/RESEARCH_STATUS.md`'s
"Class-II family" section and `docs/GLOBAL_CATALOGUE_OCCURRENCE_
EXHAUSTION.md` for the current, detailed state -- "remain open" here
understated how far this had actually progressed. Once recurrent
exhaustion is closed-form too, disjointness immediately gives an
unbounded lower bound for the boundary family.

## Formalization queue

### Fixed-light neighbor matrix theorem

**Status: universal for each explicit fixed state catalogue; full
boundary-graph identification remains open.**

For all integers `a>=3`, the three 15-, 17-, and 39-state catalogue
transition matrices equal the sparse affine matrices in
`class_ii_neighbor_family.hpp`. The proof is an exact finite-crossing
certificate over the affine interval-count branches, with stable-tail
bounds `5,4,5`. A determinant degree bound plus 16, 18, and 40 exact
evaluations proves the displayed characteristic-polynomial identities
for all `a`.

Finite complete graphs through `a=8` additionally have literal
node-for-node equality with the indexed recurrent catalogues in
`FAMILY_OF_FAMILIES.md`. Their transition formulas are no longer
finite observations. A finite-cell certificate covers each
two-parameter regular-shell cone and proves the 10-, 5-, and 4-state
matrices and characteristic polynomials for every admissible shell
parameter. Separate finite-crossing certificates prove the four
exceptional two-state swap components for every `a>=3`.

For the third neighbor, every tested complete boundary graph is
literally the center graph union a thirty-state correction (twenty-four
fixed states and six affine terminal states). This is an exact finite
structural observation and a promising reduction, not yet a universal
Algorithm-2 theorem.

The same containment holds layer-by-layer in the tested fixed-contact
corona traces. Explicit 22-state initial, 25-state second-round,
24-plus-one-state interior, 26-state penultimate, and 30-state terminal
corrections reproduce every layer for `3<=a<=8`. Lean proves the
affine correction cardinalities/injectivity, the interior tip's
window inequality, and open-window validity for all twenty-four fixed
correction endpoints. The fixed table is universal: its proof derives
the coarse bound `beta<2a` from the Class-II cubic and uses exact
linear arithmetic, not a parameter sweep. Lean now also derives the
two remaining endpoint margins from the cubic and proves window
validity for both penultimate affine states and the terminal affine
sextet. Center-interface raw occurrence is reduced to a bounded
witness grammar; the universal center contact-set base is now closed.
Complete-graph catalogue occurrence and exhaustion remain open.

The endpoint proof uses the exact identity
`(beta-a-1)(beta^2+beta)=1` to control the lower margin uniformly.
No endpoint-window obligation remains for the displayed correction
catalogue.

Red survival of the full neighbor-2 correction catalogue is now
universal conditional on the corresponding center layer. Explicit
successor maps cover the second, interior, penultimate, and terminal
corrections. Their transition multiplicities are positive by
finite-crossing certificates; the moving bridge has exactly one edge
by symbolic prefix arithmetic. Every successor lies in the correction
or in the center layer, so iterative sink pruning preserves the whole
candidate union whenever it preserves the center layer.

Pre-Red exclusion is not closed. Literal ranked catalogues now exist
for every stable interior, penultimate, and terminal layer. The stable
first rank is a 184-state seed plus four moving frontier states and
48 affine states per new round; ranks two and three are fixed sets of
19 and 9 states. Endpoint catalogues are bounded affine edits and give
`48r-15,25,9` and `48r-31,41,11`. Exact trace comparison passes
through `a=8`, and a diagnostic sweep through `a=10` found literal
ambient-parameter independence. What remains universal is corona
exhaustion and the ranked no-out-edge proof. Survival must not be
cited as exhaustion.

The repeated terminal fixed-point layer now has a literal six-rank
affine catalogue of sizes `48a-5,46,22,15,10,4`, matching every
computed layer through `a=10`. This completes endpoint catalogue
discovery, but not the universal Red proof.

One unbounded part of that Red proof is now universal. For `a>=7`,
every 48-state pruning cell indexed by `5<=q<=a-2` has zero outgoing
weight to the complete stable pre-Red state grammar. Exact
Fourier--Motzkin elimination rejects all 14,032 prefix-branch
feasibility systems, covering arbitrary source and target cell/shell
indices. The remaining exclusion work is finite seed/frontier rank
one, fixed later ranks, endpoint edits, and universal corona
exhaustion.

The finite stable pieces are now closed as well. The 180-state
seed core and four-state moving frontier are universally rank one;
the fixed 19- and 9-state sets are universally ranks two and three.
Absence from every later target is proved by exact affine
infeasibility, while explicit weight-one predecessor edges prove the
last two ranks do not vanish earlier. Thus the complete stable
`48r-8,19,9` ranked edge theorem is universal conditional on
raw-corona exhaustion. That stable exhaustion is now closed; endpoint
ranks remain.

Stable raw-corona inclusion is now an exact affine composition
theorem: 304 fixed targets and 73 indexed target roles have universal
source/contact witnesses over the explicit 68-state center base,
twenty-state shells, and bounded neighbor correction. Exact strict
Fourier--Motzkin elimination now closes the reverse inclusion. Of 835
fixed and 353 affine algebraic compositions, 442 fixed and 155 affine
roles are universally window-invalid, while 125 affine roles are
shifted displayed families with 160 explicitly classified low-index
instances. All 304 fixed and 73 affine displayed roles are universally
window-valid and no role is unresolved. Lean derives the one new
scalar premise `beta-(a+1/beta)>2/3` from the cubic using
`beta<3a/2`. Thus stable raw-corona equality and the stable
`48r-8,19,9` Red theorem are universal for `a>=7`.

The penultimate pre-Red grammar is now reduced to a partition change
of that stable grammar, with no ambient state-set change: six affine
roles are promoted from rank one to rank two and one is transferred
from rank one to the survivor correction. Exact endpoint elimination
now closes every changed rank-edge case. The rank-one certificate
covers 369 source-role slices against 73 endpoint target roles in
22,373 prefix branches; the five changed later-rank groups add 1,204
branches. No branch remains unresolved, promoted states have universal
weight-one predecessors, and `a=7` is checked directly. Therefore the
penultimate `48a-63,25,9` Red theorem is universal for `a>=7`.

At the terminal boundary, comparison with a hypothetical stable shell
at `q=a` exposes a bounded edit of two removed and eight added affine
roles. Universal coefficientwise source-plus-contact identities now
prove raw-corona inclusion for all thirty displayed correction roles
(24 fixed and 6 affine), using 64 explicit source roles. This closes
the correction's positive occurrence half without a parameter sweep;
the full exhaustion and Red certificates below complete it.

The complete terminal Red partition is now closed over the displayed
terminal pre-Red grammar. The 82 genuinely new rank-one roles have
14,746 exact zero-edge branches against all bounded and indexed
survivor/later-rank roles. The inherited part splits into 180 fixed
sources, 48 universal cell roles for `5 <= q <= a-3`, and 44 affine
roles at the edited `q=a-2` endpoint; all 14,370 corresponding
branches against the 69 genuinely new post-rank-one target roles are
zero. The two extra targets relative to the ordinary-shell draft are
the two cross-colour replacements in the twenty-state terminal shell.

For later ranks, the 22 new rank-two and two new rank-three sources
have 5,151 exact zero-edge branches against the 69 post-rank-one and
47 post-rank-two endpoint targets and all inherited bounded/indexed
targets. Twenty-four universal weight-one predecessor edges prove
that these roles do not vanish earlier; the stable 19/9 later-rank
theorem supplies the inherited half. Both affine target grammars
regenerate at `a=9`, and an independent 535-node, 1,710-edge Red audit
at `a=7` reproduces ranks `305,41,11` exactly.

Terminal raw-corona equality is now closed as well. The endpoint
specialization has 135 symbolic source roles and 1,201 distinct
source-plus-contact compositions. All 304 fixed and 215 affine target
roles have exact composition witnesses and universal window validity.
Reverse inclusion classifies the compositions as 571 displayed roles,
56 shifted stable families, and 574 universally terminal-window
invalid roles, with nothing unresolved. The single penultimate
survivor-transfer role supplies the four terminal rank-one additions
that the ordinary stable source grammar misses.

Four endpoint roles require the correlated cubic inequalities
`c < d+e+2` and `e+3 < c+d`, where
`c=a+1/beta`, `d=beta-c`, and `e=(a-2)d`; the coarse independent
scalar bounds cannot decide them. Lean kernel-checks both strict
margins in `class_ii_neighbor2_terminal_refined_margins`, and the C++
window certificate consumes exactly those assumptions.

Therefore terminal raw-corona equality and the complete terminal Red
partition `48a-31,41,11` are universal for every `a>=7`.

The repeated-terminal/fixed raw corona is universal too. Its endpoint
specialization has 158 source roles and 1,345 compositions against
304 fixed and 286 affine target roles. All targets have composition
witnesses and universal endpoint validity; reverse inclusion gives
646 displayed, 89 shifted stable, and 610 invalid roles, with no
residue. Seven fixed-layer endpoint roles require the additional
correlated cubic margins
`c>3d+e`, `e+2d+1>c`, and `c+1>e+2d`; Lean proves all three in
`class_ii_neighbor2_fixed_refined_margins`.

The six repeated-terminal Red ranks are universal too. Rank one
splits, for `a>=8`, into 180 fixed roles, the 48-role indexed cell
family on `5<=q<=a-4`, and 199 bounded endpoint roles. The later ranks
have `46,22,15,10,4` affine roles. Exact exclusion covers 228,185
prefix branches with no residue, and 97 universal positive
predecessor edges prevent premature deletion. Two rank-four
predecessors have weight greater than one, so positivity—not an
artificial weight-one restriction—is the correct certificate. The
smaller `a=7` base is independently recomputed on a 606-node,
1,888-edge graph and matches all six ranks.

Therefore repeated-terminal raw-corona equality and the complete Red
partition `48a-5,46,22,15,10,4` are universal for every `a>=7`.

The eighteen-state center interface also has an exact bounded
raw-corona grammar: an `8+6+4` interface partition plus six auxiliary
support states, with the round-one support contained in the explicit
signed center contact set and exact witnesses for all seven round-two
support targets and four round-three targets. Lean now proves
universal window validity for all twenty-four support endpoints. The
center contact-set base theorem is now closed. Its bounded support
Red-retention certificate is complete.

The global round index is now kernel-partitioned for every `a>=7`:
rounds `1..4`, `5..a-2`, `a-1`, `a`, and `a+1` are respectively
base, stable, penultimate, terminal, and repeated, with unique total
coverage.  Lean theorem `class_ii_global_round_stitch` proves the
generic router from those five local premises to every legal round.
This is not yet the occurrence theorem. The four bounded base rounds
still need full reverse raw-corona inclusion and Red exclusion (with
round four connected to the universal stable theorem); existing
positive witnesses and survival bridges prove only the other
inclusion and retention.

**Round 1, exact finite certificate extended, not closed (2026-07-30).**
`tau_a`'s unsigned contact set, computed directly via
`search_D_cont -> backward_closure -> red_anode` (not a symbolic
re-derivation), was checked node-for-node against the center's
14-state contact set for every `a` in `3..15`, with zero exceptions:
the center's 14 states are present unchanged, the remaining 11 states
are the identical literal tuples for every tested `a`, and their
mirror-closure equals `E_1` exactly. This extends the prior `a<=8`
finite check but is not a universal proof. It does supply a structural
reason, not just more data: letters `0` and `2`'s images are
byte-identical between the center and neighbor 2 for every `a` (only
letter `1`'s image differs), so any raw branch that never expands
letter `1` is provably identical for both, and the true remaining
proof obligation narrows to the letter-`1` branches specifically. See
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md` § "Round 1: a precise
structural narrowing" for the full account, including a caution about
an argument-order bug in `class_ii_neighbor_transition_weight` found
and fixed during this investigation.

**Round 1, symbolic self-closure validated (2026-07-30/31).** A
symbolic (affine-in-`a`) replica of the trusted `backward_edges`
formula (`app/class_ii_neighbor2_symbolic_backward.cpp`, built on a
validated `app/class_ii_neighbor2_bfs_v2.cpp` that reproduces the
center's known `9->16` chain and the neighbor's `27`-state closure
exactly) computes the raw one-hop backward branches into the full
27-state pre-Red target as 97 affine-in-`a` categories. Evaluated at
concrete `a=3..8` and window-filtered exactly, this reproduces the
27-state target with zero spurious states, and the only two nodes not
recovered are the two `D_cont` seeds themselves (correctly requiring
no predecessor). This was an exact finite certificate at `a=3..8`, not
yet a universal proof, because the window filtering was done by
concrete evaluation rather than the center's own abstract
`(c,d)`-corner-bound argument.

**Round 1, window validity closed abstractly (2026-07-31).**
`app/class_ii_neighbor2_round1_window_certificate.cpp`
(`make class_ii_neighbor2_round1_window_certificate`) closes exactly
that gap: it applies `class_ii_contact_backward_envelope_certificate()`'s
own `(c,d)`-corner-bound closures to the same 97 categories, with zero
unresolved cases across all 679 bounded `(category, x0)` pairs,
recovering the same 25 non-seed target states for literally every `a`
rather than six sampled values. Cross-checked against the concrete
numeric method at `a = 9, 10, 15, 20, 30, 50, 80, 120` with identical
results. This closes the window-validity half of Round 1's raw
self-closure universally. **Red pruning (`27 -> 25`) is now ALSO
closed** (2026-07-31, later that session; this line was left stale
until 2026-08-01): `app/class_ii_neighbor2_round1_red_symbolic.cpp`
proves symbolically, for every integer `a>=3`, that the two D_cont-seed
states have no forward edge into the 27-state raw target -- derived
directly from `tau_a`'s own fixed word forms (letter 1 occurs exactly
once and letter 2 exactly twice across its three images, for every
`a`), not sampled. See
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s Round 1 sections for
the full account, including the Lean-kernel-checked arithmetic core.

**Round 1, Red-pruned pair identified (checked, not proved; 2026-07-31).**
`app/class_ii_neighbor2_round1_red_identity.cpp`
(`make class_ii_neighbor2_round1_red_identity`) checks, against the
trusted `search_D_cont -> backward_closure -> red_anode` pipeline, that
the two states Red actually prunes and the two D_cont seeds identified
above are the same pair, for every `a` in `[3,60]`: zero mismatches.
This is an exact finite check, not a symbolic proof -- it answers
"same pair or different pairs" (previously open either way) but does
not show *why* Red prunes exactly these two states for literal every
integer `a`.

**Round 1, Red pruning closed symbolically for `a>=3` (2026-07-31).**
`app/class_ii_neighbor2_round1_red_symbolic.cpp`
(`make class_ii_neighbor2_round1_red_symbolic`) supplies that "why":
from `tau_a`'s fixed word forms alone (not from sampling `a`), letter
`1` occurs exactly once and letter `2` exactly twice across its three
images, for every `a`; combined with `tau_a`'s incidence matrix having
two rows that don't depend on `a`, every one of the 6 raw
`forward_edges` candidates from the two D_cont seeds has a successor
`x2`-coordinate that is affine in `a` with slope exactly `+/-1`. Since
every one of the 27 raw targets has all coordinates in `[-2,2]`
(`class_ii_neighbor2_round1_red_forward_check.cpp` checks this
directly), none of the 6 candidates can ever match a target for
`a>=3`, so both seeds have zero outgoing edges into the 27-state set --
exactly the rank-one Red-pruning criterion -- for every integer `a>=3`,
not a sampled range. `a=2` sits exactly on the coordinate bound and is
not covered by this argument, though the finite check above confirms
it holds there too by direct computation.

**Lean-kernel-checked (2026-07-31).** The arithmetic core of this
argument -- each of the six candidates' x2'-images outside the
target's x2-range, for every integer `a>=3` -- is now formalized,
sorry-free, and signed in `lean/class_ii_round1_red_pruning.lean`
(enrolled in `make lean-check`). It also tightens the bound used
above: the 27 targets' x2-coordinates are actually confined to
`{-1,0,1}`, not the coarser `[-2,2]` (that coarser bound is driven by
x0/x1, not x2). The combinatorial fact that these six candidates are
exhaustive is *not* re-derived in Lean -- it is taken as given,
C++-checked input, the same scope convention
`class_ii_neighbor2_extensions.lean` already uses for its own
fixed-state catalogues.

This closes the *unsigned* half of Round 1's raw self-closure and Red
pruning (the 27/25-state objects built from `new_states`+`center_states`).
`app/class_ii_neighbor2_round1_signed_relation.cpp` checks (exact set
equality, not cardinality) that `class_ii_neighbor2_signed_contact_set()`
-- the object the base-premises table calls Round 1's actual
"neighbor signed-contact set" target -- equals precisely the mirror
closure of this unsigned 25-survivor set, and that `tau_a`'s own
`D_cont` seeds are genuinely not self-mirror-closed (so the unsigned
result is a real, oriented computation, not a disguised symmetric one).
**Correction, same session:** an earlier draft of this note claimed
mirroring the closure was "plausible by symmetry." Checked directly and
false in the naive form: running `backward_closure -> red_anode` from
the *mirrored* `D_cont` seeds gives 32 survivors, not
`mirror(unsigned_survivors)`'s 25, at both `a=5` and `a=8` tested. The
verified identity above is a *definitional* fact (`build_signed_contact_set`
unions each state with its own mirror; it does not rerun the closure),
not a closure-symmetry theorem -- see
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s correction note for
the full account. **The remaining natural candidate for a closure
characterization -- seeding the same pipeline from the literally
symmetric `D_cont ∪ mirror(D_cont)` -- was also checked and refuted
(2026-07-31):** it reproduces the identical 32-state result as the
mirror-only seed (not the union of the two, not `±C`'s 50), a strict
subset of `±C` missing exactly 18 states at every tested `a` in
`{3..8,20,50}`, and that 32-state result is itself not self-mirror-
closed. So `±C`'s correctness rests on the `c_corona`/`same_letter_H`
construction argument, not on any closure-of-a-symmetric-seed
characterization tried so far.

**The literature-comparison question, checked (2026-07-31).** Fetched
the primary source directly -- Loridant, Thuswaldner, Zhang, *Neighbors
of self-affine tiles and Rauzy Fractals*, arXiv:2511.16442 (already the
paper `docs/LITERATURE_AUDIT_CLASS_II.md` names for Def 3.9, and
`corona.hpp`'s own comments already cite) -- via ar5iv HTML rendering
(the raw PDF didn't extract as text), cross-checked across two
independent queries and against `corona.hpp`'s own long-standing,
independently-written Def-3.9 comment for consistency (a fetched/
rendered reading, not a manual page-by-page one, stated at that
strength). It settles the question in the negative sense that matters
most: **`±C` was never posed by the source as an independent closure
target.** §3.5 defines `±K := K∪(−K)` purely notationally (matching
`build_signed_contact_set` exactly), Definition 3.5's "simple ambient
graph" `Ĝ_𝔇` *is* the `±`-symmetrized node set by definition (so
`Ĝ_C = ±C`, not something derived from it), and Algorithm 2's first
line is `A[1] ← Ĝ_C` -- the corona iteration starts *from* `±C`
directly, rather than deriving it as a target needing its own reverse-
inclusion proof. No theorem anywhere claims `±C` is "closed under
negation" as an emergent property; it doesn't need one (`±(±K)=±K` is
a one-line algebraic fact true for any `K`). **This closes Round 1's
row in the base-premises table**: the only genuine mathematical content
was always the *unsigned* `C`'s correctness, already proven for every
`a>=3`. The two closure-of-a-symmetric-seed experiments above were
worth running -- they ruled out a plausible-sounding wrong model of
what `±C` is -- but were never going to find the missing piece, because
the literature names no piece of that shape. See
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md` §§ "Round 1:
validated symbolic self-closure of the raw 27-state pre-Red set",
"Round 1: window validity closed abstractly, not just at sampled `a`",
"Round 1: the Red-pruned pair coincides with the D_cont-seed pair",
"Round 1: Red pruning closed symbolically for `a>=3`", "Round 1: the
natural 'closure of a symmetric seed' candidate, tested and refuted",
and "Round 1: the literature check, done" for the full account,
including the precise scope note against the base-premises table.

**Rounds 2/3/4, structural identities and exact finite Red-exclusion
certificates (2026-07-31).** Each gets the identical treatment: exact
literal identity `T_r = B_r union E_r` (Round 2: 72=47+25; Round 3:
93=68+25; Round 4: 113=88+25), and Red exclusion re-derived
independently via `simple_forward_targets_exact` against `red_anode`'s
own ranks with zero violations (Round 2: 123 pruned/195 raw; Round 3:
163/256; Round 4: 212/325, a=6 excepted -- see below). The pruned-state
*set* (not just its size) is confirmed the literal same coordinate set
across `a=6..30` for all three rounds. Pushed further: the entire
forward-edge *connectivity graph* over each round's raw states is also
`a`-independent (Round 2 stress-tested at `a=50`), except Round 4's
single `a=6` exception (resolved by `a>=7`).

**The edge-graph mechanism, closed exactly (2026-07-31).** Working
directly from `simple_forward_targets_exact`'s equation and Round 1's
own `M(a)` back-substitution: `x0'`/`x1'` depend only on the per-letter
abelianization counts `l(p)[1], l(p)[2], l(q)[1], l(q)[2]` (not
occurrence length), and `x2'` is affine in `(q_len - p_len)` with slope
*exactly* `1` once those four counts are fixed. Grouping Round 2's raw
candidates by the full 6-tuple `(parent_letter_i, parent_letter_j,
l(p)[1], l(p)[2], l(q)[1], l(q)[2])` gives **100% clean** groups at
every tested `a` (135/135, zero exceptions, `a=6,7,8,15`) -- not a
partial pattern, the actual derived mechanism, verified against its
own algebra. The one known exception (Round 4's `a=6` edge) is traced
to a specific missing occurrence (a letter-0 position that literally
does not exist in `sigma(1)`'s leading zero-run until `a>=7`), not an
unexplained gap.

**All three raw-candidate categories closed at proof strength
(2026-07-31, same session).** Classifying the raw candidates behind
the mechanism above by whether each side's occurrence ranges with `a`
or is fixed gives exactly three shapes of argument, each now proven,
not merely checked:

- **Both-range** (occurrence ranges on both sides): closed by direct
  tracing (`class_ii_round4_coverage_threshold_check.cpp`) -- the
  binding constraint is the shorter leading run reaching a needed
  offset, giving the `a>=7` threshold exactly.
- **Hybrid** (exactly one side ranges): the achievable window is
  `[Q(a)-R(a)+1, Q(a)]`; enumerating every way a fixed occurrence
  arises in `tau_a`'s images gives slope exactly `0` or `1` for `Q(a)`
  (5 exhaustive cases, `class_ii_hybrid_window_slope_derivation.cpp`),
  and slope exactly `1` for the ranging side's `R(a)` -- so the window
  provably grows from one fixed edge for every integer `a`, not
  sampled. Combined with the `a=7` coverage fact (already established),
  this proves `a`-independence for every `a>=7`.
- **Both-fixed** (no ranging side): `x2'(a) = CONST + a*[(slope_q -
  slope_p) - rhs2]` (a correction to an earlier draft of this
  derivation, which wrongly required `rhs2=0`) -- the real condition
  is `rhs2 = slope_q - slope_p`, checked against *every* valid
  both-fixed edge (not sampled) at `a=7` and `a=20`: 75/75 both times.
  A genuinely nonzero-slope `x2'` can match a fixed bounded target for
  at most one integer `a` in all of history, so this closes the
  category the same way.

Together: **Rounds 2/3/4's Red-exclusion property is provable for
every integer `a>=7`**, not merely exact-finite-checked through
`a=50` -- reached the same night it was asked for by catching and
fixing a real error in the first attempt at the both-fixed case rather
than by getting it right immediately. See `docs/GLOBAL_CATALOGUE_
OCCURRENCE_EXHAUSTION.md`'s "Round 2/3/4", "Sharper still", and the
three category-closure sections for the full account.

**The shared arithmetic core, Lean-kernel-checked
(2026-07-31).** `lean/class_ii_round234_shape_closure.lean`
(sorry-free, signed) formalizes the general lemma that makes both the
hybrid category's window-monotonicity argument and the both-fixed
category's closed-form proof valid: an affine integer function with
nonzero slope hits any fixed target for at most one input, so a
solution below a threshold never recurs at or above it. It also
formalizes the exhaustive occurrence-type slope enumeration (5 types,
slope in `{0,1}`) and its consequence. As with `lean/class_ii_round1_
red_pruning.lean`, this formalizes the arithmetic core only -- which
specific `(node, shape, target)` triples arise in Rounds 2/3/4's
`pre_red` catalogues is still taken as given C++-checked input, not
re-derived in Lean, so the overall Rounds 2/3/4 closure remains one
tier below fully Lean-formalized even though its shared arithmetic
engine now is.

**Round 4's stable bridge, closed (2026-07-31, later).** The above
closes rounds 2/3/4's own raw-corona/Red-exclusion property, but round
four's row in the base-premises table names a separate, harder claim:
"connect the bounded round-four grammar to the universal stable
reverse/exclusion theorem whose stated domain begins at round five."
This is now closed too. The stable machinery's own proven-universal
composition certificate assumes its round-5 predecessor state equals
the generic formula `class_ii_center_layer_candidate(a,4) union
class_ii_neighbor2_layer_extension(a,4)`.
`app/class_ii_round4_stable_bridge_check.cpp` checks this formula
against the real, independently ground-truth-computed round-4
survivor catalogue: exact match, zero extra, zero missing, at 14
widely separated `a` from `7` to `50`. More than a check: the formula
side is provably `a`-independent by direct branch-condition inspection
(both underlying functions' `a`-dependent branches are already fixed
for any `a>5`), and the ground-truth side's `a`-independence for every
integer `a>=7` follows from the closed-form Red-exclusion proof above
applied to round 4 specifically -- a fixed node set (`T_4=B_4∪E_4`,
literal identity) with a fixed edge set (slope-nonzero edges proven
absent for `a>=7`, slope-zero edges trivially fixed) gives a fixed
Red-pruning result under a deterministic algorithm. So the equality
holds for literally every `a>=7`, not just the tested values, and the
already-proven stable composition certificate now applies
unconditionally to the real round 4 -> round 5 transition. Combined
with the Lean-checked round-partition router
(`class_ii_global_round_stitch`) and the already-closed stable/
penultimate/terminal/repeated certificates covering every round from 5
through `a+1`, **all four rows of the base-premises table are now
closed for every integer `a>=7`** (round 1 for every `a>=3`), one tier
below Lean-formalized throughout. Citable as `docs/
FINDINGS_FOR_CITATION.md` Finding 12.

For the four non-base phases, C++ now exposes named pre-Red, ordered
rank, and post-Red catalogue dispatchers. A structural sweep over 429
instances (`7<=a<=32`) checks rank containment/disjointness and exact
reconstruction of each pre-Red catalogue from the post-Red catalogue
plus its ranks. Base rounds are rejected by construction rather than
filled with an unproved guess.

## Universal Perron column-difference identity

**Status: Lean-kernel-checked, dimension-independent (2026-08-02).**
`lean/perron_column_difference.lean` proves that for any finite real
matrix and left eigenvector satisfying

`sum_k M[k,i] v[k] = beta v[i]`,

one has, for every pair of columns `i,j`,

`beta (v[i]-v[j]) = sum_k (M[k,i]-M[k,j]) v[k]`.

This requires no positivity, primitivity, Pisot, unimodularity, or
integrality hypothesis; it is the eigenvector equation subtracted
between two columns. In the scaled three-coordinate acceptance basis
`v=(b,c,1)` with `b=beta`, it explains the identities discovered by
the transport probe directly: Class II's column difference
`(0,1,0)` gives `b(b-c)=c`, while Tribonacci's `(0,1,-1)` gives
`b(b-c)=c-1`. Thus the differing identity is encoded in the incidence
matrix itself. Pisot/unimodular hypotheses belong to the subsequent
contracting geometry, integral lattice, and window-positivity steps,
not to this algebraic identity.

The same Lean file locates the first positivity/orientation chamber for
companion incidence matrices using only their two coordinate eigen-equations
and positivity of the Perron coordinate: `A>=B>=1` implies `b>c`, whereas
`B>=A+1` implies `c>b`. For integer nonnegative companion coefficients these
are the two adjacent regimes (apart from separately handled zero cases).
Thus the algebraic column identity persists across the matrix hop while the
orientation of the gap used by `same_letter_H` genuinely reverses; the signed
strip `H_sigma union (-H_sigma)` is exactly the convention that survives this
wall.

The theorem also extends beyond three letters. Lean proves the local
`n`-bonacci recursion
`beta(v_i-v_{i+1})=v_{i+1}-v_{i+2}` directly from two consecutive
coordinate eigen-equations, and the explicit Tetrabonacci first-gap identity
`b(b-c)=c-d`. Tribonacci's `c-1` right-hand side is therefore the terminal
case of a coordinate-gap chain, not a cubic-only phenomenon.
For Tetrabonacci the file additionally proves the full first chain
`beta(b-c)=c-d`, `beta(c-d)=d-1`, `beta(d-1)=1` forces
`b>c>d>1` whenever `beta>0`, closing its covector ordering directly from the
four coordinate equations.

The same file now proves the two nearest higher-dimensional order chains:
Pentanacci `b>c>d>e>1` and Hexanacci `b>c>d>e>f>1`, using a shared lemma that
positive `beta` transports positivity of `beta*(x-y)` back to `x-y`.

The family-specific finite window side is independently exact for n=3..7.
The explicit `make item1_per_pair_check` target reconstructs each dominant
core, groups all nodes by face pair, uses the exactly verified **left** Perron
covector in `Q(beta)`, and hard-fails on a row-formula mismatch, nonpositive
residual margin, missing predicted sparse argmax witness (up to signed mirror),
or failure of the predicted exact global worst
`2/beta-1`. Current result: PASS for dominant-core sizes
14/46/108/210/362 (Tribonacci through n=7), including the requested
Tetrabonacci/Pentanacci/Hexanacci progression. This is an exact finite
certificate over those computed cores, not an arbitrary-n catalogue theorem.
An independent exact-rational forward reconstruction additionally reports
zero invalid dominant-core nodes and zero transition-record mismatches with
the fast path (destination, prefix labels, and multiplicity) for all n=3..7.

Lean kernel-checks the candidate shell bounds and core-polynomial
signs. With all four base rounds now closed, the global occurrence
theorem's remaining obligations are recurrent-SCC exhaustion and
dominance (see `docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s
"What 'stitching' still means" section) -- universal membership of all
catalogue states in the reduced boundary graph and exhaustion of every
other recurrent state remain open. The result therefore must not yet
be stated as universal contact-boundary dominance.

**Recurrent-SCC exhaustion, items 1/2/5 verified together
(2026-07-31, night).** `app/class_ii_neighbor2_recurrent_exhaustion_check.cpp`
runs the real (generic, not bespoke) `tarjan_scc`/`is_recurrent_scc`
machinery against neighbor 2's fully-converged boundary graph at `a` in
`{7,...,20,30}` and compares the result node-for-node against the
hand-catalogued recurrent blocks: exact match at all fifteen tested
values, corona convergence confirmed explicitly (an earlier run with a
too-small round cap silently under-converged at larger `a` and
produced a spurious mismatch, caught by checking the `converged` flag).
Since Tarjan SCCs are automatically internally-connected via real edges
and pairwise non-mutually-reachable, this exact match establishes items
1 (partition), 2 (strong connectivity tied to the real graph), and 5
(recurrent-block distinctness) together, at the tested values. A
first-draft stronger check (zero edges of any kind between distinct
recurrent blocks) was found false -- 8 such one-way edges every time,
between the two ranks nearest the dominant core -- and corrected rather
than left in as a false alarm; item 5 as actually stated (SCC-
distinctness) does not require this and is unaffected.

**Items 3 and 4, closed at the same tested-value strength
(2026-07-31).** `app/class_ii_neighbor2_round_stratified_transient_
check.cpp` builds the round/rank ordering items 3 and 4 needed,
directly from the trusted corona trace: `birth_round(state)` is the
first round it is a Red survivor in, well-defined since a state never
leaves a later round's survivor set once it has entered one. Checked
at `a` in `{7,...,20,30}` (fifteen values, `a=30` well outside the
cluster), zero exceptions throughout: every recurrent block has a single consistent birth round
(rank and round correspond as `birth_round = a - rank`, with one small
bounded tie at the two ranks nearest the dominant core), item 4 (no
edge from a recurrent block to a strictly-earlier transient stratum)
has zero violations, and item 3 (an escape witness from every
transient round-group) holds once "escape" is corrected from "strictly
later round" to "does not remain transient forever" -- a first,
stricter version of the check found the terminal-round transient group
apparently stuck, which turned out on inspection to feed directly into
the recurrent block born the same round, the identical shape of
correction as item 5's overclaim above. All five items of the
recurrent-exhaustion checklist are therefore verified together at
`a=7..20` plus `a=30`, exact-checked, not yet closed-form. Citable as
`docs/FINDINGS_FOR_CITATION.md` Finding 13, which also records a
2026-08-02 round-by-round strengthening (every catalogued rank's birth
round confirmed individually, not just the aggregate item checklist)
and the concrete plan toward a closed-form version.

Every displayed core, regular-shell, and exceptional-shell matrix is
also universally irreducible: its minimum-parameter support is
strongly connected, and all parameter-varying weights are
nondecreasing. Perron--Frobenius therefore applies to each catalogue
matrix. This closes an algebraic prerequisite, but not graph
membership or recurrent exhaustion.

1. Give this project a portable pinned Lake configuration rather than
   relying on the workspace-relative cached environment.
2. Lift the now-closed center-contact and affine endpoint machinery to
   global catalogue occurrence and recurrent exhaustion.
3. Decide whether the Perron-existence draft is worth repairing or
   should be replaced by an existing Mathlib theorem.
4. Export small graph certificates in a stable textual format so the
   finite propositions can be checked by an independent verifier.

## 2026-08-03 universal-proof campaign completion

The universal n-bonacci characteristic-polynomial theorem is complete and Lean kernel checked. The former arbitrary-`n` matrix-minor transport seam was closed through reusable erased-index transport, sparse cofactor, recurrence, and normalization operations in the reflective proof engine. The older 40-course pivot tree remains useful historical strategy infrastructure, not the current theorem status.


## 2026-08-04 — request-driven corona surface projection

This is an implementation and exact finite-certificate improvement, not a new
Lean theorem.  `D_cont` now defaults to an exact face-pattern projection rather
than exponential box enumeration.  `corona.hpp` now provides request-relative
closure certificates and projected Algorithm 2, with full legacy
materialization available on request.  The unrestricted sigma-1 projected
trace agrees node-for-node with legacy Algorithm 2.  The production adjacent
competitor proof driver now derives the 44/18-state n=5 families and the
221/80-state n=6 families without constructing a whole n=6 corona graph; all
existing macro-path and branch-profile proof checks pass.  Formal Lean status
of the surrounding dominance theorem is unchanged.

## 2026-08-09 — Non-AR transport holonomy diagnostic

The complete `sigma_{0,1}^3` return/contact lift now has an exact SCC-local
edge-defect holonomy certificate. Its 256-state offset quotient has one
recurrent 248-state SCC whose normalized occurrence-position cycle residues
generate `3Z` (gcd 3). This is evidence for a finite-base/period-three
integer-cocycle normal form,
not a universal theorem and not a Property-F closure proof. The remaining
non-AR obligation is to construct the exact intertwiner from this cocycle to
the zero-expansion graph and test whether any nonzero recurrent image survives.
The first coarse attempt is a certified negative: the existing letter-role
`Q(β)` cocycle projection misses 66 recurrent zero pairs on the powered non-AR
rule even though the full Property-F graph closes. Any successful intertwiner
must retain return-word/offset collar state; the bare role quotient is not a
valid bridge.
Retaining the full 16-state return collar still leaves 52,618 recurrent
zero-kernel misses in the phase-pair cocycle. The geometric contact coordinate
is therefore load-bearing as well; the required bridge is a three-way skew
product, not a letter-role or phase-only quotient.
The quotient classes themselves have concrete fibre sizes ranging from 3 to
21, falsifying a uniform permutation-cover model. The contact-aware object
must therefore be treated as a finite branched relation/textile extension.
The exact contact-aware certificate finds 1,292 nonempty quotient channel
pairs, all irregular (0 biregular, 0 permutation). The lift is relation-valued
at every observed channel, so a future intertwiner must preserve admissible
word relations rather than replace them by permutations.
The full contact-edge prefix calculation finds four cyclic SCCs, with one
carrying non-coboundary exact `Q(β)` holonomy. This is the first concrete
recurrent obstruction candidate in the non-AR transport layer. It is not yet
a Property-F failure, because reachability from the zero frontier in the
zero-expansion orientation has not been established.
