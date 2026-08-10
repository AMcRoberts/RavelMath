# Research status

> **2026-08-08 note.** The universal n-bonacci boundary-dominance theorem
> (`rho(G_B(n))=rho(predicted_core(n))` for every `n>=3`) is closed --
> Lean kernel checked, unconditional in `n`, independently reconfirmed
> this session. See `THEOREM_STATUS.md`'s Theorem 0.5 and
> `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md` for the full
> account and which earlier documents (shell-rank/carry-bound route,
> refuted 2026-08-05) this supersedes.
>
> **2026-08-03 proof-system note.** The n-bonacci universal-n work now serves as the integration test for the reflective proof engine. The campaign is internally closed through generic operations, but Lean kernel acceptance remains pending the complete matching toolchain and compiled dependencies. Older chunk-generator and Lua-emitter status notes are archived under `docs/archive/2026-08-03_pre_generic_proof_system/`.

This is the concise entry point for the mathematical and engineering
state of the project. Detailed claim strength is recorded in
`THEOREM_STATUS.md`; the exact active Class-II seam is recorded in
`GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`.

## Fixed-spectrum transport family (2026-08-09)

The new `property_f_family_artifact` runner studies nine orderings of the
first and second genuine fourth-generator substitutions while preserving each
incidence matrix and Pisot polynomial.  Every member closes strong coincidence
and Property (F), with no nonzero recurrent component.  Nevertheless the
Property-F graph ranges from 6--10 nodes for the first witness and 9--7,966
nodes for the second.  This isolates ordering/transport complexity from
spectral complexity.  The artifact also records a return-word/phase transport
diagnostic: some marker-0 orderings admit a closed finite phase lift, while
others fail marker recognizability.  That failure is retained as data rather
than treated as a Property-F failure.  The current hypothesis is that the
large zero-expansion graph is an adelic finite-transport lift analogous to the
earlier sofic twist; the TSV artifact is intended for the next cocycle/holonomy
comparison.  The first phase-SCC comparison now sharpens this: closed return
phase lifts have only 5--12 states, while the corresponding adelic graphs can
reach 4,277--7,966 states.  Thus ordinary return phase is not the whole lift;
the excess is carried by the exact Q(beta) displacement coordinate.
The SCC profile adds an important qualification: the 4,277- and 7,966-node
examples have essentially one SCC per node and zero nonzero cycles.  Their
extra size is therefore predominantly transient transport growth, not
nontrivial holonomy recurrence.  The sofic-twist analogy should be stated as
a finite transport extension first, with genuine holonomy only a possible
secondary layer.
An explicit cycle classifier confirms the safety boundary: every closed case
has exactly one cyclic SCC and it is zero-only. Depending on letter ordering,
that allowed recurrence appears as a self-loop or a small zero-only cycle.
No case has a mixed or nonzero cyclic component. The current family therefore
contains no holonomy that can bite Property F; any future bite would appear as
a new mixed/nonzero SCC, which the artifact now detects directly.

## Boundary-sink optimization and obstruction search (2026-08-09)

The first large four-letter inconclusive (`rndW3_10`, 200,000-node cap) was
diagnosed by rerunning the exact checker with boundary sinks elided. Retaining
those sinks consumed 162,348 of the 200,000 nodes as one-step out-of-bound
bookkeeping. The bounded interior then closed at 79,638 nodes, used about
82 MB RSS, and Property (F) held. The shared `classify_tiling` verdict path
now elides such sinks; callers requesting a diagnostic graph may still retain
them explicitly. This converts the candidate from INCONCLUSIVE to an exact
validated result without weakening the cycle criterion.
Rerunning the same 25-candidate four-letter non-unit sample through the
optimized path changes the tally from 17 established / 8 inconclusive to
20 established / 5 inconclusive, with zero failures. The remaining three
validated cases (`rndW3_21`, `rndW3_25`, `rndW3_26`) are genuine high-growth
interior searches; two additional inconclusives (`rndW3_7`, `rndW3_18`) have
untrusted non-maximal-order arithmetic and are not obstruction evidence.
Focused profiling then separates the remaining cases: `rndW3_21` closes at 6
nodes; `rndW3_25` reaches 200,002 interior nodes by depth 8 with no partial
nonzero back-edge; `rndW3_26` reaches 200,003 nodes by depth 52, with 690,430
boundary edges and no partial nonzero back-edge. The extension certificate
therefore needs separate fields for transient transport growth, boundary
crossings, and recurrent zero-only components; a single scalar state count is
not a family invariant.
The proposed parameterized bridge contract is recorded in
`docs/PROPERTY_F_SOFIC_EXTENSION_BRIDGE.md`; it is the route for extending
the companion transport solution through the full `(n,m)` parameter sheet.

## Symbolic boundary-collar model (2026-08-10)

The fixed-spectrum family autopsy now records exact SCC-condensation height
profiles. The large 4,277/5,505/7,966-node cases contain 1,881/2,425/3,506
positive-height transient nodes, while their largest SCCs have size at most
five. This supports a metastable shell-stack interpretation: the graph grows
through transient transport, not a new recurrent obstruction.

The existing Class-II neighbor-2 grammar supplies a concrete symbolic model
for that stack. For the extended `a=7..20` sweep, round-stratified birth analysis gives one birth
round per recurrent block, zero returns from recurrence to an earlier transient
stratum, and an escape from every transient group. The boundary size is
`20*a+38`, transient count `16*a+3`, and the recurrent catalogue has `a+1`
blocks with eight cross-block edges. The remaining Property-F task is to
derive an analogous birth-round/transport grammar for the adelic lift and
compose it with the exact SCC rank, rather than treating the finite rank table
as a universal theorem.

The large rows also reject a single-chain interpretation: positive-height
nodes are about 44% of each graph and have mean positive height about 4.8--5.0,
despite maximum heights 55--59. The transient collar is broad and repeated,
which is the structural feature the symbolic proof must capture.

## Static integrity audit (2026-08-10)

The Property-F SCC rank certificate now independently replays the finite graph
partition and rejects merged labels even when their quotient is acyclic. The
same audit found and fixed a dangling reference in the iterative Tarjan
implementation used by the exact classifier: a frame value was copied before
its work-stack entry was popped. The nine-row family autopsy and the full
native regression remain green; only unrelated mini-GMP warning diagnostics
remain.

## Overnight bridge replay (2026-08-10)

The post-repair focused bridge checks remain green: the non-AR bridge
executable exits 0, the transport certificate passes, and the three fixed
family orderings pass their contact-to-Property-F checks. The intentional
`first_100` coarse digit-role miss remains 54/81 and is reported as a bridge
boundary diagnostic, not a Property-F failure.

The Class-II symbolic precedent was also replayed from its own grammar. The
round-1 Red candidates have the exact affine coordinate forms
`0-a`, `0+a`, `1+a`, and `1-a`, so bounded targets are excluded by an explicit
linear escape argument. The real round-stratified traces for `a=7..10` then
report one birth round per recurrent block, complete transient escape, zero
earlier returns, and the expected eight cross-block edges. This strengthens
the shell-stack template, but it does not derive the analogous birth-round
grammar for the adelic Property-F lift; that remains the theorem-facing seam.

The powered non-AR graph now has a focused rank-shape regression: all 95
Property-F edges either remain inside an SCC or strictly lower the SCC
sink-distance, with zero cross-SCC nondecreasing edges. The observed drops
are 64 of size 1, 21 of size 2, 5 of size 3, and 2 of size 4; only three
edges are internal SCC edges. This is stronger than merely observing an
acyclic condensation and explains why the finite rank behaves as a genuine
boundary potential on this control. It is still an instance-level fact until
the ordered-prefix grammar supplies the corresponding family argument.

The finite derivation seam is now reusable in
`include/adelic/property_f_birth_round_grammar.hpp`. It replays the exact SCC
rank, records terminal-distance birth layers, separates internal transport
from cross-SCC transitions, and emits the complete shell-drop histogram. The
powered non-AR regression recovers seven layers with sizes
`54,18,9,5,3,3,3` and the drop profile above. This is an auditable
birth-round/transport grammar for a closed graph, not yet a uniform theorem
for all ordered prefixes; the genuinely non-unit extension must add its
valuation fibres explicitly rather than being inferred from this unit control.

The same grammar is now replayed across the Class-II
`sigma_{a,1}` family, including the non-AR member `a=0`. Exact finite closure
holds for `a=0..16`; for every `a=4..16` the rank has exactly `2*a+3` layers.
The largest checked point, `a=16`, closes at 613,048 states and 613,048
recorded edges in about 40 seconds at about 0.96 GB RSS. The default regression
stops at `a=13` under a 300,000-node cap; larger points are opt-in probes via
`CLASSII_BIRTH_MAX_A` and `CLASSII_BIRTH_NODE_BUDGET`. This is strong finite
evidence for a linear height law and a useful target for symbolic derivation,
not a proof that all `a` close or that the law is universal.

The height law has an explicit replayable spine, not just a scalar fit. For
`a>=4`, the graph contains an alternating coefficient sequence in the power
basis: the interior pairs are
`(-k, -(k(a+1)-1), k)` and `(k+1, k(a+1), -k)`, followed by three boundary
states. `include/adelic/property_f_class_ii_rank_spine.hpp` checks their exact
node keys, edges, SCC heights, and digit labels. Its checked schedule is
`1,a,(1,a-1),(2,a-2),...,(a-2,2),a-2,0,a-1,0` in the `[0,j,0]` coefficient
notation. This proves a concrete height-`2a+2`
lower-bound path in every checked member; the missing upper bound is to show
that no other admissible transport branch exceeds this spine.

The recurrence itself is now isolated in
`include/adelic/property_f_class_ii_affine_tail.hpp`: it independently checks
the coefficient formulas and digit schedule under the exact integer
beta-inverse carry map, before the graph replay is consulted. This separates
the symbolic affine calculation from the finite Property-F enumeration. The
cheap recurrence sweep reaches `a=256`; graph closure remains capped separately.
The certificate rejects parameters above its explicit signed-64-bit safety
bound `a=2,000,000` instead of allowing arithmetic wraparound.

The branch census now makes that proof target explicit. In every checked
`a=4..16` graph, every node above collar height 5 is one of the spine states,
each high layer contains exactly one node, and the only high-to-high edges are
the consecutive spine links after internal SCC transport is factored out (the
allowed zero-only recurrence is retained separately). All remaining branching
exits enter the finite height-`<=5` collar. Thus the uniform upper-bound lemma
can be organized as a finite collar grammar plus a deterministic tail
recurrence, rather than as a cardinality estimate over the full expanding
graph.

The collar grammar is now explicit in
`include/adelic/property_f_class_ii_collar_grammar.hpp`. Across the post-label
`a=4..14` replay, its height support is exactly
`1->0; 2->{0,1}; 3->{0,1,2}; 4->{1,2,3}; 5->{2,3,4}`, with zero collar-to-tail
edges and zero same-SCC collar transport. The extractor retains the coefficient-string transport labels and
counts branching nodes, internal edges, and height-pair multiplicities. This
is the finite grammar premise for the upper-bound proof; it does not promote
the observed support to a theorem for all `a`.

The new tail-candidate census sharpens the mechanism. In every checked
`a=4..14` graph, each letter-0 tail state realizes all zero-prefix labels
`j=0,...,a-1` as collar alternatives, while the single letter-1 tail state
has only its forced return edge. No alternate candidate reaches another high
state. This is the concrete role-level premise for an induction: the affine
candidate is the only high-tail continuation, and every other digit is
absorbed by the collar. The symbolic proof that the same role law holds for
all `a` remains open.

The word-level half is now parameter-uniform in
`include/adelic/property_f_class_ii_prefix_role_grammar.hpp`. Direct image
enumeration proves the exact channels: target 0 comes from parents 0 and 1 at
prefix lengths `0..a-1` and parent 2 at length 0; target 1 has only parent 0
at length `a`; target 2 has parent 0 at `a+1` and parent 1 at `a`. The graph
candidate census composes this role grammar with the affine tail and checks
that the observed alternatives are precisely the collar exits.
The same census now checks the reverse induction direction: every high-tail
state has one affine predecessor, with no collar or alternate-high predecessor.

The coefficient-level phase invariant is now executable in
`include/adelic/property_f_class_ii_phase_strip.hpp`. Writing
`eta=c0+c2` and `q=c1+(a+1)c2`, all high-spine states alternate between
`(0,1)` and `(1,0)`. For each zero-letter tail state, exhaustive symbolic
digit evaluation finds exactly one `j in [0,a)` preserving that strip, and it
is the schedule digit. The certificate now also checks the closed formula
`j=1-eta-c1+a*c0`, derived from the exact identity `q'=eta`; this turns the
forcing scan into a two-state quotient calculation. This is the algebraic
forcing lemma needed before the remaining rank-absorption argument.

The exact local deviation law is now factored into
`include/adelic/property_f_class_ii_carry_deviation.hpp`. For forced digit
`j*` and arbitrary digit `j`, writing `d=j-j*` gives `q'=eta` and
`eta'=1-eta+d`; therefore only zero deviation can remain in the two-state
phase strip. The focused Class-II regression checks the identity and the
immediate strip exit for every digit on every symbolic spine state through
`a=256`. This is a stronger algebraic no-deviation premise, but it is still
not the Akiyama-to-literal-contact intertwiner or the uniform collar-absorption
lemma.

The literature-to-spine coordinate adapter is now executable in
`include/adelic/property_f_class_ii_akiyama_spine.hpp`. It represents
`U=a+beta^(-1)` as `[-1,-a,1]`, verifies Akiyama's recursive identity for
`kU`, and identifies `kU-(k-1)beta` with the negative spine state
`[-k,-(k(a+1)-1),k]`. The two successive affine steps use digits `k` and
`a-k`, while the positive state is the corresponding residual of
`beta^2-(k+1)U` after `(a-k)beta`; this reproduces both published expansion
families and the alternating schedule through `a=256`. The certificate also
reconstructs both fundamental elements after restoring the stripped leading
terms, isolating the exact prefix context that a future bounded-correction
intertwiner must preserve.

The literal prefix seam is now factored into
`include/adelic/property_f_class_ii_prefix_correction.hpp`. Every ordinary
`0^j` role contributes the integer `j`, while the sole exceptional `0^a1`
prefix has reduced label `beta^2-1 = U+a*beta`; after the inverse step its
contribution is `a+beta^(-1)U = [a+1,a+1,-1]`. Thus the contact grammar adds
one explicit Akiyama-unit correction and otherwise stays in the integer digit
window. The remaining theorem is to propagate this finite correction through
arbitrary branches and prove collar absorption. The symbolic role grammar also
shows that the exceptional role cannot occur on the high letter-0 tail, so
that induction genuinely has only the ordinary integer deviation cases.

The next finite reduction is exposed by
`include/adelic/property_f_class_ii_akiyama_window.hpp`. Akiyama's Lemma 4
states `|x_0+x_2|<=2` for `a>=7`; under the pending identification of that
coordinate with `eta`, the deviation law leaves exactly five possible `d`
values at every high step. The certificate enumerates and classifies those
five values through `a=256`. This is the finite candidate window to feed into
the bounded-correction case split, not yet a proof of the coordinate
identification.
The realized-graph companion
`include/adelic/property_f_class_ii_akiyama_window_census.hpp` now applies
that window to actual high-tail edges. Across the checked `a=7..13` rows,
all nonzero bounded deviations land in the collar; the only bounded high-tail
nonzero edge is same-SCC transport and is retained as harmless recurrence.
The census records exact digit labels and deviation histograms, so the next
symbolic proof can target the finite collar cases rather than the full graph.
The opt-in `CLASSII_AKIYAMA_WINDOW_PROBE=1` output prints those exact
deviation-to-label sets for autopsy.
This closes the coordinate-level analogue, but not the beta-expansion
inequalities or the literal prefix/contact-cocycle intertwiner.

The graph-level phase-absorption certificate now closes the finite version of
that seam. In every checked Class-II graph, every off-strip node above the
height-5 collar is the single distinguished top zero seed; all other
off-strip nodes are already collar states. Thus the high-rank tail is a forced
affine carry ray with a finite absorbing collar, rather than a second family
of high-rank branches. The uniform theorem still needs the symbolic step that
derives this absorption directly from the parameter-uniform prefix roles.

The algebraic no-deviation premise is now factored into
`property_f_class_ii_symbolic_tail_grammar.hpp`. Forward and inverse affine
role enumeration both give exactly one spine continuation/predecessor and zero
alternate spine hits through `a=256`. This removes graph enumeration from the
spine-uniqueness half of the argument; the remaining uniform seam is strictly
off-spine collar absorption.
The only degree-overflowing role, `0^(a+1)`, is handled through its reduced
coefficient label `[-1,0,1]`, so the inverse check is using the actual cocycle
and not a raw prefix-length surrogate.

The remaining growing coordinate now has a closed carrier law. On the phase
strip, the zero-prefix alphabet admits `eta=0` exactly for
`c0 in [-a+1,0]` and `eta=1` exactly for `c0 in [2,a+1]`; two forced steps
translate `c0` by one. The carrier contributes `2(a-1)` scalar steps, and the
five fixed boundary corrections account for the observed height `2a+2`.
This is the uniform height mechanism; the unresolved part is still proving
that every off-carrier branch enters the finite collar.

The non-unit side now has its explicit local-fibre premise as well. The
valuation-fibre certificate checks, for every prime dividing the determinant,
that the maximal Dedekind factors and independent ideal-lattice product agree
and that their local degrees satisfy `sum(e_i f_i)=deg(f)`. The existing
combined p-adic bound is accepted only when this partition is certified. This
separates genuine valuation geometry from the unimodular Class-II phase tail.

## Effective-potential analogy (bounded modeling hypothesis, 2026-08-10)

The observed “large, long-lived, then dispersing” cases invite a useful but
strictly provisional analogy with a metastable gravitational cluster.  The
analogy is mathematical rather than physical: SCC-condensation escape height
acts as an effective potential, transition multiplicity creates funneling into
low-height regions, and the transport/phase cocycle acts like an
angular-momentum or holonomy coordinate.  A broad transient collar is then a
bound-looking cloud with a small escape conductance, while a recurrent
nonzero SCC would be the genuinely bound obstruction.

This suggests a testable slow--fast model for future work: assign each finite
state a height potential and each transport fibre a phase, then measure basin
mass and boundary conductance before exploring the exact graph.  The model
predicts that “large but harmless” instances have large basin mass and long
residence time but no recurrent nonzero phase component; a mixed/nonzero SCC,
or a family whose escape conductance tends to zero while recurrence appears,
would falsify the harmless-metastable interpretation.  It is a diagnostic
organizing principle only, not an additional Property-F argument.

There is also a necessary caveat: the endpoint maps are unimodular on the
coefficient lattice, so this is not literal volume-contracting gradient flow.
The apparent attraction comes from contraction in the reverse `beta`
direction, finite admissibility, and many-to-one graph incidence; the forward
`beta^(-1)` walk expands toward escape. SCC height is a combinatorial potential
layered over that arithmetic mechanism.

## Generalized-multinacci bridge rectangle (2026-08-09)

The common sofic/adelic certificate has now been run on all 18 points
`d in {2,3,4}`, `m=1..6`. Every point has strong coincidence, finite
Property-F closure, zero nonzero-cycle components, and no partial nonzero
back-edge. Property-F nodes by increasing `m` are `5,4,4,4,4,4` for `d=2`,
`33,43,54,102,229,395` for `d=3`, and
`350,912,2761,7386,16895,33837` for `d=4`; maximum observed depth is 17.
The same test invokes the symbolic general-m scheduler, so these are paired
instances rather than separate experiments. This is evidence for the bridge
contract, not yet a uniform all-`d,m` theorem: the remaining task is to turn
the finite transport growth pattern into a parameterized rank or adelic bound.
An opt-in extended regression now closes `(5,1)`, `(5,2)`, `(5,3)`, `(5,4)`,
and `(6,1)` (8,307; 67,007; 792,212; 1,201,687; and 188,545 nodes). A larger
exact run closes `(6,2)` at 4,914,771 nodes (9,802,230 boundary edges), in
12:56 with 4.91 GB peak RSS. The earlier 500,009-node result was therefore a
resource-bound partial search, not an obstruction. The high-dimensional
growth is large-but-harmless finite transport, though still not a proof of the
missing uniform bound.
The next fixed-degree point `(5,5)` reaches 2,000,001 nodes and 8,710,307
boundary edges at 6:29 with 1.67 GB peak RSS, and remains intentionally
inconclusive. The new growth-profile test sweeps all 42 points through
`d=8,m=6`, checking the exact channel count, coefficient dimension, and
`rho<1`; its scale factors are 13.52 at `(6,3)`, 16.55 at `(7,2)`, and 18.93
at `(8,1)`. These are profile-only rows, not closure claims, and identify the
uniform reachable-vector rank bound as the next theorem-facing deliverable.
The paired structural sweep now checks strong coincidence, the symbolic
general-`m` scheduler, and the return-block schema on all 42 points through
`d=8,m=6`; every point passes. The missing datum on profile-only rows is
therefore only the expensive exact Property-F closure, not family validity.
The profile's additional proxy `m*rho/(1-rho)` is the forward-contraction
carry radius, separating the contraction threshold from the subsequent count
of admissible coefficient vectors.
The radius alone does not predict node count: `(4,6)` has proxy radius about
34.2 but 33,837 closed states, while `(6,2)` has radius about 20.1 and
4,914,771. The difference is the denser six-coordinate, 11-channel lattice
reachability, so the uniform argument must control cardinality/branching as
well as norm radius.
The exact digit/transition identity is now checked by
`generalized_multinacci_affine_transport.hpp`: `delta(0^k)=k*beta` and
`gamma' = beta^(-1)gamma+k`. This isolates the future rank problem into the
carry alphabet size `m` and the degree-`d` beta-inverse reduction.
The return-to-zero block quotient is also explicit: exactly `(d-1)m+1`
channels, each of length at most `d`, with words `[k,m,...,m]`. This is the
finite forest on which the remaining reachability/rank proof should operate.
The cross-dimension law is now explicit and differs from the sofic one. Sofic
transport is a dimension-free renewal suspension with dimension-specific edge
deletion. Affine transport is a graded height/color tower: `d -> d+1` keeps the
old blocks, turns the old terminal into the new zero-carry height-
`d-1` block, and adds `m` channels; `m -> m+1` relabels forced tails and adds
one carry color at each nonterminal height. The cross-dimension test checks 42
of the former lifts and 35 of the latter. This transfers the return language,
not Property-F closure, because the beta-inverse matrix changes with the
parameters.
The unit/non-unit split is now explicit. Every generalized-multinacci member
has constant term `-1` and is unimodular, so its Condition-F wall is the thin
finite-language/recurrence bridge with no valuation coordinates. The genuinely
non-unit witnesses have the same finite base and cocycle logic, but their fiber
must be enlarged by every prime-ideal valuation; the p-adic integrality bound
and contact-to-adelic recurrence bridge are the remaining thin wall there.
The expected paired extension is now explicit: the same prefix cocycle acts
on the archimedean coefficient coordinate and, in the non-unit case, on one
local coordinate for each prime ideal above a divisor of `(beta)`. The current
`make_combined_padic_bound` implements the product integrality predicate; the
next implementation seam is a typed local-fiber wrapper that exposes those
coordinates and their holonomy to the same recurrence classifier.
The typed seam is now implemented in `adelic_cocycle_extension.hpp`: the unit
case specializes to an empty local fiber, while the non-unit case requires
prime-ideal descriptors and preserves the shared-prefix cocycle across all
forgetful projections. This is the first concrete version of the expected
third extension, not merely a diagram.
The first forest-level widening is now executable: the monotone coefficient
cone certificate accepts arbitrary positive nonincreasing coefficient vectors,
applies the Condition-F criterion, and identifies `a_0` as the determinant
that decides whether valuation fibers are present. The generalized family is
the unit ray `(m,...,m,1)`; vectors with `a_0>1` are the first controlled
non-unit targets.
The resulting strong hypothesis is recorded in
`docs/TRANSPORT_CROSS_DIMENSION_HYPOTHESIS.md`: the sofic phase is the finite
base, while transport is a graded height-by-carry-color cocycle. Its proposed
obstruction-localization principle says that new nonzero recurrence can only
enter through newly added top-height/new-color channels. This is supported by
the lift certificates and all current Property-F data, but awaits a rank
decrease proof.
The direct paired Property-F experiment now tests the hypothesis on 15 exact
dimension lifts (`2->3` and `3->4` for all `m=1..6`, plus `4->5` for
`m=1..3`). Every pair closes with zero nonzero SCCs and zero partial nonzero
back-edges; the lifted channel increment is exactly `m` in every case. This
is evidence for obstruction localization, not yet an induction theorem.
The new-height endpoint maps narrow the possible effect further: all carry
colors share the same linear part `A^d` and differ only by offsets
`k A^(d-1)e_0`; the terminal is one extra `A` step. Hence a dimension lift
adds translations and one existing-generator power, not a new linear
generator. This is the strongest current restriction on what a new fiber can
do.
The graph-level interaction classifier confirms the corresponding direction:
in every tested lifted graph, `new->new=0` and `top->new=0` in the backward
orientation, while inherited states enter the new terminal class and new
states descend to the old top class. The new fiber is therefore a one-way
transit layer in the tested exact graphs, not an autonomous recurrent block.
There is now a symbolic reason for this, independent of the affine coordinates:
the lifted substitution has `sigma(i)=0^m(i+1)` for `i<d` and `sigma(d)=0`.
Thus the new letter `d` occurs only in `sigma(d-1)`, and the new image contains
neither `d` nor `d-1`. The backward prefix-automaton incidence therefore forces
new -> old-top and forbids both new -> new and old-top -> new before any finite
Property-F enumeration is run.
The delayed-return certificate now gives the corresponding normal form for any
putative cycle through the new fiber: mandatory descent
`d,d-1,...,1,0`, an inherited return language based at `0`, then the unique
re-entry `0->d`. Its mandatory affine contribution is exactly
`A^(d+1) gamma + m*(A + ... + A^d)`. The unresolved theorem is consequently
the rank/decrease statement for the inherited zero plateau.
The existing Condition-F/CSY branching-reduction interface has now been
generalized from ordinary `n`-bonacci to the full `(d,m)` polynomial family.
Its theorem-level certificate supplies a finite zero-language quotient for the
finite carry alphabet and treats terminal transport fibres as permutation
lifts, preserving path growth. This closes the abstract Condition-F existence
route; the zero-plateau rank remains the constructive bound explaining the
finite graph sizes and the resource behavior of direct enumeration.
The generalized Pisot premise is now justified by the primitive nonnegative
companion matrix and Perron-Frobenius: the positive root is the unique
dominant root. Its polynomial is `x^d-m(x+...+x^(d-1))-1`, with coefficient
sequence `(m,...,m,1)`, and the constant term `-1` makes the whole family
unimodular, including `m>1`.
The actual Condition-F implication is supplied by the Frougny-Solomyak
coefficient criterion `a_(d-1) >= ... >= a_0 > 0`: the generalized family has
the constant coefficient sequence `(m,...,m)`, so it satisfies that criterion
uniformly. This is stronger than merely observing regularity in a bounded CSY
automaton.
Each channel is now compiled to an exact integer affine endpoint map on the
`d`-coordinate coefficient lattice; all 19 `(4,6)` maps are replay-checked
against `Q(beta)`, while the 42-point sweep checks the parameterized map shape.
The endpoint-map layer makes the rank obligation explicit: bound the set of
reachable coefficient vectors under this finite affine family, rather than
enumerating the raw zero-expansion graph.
Because the defining polynomial has constant term `-1`, `beta` is a unit and
the endpoint linear maps are unimodular: coefficient-lattice volume is
preserved. In the return-to-zero inequality, multiplication by `beta` contracts
secondary embeddings; the enumerated `beta^(-1)` endpoint walk expands in
those coordinates and escapes the admissible frontier. Finiteness therefore
comes from the admissible return language and boundary pruning rather than a
scalar lattice contraction. This explains why a one-dimensional rank has not
emerged.
The growth profile also pushes the embedding-radius proxy through the inverse
Vandermonde matrix to estimate a coefficient box. The resulting log10 box
sizes are 6.82 at `(4,6)`, 9.59 at `(6,2)`, and 10.94 at `(6,3)`. Exact
reachable sets are much sparser than these boxes, but the estimate places the
next dimensions beyond the practical exhaustive-search envelope; it is
explicitly a floating-point resource diagnostic, not a theorem.
Root diagnostics explain the scaling: the weakest secondary modulus rises
from `0.8508` at `(4,6)` to `0.9094` at `(6,2)`, while the degree and channel
count increase the coefficient-state branching. This matches the observed
large-but-harmless transient clouds and gives a concrete target for the rank
bound. An optimized endpoint-only block search did not materially collapse
the `(6,2)` frontier, so forced descent is not the main cost; the remaining
problem is bounding the number of distinct reachable coefficient vectors.

## Verified engineering boundary

Historical complete-tree validation passed `make check` and `make lean-check` with the enrolled Lean files sorry-free. The compressed handoff does not currently contain a complete executable Lean/Mathlib build, so this package must not repeat that claim until `scripts/safe_lean_check.sh` succeeds under the matching restored artifacts.

## Class-II family

The balanced-pair side has an explicit recurrent eight-state family for
every `sigma_{a,1}`, `a>=2`, with exact reductions, strong connectivity,
and characteristic polynomial.

The contact side has an explicit rigid dominant core, a twenty-role
affine interior shell, exact non-base catalogues, universal window and
ranked Red certificates on their stated domains, and a kernel-checked
partition of every legal round for `a>=7` into five phases.

The global occurrence theorem's four-round base seam is now closed
(2026-07-31, night): all four exceptional base transitions (items 1-4
below) are closed for every integer `a>=7` (item 1 for every `a>=3`),
one tier below Lean-formalized. This does not by itself complete the
global occurrence theorem -- recurrent-SCC exhaustion and dominance
remain a separate, later step -- but the base-round seam that has been
this investigation's active target for two days is done. Item 5
documents the shared argument that closes items 2-4; it names no open
work of its own.

1. signed-contact reverse inclusion at the neighbor base -- **closed
   for every `a>=3` (2026-07-31)**: the *unsigned* raw 27-state
   self-closure and its Red-pruning to 25 survivors are proved for
   every `a>=3` (window validity abstractly, Red pruning symbolically
   from `tau_a`'s own fixed word forms), not just checked at sampled
   `a`. The full *signed* 50-state contact set is a verified identity
   (`class_ii_neighbor2_signed_contact_set()` equals the unsigned
   survivors' mirror-closure, exactly), and **checked directly against
   the primary source (Loridant-Thuswaldner-Zhang, arXiv:2511.16442,
   2026-07-31) that this identity is the whole obligation**: the
   paper's own `±K := K∪(−K)` (§3.5) is a pure notational
   symmetrization, never an independently-closed object, and its
   corona iteration (Algorithm 2) starts *from* `±C` directly (`A[1] =
   Ĝ_C`) rather than deriving it as a closure target. There is no
   "reverse inclusion for `±C` as its own object" obligation in the
   source material beyond the already-closed unsigned half. Two
   candidate closure-of-a-symmetric-seed hypotheses were tried and
   refuted first (mirror-only seed, and `D_cont ∪ mirror(D_cont)`,
   both giving the same non-self-mirror-closed 32-state subset of
   `±C` at `a` in `{3..8,20,50}`) before the literature check settled
   that neither was actually the missing piece, because there was no
   missing piece of that shape. The Red-pruning argument's arithmetic
   core is Lean-kernel-checked (`lean/class_ii_round1_red_pruning.lean`,
   sorry-free, signed) -- the combinatorial exhaustiveness fact behind
   it is still taken as given C++-checked input, not re-derived in
   Lean, so Round 1 sits at the same "proven, one tier below
   Lean-formalized" strength as items 2-4. See `docs/GLOBAL_CATALOGUE_
   OCCURRENCE_EXHAUSTION.md`'s Round 1 sections for the full account,
   including a caught-and-corrected overclaim about mirror-closure
   preserving correctness and the literature-check section that closed
   this item.
2. round-two raw reverse inclusion and Red exclusion -- **closed for
   every integer `a>=7` (2026-07-31)**: `T_2 = B_2 union E_2` exactly
   (literal identity), and Red exclusion (123 states across three
   ranks) is now provable for every `a>=7`, not just exact-finite-
   checked -- see item 5 below for the argument that closes rounds
   two through four together.
3. round-three raw reverse inclusion and Red exclusion -- **closed for
   every integer `a>=7`**, same argument as round two (`T_3 = B_3
   union E_3` exact, 93 = 68 + 25; 163-state Red exclusion).
4. the round-four reverse/exclusion bridge into the stable theorem --
   its raw-corona occurrence/exclusion property (`T_4 = B_4 union E_4`
   exact, 113 = 88 + 25; 212-state Red exclusion) is **closed for every
   integer `a>=7`** by the same argument. **The actual "connect to the
   round>=5 universal theorem" claim this item names is now closed too
   (2026-07-31, later)**: the already-proven stable raw-corona
   composition certificate (universal for `a>=7`) assumes a specific
   generic-formula source at round 5 (`class_ii_center_layer_candidate
   (a,4)` union `class_ii_neighbor2_layer_extension(a,4)`), and that
   formula is checked, node-for-node, exactly equal to the real
   round-4 post-Red survivor catalogue at 14 widely separated `a`
   (`app/class_ii_round4_stable_bridge_check.cpp`). More than a check:
   the formula side is *provably* `a`-independent by direct code-path
   inspection (its branch conditions are always satisfied once `a>5`),
   and the ground-truth side's `a`-independence for every `a>=7`
   follows from Round 4's own closed-form Red-exclusion proof (a fixed
   node set with a fixed edge set gives a fixed Red-pruning result) --
   so the equality holds for literally every `a>=7`, not just the
   tested values. This closes round four's row in the base-premises
   table.
5. **the argument that closes items 2-4 (2026-07-31, same session):**
   the raw candidates behind Rounds 2/3/4's Red exclusion split into
   exactly three shapes by whether each side's occurrence ranges with
   `a` -- both-range (4 shapes, closed by direct tracing), hybrid (20
   shapes, closed by an exhaustive slope enumeration showing the
   achievable window always grows from one fixed edge), and both-fixed
   (20 shapes, closed by the corrected condition `rhs2 = slope_q -
   slope_p`, found after an earlier draft's `rhs2=0` guess produced
   real counterexamples). All three checked exhaustively (every valid
   edge, not sampled states) at widely separated `a`, zero exceptions.
   Still one tier below Lean-formalized. See `docs/GLOBAL_CATALOGUE_
   OCCURRENCE_EXHAUSTION.md`'s category-closure sections for the full
   account.

Across all four rounds, the "are the pruned-state coordinate sets
literally `a`-independent" question is now resolved: **no**, not
uniformly. Round 1 is a genuine outlier (pruned coordinates affine in
`a`, small 27-state raw target, already closed symbolically and now
Lean-formalized for its arithmetic core). Rounds 2/3/4 share a second
pattern instead: a fixed, `a`-independent pruned-state list with only
the forward-edge weights varying by `a`. This means the still-open
symbolic Red-exclusion proof is one argument applicable to all three
of rounds 2/3/4, not three independent derivations.

**Sharper still (2026-07-31):** the entire forward-edge connectivity
graph over each round's raw pre-Red states -- not just the pruned node
set -- checks out `a`-independent too, at 7-8 sampled `a` values per
round including one far outside the cluster (Round 2 stress-tested at
`a=50`), with a single genuine exception at Round 4's `a=6` (resolved
by `a>=7`).

**The mechanism behind this, closed exactly (2026-07-31, same
session):** the general lemma the paragraph above still needed --
"forward-edge coordinates are affine in `a`" -- turned out to need a
different independent variable than `a` itself. Working directly from
Round 1's own `M(a)` back-substitution shows `x0'`/`x1'` depend only
on the per-letter abelianization counts of the occurrence prefixes
(not their length or `a`), and `x2'` is affine in the occurrence-index
*difference* with slope exactly `1` once those counts are fixed.
Grouping raw candidates accordingly gives **100% clean** agreement at
every tested `a` (135/135 groups, zero exceptions) -- verified against
its own derivation, not curve-fit. The one known exception (Round 4's
`a=6`) is traced to a concrete missing occurrence, not an unexplained
gap.

**Closed for every integer `a>=7`, same session, later.** The raw
candidates split into exactly three shapes of argument (both-range,
hybrid, both-fixed) by whether each side's occurrence ranges with `a`.
All three are now proven, not merely checked: both-range by direct
tracing; hybrid by an exhaustive slope enumeration (every fixed
occurrence in `tau_a`'s images has slope exactly `0` or `1`, forcing
the achievable window to grow monotonically from one fixed edge);
both-fixed by the corrected condition `rhs2 = slope_q - slope_p`
(an earlier draft wrongly required `rhs2=0` and found real
counterexamples -- checked exhaustively, not sampled states, at two
widely separated `a`, then closed in actual closed form: every
slope-nonzero case's required `a` is solved for directly and shown
`<7`, not sampled further). **Rounds 2/3/4's Red-exclusion property is
therefore provable for every integer `a>=7`**, not merely exact-
finite-checked through `a=50`. The shared arithmetic core (why a
solution below a threshold never recurs above it) is now Lean-
kernel-checked (`lean/class_ii_round234_shape_closure.lean`,
sorry-free, signed) -- the specific `pre_red` catalogues themselves
remain C++-checked input, not ported to Lean, so the overall closure
is one tier below fully Lean-formalized. See `docs/GLOBAL_CATALOGUE_
OCCURRENCE_EXHAUSTION.md`'s "Sharper still" section and the three
category-closure sections following it for the full account.

Positive witnesses establish inclusion and survival but do not establish
reverse inclusion or exclusion. Cardinality agreement is discovery
evidence, not set equality.

After layer occurrence is closed, recurrent-SCC exhaustion and dominance
remain separate. Layer equality alone does not prove that a displayed
block is exactly a recurrent SCC.

**Progress on this (2026-07-31/08-01, night):** all five support
statements are now verified together for neighbor 2 at `a` in
`{7,...,20,30}` (fifteen values, `a=30` well outside the cluster), zero exceptions. Items 1
(core/shell/transient partition), 2 (strong connectivity tied to the
real graph), and 5 (recurrent-block distinctness) follow from an exact
match between the real, Tarjan-derived SCC decomposition of the
fully-converged boundary graph and the hand-catalogued recurrent
blocks (also checked separately at `a=30`) -- a Tarjan decomposition
guarantees all three by construction once the match is exact. Items 3
and 4 needed a round/rank ordering on transient states, built directly
from the trusted corona trace (`birth_round(state)` = the first round
it survives Red in, well-defined since Red never re-removes a state
from a later round once it has survived one). That ordering also
answered the scoping question it was built to resolve: rank and round
correspond cleanly. **Made exact (2026-08-01, was "one small bounded
tie near the dominant core"):** `birth_round(rank) = a - rank` for
`0<=rank<=a-2`, `birth_round(a-1)=2` (not `1`, a genuine +1 exception,
not a tie with `a-2`), `birth_round(a)=1` (the core). Traced to a
confirmed mechanism, not yet a full symbolic proof: the corona
construction's own seed (`plus_minus_C`, 50 fixed states) has hop
coordinates of magnitude up to 2, not 1, so a single expansion step
from the core reaches two shells' worth of new territory at once --
which is exactly why `rank=a-1` and `rank=a-2` both first appear one
round after the core rather than `a-1` needing an extra round. Item 4
(no return to an earlier transient stratum): zero violations. Item 3
(escape from every transient block): held once "escape" was corrected
from "strictly later round" to "does not remain transient forever"
(same-round absorption into a recurrent block counts) -- the strict
version found one apparent gap (the terminal-round transient group)
that turned out to escape into the same-round recurrent core instead,
the same kind of over-strict-then-corrected pattern as item 5's "zero
inter-block edges" earlier. All checked, not yet fully closed-form --
see `docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s "Items 1, 2, and
5" and "Items 3 and 4" sections for the full account.

**Separately (2026-08-01): Round 1's target-set decomposition is now
proven outright, not swept.** `class_ii_neighbor2_signed_contact_set()`
(the 50-state target) equals exactly `class_ii_contact_set()` union its
own mirror union `class_ii_neighbor2_initial_extension_states()` --
all three parameter-free, so the identity holds for literally every
integer `a` at once (`app/class_ii_neighbor2_round1_union_hypothesis.cpp`,
enrolled permanently in `tests/substitution_neighborhood_test.cpp`).
This is the target formula's own internal decomposition (proven).

**Stale-pointer correction (2026-08-02): the next sentence here used to
say this does not prove reverse inclusion and that it remains open.**
It was true when written but was overtaken later the same night: a
literature check (`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s
"Round 1: the literature check, done" section) found that `±C` is a
definitional symmetrization in the primary source, not an independently
closed object -- so reverse inclusion for round 1 was never a real
proof obligation in the first place, and closes on that basis rather
than via a symbolic construction. `class_ii_neighbor2_first_missing_
premise` (`include/ravel/class_ii_neighbor2_pruning.hpp`) still
literally reports round 1 as `open_reverse_inclusion` -- flagged, not
fixed, as its own possible staleness in that file's header comment
(2026-08-02), since it is not yet verified whether that function's
specific technical sense of "reverse inclusion" is the same claim the
literature argument closed.

## Exact and formal machinery

The native math library provides arbitrary-precision integer and
rational arithmetic, polynomial and matrix arithmetic, `Q(beta)`
arithmetic and linear algebra, Sturm sequencing and root isolation,
exact characteristic polynomials, Perron certificates, rational
intervals, tunable binary big floats, and FFT implementations.

Lean modules formalize the free-involution descent mechanism, return/
contact projection, the conditional determinant reduction, Class-II
affine shells and margins, endpoint catalogues, support identities, and
the global round partition. The explicitly named existence draft is not
an enrolled theorem.

## Adelic and CSY boundary

The adelic classifier includes number-field factorization, ideal
arithmetic cross-checks, local-field arithmetic, prefix automata,
coincidence, and property-(F) components.

The CSY power machinery has exact recurrence and companion-matrix routes
with bounded memo residency. This does not prove globally bounded
memory: exact outputs, arithmetic temporaries, and live polynomial
contexts remain separate resource dimensions.

The current `CSYAutomaton` is a bounded absolute-position prefix object,
not yet the finite local carry quotient of the cited construction.
Depth exhaustion is reported as truncation.

**General number-field machinery, scoped and started (2026-07-31).**
Separate from the Class-II seam: after correcting an earlier bib note
that falsely connected OpenAI/Sawin's unit-distance disproof to this
project's Pisot/Rauzy-fractal machinery (see `refs/references.bib`'s
`Williams2026OpenAIUnitDistance` correction -- that construction needs
a Golod-Shafarevich class-field-tower argument with no substitution or
tile shape anywhere in it), a genuine, tracked initiative to build the
general number-field layer that construction would need was started,
in bounded pieces validated by property/parity tests rather than
assumed correct. First finding: `include/adelic/dedekind_
factorization.hpp` and `ideal_arithmetic.hpp`'s existing Dedekind-
criterion and HNF-ideal machinery, built and tested only against Pisot
substitution characteristic polynomials, turns out to be genuinely
field-generic -- checked against Q(sqrt(-5)), a totally imaginary
field with no real embedding at all, reproducing the classical
`6=2*3=(1+sqrt(-5))(1-sqrt(-5))` non-unique-factorization example with
zero code changes (`tests/number_field_parity_test.cpp`). Second:
`math/poly_discriminant.hpp` extends `maximal_order.hpp`'s
long-long-only discriminant computation to arbitrary precision, and in
doing so found a real, previously undetected bug -- the long-long path
silently returns wrong values (not merely too-large-to-hold ones)
starting at degree 9, from intermediate Bareiss-elimination overflow,
confirmed against an independent closed-form re-derivation
(`tests/poly_discriminant_bigint_test.cpp`, documented directly in
`maximal_order.hpp`). Third: `include/adelic/dedekind_
factorization.hpp`'s `factor_fp` (linear-factor root extraction only,
assuming any degree>=2 residual is a single irreducible) is replaced
with `fp_poly_factor.hpp`'s general, any-degree algorithm (squarefree
+ distinct-degree + Cantor-Zassenhaus equal-degree factorization,
Cohen ch. 3.4) under the same name -- every caller gets the fix with
no call-site changes. Found and fixed a real bug this same way: `x^4+1
mod 5` factors as two irreducible quadratics, which the old code
reported as one wrong quartic, propagating to `factor_prime_in_qbeta`
wrongly reporting one prime above 5 (residue degree 4) instead of two
(residue degree 2 each) -- demonstrated at both levels in
`tests/fp_poly_factor_test.cpp`. `FpPoly`/`FpFactor` and the shared
primitives were extracted to a new `fp_poly.hpp` to avoid a circular
include between the two files. Fourth: `maximal_order.hpp`'s
Pohst-Zassenhaus Round 2 enlargement gets the same arbitrary-precision
treatment (`enlarge_order_round2_bigint`), and the long-long path's
overflow risk turns out to have a concrete, ugly consequence: for
`x^10-2` at `p=2` it doesn't just lose precision, it throws a
*spurious* runtime exception (an int64 overflow masquerading as a
"construction invariant violated" math bug). The bigint version
computes cleanly and passes an independent sanity check --
`disc(O)/disc(O') = 262144 = 512^2`, a perfect square, exactly as the
discriminant-under-enlargement identity requires
(`tests/maximal_order_bigint_test.cpp`). A `needs_another_round`
necessary-condition check is included. Fifth: that general
p-radical-from-structure-constants algorithm (Cohen §6.1.3, Ore's
Frobenius method) is now built (`GeneralOrder`,
`p_radical_from_structure_constants`) and available as a genuine C++
overload of `enlarge_order_round2_bigint` -- the fast, monogenic-
shortcut path stays default for round 1 (Z(β) is always monogenic
there), and the more expensive general path is what round 2+ would
need once the enlarged order may no longer have a single generator.
Cross-checked against the trusted monogenic-shortcut answer on the
same Dedekind cubic, at `p=2, n=3` (the harder `p<=n` regime a
trace-form shortcut wouldn't even cover): exact agreement, `-2012 ->
-503`, computed without ever factoring a defining polynomial
(`tests/general_order_radical_test.cpp`). Sixth: the one remaining gap
in that chain -- deriving a new `GeneralOrder`'s own structure constants
from an enlarged order's HNF basis -- is now closed
(`structure_constants_from_basis_change`), letting a caller represent the
first enlarged order without a defining polynomial. The representative
quartic now has a verified closed first-round basis (`-86320 -> -21580`), but
the generic subsequent-round criterion in a ramified maximal order remains
open and is not used for certification.

An earlier task dependency wrongly assumed Golod-Shafarevich
verification needed class number/class group computation first;
rereading Sawin's paper directly showed neither Lemma 11 (the
Golod-Shafarevich inequality itself) nor its use in the paper's main
construction needs a computed class group -- only primality testing,
prime enumeration, and the Legendre/Kronecker symbol, all of which were
also confirmed genuinely absent from the codebase (a real first-time
gap, not documentation drift). Seventh: `math/primality.hpp` now
provides `is_prime` (mini-gmp's own unused BPSW + deterministic
Miller-Rabin test), `next_prime`, `sieve_of_eratosthenes`, and
`kronecker_symbol` (Cohen Algorithm 1.4.10, written from scratch, since
mini-gmp has no Jacobi/Kronecker symbol at all), validated against
Euler's criterion for every odd prime <=97
(`math/tests/test_primality.cpp`, 58 checks). Eighth:
`include/adelic/golod_shafarevich.hpp` uses that machinery to check the
Lemma 11 inequality directly, and reproduces Sawin's own published
parameters (`T` of 13 primes, `S_Q` of 22 primes) exactly, including
the equality `36 <= 36` the paper reports and the precise
ramified/split/inert partition of `S_Q` it describes
(`tests/golod_shafarevich_test.cpp`, 16 checks). Class number/class
group computation remains unbuilt but is no longer on the critical
path to this verification; it stays independently interesting as its
own piece. Ninth: while starting task #6 (verifying Sawin's
Proposition 10 exponent formula, which needs logarithms), built
`bigfloat_log` and in doing so found a real, previously-undetected bug
in the pre-existing `bigfloat_exp` -- no argument reduction despite its
own header comment claiming otherwise, so `bigfloat_exp(-20)` (and
anything with `|x| >~ 20`) silently hit a hardcoded 1000-term
Taylor-series cap before converging. First looked like genuine
run-to-run nondeterminism in the shared math-library test binary; an
isolated reproducer showed it was fully deterministic once separated
from the rest of that binary's tiers, at which point the mechanism was
easy to see and fix with the standard `exp(x) = exp(x/2^k)^(2^k)`
range-reduction trick (mirroring `bigfloat_sin`/`bigfloat_cos`'s
existing reduction). `bigfloat_exp` had zero callers anywhere in the
repo before this, so the fix is purely an accuracy improvement, not a
behavior change any caller could depend on. Fifteen repeated runs of
the full test binary after the fix, all clean. Tenth: with
`bigfloat_log` actually working, task #6 itself is now done --
`include/adelic/sawin_exponent.hpp` implements Proposition 10's own
exponent formula (equation 11) directly and reproduces, independently
(not via Sage or any external CAS), the paper's own published
intermediate values for its worked example: numerator `3.8822...`,
denominator `275.055...`, and `delta = 0.014114...`, giving Theorem 1's
published exponent `1.014114` exactly to the paper's own stated
precision (`tests/sawin_exponent_test.cpp`, 8/8 checks, confirmed
deterministic across ten repeated runs given the earlier scare with
`bigfloat_exp`). Ten for ten so far: every property test either
confirmed existing machinery was more general than it had been
exercised as, found a real bug in already-shipped code, or produced a
genuine, independent reproduction of a published result. Both tasks
directly aimed at the OpenAI/Sawin unit-distance construction (#5, #6)
are now complete; only class number/class group computation (#4,
independently interesting, not on any critical path anymore) remains
open from this initiative.

Eleventh: task #4 itself, started. `include/adelic/quadratic_class_group.hpp`
computes class numbers of imaginary quadratic fields via binary
quadratic forms (Cohen Ch. 5), deliberately scoped to that case rather
than general-degree-n class groups (which would need LLL/Buchmann-style
machinery this project has no other use for). Checked against all nine
Heegner numbers (the complete list of negative fundamental
discriminants with class number 1) via an independently-constructed
principal form, several tabulated `h>1` cases, and a cross-check
between two different code paths -- `qform_reduce`'s iterative
transformation against `enumerate_reduced_forms`'s direct search --
which caught a real off-by-one bug in the reduction step's range
formula before it shipped. Class number is done. Twelfth: class *group*
structure too, now -- `qform_compose` gives the actual group law, via
the ideal correspondence (`a*Z + ((-b+sqrt(D))/2)*Z`, reusing
`ideal_arithmetic.hpp`'s already-tested general ideal multiplication)
rather than a hand-derived, easy-to-misremember Gauss/Dirichlet
composition formula. Checked directly against the group axioms
(identity, two-sided inverses, closure, commutativity, associativity
over all 27 triples for D=-23, Lagrange's theorem) across four test
discriminants, deliberately NOT against a memorized "D=X has structure
Y" table -- a real sign/conjugation bug was caught this way during
development (identity composed with a form gave that form's inverse
instead of itself), and the group-theoretic prime-order argument (a
group of prime order 3 must be cyclic) confirmed D=-23's structure with
zero external data needed. A search across small discriminants for a
non-cyclic example (skipping h in {2,3,5,6,7}, which group theory alone
forces cyclic) found D=-84 has a non-cyclic class group -- discovered,
not asserted from possibly-misremembered reference tables. Thirteenth:
Sawin's Lemma 9 class-number bound itself, checked (not just Lemma 11
and Proposition 10 that USE it) -- `include/adelic/sawin_lemma9.hpp`,
specialized to the base case (F=Q, K imaginary quadratic), confirms the
bound holds for the nine Heegner numbers and several `h>1`
discriminants using this session's own independently-computed class
numbers, not an assumed or trusted-from-the-paper value. Thirteen for
thirteen so far.

## Research directions

Stale as of an earlier pass; corrected here (2026-08-01) against
`GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s actual current state, not
carried forward unchecked. The four-transition base seam AND recurrent-
SCC exhaustion/dominance (both named below as "next" in the previous
version of this section) are done: the base seam closed 2026-07-31
night (see "Class-II family" above), and all five recurrent-exhaustion
support statements are verified together at `a` in `{7,...,20,30}`
(fifteen values, zero exceptions) -- checked, not yet closed-form, but
no longer "next."

The direct next theorem work, per `GLOBAL_CATALOGUE_OCCURRENCE_
EXHAUSTION.md`'s own "Next implementation" section, is a symbolic
backward-envelope certificate for round 1's neighbor signed contact set
-- exact affine category classification of `tau_a`'s own raw backward-
closure branches, reduced via the Class-II cubic to a bounded number of
`x0` slices, closed by affine endpoint classification, the same method
that closed the center's 16-state pre-contact envelope
(`docs/CLASS_II_CONTACT_BASE_PROOF.md`). **One piece of this is now
done (2026-08-01):** the target formula itself,
`plus_minus_C(tau_a) = plus_minus_C(sigma_a) union E_1`, is proven --
not swept, proven outright, since all three named catalogues are
parameter-free and the identity holds for literally every `a` at once
(`app/class_ii_neighbor2_round1_union_hypothesis.cpp`,
`make class_ii_neighbor2_round1_union_hypothesis`). What remains is the
harder half: proving this 50-state target is the CORRECT reverse-
inclusion closure of `tau_a`'s own `D_cont` seeds for every `a`, which
is what would let Round 1's own base premise (currently
`open_reverse_inclusion` per `class_ii_neighbor2_first_missing_premise`)
close symbolically instead of remaining exact-finite-checked. A
secondary, more mechanical direction: upgrading the recurrent-SCC
exhaustion items above from "checked at 15 values of `a`" to actual
closed-form proofs, in the same
spirit as this session's earlier "prove even the counterexamples"
upgrades to Rounds 2/3/4.

Parallel directions include:

- extending contact-side family proofs to neighboring substitutions
  (neighbors 0 and 1 currently lack the complete raw-corona/Red grammar
  neighbor 2 has -- a separate, earlier-stage problem);
- constructing finite local carry quotients and bounded corrections
  (the current `CSYAutomaton` is a bounded absolute-position prefix
  object, not yet the finite local carry quotient of the cited
  construction);
- comparing geometric windows, labelled dynamics, twisted operators,
  zeta/L-function packages, and spectral embeddings with an explicit
  preservation/forgetting ledger;
- retiring Python prototypes only after bounded native parity or a
  documented correction;
- porting the still-C++-only combinatorial-exhaustiveness inputs behind
  Round 1's Red-pruning and Rounds 2/3/4's shape-closure arguments to
  Lean, to bring the whole base seam up from "one tier below
  Lean-formalized" to fully formalized.

## 2026-08-10 — Exact published analogue for the Class-II tail

The online literature search found an exact match rather than only a
structural analogy: Akiyama's Section 4 proof of Property (F) for
`x^3-a*x^2-(a+1)*x-1` (bibliography key
`Akiyama2000CubicPisotFiniteBeta`). Its Lemma 7 gives the same alternating
`k, a-k` finite-expansion schedule as the observed Class-II affine rank spine.
The paper's proof bounds the conjugate-coordinate critical set, proves a
small integral correction bound for equal leading coordinate (Lemmas 4--6),
and then resolves the finite correction cases by explicit expansions. The
transfer constants are concrete: `|x_0+x_2|<=2` for `a>=7`, equal-leading
corrections `|Delta x_1|<=2`, `|Delta x_2|<=1` for `a>=12`, and a finite
correction window `|kappa_1|<=2`, `|kappa_2|<=1` (the sharp critical-coordinate
bound is established for `a>=15`).

This suggests a concrete acceleration path for the non-AR seam: identify our
phase carrier with Akiyama's leading coefficient, transport the conjugate
window bound to the Property-F coefficient lattice, and prove that every
non-carrier prefix branch is one of the finite correction cases. That would
replace a graph-level collar observation with a symbolic bounded-correction
lemma. The source is archived under `refs/FullText/`; the substitution
contact/prefix intertwiner remains separate and is not claimed closed.

## 2026-08-10 — Finite correction absorption bridge

`include/adelic/property_f_class_ii_correction_absorption.hpp` now composes
the next four local facts into one named certificate. On each realized
`a>=7` Class-II graph it checks: (i) the phase identity makes a nonzero digit
deviation leave the two-state strip; (ii) the Akiyama-window candidates have
no non-seed high recurrence; (iii) the sole retained high correction is the
zero-seed self-loop, hence internal SCC transport; and (iv) every other
bounded correction enters the height-5 collar, whose grammar has no tail
re-entry. The focused family test passes for `a=7..13`; an opt-in
400,000-node run also passes at `a=14` (`362,050` nodes, `31` layers).

This is the forest-level assembly rule we were missing: affine continuation
is the recurrent spine, bounded deviations are a finite correction set, and
the collar is a transient quotient. It remains a graph-level bridge, not a
uniform Property-(F) theorem. The unresolved obligation is now sharply
localized: derive the phase-absorption premise and the Akiyama-to-literal
prefix/contact coordinate transport from the ordered-prefix grammar for all
parameters, rather than replaying them on closed graphs.

## Validation commands

```sh
make class_ii_corona_literature_probe
make class_ii_symmetry_probe
make class_ii_bp_family_probe
make lean-check
make check
```

The detailed terminal transport audit is opt-in:

```sh
make class_ii_terminal_transport_probe
```
