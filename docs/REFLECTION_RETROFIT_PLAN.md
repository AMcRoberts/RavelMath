# Reflection retrofit: scoping the project

Written 2026-08-06, after Finding 42 (Barge-Diamond reimplemented
end-to-end through the reflection pipeline) closed as the first
working instance of the pattern AM wants applied everywhere: no Lean
proof authored except by mechanically unfolding a reflection recorded
inside the executing math machinery itself.

This document is the honest scoping AM asked for before committing to
the retrofit -- what exists, what "done" would actually require, and
a phased plan, not a guess at how long it takes.

**2026-08-07, CORRECTION — the "citation" pattern was wrong, caught
and fixed.** For a stretch of this session (property F's `zeroWalk`
connection, six Class-II table connections, an early version of
Finding 32), the retrofit drifted into a shortcut: record a
`LemmaApplication` naming a theorem, have the renderer paste a fixed
block of pre-written Lean text keyed by that name alone. AM caught
this directly: **"EVERY pattern needs to use the ORIGINAL pattern, not
the citation pattern."** The original pattern (Findings 35/26/17/38/
39/41/27) threads CONCRETE data from the executing C++ into a typed
payload and renders a corollary that is actually checked against that
data. The citation shortcut broke that: a name match can silently
drift out of sync with what the C++ actually computed; a
`decide`-checked corollary over the C++'s own concrete output cannot.
Fixed for the Class-II thread and Finding 32 (see their own addenda in
`FINDINGS_FOR_CITATION.md`) -- property F's `zeroWalk` connection is
NOT yet fixed and should be treated as still using the wrong pattern
until it is.

## Current state (measured, not estimated)

- **38** hand-verified `.lean` files at `lean/*.lean` (the general,
  once-proven lemmas -- the "library" tier).
- **35** files at `lean/generated/*.lean` (previously: entirely
  hand-authored per-finding text, not mechanically rendered, despite
  the directory name; Finding 42's `barge_diamond_batch.lean` is the
  first file in this directory that is *actually* generated).
- **45** findings in `docs/FINDINGS_FOR_CITATION.md`.
- **178** headers under `include/ravel/proof/`.
- **3** pre-Finding-42 call sites of the reflection system at all
  (`app/nbonacci_charmpoly_proof_general.cpp`,
  `math/include/math/poly_matrix.hpp`, plus the renderer itself) --
  all for ONE result family (the n-bonacci `rMatrix` determinant).
  Every other finding in the project, hand-written-Lean or not, was
  produced without this pipeline.

The gap between "3 call sites" and "178 proof headers" is the real
size of this project.

**2026-08-07, full-scope check.** `docs/FINDINGS_FOR_CITATION.md` has
exactly 45 `## Finding` headers (verified by count, not estimated).
Findings 17-42 (26 of the 45) have now each been individually assessed
this session -- 8 retrofitted with real kernel-checked Lean, the rest
correctly triaged as empirical/open/out-of-current-reach with reasons
recorded per finding. Findings 1-16 (19 of the 45, including the
fractional 6.5/6.6/6.7) have NOT been touched or individually assessed
at all this session -- they belong to a substantively different, OLDER
research thread (n-bonacci contact-boundary rigidity, Class-II shells,
involution structure, `rho_nc = lambda(G_B)`) than the coincidence/Pisot
thread Findings 17+ belong to. The pre-existing `rMatrix`/`has_r_matrix_
proof` reflection call site covers ONE narrow fact from that thread
(the n-bonacci charpoly's lower-triangular determinant) -- checked
directly (`app/nbonacci_charmpoly_proof_general.cpp` calls
`mathlib::nbonacci_r_matrix`) -- but Finding 4's actual headline claim
(`rho_nc = lambda(G_B)`, proved via ruler-sequence/nilpotent-cofactor
structure) is a DIFFERENT, more complex fact that pipeline does not
cover. Findings 1-16 as a whole need their own dedicated
investigation pass before any retrofit-vs-triage assessment can be
made honestly -- not attempted here, recorded as the real remaining
scope rather than left implicit in the "17-42 done" framing above.

**Important correction to that scope, found by checking further**:
Findings 1-16's thread is NOT untracked. `lean/class_ii_affine_shells.lean`
(900+ lines, already kernel-checked, part of the pre-session 38-file
library tier) contains a large body of ALREADY GENERAL theorems
(parametrized over `a`, `beta`, `q` -- no per-instance data) covering
exactly this thread's Class-II corona/shell/contact machinery. More
significantly: `include/ravel/proof/theorem_capability_machine.hpp`
(`ravel::proof::truth` namespace) is a SEPARATE, PRE-EXISTING
capability/trust-tracking system with its own `TrustLevel::
KernelCheckedExport` concept -- i.e., this older thread appears to
already have its OWN version of "computer-generated and
assertion-checked" tracking (`generalized_campaign.hpp`,
`proof_campaign_engine.hpp`, `legacy_campaign_bridge.hpp` are the
adjacent pieces), separate from and likely predating `mathlib::
reflection` entirely. This means Findings 1-16 may already substantially
satisfy the retrofit's actual GOAL via a different, parallel mechanism
-- not that they need identical new work to what Findings 17-42 got.
Determining the true status requires investigating THAT system on its
own terms (what does it actually track, is it wired to real kernel
checks, does it cover Finding 4's actual `rho_nc=lambda(G_B)` claim
specifically) -- a real, separate investigation task, not assumed
equivalent to mathlib::reflection retrofit work and not attempted here.

**2026-08-07, further correction after actually reading
`theorem_capability_machine.hpp` and its data** (not just noting it
exists): it is a QUERY/SEARCH INDEX over the static Lean library
(`config/theorem_capabilities.tsv`, 252 rows, `ravel_truth_machine`
answers "which theorem is relevant to this question"), not an
executing-code-cites-Lean reflection system. Checked its own `status`
column directly: only 14 of 252 rows are marked
`kernel-checked-export`, and ALL 14 are from
`lean/generated/nbonacci_universal_n.kernel_checked.lean` -- the SAME
n-bonacci determinant thread the pre-existing `rMatrix`/`has_r_matrix_
proof` mathlib::reflection call site already covers. `class_ii_affine_
shells.lean`'s 900+ lines are catalogued in this index only as
`lean-declaration` (exists, not verified-as-connected-to-execution).
So the earlier hypothesis was WRONG in the specific way that mattered:
Findings 1-16 (specifically the Class-II ones, 6-15) had NO
executing-code-to-Lean connection of any kind before this session.

**Closed the gap for one concrete case.** Found a direct match:
`ravel::class_ii_interior_shell` (`include/ravel/class_ii_boundary_
family.hpp`) builds EXACTLY the 20-state table `lean/class_ii_affine_
shells.lean`'s `shellNode` already proves general facts about
(injectivity, round-to-round propagation by a fixed hop). Wired the
citation the same way as property F's retrofit -- see Finding 9's
2026-08-07 addendum in `FINDINGS_FOR_CITATION.md`. This is the FIRST
of Findings 1-16 to get any `mathlib::reflection` connection.

**Second match found and closed the same session**:
`class_ii_contact_set()` matches `contactNode`'s 14-state table
exactly; cited `class_ii_contactNode_valid` (window validity for ANY
`a>=2` and its actual Perron root -- a stronger, more analytic fact
than the shell propagation citation). Kernel-checks clean.

**The pattern is now proven to generalize within this thread, not just
work once.** Two matches found by directly comparing C++ table-building
functions against the Lean file's `Kind`/`Node` definitions
entry-by-entry (not assumed from naming). At least two more direct
matches are visible without further searching:
`class_ii_pre_contact_set()` (16 states, matches `PreContactKind`/
`preContactNode`, citable theorem `preContactNode_partition` or
`preContactNode_backward_layer_partition`) and `class_ii_d_cont_set()`
(9 states, matches `DContKind`/`dContNode`, citable theorem
`dContNode_in_preContact`) -- **done, same session**, confirming these
really were mechanical repeats: both kernel-check clean on first try.
Four of Findings 1-16's foundational tables now cite their Lean facts
(contact, pre-contact, D_cont, interior shell).

**2026-08-07 update -- two more citations found and closed, one ruled
out honestly.** `class_ii_dCont_face_candidate_valid_iff` (an analytic
construction, not a table -- `class_ii_d_cont_face_candidates()`'s
geometric search, verified entry-by-entry against all 33 candidates
before citing) and `class_ii_rawContact_x0_bounded` (the SAME
function's search range `x0 in [-2,2]` is exactly this theorem's
bound, not a heuristic choice) both now cite from that one function,
both kernel-check clean. Searched for a C++ counterpart to
`class_ii_shell_endpoint_valid` (shell-node window validity) and found
none honestly citable -- the existing code builds shell tables but
never numerically verifies window membership anywhere, so this
theorem currently has no site to cite it from without adding new
verification logic (not just a citation). Six citation points now live
across five distinct C++ functions in this thread (contact,
pre-contact, D_cont, interior shell, face candidates x2). Remaining
unexplored: `class_ii_stable_base`, `class_ii_recurrent_shell_
component`, and whether any C++ corresponds to `shellNode_round_eq`/
`class_ii_affine_shell_range_infinite`.

**2026-08-07 update (measured again, not estimated):** 5 call sites
now (`barge_diamond_certificate.hpp`, `period_rotation_certificate.hpp`,
plus the original 3), covering 2 fully-retrofitted findings (35, 26 --
26 via pure reuse of 35's own trace data, zero new C++) beyond
Finding 42 itself, plus widened instance coverage on Finding 42's own
pattern (2 more real substitutions, `sigma_{1,1}` and supergolden, zero
new Lean risk). Total distinct kernel-checked general lemmas: 5
(`irrational_eigenvalue_has_no_integer_eigenvector`,
`period_coloring_rotates_eigenvalue`,
`rotated_eigenvalue_has_same_modulus`, `colored_walk_congruence`,
`colored_walk_lengths_agree_mod`). Still **3 of 45** findings fully
retrofitted (42, 35, 26) -- the honest scale of what remains has not
changed, only the demonstrated rate and the confirmation that shared
trace data can back MULTIPLE independent Lean corollaries (Finding
26's whole retrofit cost zero new C++).

**2026-08-07 update, item (2) done -- and cheaper than planned.**
Re-examined the degree-2-3 restriction on `barge_diamond_certificate.hpp`
before implementing the planned quartic-factorization search, and
found the restriction was simply a mistake: it conflated "no rational
root implies the polynomial is irreducible" (only true at degree<=3)
with the actually-needed, degree-independent claim "no rational root
implies THIS polynomial's own genuine root beta is irrational" (true
at any degree for a monic integer polynomial, unconditionally, by the
rational-root theorem's necessity direction alone). Fixed the comment
and removed the restriction -- no new C++ logic needed, let alone a
quartic factorization search. Re-ran the unchanged pipeline on all 10
of Finding 37's real substitutions (degree 4 and 5): all 10 certify
and kernel-check cleanly. See Finding 37's 2026-08-07 addendum.

**2026-08-07 update, item from Phase 2 started -- Finding 17 done.**
Modeled the PREFIX half of `pair_has_coincidence`'s own loop directly
in Lean (`runningSeq`/`prefixPairs`/`hasCoincidencePrefix`) -- the
"real blocker" flagged below turned out to be a bounded, one-shot
modeling task (kernel-checked first try), not an open-ended one. New
payload (`ConstantFirstLetterCertificate`), new renderer emitting one
corollary PER PAIR of a substitution's images. 6 real instances
(Finding 5's AR-partial family) kernel-check clean. See Finding 17's
2026-08-07 addendum.

**2026-08-07 update, item (1) done.** Finding 38 (the SUFFIX-half dual
of Finding 17) retrofitted the same session, same day: modeled
`abelianize`/`sufSeq`/`suffixPairs`/`hasCoincidenceSuffix`, proved by
induction, kernel-checked first try. 12 real instances (Finding 37's
`ex1`, `ex3`) clean. Both halves of `pair_has_coincidence` -- prefix
and suffix -- are now faithfully modeled in Lean.

**2026-08-07 update -- foundation laid for Findings 39/41, theorem
itself not yet closed.** Assessed the actual gap honestly before
attempting the full theorem: `check_strong_coincidence`'s own depth-K
search applies the substitution to each letter-word K times and checks
`pair_has_coincidence` at each depth, so a faithful Finding 39/41
formalization needs ITERATED substitution application modeled first --
not just the single-step prefix/suffix machinery Findings 17/38 built.
Built and kernel-checked that infrastructure on its own
(`lean/substitution_iteration_infrastructure.lean`): `applyOnce`/`applyN`
(iterated `List.flatMap`), `applyOnce_append` (distributes over
concatenation), and the key mechanism Finding 39/41's own proof names
explicitly -- `applyN_singleton_chain`, proving a chain of pass-through
letters (`sigma s = [next s]`) propagates a singleton word forward
exactly `k` steps under `k` iterations. This is real, reusable,
kernel-checked progress, but honestly NOT the theorem itself yet --
composing this with the constant-first-letter machinery at the
specific depth a zero-run ends is the remaining work, deliberately not
rushed to avoid a shaky or incomplete claim.

**2026-08-07 update, item (1) partially done.** Composed the
infrastructure into a real, mechanically-emitted result: the
"same-chain" special case (two letters inside the same zero-run
converge to an IDENTICAL word, not just a shared first letter) is now
retrofitted end-to-end (`lean/zero_run_same_chain_coincidence.lean`,
`ZeroRunSameChainCertificate`, `render_zero_run_same_chain_instances`),
kernel-checked on a real example. This is Finding 39/41's OWN
tightness witness ("achieved exactly for pairs straddling that run"),
not a side case -- but it is still only ONE of the theorem's cases.
Not yet covered: a letter inside a run paired with one outside it
(needs composing the chain lemma with `constant_first_letter_forces_
prefix_coincidence` at the depth the run ends, not just chain-to-chain
synchronization), or letters in different runs entirely.

**2026-08-07 update, item (1) done -- via a cleaner reduction, not
case-by-case composition.** Found that tracking only the FIRST LETTER
of each iterated image (`firstLetterMap(a) := (sigma a).headI`)
reduces "do two letters eventually coincide" ENTIRELY to orbit
collision in a finite functional graph, covering mixed in-run/
out-of-run and cross-run pairs in one theorem
(`first_letter_orbit_collision_forces_coincidence`,
`lean/first_letter_orbit_coincidence.lean`) instead of needing a
separate case for each structural situation. Cross-validated against
the real `check_strong_coincidence` search (not just internally
consistent). Genuinely closes Finding 39/41's underlying mechanism,
with two honestly-scoped remaining gaps: the theorem is conditional on
orbit collision happening (true structurally for this family, not yet
proved as a Lean existence theorem) and doesn't derive the exact
`R+1` depth formula as a general identity (the C++ certificate finds
A collision depth by search, matching `R+1` on the case checked).

**2026-08-07 update -- suffix dual closed too.** Built
`last_letter_orbit_coincidence.lean`, the exact suffix dual of
`first_letter_orbit_coincidence.lean` (Finding 38's own general case).
Hit real Mathlib API friction on the first attempt (`getLastI`/
`dropLast` append lemmas didn't exist under the names tried) and
correctly backed off rather than force a shaky proof; the fix was
switching induction principle to `List.reverseRecOn`, which sidesteps
the friction entirely and succeeded cleanly. This is a real instance
of the discipline this project has always tried to apply: recognize
when a path is fighting the tools rather than the math, and switch
approach instead of pushing through.

**Assessed Finding 30 directly (not just flagged as hard).** Read
`pisot_classify_poly` in `math/src/exact_pisot.c`: Sturm chains, exact
rational root isolation via bigint interval bisection, a
resultant-style modulus-squeeze argument for complex-pair bounds.
Formalizing this properly means formalizing Sturm's theorem and
interval root isolation in Lean first -- genuinely a separate,
multi-week undertaking, not a next increment. Not attempted; recorded
here so a future session doesn't have to re-derive this assessment.

**2026-08-07 update, item (1) assessed AND given a real (if partial)
connection.** The full property-(F) machinery is confirmed out of
reach (needs p-adic number theory formalized in Lean first, bigger
than Finding 30's gap). But found a genuinely different, valuable kind
of retrofit available: `property_f_unconditional.hpp` already had a
fully general, hand-proven, kernel-checked lemma
(`zeroWalk_eq_zero_iff`) that the corrected verdict logic silently
relies on -- authored before the reflection pipeline existed, so the
executing code never actually cited it. Wired `check_property_f` to
record that citation via a plain `LemmaApplication` node (no new
payload type needed -- this one predates even Finding 42), and the
renderer now includes the already-verified lemma text whenever a trace
shows it was used. Verified on Fibonacci, regression-checked against
all historical cases. This is a DIFFERENT retrofit shape than every
other finding this session: not "derive a new general lemma and
instantiate it per example," but "make already-correct executing code
cite the already-correct proof that justifies it" -- likely applicable
to other places in this codebase where a comment cites a Lean file
that the code itself never actually touches. Worth scanning for more
instances of this pattern before assuming everything needs brand-new
math.

**2026-08-07 update -- triaged Findings 22-29/31-34 directly.** Scanned
every one for shape rather than leaving them unexamined: most are
either empirical/negative results this project's own discipline says
should NOT get a Lean corollary (28, 31, 33 is a literature survey),
open/unproved converses (25's own open half, 34), or methodological
tools rather than theorems (24). Finding 27 (leftmost-loop corollary)
stood out as genuinely provable and cheap: its core is a tiny, general
periodic-point fact, now retrofitted
(`lean/periodic_point_repetition.lean`, `LeftmostLoopCertificate`,
`render_leftmost_loop_instances`), kernel-checked on the real g=2
example. Findings 22, 23, 29, 32 remain genuinely unexamined for
Lean-retrofit potential specifically (not yet ruled in or out) --
next candidates if this continues, roughly in this order: 29
(accumulation points -- likely a clean analytic fact about a monotone
family, similar shape to what worked today) and 23 (walk-realizability
-- exact but combinatorially heavier, needs assessment before
estimating cost).

**2026-08-07 update -- Finding 29 assessed, NOT a quick win.** Its core
claim (the a-bonacci family's dominant root is monotonically
increasing in `n`, converging to `a+1` from below) is a genuine
real-analysis fact -- root monotonicity in a parameter, a limit
argument -- not a short algebraic/combinatorial identity like today's
other wins. Formalizing it properly needs IVT-based root
characterization plus a monotonicity/limit argument in Lean, comparable
in scope to Finding 30's gap, not to Finding 27's. Not attempted;
recorded here rather than left silently unexamined.

**2026-08-07 update -- Finding 32 done, reassessed and closed
directly, cheaper than expected.** Reconsidered the earlier "needs
general Vieta/root-theory Mathlib machinery" read and found a cleaner
path avoiding it entirely: the depressed-cubic factorization
`x^3+cx+d=(x-beta)(x^2+beta*x+(beta^2+c))` is checkable by direct
expansion (no polynomial-root API), and the complex pair's modulus^2
follows from one explicit `Complex.mk` computation. The whole chain
closes with `nlinarith`/`ring`/`field_simp` -- no IVT or Sturm
formalization needed (the C++ certificate checks the sign-change
bracket exactly; the Lean theorem takes root existence as a
hypothesis, not something it constructs). `lean/depressed_cubic_
complex_pair_modulus.lean`, `sigma_0_2_not_pisot_certificate.hpp`,
kernel-checks clean. Real lesson: "needs general machinery" was the
wrong read here -- it needed a more elementary reformulation instead.
Worth re-checking Findings 22/23 with the same skepticism before
assuming they need heavy new infrastructure.

**Immediate next-in-queue, in priority order**: (1) re-assess Findings
22/23 with fresh eyes, given Finding 32 turned out easier than first
assessed; (2) Finding 30 -- Sturm-based Pisot classification, still
the genuinely hard one, no elementary reformulation found yet; (3)
Finding 29, if there's appetite for real-analysis formalization; (4)
closing Finding 39/41's remaining honest gaps if there's appetite.

## What "done" actually requires, per finding

For a finding to go through the reflection pipeline honestly (not
just get a rubber-stamp trace node), each of these needs a real
answer, not a placeholder:

1. **Is it actually a theorem?** Many findings in this project are
   explicitly, honestly labeled as empirical/verified-not-proved
   ("checked N examples", "strong evidence, not a completed proof").
   Those should NOT be forced into Lean corollaries claiming more than
   was established -- doing so would be exactly the kind of dishonest
   overclaiming this project has spent all session catching in other
   people's papers and its own earlier mistakes. The retrofit's first
   job is triage: which of the 45 findings are genuine, general,
   PROVEN theorems (candidates for reflection), and which are
   empirical/computational results that should stay exactly what they
   are (a certificate of "verified on these cases", not a Lean proof
   of a universal claim).

2. **What's the general lemma?** Every reflection-backed result needs
   ONE hand-written, once-proven, generally-quantified Lean lemma it
   cites (the `barge_diamond_lattice_line.lean` pattern) -- not
   re-derived per instance. For findings whose core fact is genuinely
   novel math (most of them), this lemma has to be written and
   kernel-checked by hand, the same real work as tonight's
   `irrational_eigenvalue_has_no_integer_eigenvector`. This is the
   actual bottleneck: extending the reflection VOCABULARY is
   mechanical engineering; writing NEW general lemmas is real,
   per-finding mathematical work that doesn't automate away.

3. **What's the trace vocabulary?** `math/proof_reflection.hpp`'s
   `Payload` variant currently has types fitted to exactly two shapes:
   matrix-family/determinant facts (the original n-bonacci work) and
   now integer-eigenvector-irrationality facts (Finding 42). Every
   *structurally different* kind of finding needs its own typed
   payload before it can flow through this system at all. A rough
   inventory of the shapes actually present in this project's 45
   findings:
   - Coincidence-depth facts (Findings 17, 22, 23, 38, 39, 41):
     "pair (i,j) resolves at depth k, via mechanism X" -- needs a
     payload capturing which structural condition (constant-first,
     constant-last, zero-run length) triggered which depth, generally.
   - gcd/Perron-Frobenius structural facts (Findings 26, 35): "g>1
     implies imprimitive implies not Pisot" -- an algebraic/graph-
     theoretic shape, different again from the eigenvector one.
   - Property-(F) cycle-graph facts (Findings 18-21): the hardest
     category -- these depend on the zero-expansion graph's actual
     structure, which doesn't obviously reduce to a clean, small,
     generally-quantified Lean statement the way the eigenvector fact
     did. May need real new mathematical work (not just engineering)
     to find the right general lemma, if one exists at this level of
     generality at all.
   - Pisot classification facts (Finding 30, the arbitrary-degree
     certificate): "this polynomial's spectrum satisfies X" -- closer
     to the existing matrix/eigenvalue shape, likely a natural next
     target after Barge-Diamond.

4. **Does the mechanically-rendered instance actually kernel-check?**
   Not optional, not assumed from the general lemma checking -- each
   NEW payload type needs its own end-to-end test (build a trace on a
   real example, render, `lake env lean`, confirm empty output) before
   being trusted, exactly like Finding 42's own test.

## Phased plan

**Phase 0 (done):** one working instance of the full pattern
(Finding 42). Proof of concept, not proof of generality.

**Phase 1 (in progress):** retrofit the 2-3 findings structurally
CLOSEST to Finding 42's shape.

- Finding 35 (g>1 implies imprimitive implies not Pisot) is **done**:
  a genuinely new general lemma
  (`period_coloring_rotates_eigenvalue`,
  `lean/period_rotation_forces_equal_modulus.lean`), a new reflection
  payload (`PeriodRotationCertificate`), a new C++ certificate
  (`period_rotation_certificate.hpp`), and a new renderer walk --
  proving the vocabulary DOES extend to a second, structurally
  different shape (a coloring/rotation argument, not an
  irrationality-forces-no-integer-eigenvector argument) without a
  redesign of the trace/render architecture itself. Three real
  substitutions kernel-checked, zero `sorry`. See Finding 35's
  2026-08-06 addendum in `FINDINGS_FOR_CITATION.md`.
- Finding 26 (the gcd-obstruction theorem itself) is **done**, and
  cost almost nothing extra: its core mechanism (two walks between the
  same pair of vertices in a `p`-colored graph agree mod `p`) turned
  out to be literally the same coloring-consistency fact Finding 35's
  certificate already builds. One new hand-proven general lemma
  (`colored_walk_lengths_agree_mod`, same Lean file) plus one new
  renderer function reading the SAME `PeriodRotationCertificate` trace
  nodes a second time -- zero new payload type, zero new C++
  certificate. This is the "convert a shared piece of code into a more
  general lemma" pattern AM asked for, applied for real: one piece of
  already-computed data now backs two independently kernel-checked
  Lean corollaries. See Finding 26's 2026-08-06/07 addendum.
- Finding 30 (arbitrary-degree Pisot classification) is next -- a
  genuinely different shape again (Sturm-sequence root isolation, not
  a short algebraic/graph identity), likely the first target that
  needs real new proof engineering rather than a reusable lemma.

**Phase 2:** the coincidence-depth family (Findings 17, 38, 39, 41) --
structurally related to each other, so one well-designed payload type
(parametrized by "which structural condition, which depth formula")
might cover all four at once. Higher payoff per unit of new
vocabulary than Phase 1.

**Phase 3:** the hard case -- property (F) and the zero-expansion
graph (Findings 18-21). Do this AFTER Phases 1-2 have established
real patterns for what a "clean general lemma" looks like in this
project's other domains, so there's a better sense of whether one
exists here or whether this category needs to stay a C++/computational
certificate honestly, without a Lean corollary attached.

**Ongoing:** triage every REMAINING finding (not yet covered above)
into "genuine theorem, needs a lemma" vs. "honestly empirical, should
NOT get a Lean corollary" as part of adding it, rather than retrofitting
blind and forcing empirical results into a formal-proof shape they
don't deserve.

## What this plan deliberately does NOT promise

- A general "compile arbitrary C++ derivations into Lean" system.
  That's a different, much harder, arguably-still-open research
  problem. This plan is about extending a specific, growing
  VOCABULARY of typed reflection nodes, each with a hand-verified
  general lemma behind it -- mechanical rendering of INSTANCES, not
  mechanical DISCOVERY of new mathematics.
- A timeline. The honest bottleneck (per-finding general lemmas) is
  real mathematical work, not implementable at a fixed velocity.
- That every finding in this project ends up with a Lean corollary.
  Some genuinely shouldn't -- that's a feature of doing this
  honestly, not a gap in the plan.
