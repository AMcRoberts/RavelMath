# Universal open-obligation audit — round 55

## 1. Parametric maximum-shell rank

**Present evidence:** exact block identity; kernel-checked shell-return reduction;
finite native ranks for n=3,4 over several shell radii; controller and cyclic
splice infrastructure; several phase refinements and defect identities.

**Audit:** genuinely open.  Existing phase ranks remain finite-instance
certificates, and the defect-corrected phase factors through the old sign/gap
phase rather than resolving its collisions.  This obligation is the principal
upstream blocker for the universal recurrent-hull bound.

## 2. Universal predicted-core SCC identification

**Present evidence before this round:** formula membership and exact successor /
predecessor operations; closed size and edge formulas; bounded shadow-distance
formulas; whole-graph BFS/Tarjan checks; generic Lean shadow-connectivity and
exact-SCC theorems.

**Audit correction:** this mixed internal strong connectivity with exact SCC
identification inside the full literal graph.  The former can be completed
without the shell theorem; the latter cannot.

**Round-55 result:** internal strong connectivity is complete via local symbolic
shadow paths and dimension induction.  Literal exact-SCC identification is now
a separate node depending on the recurrent-hull bound and no-return after exit.

## 3. Full-hull non-core grade exhaustion

**Present evidence:** finite ternary catalogues through n=7; persistent boundary
atom witnesses; grade descent and transport; completed rejected-boundary product
through the n=7 transition; support cutoff for the grade-two renewal exception.

**Audit:** materially advanced but open.  The rejected-boundary theorem is not
the full arithmetic-hull theorem, and higher-grade recurrent families have not
been exhausted uniformly.  It also depends on the recurrent-hull bound.

## 4. Symbolic uniform spectral dominance

**Present evidence:** exact finite Collatz-Wielandt comparisons; rank-one and
phase-aware simulations; exact macro-renewal block identities for the n=7 theta
competitors; core characteristic and edge-count formulas.

**Audit:** open.  The engine contains several reusable exact comparison modes,
but no single symbolic certificate covers every uniformly classified competing
grade for arbitrary n.  This remains downstream of grade exhaustion.

## Selected obligation

Universal predicted-core internal SCC identification was selected because its
remaining proof shape already matched a standard shadow-induction pattern:

1. prove a strongly connected base;
2. embed the previous object as a shadow;
3. derive bounded paths from the shadow to every new state and back;
4. compose through the shadow.

The repository already contained the kernel theorem for step 4.  Round 55 adds
the missing evidence-producing operation for step 3 without whole-graph SCC
search.
