# Reflection retrofit: scoping the project

Written 2026-08-06, after Finding 42 (Barge-Diamond reimplemented
end-to-end through the reflection pipeline) closed as the first
working instance of the pattern AM wants applied everywhere: no Lean
proof authored except by mechanically unfolding a reflection recorded
inside the executing math machinery itself.

This document is the honest scoping AM asked for before committing to
the retrofit -- what exists, what "done" would actually require, and
a phased plan, not a guess at how long it takes.

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

**Immediate next-in-queue, in priority order**: (1) finish Findings
39/41's remaining cases (in-run vs out-of-run, cross-run pairs); (2)
Finding 30 -- Sturm-based Pisot classification, a genuinely different,
harder shape (no short algebraic/graph identity to lean on); (3)
property (F) / the zero-expansion graph (Findings 18-21), the hardest
category, deliberately last per the original phased plan.

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
