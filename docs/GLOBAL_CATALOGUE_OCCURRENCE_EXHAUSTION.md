# Global catalogue occurrence and exhaustion

Status: active stitching theorem after closure of the center contact
base, 2026-07-29.

## What is now available

The center contact base is no longer a hypothesis:

```text
9 D_cont seeds -> 16 pre-contact states -> 14 contact states
```

for every `a>=2`.  The reverse certificate derives all raw symbolic
prefix families, classifies all 255 bounded window cases with zero
unresolved, and counts `6a+21` restricted branches.

For fixed-light neighbor 2, the local layer obligations are also
separated and individually exact on their stated parameter domains:

1. center-interface support has an exact contact-base chain and
   `7+4` bounded composition witnesses;
2. stable raw-corona inclusion and window exhaustion;
3. stable Red ranks, including later-rank predecessor maps;
4. penultimate repartition, raw inclusion, and all three Red ranks;
5. terminal affine edit, raw inclusion/window exhaustion, and rank
   edit;
6. repeated-terminal raw exhaustion and the six Red ranks;
7. explicit Red bridges from every correction layer into a surviving
   correction or center state.

The enrolled test now makes the first dependency explicit:
`ClassIINeighbor2CenterInterfaceCertificate::exact()` requires the
universal center-contact certificate, not merely literal containment
in a hand-written fourteen-state set.

## What “stitching” still means

Lean file `class_ii_global_round_partition.lean` now proves that for
every `a>=7` the five domains below are disjoint and exhaustive for
`1<=r<=a+1`.  Its theorem `class_ii_global_round_stitch` also proves
the total logical router: any round proposition follows globally once
it is supplied on the five displayed domains.  The remaining
mathematical induction object must state:

```text
B_r(a) = the displayed pre-Red catalogue at round r
A_r(a) = the displayed post-Red catalogue at round r
```

and prove, with every exceptional range explicit,

```text
raw_corona(A_{r-1}(a), +/-C(a)) = B_r(a)
Red(B_r(a)) = A_r(a).
```

The proof must route rounds into:

- bounded base rounds;
- stable interior rounds;
- penultimate;
- terminal;
- repeated terminal/fixed point.

The existing certificates prove the branch-local equalities used in
the stable and endpoint cases.  The total phase partition and generic
induction router are now kernel checked.  The remaining seam is to
instantiate their proposition with the actual `A_r/B_r` definitions,
promote the four bounded base rounds from support/composition
certificates to full raw-corona/Red equalities, and then feed all five
equalities to the router.  Small parameters below the stable threshold
must remain a separate finite table instead of pretending the `a>=7`
grammar covers them.

The C++ side now names these schematic objects for every closed
non-base phase:

- `class_ii_neighbor2_nonbase_pre_red_catalogue(a,r)` is `B_r`;
- `class_ii_neighbor2_nonbase_pruning_ranks(a,r)` is the ordered Red
  partition;
- `class_ii_neighbor2_nonbase_post_red_catalogue(a,r)` is `A_r`.

The dispatcher is total on stable, penultimate, terminal, and repeated
rounds and deliberately rejects base rounds. The enrolled test checks
429 instantiated non-base catalogues for `7<=a<=32`: every rank lies
in `B_r`, ranks are disjoint, and reinserting all removed ranks into
`A_r` reconstructs `B_r`. These are named definitions and structural
regression checks; the universal local certificates remain the
evidence for their transition meaning.

### The four base premises, explicitly

The word “base” must not collapse four different statements:

| round | displayed neighbor-2 correction | already closed | still needed for the global equality |
|---:|---|---|---|
| 1 | mirror-closed `E_1`, 22 states | universal center contact base; literal signed-contact partition; window validity | reverse inclusion for the neighbor signed-contact set |
| 2 | fixed `E_2`, 25 states | center/correction positive composition witnesses and explicit Red successors/bridges | full raw-corona reverse inclusion and full Red exclusion |
| 3 | 24 fixed roles plus the first tip | positive center-interface/correction witnesses, window validity, Red retention | full raw-corona reverse inclusion and full Red exclusion |
| 4 | 24 fixed roles plus the affine tip | interior coefficientwise positive transport and Red retention | connect the bounded round-four grammar to the universal stable reverse/exclusion theorem whose stated domain begins at round five |

These are occurrence/exhaustion gaps, not missing endpoint-validity or
survival lemmas. In particular, the `7+4` center-interface witnesses
do not prove that no other raw state occurs, and a positive Red bridge
does not prove that every undesired state is pruned.

The exact center trace provides a useful finite target without proving
the neighbor transition: for every sampled `a>=4`, its post-Red base
sizes are `28,47,68,88` and its raw sizes at rounds two through four
are `104,167,238` once `a>=5`; the collision at `a=4` gives raw
round-four size `241`. The neighbor transport formula then proposes
post-Red corrections of sizes `22,25,25,25` (the last two are the
24 fixed roles plus one tip). These counts are discovery data and
implementation targets; the missing reverse/exclusion proofs must
identify the literal sets, not merely reproduce their cardinalities.

The independent exact trace comparison confirms the proposed
neighbor post-Red base sizes for `a=7,8`:

```text
round             1   2   3    4
center A_r       28  47  68   88
neighbor A_r     50  72  93  113
literal E_r      22  25  25   25
```

Every center layer is literally contained and the set difference is
the displayed correction. This pins down the four `A_r` targets. It
remains finite discovery evidence until the corresponding raw
`B_r` reverse inclusions and Red exclusions are proved universally.

### Round 1: a precise structural narrowing (2026-07-30, extended verification, not a closed proof)

The unsigned (pre-mirror) contact set of `tau_a`, obtained by running
`search_D_cont -> backward_closure -> red_anode` directly (the same
trusted functions `app/class_ii_neighbor_probe.cpp`'s `corona_trace`
already calls) rather than by re-deriving a symbolic envelope, has
been dumped and compared node-for-node against the center's own
14-state contact set for every `a` in `3..15`, with zero exceptions
(`app/class_ii_neighbor2_dump_backward.cpp`):

- the center's 14 states are, literally, node-for-node, all present
  in `tau_a`'s own 25-state unsigned contact set, for every tested
  `a`;
- the remaining 11 states are new to `tau_a`, and are the *exact same
  11 literal `(i,x0,x1,x2,j)` tuples*, with no `a`-dependence, for
  every tested `a`;
- the mirror-closure of those 11 states (`[i,x,j] -> [j,-x,i]`)
  equals `class_ii_neighbor2_initial_extension_states()` (`E_1`)
  exactly, node-for-node, for every tested `a`.

This is exact finite verification extended well past the previous
`a<=8` ceiling, not a universal proof, and should not be cited as
closing Round 1.

What it does supply is a genuine narrowing of the remaining proof
target, for a structural reason rather than a numerical coincidence:
`class_ii_neighbor_image_segments` shows letters `0` and `2`'s images
are byte-identical between the center (`sigma_a`) and neighbor 2
(`tau_a`) for every `a` -- only letter `1`'s image differs
(`0^a 2` vs `0^(a-1) 2 0`). Any raw backward branch whose prefix-family
decomposition never has to expand letter `1`'s image is therefore
provably identical for center and neighbor, which is the structural
reason the center's 14 states survive unchanged rather than an
empirical accident. The remaining reverse-inclusion obligation is
consequently narrower than "re-derive the whole envelope from
scratch": it is "prove that branches touching letter `1`'s image
produce exactly these 11 states, and no others, for every `a`" --
reusing, rather than re-deriving, everything the center's closed
certificate already established for the letter-`0`/letter-`2`
branches. That symbolic step (an `E_1`-specific window-validity
margin and category table, analogous to the center's own, but scoped
to only the letter-`1` branches) has not been built. See
`app/class_ii_neighbor2_dump_backward.cpp` (ground-truth extraction),
`app/class_ii_neighbor2_e1_harvest.cpp` and
`app/class_ii_neighbor2_e1_window_check.cpp` (an earlier, incompletely
scoped attempt at the same question, kept for its diagnostic value
and as a caution: an argument-order bug in
`class_ii_neighbor_transition_weight` calls silently inverted the
parent/child relation there before it was caught and fixed -- read
the fix before reusing the pattern).

### Round 1: validated symbolic self-closure of the raw 27-state pre-Red set (2026-07-30/31)

A direct (non-forked) attempt built a symbolic (affine-in-`a`)
replica of the *actual* trusted `backward_edges` formula from
`corona.hpp` (`M x' = x + l(p1) - l(q1)` for type 1, roles swapped and
`M` negated for type 2) -- not `class_ii_neighbor_transition_weight`,
which was traced carefully and shown not to reproduce `backward_edges`
at all, even after fixing an argument-order bug and adding the
window-validity gate: on the *center's own known* `9->16` chain (the
one universal-truth check this whole attempt was built to survive),
a naive replica using `class_ii_neighbor_transition_weight` blew
straight past a 500-node BFS cap. The correct replica, built instead
from `class_ii_neighbor_symbolic_prefix_families` (the low-level
"where does this letter appear in that image" enumeration, which was
never the bug) combined with the verified `backward_edges` formula,
reproduces the center's 16 states exactly, and the neighbor's 27
states exactly, node-for-node, for `a` in `3..8` --
`app/class_ii_neighbor2_bfs_v2.cpp` (a second bug, in
`abelianize_prefix` confusing a segment index with a flattened
position, was found and fixed the same way, by validating against
this known-correct case before trusting it on the neighbor).

Built on that validated formula, `app/class_ii_neighbor2_symbolic_backward.cpp`
computes the *symbolic* (`ClassIIAffineValue`, affine-in-`a`) raw
one-hop backward branches into the full 27-state pre-Red closure (the
center's 14, the 11 new states, and the 2 states Red later prunes),
using the neighbor's actual image structure throughout. This produces
97 raw branch categories with affine `(min,max)` ranges for `x0`, with
zero violations of the (checked, not assumed) structural fact that
categories 1 and 2 (the `x1`,`x2` coordinates) are always plain
integers, never `a`-dependent.

Evaluated at concrete `a` and window-filtered with the exact (not
floating-point) `in_H_sigma_exact` predicate, this reproduces the
target 27-state set **exactly** for every `a` in `3..8`: zero spurious
states, and the only two nodes not recovered as one-hop predecessors
are `(1,(0,0,1),1)` and `(2,(0,1,-1),1)` -- which are precisely the
two `D_cont` seeds present in the target set, correctly requiring no
predecessor since they are the closure's own base case, not a gap.

This is real progress, precisely bounded: it is an exact finite
certificate (checked `a=3..8`, not universal) that a *symbolic,
affine-in-`a`* raw-branch derivation, built from validated primitives,
correctly self-closes the full 27-state target. What it is not yet:
the window-validity filtering above was done by concrete evaluation
at each tested `a`, not by the abstract `(c,d)`-corner-bound argument
the center's own certificate uses (`universally_nonnegative` etc. in
`class_ii_contact_backward_envelope_certificate()`) to make the claim
hold for literally every `a`. Promoting these 97 categories' window
tests to that abstract, `a`-independent form -- deriving whatever
margin(s) in `(c,d)` play the role of the center's `2/3<d<1`, for
these specific categories -- is the concrete remaining step, and it
is now well-scoped: the categories, their affine ranges, and the
exact target they need to hit are all in hand and validated. Red
pruning (27 -> 25, matching the already-known post-Red survivor sets)
has not yet been attempted symbolically.

### Round 1: window validity closed abstractly, not just at sampled `a` (2026-07-31)

The remaining step named above -- promoting the 97 categories' window
test from concrete evaluation to the center's own abstract
`(c,d)`-corner-bound argument -- is done.
`app/class_ii_neighbor2_round1_window_certificate.cpp`
(`make class_ii_neighbor2_round1_window_certificate`) rebuilds the same
97 categories and applies `class_ii_contact_backward_envelope_certificate()`'s
own `AffineCD`/`universally_nonnegative`/`universally_positive` closures
verbatim (duplicated, not refactored, to avoid touching that already-verified
function) to every category and every `x0` in `[-3,3]` (wider than the
center's `[-2,2]`, since this family's category `x2` range is `[-2,2]`
rather than the center's `[-1,1]`). Result: all `679` bounded cases
resolve with **zero unresolved cases** -- the same margin bounds the
Lean-derived `2/3<d<1` gives the center decide every neighbor case too,
with no dependence on `a` at all. The window-valid-and-occurring node
set is exactly the 25 non-seed states of the 27-state raw target once
the same all-zero `i<j` triviality convention the center's certificate
already uses is applied (four `(x0,x1,x2)=(0,0,0)` mirror duplicates
would otherwise appear as false positives); the two absent states are
exactly the two D_cont seeds (`{1,{0,0,1},1}` and `{2,{0,1,-1},1}`,
confirmed against `class_ii_d_cont_set()`), which correctly need no
predecessor. Cross-checked against the concrete numeric method at
`a = 9, 10, 15, 20, 30, 50, 80, 120` (beyond the originally tested
`3..8`): identical `window_valid=25, extra=0`, same two missing seeds,
at every value.

What this does and does not prove, precisely: it proves the
window-validity half of Round 1's raw self-closure universally --
every one-hop backward branch that occurs in the bounded region also
lands in the stepped-hyperplane window if and only if it is one of the
25 non-seed target states, for every integer `a`, not six sampled
ones. It says nothing on its own about Red (the two D_cont seeds shown
to need no predecessor here are not, by this argument alone, the same
two states the post-Red survivor accounting elsewhere calls "pruned by
Red").

### Round 1: the Red-pruned pair coincides with the D_cont-seed pair (checked, not proved; 2026-07-31)

That "different, unverified claim" from the paragraph above has now
been checked directly against the trusted ground-truth pipeline
(`search_D_cont -> backward_closure -> red_anode`, the same functions
`app/class_ii_neighbor2_dump_backward.cpp` already calls) rather than
left as a coincidence of the counting. `app/class_ii_neighbor2_round1_red_identity.cpp`
(`make class_ii_neighbor2_round1_red_identity`) computes, for every `a`
in `[3,60]`, the actual rank-one Red-pruned pair and compares it against
the two D_cont seeds `{1,{0,0,1},1}` and `{2,{0,1,-1},1}`: **zero
mismatches** across all 58 tested values. So, as an exact finite check
(not yet a symbolic proof for every `a`): the two states Red prunes and
the two states needing no backward predecessor are the same pair.

This is real progress on "which two states" but is explicitly **not**
the symbolic Red-pruning proof Round 1 still needs. It answers a
question (are they the same pair, or two different pairs?) that was
previously open and could have gone either way; it does not yet show
*why* Red prunes exactly these two for literal every integer `a` --
that requires a symbolic argument about `forward_edges`/
`induced_restricted_edges` applied to these two specific seed nodes
(a different derivation direction from the backward-branch categories
used above).

### Round 1: Red pruning closed symbolically for `a>=3` (2026-07-31)

That symbolic argument is now done.
`app/class_ii_neighbor2_round1_red_symbolic.cpp`
(`make class_ii_neighbor2_round1_red_symbolic`) derives, from `tau_a`'s
own fixed word forms (`sigma(0)=0^a 1 2`, `sigma(1)=0^(a-1) 2 0`,
`sigma(2)=0`) rather than from sampling `a`, that letter `1` occurs
exactly once across all three images and letter `2` occurs exactly
twice, for every `a` -- a structural fact about the definition, not an
observation. This pins `parent_decompositions` to fixed cardinalities
(1 for letter `1`, 2 for letter `2`), so `forward_edges_type1`/`type2`
applied to the two D_cont seeds `A={1,{0,0,1},1}` and `B={2,{0,1,-1},1}`
enumerate exactly 2 raw candidates for `A` and 4 for `B` -- never more,
for every `a`. `tau_a`'s incidence matrix is the fixed form

```text
M(a) = [[a, a, 1],
        [1, 0, 0],
        [1, 1, 0]]
```

Rows 1 and 2 don't depend on `a`, so back-substitution from those rows
first, then row 0, gives `x2' = rhs[0] - a*rhs[2]` for the solved
successor coordinate. The file computes `rhs[0]` and `rhs[2]` for all 6
raw candidates using `class_ii_neighbor_symbolic_prefix_families`
(the same trusted engine the window certificate above already builds
its 97 categories from) and confirms, for every one of the 6: `rhs[2]`
is itself `a`-independent (slope 0, not assumed) and the resulting
`x2'(a)` is affine in `a` with slope exactly `+/-1` -- reproducing
exactly the six affine forms (`-a, a, a+1, a, 1-a, -a`) a first hand
derivation against the same matrix found independently (self-corrected
once, an arithmetic slip on which row of `M` to use for the second
equation, caught by cross-checking against the numeric ground truth
rather than trusted on the first pass).

Since every one of the 27 raw pre-Red target states has all three
coordinates in `[-2,2]`
(`app/class_ii_neighbor2_round1_red_forward_check.cpp` checks this
directly), and every raw candidate's `x2'` has slope exactly `+/-1`
(not some fraction that could vanish), `|x2'(a)| >= a - 1` at worst --
so for `a>=3` no raw candidate, surviving `tau_a`'s window filter or
not, can ever equal any of the 27 targets. `induced_restricted_edges`
therefore finds zero edges from `A` or `B` into the 27-state set for
every `a>=3`, which is exactly `red_anode`'s rank-one pruning criterion.
This is symbolic, not sampled: it holds for literally every integer
`a>=3`, matching the exact finite check at `a=3..60` above.

Scope, precisely: the clean bound argument needs `|a|>2`, i.e. `a>=3`
(matching this whole Round-1 investigation's tested domain throughout).
At `a=2`, `x2'=+/-a=+/-2` sits exactly on the target bound rather than
outside it, so this specific argument does not automatically cover
that case; `class_ii_neighbor2_round1_red_identity.cpp` was re-run at
`a=2` directly and still found zero mismatches there, but by the
ground-truth pipeline, not by this bound argument -- `a=2` is not
claimed closed by the symbolic proof above, only by the exact finite
check.

Together with the abstract window-validity certificate, Round 1's raw
27-state self-closure and its Red-pruning down to the 25-state target
are now both closed for `a>=3`: the full `27 -> 25` transition no
longer depends on a sampled range of `a`.

**Lean formalization (2026-07-31):** the arithmetic core of this
argument -- that all six raw candidates' x2'-images (`-a, a, a, a+1,
-a, 1-a`) lie outside the 27-state target's x2-range for every integer
`a>=3` -- is now Lean-kernel-checked, sorry-free, and signed:
`lean/class_ii_round1_red_pruning.lean`. This is a genuine tier
upgrade for that piece specifically, from "checked C++, exact rational
arithmetic" to "Lean-kernel-checked," matching this project's own
claim-strength vocabulary. It formalizes the arithmetic only, not the
combinatorial fact that these six candidates are exhaustive (that
still rests on `tau_a`'s fixed word forms as established and checked
in the C++ file, taken as given input here -- the same scope
convention `class_ii_neighbor2_extensions.lean` already uses for its
own fixed-state catalogues). The Lean file also tightened the target
bound used in the proof: the 27 targets' x2-coordinates are actually
in `{-1,0,1}`, not the coarser `[-2,2]` used in the prose above (that
coarser bound comes from the x0/x1 coordinates, not x2) -- checked
directly, not assumed, and the tightened bound still gives exactly the
same `a>=3` threshold (the binding case, `1-a`, sits on this tighter
boundary at `a=2`, matching the scope note already given above).

**Scope note, precisely -- corrected after checking, not guessed:**
the "27-state" and "25-state" objects here are the *unsigned*
`new_states`(11) + `center_states`(14) [+ 2 seeds for the raw 27], not
directly `class_ii_neighbor2_signed_contact_set()` (50 states) that the
base-premises table's Round 1 row calls the "neighbor signed-contact
set." Checked their exact relationship rather than assuming one:

- `search_D_cont` for `tau_a`'s own 9 seeds is **not** self-mirror-closed
  (verified directly: mirroring the 9 seeds gives 9 *different* states,
  zero of the originals have a mirror partner among themselves) -- so
  the raw `backward_closure(D_cont)` this whole investigation has been
  proving is a genuinely asymmetric (unsigned, oriented) computation,
  not secretly already-symmetric.
- Running the *actual*, unrestricted `backward_closure -> red_anode`
  pipeline on `tau_a`'s real `D_cont` (not the hypothesized 27-state
  target) reproduces exactly `27` raw states and `25` survivors, at
  every tested `a` -- confirming the unsigned 25-survivor set
  (`center_states`+`new_states`) genuinely *is* the real ground-truth
  answer, not an approximation of some larger true object.
- `class_ii_neighbor2_signed_contact_set()` is verified **exactly**
  (not approximately) equal to (unsigned 25 survivors) `union`
  mirror(unsigned 25 survivors) -- checked as a literal set equality,
  50 = 50, zero extra, zero missing.

**Correction, same session:** the paragraph originally here claimed
"(2) the general fact that mirroring a correct backward-closure/Red
result gives a correct backward-closure/Red result for the mirrored
destinations" was "plausible by symmetry." That specific claim was
checked directly and is **false in the naive form**: running
`backward_closure -> red_anode` from the *mirrored* `D_cont` seeds does
**not** reproduce `mirror(real_survivors)` -- it gives 32 survivors,
not 25, at both `a=5` and `a=8` tested. So the unsigned result does not
transport to the signed one by simply "mirror the seeds and rerun the
same closure"; whatever makes `class_ii_neighbor2_signed_contact_set()`
the correct `±C` for later corona composition, it is not that.

What the identity above *does* establish: `class_ii_neighbor2_signed_contact_set()`
is definitionally `unsigned_survivors ∪ mirror(unsigned_survivors)` --
a fact about how the object is *built* (`build_signed_contact_set` just
unions each state with its own mirror, it is not a rederived closure),
which is exactly why `same_letter_H`/`c_corona` check both a state and
its mirror explicitly rather than relying on the raw backward-closure
being symmetric. That construction step is definitionally sound
independent of any closure-symmetry question. But "reverse inclusion
for the neighbor signed-contact set" as a *closure* question -- does
`±C` actually equal the full backward-closure of *something* symmetric
-- is not answered by this note and should not be assumed from it. The
honest state: the *unsigned* result is closed (`a>=3`); its relationship
to the signed object is a definitional union, verified exactly; whether
that union is *itself* the correct/complete signed-contact answer in
the closure sense the base-premises table means is still open, and is
now a narrower, better-specified question than "mirror-transport,
plausible by symmetry" was.

**One more piece of context, found while looking at how `±C` is
actually consumed.** `c_corona` (Def 3.9) takes `pmC` as a *given* hop
set and composes with it via `same_letter_H`, which explicitly checks
a state *and* its mirror at the destination
(`include/ravel/corona.hpp`, the `c_corona` comment: "The
`same_letter_H` fix (both endpoints) is essential"). That is, the
literature's own `±C` convention is a *definitional* doubling
(`±C := C ∪ mirror(C)`) applied so corona composition can be checked
symmetrically without assuming the underlying oriented closure is
symmetric -- not a claim that redoing the closure from mirrored seeds
would reproduce the same answer. Read this way, the naive check this
note just refuted was never actually load-bearing for `c_corona`'s
correctness: `±C`'s job is to supply both orientations as hops, and it
does that by construction, exactly as verified above. What would still
need an argument, if the base-premises table means something beyond
the definitional doubling, is a comparison between this project's
`class_ii_neighbor2_signed_contact_set()` and whatever independent
characterization of "the neighbor's `±C`" the cited literature (Def
3.9's source) actually specifies -- which is a literature-comparison
question, not a code-verification one, and is outside what this
investigation checked.

### Round 2: reconnaissance only, not attempted (2026-07-31)

With Round 1's window-validity and Red-pruning both closed above
(`a>=3`, unsigned half), I looked at whether the same playbook
transplants quickly to Round 2 (`E_2`, the fixed 25-state correction).
It does not look like a quick follow-on, and I want the reason on
record rather than a silent "ran out of time."

`docs/CLASS_II_NEIGHBOR2_TRANSPORT_THEOREM.md` shows Round 2's
surrounding machinery is already considerably more mature than Round
1's was before today: window validity for the fixed 24-state interior
correction is already Lean-kernel-checked (`lean/class_ii_neighbor2_extensions.lean`,
derived from the same cubic margin `2/3<d<1`-style bound Round 1
reused); corona propagation has an exact witness table
(`class_ii_neighbor2_interior_composition_witnesses`, checked through
`r=128`); Red survival is closed *conditionally on center-layer
survival* for every correction state. What the base-premises table
still lists as open for Round 2 -- "full raw-corona reverse inclusion
and full Red exclusion" -- is the same *shape* of gap Round 1 had
(prove the raw one-hop composition self-closes to exactly the target,
then prove Red removes exactly the right non-survivors), but the input
to that composition is `T_1 = B_1 union E_1` run through `c_corona`
against the fifty-hop signed contact set, not a direct
`backward_closure(D_cont)` -- a genuinely different (and, from the
Lemma-2/3 write-up above, evidently more involved) derivation, not a
copy-paste-with-different-data version of Round 1's `app/class_ii_neighbor2_symbolic_backward.cpp`.

Given the size of that surrounding machinery, attempting Round 2's
remaining gap in the same session, on top of Round 1's already
substantial work today, would have meant starting a comparably-scoped
derivation with materially less time to catch a subtle bug the way
Round 1's session caught three (the `struct`/`import struct` bug in
NetHack's `repl.py`, the double-temporary UB in the window certificate,
and the arithmetic slip in the by-hand `M` derivation). Better to name
this precisely and stop than rush it. A future session picking this up
should start from `class_ii_neighbor2_interior_composition_witnesses`
and the twelve-state/nine-state round-two/round-three center-interface
functional graphs mentioned in Lemma 4 above, not from scratch.

One concrete thing to start from: `app/class_ii_round2_recon.cpp`
(`make class_ii_round2_recon`) confirms, against the same trusted
`algorithm2_trace` pipeline `class_ii_neighbor_probe.cpp`'s
`corona_trace` already calls (not new machinery), that the per-round
survivor-count difference between `tau_a` and `sigma_a` is exactly `22`
at round 1 (matching `E_1`) and exactly `25` at rounds 2, 3, and 4
(matching `E_2`/`E_3`/`E_4`), at `a=7,8`. This is confirmation of the
target *sizes* using existing trusted code, not a derivation of *why*
-- Round 2's actual reverse-inclusion/Red-exclusion gap is unaffected
by this and remains exactly as open as stated above.

**A lead for whoever picks Round 2 up next, now checked more
concretely than a first guess, but still not a proof.** The same
recon checked whether rounds 2 and 3's actual *state sets* (not just
their sizes) are literally identical across different `a` -- not
affine-in-`a` like Round 1's raw candidates were, but exactly,
bit-for-bit the same set, checked at `a=5,6,7,8`. They are (round 4
differs at `a=5` only, consistent with `a=5` being a documented
small-parameter exception elsewhere). The reason this might matter:
Round 1's candidates needed genuine affine-in-`a` treatment because
they came directly from backward-closing into the `0^a` run. Round
2's candidates instead come from `c_corona(T_1, +/-C)`. Checked
directly, not assumed: for `tau_a`, `T_1` **is** `+/-C` -- both are
literally `class_ii_neighbor2_signed_contact_set()`, 50 states, the
same object, confirmed equal at every tested `a` (an earlier draft of
this note incorrectly guessed `T_1` was the *center's* 28-state signed
contact set; it is not, for the neighbor's own corona construction).
Both are `a`-independent sets by construction, so `c_corona`'s
vector-addition step `y + delta` never touches `a` at all --
`app/class_ii_round2_ccorona_stability.cpp`
(`make class_ii_round2_ccorona_stability`) calls the real, trusted
`c_corona()` directly (no reimplementation) and confirms its raw
output size is identically `195` at `a=6,7,8,15`.

The hoped-for shortcut from the paragraph above -- "if Lemma 2 already
covers every candidate, reverse inclusion is a single finite check" --
was checked directly and only half holds.
`app/class_ii_round2_structure.cpp`
(`make class_ii_round2_structure`) matched the 195 raw candidates
against `lean_neighbor2_fixed_states()`'s 24 entries: only 24 of 195
land there. `c_corona()` already applies `same_letter_H` internally, so
all 195 *are* window-valid by construction -- but 195 is not `T_2`
(72 states); it is the pre-Red input to a separate Red-pruning step.
So the actual gap was mis-shaped in the earlier draft: it isn't
(mostly) a window-validity question at all. It's specifically:

- `T_2 = B_2 union E_2` **exactly** (72 = 47 + 25, checked as a literal
  set identity, not just matching sizes, at `a=6,7,8`);
- `E_2`'s 25 states decompose as
  (`class_ii_neighbor2_fixed_extension_states()`'s 24 states, the same
  ones `Neighbor2FixedKind` covers) minus `{2,{1,-2,-1},0}` plus
  `{2,{-2,2,-1},0}` (the general interior tip at `r=2`, *also* already
  covered by `neighbor2InteriorTip_in_open_strip`) plus `{0,{0,1,-1},0}`
  (genuinely new, but its window validity is a one-line corollary of
  the same `c>3, 1/2<b-c<1` bounds Lemma 2 already uses: height
  `= c-1 > 2 > 0`, and `c-1<b` iff `b-c>-1`, true since `b-c>1/2`). So
  **`E_2`'s window validity needs no new derivation** -- every one of
  its 25 states already falls under bounds proved for a different
  purpose.
- The real open gap is Red exclusion, and it is bigger than Round 1's:
  of the 195 raw candidates, exactly 25 land on `E_2` and 47 on `B_2`
  (both fully covered, not just checked in aggregate) -- the remaining
  **123 get removed by Red, across three ranks (98, 15, 10)**, stable
  across `a=6,7,8`. Round 1's Red exclusion was two states in one rank
  with unbounded-in-`a` forward images, which made the argument
  essentially free once written down. These 123 states' coordinates
  only reach `|x_k|=3` against the survivors' `|x_k|<=2` -- there is no
  simple "unbounded in `a`" escape hatch here. A real multi-rank
  forward-edge argument is needed, and it has not been attempted.

This is a materially more precise (and more honest) picture than the
"structurally easier than Round 1" guess in the previous paragraph,
which undersold the actual size of Round 2's remaining obligation.

### Round 2: Red exclusion has an independent exact finite certificate (2026-07-31)

The key missing piece for a symbolic Red-exclusion argument turned out
to matter: `algorithm2_trace`'s Red step does *not* determine edges by
plain vector addition over the fixed 195-state raw set. It calls
`simple_forward_targets_exact`, the same kind of genuine
parent-decomposition-plus-linear-solve machinery Round 1's
`forward_edges` used -- so the pruning graph is not automatically
`a`-independent just because its node set is.

Checked directly rather than assumed: for every one of the 123 pruned
states (all three ranks, `a=6,7,8`), calling
`simple_forward_targets_exact` and comparing each resulting edge
against `red_anode`'s own reported ranks confirms every edge landing
inside the 195-state set goes to a node already pruned at an
earlier-or-equal rank -- never to a survivor, never to a later rank.
Zero violations across `369` total checked states (123 x 3 tested `a`
values) and `90` edges found within the 195-set (30 per `a`).
`app/class_ii_round2_red_exclusion_check.cpp`
(`make class_ii_round2_red_exclusion_check`). This is an *independent*
recomputation, not a re-report of `red_anode`'s own bookkeeping --
exactly the "check against something other than the derivation itself"
discipline this project applies elsewhere, now applied to Red's own
correctness rather than assumed from the fact that `red_anode` is
already trusted code.

What this establishes: an exact finite certificate, at `a=6,7,8`
specifically, that Red's ranking is internally consistent (not a
symbolic proof for literal every `a`, and not yet a proof that these
*are* the only possible edges -- it confirms the observed edges are
consistent with the observed ranks, which is the whole of what
"Red-exclusion is correct" means operationally, but the underlying
`simple_forward_targets_exact` computation has not been derived by
hand the way Round 1's `forward_edges` was). The genuinely open step
is promoting this to a symbolic argument valid for every integer `a`
-- tractable in principle (same machinery as Round 1), but a
materially larger undertaking: several of the 123 states have on the
order of `200+` raw forward-target candidates before window filtering
(Round 1's two pruned states had 1 and 2), so a from-scratch by-hand
derivation for each is not a quick afternoon's work.

### Round 2: the 123 pruned states are themselves `a`-independent (2026-07-31)

A narrower recon question worth checking before attempting the
symbolic argument above: is the pruned-state *set* itself the same
coordinate set across `a`, with only the forward-edge weights varying,
or does the set of states needing pruning change shape as `a` grows?
Checked directly, not assumed: `app/
class_ii_round2_pruned_states_a_independence_probe.cpp` (`make
class_ii_round2_pruned_states_a_independence_probe`) recomputes the
123-state pruned set at `a=6,7,8,9,10,12,15,20,30` and finds it is the
literal same 123-state coordinate set every time (max `|x_k|=3`). This
matches the pattern Round 1 already established (fixed states, `a`
enters only through the forward-map weights) and means the eventual
symbolic proof does not need to also handle a changing enumeration of
which states require pruning as `a` varies -- only per-category
M-matrix arguments for a fixed, already-known list of 123 states.

### Round 3: the same treatment, same scale (2026-07-31)

`app/class_ii_round3_structure.cpp` applies the identical checks to
Round 3: `T_3 = B_3 union E_3` exactly (93 = 68 + 25, literal identity
at `a=6,7,8`), with `E_3` this time being exactly
`class_ii_neighbor2_interior_extension_states(3)` (no special swap the
way Round 2 needed one). Red exclusion also has an independent exact
finite certificate: 256 raw candidates, 93 survivors, 163 pruned
across 3 ranks, zero violations under an independent
`simple_forward_targets_exact` re-derivation at `a=6,7,8`. Round 3 is
comparable in scale to Round 2, not smaller -- there is no shortcut
via "later rounds are easier."

One thing flagged, not resolved: whether
`neighbor2InteriorTip_in_open_strip`'s hypothesis `r*(b-c)+1<c`
actually holds at `r=3` under the coarse `c>3` bound is unchecked here
-- at `r=2` it held trivially (`2*(b-c)+1<3<c`), but at `r=3` the same
style of bound gives `3*(b-c)+1<4`, which needs `c>=4`, tighter than
the generic `c>3`. This may need a sharper, `a`-specific bound rather
than the coarse one Round 2 could use for free.

### Round 4: same structural shape; the real gap is elsewhere (2026-07-31)

`app/class_ii_round4_structure.cpp` extends the identical checks to
Round 4: `T_4 = B_4 union E_4` exactly (113 = 88 + 25, literal identity
at `a=6,7,8`), `E_4` again exactly
`class_ii_neighbor2_interior_extension_states(4)` (no swap, matching
Round 3, not Round 2). Red exclusion: 325 raw candidates, 113
survivors, 212 pruned across 3 ranks, zero violations under an
independent `simple_forward_targets_exact` re-derivation. The `B_4=88`
figure matches the base-premises table's own exact center trace
(`center A_r` row: `28,47,68,88`), an independent cross-check that
this file's `trace_c.layers[3]` is picking up the right object.

**This does not close Round 4's actual stated gap.** Per the
base-premises table above, Round 4's "still needed" column reads
differently from Rounds 2/3: not a bare occurrence/exclusion checklist
item, but "connect the bounded round-four grammar to the universal
stable reverse/exclusion theorem whose stated domain begins at round
five." What this file establishes is that Round 4's raw-corona
occurrence/exclusion *shape* is the same kind of object as Rounds 2/3
(same identity pattern, same kind of finite certificate) -- useful
groundwork, since it means round four is not structurally exceptional
in the way that would block a round-five-style argument from reaching
backward to cover it, but the actual bridging theorem asked for in the
table is a separate, harder claim not attempted here.

### All four base rounds: the a-independence hypothesis, resolved (2026-07-31)

AM asked directly whether the pruned-state coordinate sets are the
literal same across all four rounds. Checked now for every round, not
assumed:

```text
round   pruned states a-independent across a?
1       NO -- pruned coordinates are affine in a with nonzero slope
        (the M(a) argument; -a, a, a, a+1, -a, 1-a), closed
        symbolically for a>=3, now Lean-formalized
        (lean/class_ii_round1_red_pruning.lean)
2       YES -- 123 pruned states, literal same coordinate set at
        a=6,7,8,9,10,12,15,20,30
3       YES -- 163 pruned states, same
4       YES -- 212 pruned states, same
```

The hypothesis as originally posed ("all four are the literal same")
is **false**: Round 1 is a genuine outlier, not an oversight -- its
raw target only has 27 states (versus 195/256/325 for Rounds 2/3/4),
small enough that the pruned pair's forward images are simple closed
affine forms rather than a fixed catalogue. Rounds 2/3/4 share a
different, second pattern: a fixed, a-independent list of pruned
states with only the forward-edge *weights* varying by `a`. This
resolves the reconnaissance question cleanly: there are two regimes,
not four separate ones and not one uniform one, and Round 1 is already
closed by the regime it belongs to (affine-in-a, small raw target).
Rounds 2/3/4 remain open in the regime they share (fixed pruned-state
list, `simple_forward_targets_exact`-driven weights) -- the natural
next symbolic-proof strategy is therefore one argument applicable to
all three of Rounds 2/3/4, not three separate derivations, since they
are now confirmed to share the same shape of gap.

### Sharper still: the entire forward-edge graph is a-independent, not just the pruned-state set (2026-07-31)

A background continuation of this session pushed the question above
one step further and found something stronger: not just the pruned
*node* set, but the entire forward-edge *connectivity* among the raw
pre-Red states -- which state's `simple_forward_targets_exact` output
includes which other states, as a set of destinations, not merely the
exact rational edge weights -- is itself the literal same graph across
`a`, for all three of Rounds 2/3/4.

- Round 2 (195 raw states): identical graph at `a=6,7,8,9,10,12,15`,
  and stress-tested at `a=50` (far outside the sampled cluster) with
  zero mismatches. `app/class_ii_round2_edge_structure_a_independence_probe.cpp`,
  `app/class_ii_round2_edge_structure_far_a_check.cpp`.
- Round 3 (256 raw states): identical graph at `a=6,7,8,9,10,12,15`.
  `app/class_ii_round3_edge_structure_a_independence_probe.cpp`.
- Round 4 (325 raw states): identical graph at `a=7,8,9,10,12,15,20,30`,
  with **one genuine exception at `a=6`** -- state
  `{i=0,x=(3,-3,0),j=0}` gains an extra destination,
  `{i=1,x=(-3,3,-2),j=0}`, only at `a>=7` (14 destinations at `a=6`,
  15 at `a=7` and up). `a>=7` is already this project's established
  general round-family stability threshold elsewhere (the point where
  the "stable" non-base dispatcher's own domain begins), so this reads
  as the same kind of boundary exception as Round 1's `a=2`, not a
  counterexample to the pattern. `app/class_ii_round4_edge_structure_a_independence_probe.cpp`.

**Why this narrows the remaining gap substantially.** The previous
framing ("one symbolic argument applicable to all three of Rounds
2/3/4") still implicitly meant a per-state affine-in-`a` argument
scaled up from Round 1's method, applied across 123+163+212 states.
This finding suggests a cheaper route: any forward-edge coordinate
that is affine in `a` (the general shape the `M(a)` back-substitution
mechanism produces, established for Round 1's two states and
plausible generally, though not yet proven for the full 195/256/325
state sets) and that lands within a *fixed, bounded, a-independent*
target set at two or more distinct values of `a` must, by the basic
fact that an affine function equal to a constant at two points is that
constant everywhere, be `a`-independent for literal every integer `a`.
Since the pruned/survivor node sets are already confirmed
`a`-independent (above), and the connectivity among them now checks
out identical at 7-8 sampled points per round including one far
outlier, this is strong -- but still not yet complete -- evidence
that the whole graph, not just its sampled snapshots, is fixed for
literal every `a` in each round's stable domain (`a>=3` for Round 2/3,
`a>=7` for Round 4).

**What remains, precisely, to turn this into an actual proof**: the
general "forward-edge coordinates are affine in `a`" lemma itself is
not yet established beyond Round 1's two specific states -- it needs
either a direct symbolic derivation from `parent_decompositions` and
`class_ii_neighbor_symbolic_prefix_families`'s general behavior (the
same machinery, not yet shown to always produce affine, rather than
higher-degree, forms), or enough additional sampled points per state
to make higher-degree agreement implausible. This is a smaller, more
tractable target than the original per-state M-matrix undertaking,
but it is not yet closed -- reported at its real strength (exact
finite certificate across 7-8 sample points per round, not a
universal proof), not overclaimed.

### The actual mechanism, found on one representative state (2026-07-31)

Rather than guess at why the graph is `a`-independent, the raw
`(p_k, q_k)`-indexed candidate list itself was inspected directly for
Round 4's state `{i=0, x=(3,-3,0), j=0}` (the state whose extra edge
at `a=6` was the one exception found above), at `a=6,7,15`, printing
every surviving `(p_k, q_k, destination)` triple rather than just
counts.

**What the raw data shows, plainly:** the *number* of raw
`(p_k, q_k)` candidate pairs surviving `is_valid_simple_node` grows
with `a` exactly as expected (32 edges at `a=6`, 45 at `a=7`, 149 at
`a=15` -- `parent_decompositions` for the heavily-repeated letter `0`
really does have `O(a)` entries, confirming the earlier "some states
have 200+ raw candidates" remark is about exactly this). But the
*destinations* those edges land on are overwhelmingly repeated: `x0'`
and `x1'` are the same fixed constants (`-3, 3` for this state's `i=0`
family) across every single surviving `(p_k, q_k)` pair at every
tested `a`, and `x2'` depends only on the *difference* `q_k - p_k`,
not on `p_k`, `q_k`, or `a` individually -- e.g. every pair with
`q_k - p_k = -5` gives `x2' = -2`, whether that pair is `(p_k=5,
q_k=0)` at `a=6` or `(p_k=6, q_k=1)` at `a=15`.

This is the same mechanism Round 1's proof already used (`M(a)`'s rows
1 and 2 don't depend on `a`, so `x0'`/`x1'` come out as fixed
formulas; only the row-0 back-substitution carries an `a`-dependent
term), applied to a *different* free parameter: Round 1's argument
made `x2'` affine in `a` itself (because its two D_cont seeds are
genuinely fixed nodes with a single raw candidate each). Here, `x2'`
is instead affine in `q_k - p_k`, a parameter that both `p_k` and
`q_k` range over widely as `a` grows -- so the *set* of achievable
`x2'` values that fall inside a small bounded target window stabilizes
once `a` is large enough for that range to be fully covered (matching
the observed `a>=7` stabilization point for this exact state), even
though the raw candidate *count* keeps growing.

**Scope, precisely:** this is verified on one representative state,
not yet the general lemma. What a full proof needs, now precisely
named rather than vague: (1) show `x0'`/`x1'` are constant across all
valid `(p_k, q_k)` pairs for every state in the fixed pruned-state
catalogues (plausible from the same `M(a)` row-independence Round 1
already used, but not yet checked state-by-state); (2) show `x2'` is
affine in `q_k - p_k` with a fixed, known slope, for every state; (3)
show the achievable range of `q_k - p_k` (as a function of `a`) covers
every value landing in the bounded target window once `a` crosses each
state's own threshold, and that this threshold is uniformly bounded
(not, say, growing without limit for some pathological state). None of
these three are yet shown beyond this one worked example, but each is
now a concrete, checkable claim rather than an open question about
*why* the empirical agreement holds.

**Checked immediately, not left as an assumption: (1) does not
generalize.** `app/class_ii_round2_edge_mechanism_generality_check.cpp`
tested the representative state's one-parameter model against all 195
of Round 2's raw states at `a=8`: of the 73 states with more than one
surviving edge, only 17 have `x0'`/`x1'` constant across every edge
(claim (1), false in general), and 65 have `x2'` a function of
`q_k - p_k` alone (claim (2)'s weaker cousin, still not universal).
The representative state happened to have `node.i = node.j = 0`,
which is a special case; most states mix contributions from letter 0
(whose occurrence count scales with `a`, per `tau_a`'s fixed word
forms) and letters 1/2 (whose counts don't, per Round 1's own
finding), giving more than the one free parameter the representative
case had. The real mechanism generalizing the graph's `a`-independence
across all of Rounds 2/3/4 is therefore still open -- the
representative-state derivation above is a genuine partial insight
(one concrete case fully understood, not just observed) but not yet a
template that transfers directly to the rest of the catalogue.

**A cleaner refinement, found immediately after.** Splitting Round 2's
73 multi-edge states by whether `node.i`/`node.j` are letter `0`
(`tau_a`'s only occurrence-count-scales-with-`a` letter) or not:
same-regime pairs are clean -- `node.i=node.j=0` (22 states) and
`node.i,node.j` both in `{1,2}` (8 states) each get `x2' = f(q_k-p_k)`
for **100%** of states, no exceptions. Mixed-regime pairs (one letter
`0`, one in `{1,2}`) are mostly clean too (35/43) but have exactly 8
exceptions, and every single exception involves letter `2` specifically
(never letter `1`) -- e.g. `{i=0,x=(-1,1,0),j=2}`, `{i=2,x=(0,0,0),j=0}`.
This tracks with Round 1's own finding that letter `2` (not letter `1`)
has *two* occurrences across `tau_a`'s images rather than one, which
plausibly gives letter-2-involving mixed pairs an extra degree of
freedom the letter-1 pairs don't have. This is a real, promising
further lead -- not pursued to a full characterization tonight, but
precisely enough located (letter `2`'s double occurrence, in mixed
regime only) that a future session should start here rather than
re-deriving the coarse picture.

**One more refinement before stopping this thread.** The pooled tests
above mix surviving edges from genuinely different
`(parent_letter_i, parent_letter_j)` *types* -- Round 1's own proof
never did this; it treated "A type1", "A type2", "B type1", "B type2"
as separate categories throughout. `app/
class_ii_round2_edge_mechanism_by_type_check.cpp` regroups by type
first, then tests the stricter claim "`x0'`/`x1'` constant and
`x2' = (q_len - p_len) + a fixed offset`" within each type: 94/139
multi-edge types are clean at `a=8`, a real improvement over the
pooled test (which had effectively 0 types passing that exact test)
but still not universal. The untested next step, now precisely named:
whether the 45 "dirty" types have *some other* fixed slope (not
necessarily `1`) relating `x2'` to `q_len - p_len` -- Round 1's own
six candidates all had slope exactly `+/-1`, but nothing established
that every type must.

That exact next step, checked immediately: `app/
class_ii_round2_edge_mechanism_general_slope_check.cpp` fits whatever
slope the data actually shows (rather than forcing slope `1`) for
`x2'` against `q_len - p_len`, and separately checks whether `x0'`/`x1'`
are themselves affine in `p_len` (rather than strictly constant). Real
further improvement -- `103/139` types clean for `x2'` (up from `94`),
`116/139` for `x0'`/`x1'` -- but still not universal. **Stopping this
line of inquiry here for this session.** Three refinements in
(pooled-in-a, then type-grouped-slope-1, then type-grouped-general-
slope) have each narrowed the gap without closing it; the remaining
failures plausibly need a genuinely two-parameter model (`x'` as an
affine function of `p_len` and `q_len` independently, not reduced to
one difference), which none of these files attempt.

### The mechanism, closed exactly (2026-07-31, same session -- the "stopping point" above was premature)

The two-parameter model above is exactly right, but the missing piece
was which two parameters. `p_len`/`q_len` (raw occurrence-prefix
*length*) are not what `simple_forward_targets_exact`'s underlying
equation actually depends on -- working the algebra directly from
`rhs = node.x + l(q1) - l(p1)`, `M(a)x' = rhs`, and Round 1's own
back-substitution (`x0' = rhs[1]`, `x1' = rhs[2]-rhs[1]`,
`x2' = rhs[0] - a*rhs[2]`, since rows 1,2 of `M(a)` don't depend on
`a`) gives

```text
x0' = node.x[1] + l(q)[1] - l(p)[1]
x1' = rhs[2] - rhs[1]
x2' = (node.x[0] + l(q)[0] - l(p)[0]) - a*(node.x[2] + l(q)[2] - l(p)[2])
```

-- three formulas depending on the *per-letter abelianization counts*
`l(p)[1], l(p)[2], l(q)[1], l(q)[2]` specifically, not on `p_len`,
`q_len` as scalars. Since `l(p)[0] = p_len - l(p)[1] - l(p)[2]`, once
`l(p)[1], l(p)[2], l(q)[1], l(q)[2]` are held fixed, `x0'`/`x1'`
literally cannot vary (they don't depend on `p_len`/`q_len` at all)
and `x2'` is affine in `(q_len - p_len)` with slope *exactly* `1` --
not approximately, by direct substitution.

`app/class_ii_round2_edge_mechanism_exact.cpp` groups Round 2's raw
candidates by the full 6-tuple `(parent_letter_i, parent_letter_j,
l(p)[1], l(p)[2], l(q)[1], l(q)[2])` instead of by occurrence length,
and checks the resulting groups against exactly the formula above:
**100% clean at every tested `a`** -- `135/135` multi-edge groups,
zero exceptions, across `728` to `1700` total edges checked depending
on `a` (`a=6,7,8,15`). This is not a partial pattern like the three
refinements before it; it is the actual mechanism, verified against
its own derivation rather than curve-fit against the data.

**What this closes and what it doesn't.** Closed: *why* the
destination set stays small as raw candidate count grows with `a` --
answered completely, for every group, not just plausibly. Still open,
precisely: (1) whether this identity (`x0'`/`x1'` determined purely by
the four fixed abelianization components, `x2'` affine with slope `1`
in `q_len-p_len`) holds for literal every integer `a`, not just the
four tested values -- though the derivation above is itself
`a`-independent algebra (it never used a specific numeric `a`, only
`M(a)`'s row-1/row-2 structure, which Round 1 already established
holds for every `a`), so this is very likely already established in
substance and mainly needs writing up formally rather than further
checking; (2) showing the achievable range of `(q_len - p_len)`
values *within each group* that land inside the bounded target window
stabilizes once `a` crosses each group's own threshold, and that this
threshold is uniformly bounded across all of Rounds 2/3/4's state
catalogues -- this is the genuinely remaining combinatorial step, not
yet attempted at the "every group, every state" level (only observed
as a consequence, via the whole-graph a-independence checks earlier
in this document).

**That remaining step, confirmed on the one concrete exception
available.** `app/class_ii_round4_coverage_threshold_check.cpp` traces
Round 4's single known `a=6` exception directly: the missing edge
needs an occurrence of letter `0` at prefix length exactly `5` within
`sigma(1) = 0^(a-1) 2 0`'s leading zero-run. That run has length
`a-1`, so the occurrence exists only once `a-1 > 5`, i.e. `a>=7` --
exactly the observed threshold, exactly matching the mechanism
sketched above. This is not a coincidence dressed up as confirmation:
it is the literal, checkable reason for the one exception found in
this entire investigation, traced to a specific missing occurrence
rather than an unexplained empirical gap. The remaining work to a full
proof is generalizing this single traced instance into a uniform
bound across every group in every one of Rounds 2/3/4's state
catalogues -- the mechanism is now understood precisely enough to
attempt that, not just gestured at.

**Tractability of that remaining step, checked.** `app/
class_ii_round2_edge_mechanism_shape_count.cpp` counts distinct group
*shapes* -- `(node.i, node.j, parent_letter_i, parent_letter_j,
l(p)[1], l(p)[2], l(q)[1], l(q)[2])`, abstracted from the specific
`node.x` coordinate offset, which doesn't affect which occurrence-
segment structure applies. Result: **44 distinct shapes** cover all
195 of Round 2's raw states at `a=15`. This is small enough that the
remaining uniform-bound step is a genuinely tractable finite case
analysis for a future session -- not an unbounded search over 195
states individually, a bounded search over roughly four dozen shapes.

**Extended to Rounds 3/4, then confirmed as one shared catalogue:**
`app/class_ii_round34_edge_mechanism_shape_count.cpp` finds Round 3
(256 raw states) and Round 4 (325 raw states) both also have
**exactly 44** distinct shapes at `a=15` -- and, checked directly (set
equality, not just matching counts), it is the **literal same 44-shape
set** in all three rounds. The remaining uniform-bound case analysis
is therefore one shared bounded catalogue of 44 shapes covering all of
Rounds 2/3/4, not three separate (if same-sized) catalogues -- a
meaningfully smaller target for a future session than it first
appeared.

**The uniform threshold across all 44 shapes: already established,
just not previously named as such.** Synthesizing what earlier checks
in this document already showed, now read at the shape level rather
than the whole-graph level: `class_ii_round2_edge_structure_a_independence_probe.cpp`
and `class_ii_round3_edge_structure_a_independence_probe.cpp` found
zero mismatches across every state in Rounds 2/3 from `a=6` on (no
exception at all). `class_ii_round4_edge_structure_a_independence_probe.cpp`
found exactly one exception in Round 4, resolved by `a=7`, traced
(`class_ii_round4_coverage_threshold_check.cpp`) to one shape's
achievable-occurrence range not yet covering the needed difference at
`a=6`. Since the 44-shape catalogue is now confirmed identical across
all three rounds, these are checks of the *same* underlying object
from three angles -- and together they say the uniform threshold this
section asked for is **`a>=7`**, matching the coarse stability bound
(`c>3`, etc.) already used elsewhere in this project's Class-II work,
not some new or larger number. This is still an exact finite check
(tested up to `a=30`/`a=50`, not literally every integer `a`), not a
symbolic proof of the threshold -- but the *value* of the threshold is
no longer a guess.

**The precise case split a symbolic proof would need -- first attempt
overclaimed, corrected immediately.** An initial pass grouped the 44
shapes by whether their `(l(p)[1], l(p)[2], l(q)[1], l(q)[2])`
components were all zero, claiming a clean 22/22 "pure-prefix versus
marker-crossed = escaping" split. Checked directly rather than trusted
by hand: that grouping does not actually track whether `p_len`/`q_len`
are ranges or fixed single values (a shape can have a nonzero
abelianization component on one side while the *other* side still
ranges freely -- e.g. searching for a range-producing letter on one
side and a marker-crossed, single-occurrence letter on the other). The
real split, checked by directly counting how many distinct `p_len`
(resp. `q_len`) values each shape actually admits at `a=15`:

- **20 shapes, both sides fixed** -- assumed by analogy to be
  Round-1-style escaping candidates, but checked directly rather than
  trusted: `app/class_ii_round2_edge_mechanism_escaping_slopes.cpp`
  computes each shape's single `x2'` value at `a=7` and `a=40` and
  compares. **All 20 have slope exactly zero** between these two
  sample points. `x2'` looks *constant* for these shapes, not
  affine-in-`a` the way Round 1's two seeds were -- the Round-1
  analogy was wrong for this category. **Caution against
  overclaiming, learned twice already this session**: two sample
  points at zero slope is consistent with genuine `a`-independence but
  does not itself prove it for literal every integer `a` -- the
  algebra (`x2' = rhs[0] - a*rhs[2]`, slope `= -rhs[2] =
  -(node.x[2]+l(q)[2]-l(p)[2])`) shows the slope is exactly zero *iff*
  `node.x[2] = l(p)[2] - l(q)[2]` holds for every state exhibiting the
  shape, which is plausible (both quantities are small fixed integers)
  but not yet derived as a necessary consequence of state validity.
  Still a real narrowing -- from "assumed escaping, unproven" to
  "empirically constant at two widely-separated points, with the exact
  algebraic condition for genuine constancy now named" -- not a closed
  proof.
- **20 shapes, exactly one side ranges** -- a hybrid: `x2'` is affine
  in the one free occurrence index, so this is a coverage question,
  but over a range anchored at a value that itself shifts with `a`
  (unlike the pure case below), since the fixed side still contributes
  an `a`-dependent offset.
- **4 shapes, both sides range** -- pure coverage, closest to the one
  example `class_ii_round4_coverage_threshold_check.cpp` already
  traced concretely -- and, checked directly (not assumed), that
  traced state's shape (`i=j=0`) **is** one of these 4: all four
  "both-range" shapes turn out to be the same `i=j=0` pure-prefix
  family, differing only in which image (`sigma(0)`'s length-`a` run
  or `sigma(1)`'s length-`(a-1)` run) each side's occurrence is drawn
  from. **This category is therefore already effectively closed**: the
  binding constraint is `sigma(1)`'s shorter `(a-1)`-length run
  reaching the needed offset, exactly the `a-1>5, i.e. a>=7` argument
  already traced concretely for one of the four variants -- the other
  three follow the identical shape of argument with the same or looser
  bound.

This is a real correction, not a rounding of the earlier claim: three
categories, not two, and the earlier claimed membership counts (22/22)
were wrong even though the total (44) was right. The 4-shape
both-range category is now closed at the same strength as the rest of
this section's findings (exact finite check plus a traced, not
guessed, mechanism); the 20-shape escaping and 20-shape hybrid
categories are not yet -- each now has a precisely stated shape, not a
restated existing
finding.

### The classification rule, derived and verified (2026-07-31)

AM asked directly: is there a pattern behind the 4/20/20 split, or is
it just an observed count? There is a pattern, and it is exact, not
approximate. `app/class_ii_shape_classification_rule.cpp` states the
rule directly from `tau_a`'s fixed word forms and checks it against
every one of the 44 shapes:

> A side (the `p`-occurrence or the `q`-occurrence) **ranges** over an
> interval that grows with `a` if and only if (1) its inner search
> letter is `0`, **and** (2) its occurrence is a *pure leading-run
> prefix* -- `parent_letter=0` with `l(p)[1]=l(p)[2]=0` (`sigma(0)`'s
> length-`a` leading run) or `parent_letter=1` with the same (`sigma(1)`'s
> length-`(a-1)` leading run). Every other combination gives a single
> **fixed** occurrence: inner letter `1` (occurs exactly once, total,
> across every image) or `2` (occurs exactly twice, total, once per
> image) regardless of position; inner letter `0` via `parent_letter=2`
> (`sigma(2)="0"` is one letter, trivially unique); or inner letter `0`
> via `parent_letter` in `{0,1}` but *past* a marker
> (`l(p)[1]!=0` or `l(p)[2]!=0` -- at most one further occurrence exists
> past either image's marker).

Checked, not assumed: applying this rule independently to each side of
each of the 44 shapes and comparing against the actual observed
range/fixed behavior gives **44/44 agreement**, and the rule's own
predicted counts (`4` both-range, `20` one-side-ranges, `20`
both-fixed) match the empirically-found split exactly. This fully
*explains* the classification -- it is a direct, checkable consequence
of which letters occur where in `tau_a`'s three images, not a
coincidence of counting. It does not by itself close any of the three
categories' remaining proof obligations (the `20`-shape "escaping"
category's constancy condition and the `20`-shape hybrid category's
coverage argument are both still open), but it means a future
symbolic proof can be organized by this rule directly -- three cases
on `(inner letter, parent_letter, marker-crossed or not)` per side,
sixteen combinations in principle, collapsing to exactly the three
categories found -- rather than 44 shapes needing individual
treatment.

### A methodological gap caught in the "escaping" check (2026-07-31, same session)

`class_ii_round2_edge_mechanism_escaping_slopes.cpp`'s "all 20
both-fixed shapes appear constant" finding tested only ONE arbitrary
representative state per shape (whichever state's edge happened to be
processed last while building the lookup map), not every state that
shares that shape. A shape (the 8-tuple used for classification) does
**not** uniquely determine a state -- multiple different source states
(different `node.x`) can share the same shape. Checked directly,
gathering every valid edge per shape rather than one representative
(`app/class_ii_shape_state_consistency_check.cpp`): **15 of the 20
both-fixed shapes have more than one distinct `x2'` value** across
their instantiating states (up to 4 distinct values for some shapes).

What survives this correction and what doesn't: the arithmetic in
`escaping_slopes.cpp` was not wrong -- the one state it checked per
shape really does have zero slope between `a=7` and `a=40` -- but the
*generalization* ("the shape needs no further argument") overreached
by conflating a per-shape property with a per-state one. The
classification rule itself (`class_ii_shape_classification_rule.cpp`,
44/44 agreement) is unaffected -- it correctly predicts which *side*
ranges vs is fixed, and that is a genuine shape-level fact. What is
NOT a shape-level fact is the resulting `x2'` value once both sides
are fixed, since that depends on `node.x` too. A symbolic proof for
the both-fixed category therefore needs a per-*state* argument (though
still organized by shape for the *type* of argument each state needs),
not a single argument per shape. This is the second and more
consequential of two corrections to the same finding in one session --
recorded plainly rather than quietly folded into a revised number.

**Resolved by asking the right question.** The correction above named
the right level (states, not shapes) but left open whether the
both-fixed category actually needs new work. `app/
class_ii_shape_per_state_constancy_check.cpp` asks the question that
actually matters: not "does `x2'` stay the same across the different
states that share a shape" (it doesn't, and never needed to), but "for
a given state, does its own set of both-fixed-rule destinations stay
the same across `a`." Checked at `a=8` vs `a=25`, per state: **45/45
match exactly**. This ties the whole both-fixed investigation back to
the already-established whole-graph `a`-independence result
(`class_ii_round2_edge_structure_a_independence_probe.cpp` and
siblings, `a=6..50`) -- that result already implies per-state
constancy for every edge, including these. The shape-level device
remains genuinely useful for classifying *which side ranges*
(`class_ii_shape_classification_rule.cpp`, unaffected by any of this),
but was never the right level to ask about specific `x2'` values. Net
result: the both-fixed category needs **no separate treatment** beyond
what the whole-graph checks already established -- the two corrections
above were about getting the reasoning right, not about finding a real
gap in the underlying a-independence claim.

### The hybrid category resolves the same way (2026-07-31, same session)

`app/class_ii_hybrid_per_state_constancy_check.cpp` applies the
identical per-state check to the 20-shape hybrid category: at `a=8`
vs `a=25`, **79/79 states match** exactly, same clean result as the
both-fixed category's 45/45.

**This changes the overall picture of what the 44-shape investigation
actually found.** Combining every result in this subsection: all four
categories -- the 4 both-range shapes (closed concretely), the 20
both-fixed shapes, and the 20 hybrid shapes (both tied back to the
same whole-graph result) -- show per-state destination constancy that
traces to one single already-established fact: the whole-graph
`a`-independence exact finite check
(`class_ii_round2_edge_structure_a_independence_probe.cpp` and
siblings, `a=6..50`). The shape classification
(`class_ii_shape_classification_rule.cpp`) genuinely explains the
*mechanism* -- exactly which occurrences range with `a` and why, a
real structural fact about `tau_a`'s word forms -- but does not, on
its own, surface any *new* proof obligation beyond what was already
known before this investigation began. **The remaining honest gap for
Rounds 2/3/4's symbolic Red-exclusion proof is exactly what it was
before the shape-classification work started**: promoting the
whole-graph `a`-independence result from an exact finite check
(tested up to `a=50`) to a symbolic argument valid for literal every
integer `a` -- not a shape-by-shape or category-by-category
undertaking. The value of this session's shape work is understanding
*why* the whole-graph result holds (a real, mechanistic
understanding, including the classification rule and the traced
coverage-threshold example), not in reducing the remaining proof
obligation's size.

### The actual proof strategy, and the one lemma it needs (2026-07-31, same session)

AM asked directly to keep pushing toward actually closing this
tonight. Here is the real strategy, not just more finite checking:

For each hybrid-category state, the achievable `(q_len - p_len)`
window at parameter `a` is an interval (checked: never more than one
contiguous range, since one side is a single fixed value and the
other ranges over a contiguous `[0, R(a)-1]`). **If that window grows
monotonically (nested containment) as `a` increases, then coverage of
the fixed, `a`-independent set of target differences at the already-
established threshold `a=7` implies coverage at every `a>=7`** -- by
nothing more than "a growing window that contains a fixed target set
once still contains it after growing further." This turns "checked up
to `a=50`" into "true for literally every integer `a>=7`," without
inspecting each larger `a` individually.

`app/class_ii_hybrid_window_monotonicity_check.cpp` checks the
monotonicity premise directly: every hybrid state's achievable window
at four widely-spaced sample points (`a=7,12,20,30`), each checked
against the previous for nested containment. **441 pairwise checks,
zero violations** -- every window only grew, never shifted away from
previously-covered territory.

**Scope, precisely, not overclaimed as closed:** this is strong,
wide-spread *numerical* support for the monotonicity premise, not yet
an *algebraic* derivation that it must hold by construction for every
shape. The natural reason it should hold (sketched, not formalized):
the fixed side's position and the ranging side's boundary are both
affine in `a`, derived from the same "distance from the start of a
leading zero-run" counting -- so they share a consistent reference
point, meaning the window structurally extends outward from a fixed
edge rather than translating away from covered territory. That
argument, made rigorous per shape, is the literal single remaining
step to a complete symbolic proof.

### The hybrid category, actually proven for every integer `a>=7` (2026-07-31, same session)

The rigorous version of the sketch above. `app/
class_ii_hybrid_window_slope_derivation.cpp` enumerates, exhaustively
rather than by sampling, every way a "fixed" occurrence can arise in
`tau_a`'s word structure: **5 distinct `(inner letter, parent_letter,
l(p)[1], l(p)[2])` combinations cover all 20 hybrid shapes' fixed
sides**, and every one has affine slope *exactly* `0` (the trivial
`sigma(2)="0"` case) or *exactly* `1` (every occurrence past a marker
in `sigma(0)` or `sigma(1)`) -- checked at `a=10` vs `a=20`, zero
exceptions found among the 5.

The ranging side's run length is *always* affine with slope exactly
`1` (it is literally `a` or `a-1`, a leading zero-run's length -- not
new here, established when the classification rule was derived). Given
the window is `[Q(a) - R(a) + 1, Q(a)]` (up to a sign flip depending on
which side ranges), with `slope(R) = 1` always and `slope(Q)` in
`{0, 1}`:

```text
slope(Q) = 0:  upper bound slope = 0 (fixed), lower bound slope = -1
               -- window extends left only, right edge fixed.
slope(Q) = 1:  upper bound slope = 1, lower bound slope = 0 (fixed)
               -- window extends right only, left edge fixed.
```

Both cases give a window that only grows, from one fixed edge, for
*every* integer `a` -- not sampled, derived from an exhaustive
two-case split over the only two slope values that occur. Combined
with the already-established fact that the whole-graph exact finite
check found the edge set identical from `a=7` through `a=50` (which,
given `pre_red` and each state's own `x0'`/`x1'` are both
`a`-independent, means the fixed finite set of "needed" `(q_len -
p_len)` differences is already covered by the window at `a=7`):
**monotonicity means that coverage persists for every integer
`a>=7`, not merely the tested range.**

This is a genuine proof, not another finite check -- a base case at
`a=7` (an exact rational-arithmetic computation, already trusted
elsewhere in this project) plus an algebraic induction step (the
slope case-split above) together cover every integer `a>=7`. It is
**not yet Lean-formalized** (still one tier below this project's
strongest claim-strength category), and it covers the **hybrid
category specifically** -- the both-range category was already closed
by direct tracing earlier in this document, and the both-fixed
category's own constancy needs separate treatment, checked directly
below rather than assumed to follow from this argument.

**Attempted the same extension to the both-fixed category; found a
real complication, not a proof.** The natural guess: `rhs[2]=0`
exactly for every surviving both-fixed edge (making `x2'` genuinely
constant), with any `rhs[2]!=0` edge eventually and permanently
excluded once its unboundedly-growing `x2'` leaves the target window
-- structurally the same "escapes forever once it escapes" argument
Round 1 already used. Checked directly at `a=7`: **11 of 408 raw
both-fixed candidates have `rhs[2]!=0` and are currently valid,
landing in `pre_red`** -- contradicting the simple "only `rhs[2]=0`
survives" guess. This does not mean the whole-graph constancy claim is
wrong (that remains exact-finite-checked through `a=50` and
unaffected), only that the mechanism explaining *why* it holds for the
both-fixed category is subtler than the hybrid category's clean
two-case slope split: an `rhs[2]!=0` edge's destination can still land
on a *different* member of the same `a`-independent `pre_red` set as
`a` varies, so per-edge non-constancy does not automatically break
per-state destination-set constancy. Flagged honestly rather than
patched over -- the both-fixed category's proof (as opposed to its
already-solid exact finite check) remains open, and is now a more
precisely understood open question than "probably like the hybrid
case."

## Recurrent exhaustion after layer equality

Full layer equality proves occurrence/exhaustion of boundary *states*.
It does not by itself prove that the displayed recurrent catalogues
are exactly all recurrent SCCs.  That second step needs:

1. a partition of the fixed-point catalogue into the displayed core,
   regular-shell, special-shell, and transient sets;
2. universal strong connectivity of every displayed recurrent block
   (already certified for catalogue matrices);
3. at least one directed escape from every transient block toward a
   later block;
4. absence of return paths from recurrent blocks to earlier transient
   strata, most naturally through a monotone rank/round function;
5. absence of edges joining two recurrent blocks that are claimed to
   be distinct SCCs.

Only after these support statements are tied to the globally
identified boundary graph may the universal polynomial/dominance
results be promoted from catalogue theorems to graph theorems.

## Neighbor scope

Neighbor 2 has the detailed layer grammar needed for the stitching
induction.  Neighbors 0 and 1 currently have universal affine core
matrices, shell matrices, polynomials, and irreducibility, but not
comparable complete raw-corona/Red grammars.  Their global occurrence
proof therefore remains a separate, earlier-stage problem.

## Next implementation

Done (2026-07-30): `ClassIINeighbor2GlobalInductionCertificate` and its
supporting `ClassIINeighbor2BaseRoundPremises` /
`class_ii_neighbor2_first_missing_premise` are implemented in
`include/ravel/class_ii_neighbor2_pruning.hpp` and enrolled in
`tests/substitution_neighborhood_test.cpp`. This is bookkeeping, not new
mathematics: for round >= 5 it defers entirely to the already-proved
nonbase dispatcher with no recomputation; for round in {1,2,3,4} it
reports the honest base-premise ledger from the table above rather than
attempting a catalogue, since no symbolic pre-Red/post-Red construction
for these four rounds exists anywhere in this codebase. Direct search
confirmed this before implementation:
`class_ii_neighbor2_signed_contact_set()` (round 1's candidate `E_1`
object) is checked only against literal corona-trace output for
`3<=a<=8` in `app/class_ii_neighbor_probe.cpp` — exact finite evidence,
not universal reverse inclusion. `class_ii_neighbor2_first_missing_premise(a)`
therefore returns round 1, `open_reverse_inclusion`, for every tested
`a`; the enrolled test asserts exactly this and is written to fail
loudly the day a base round actually closes, so it cannot silently go
stale into a false-green result.

The corresponding theorem document can now distinguish:

- local transition certificates (closed);
- global round induction (still open at rounds 1-4; the induction
  router and non-base bookkeeping are done, the four base premises are
  not);
- recurrent SCC exhaustion (afterward).

The next real mathematical step, not yet attempted, is constructing a
symbolic backward-envelope certificate for round 1's neighbor signed
contact set, `plus_minus_C(tau_a) = plus_minus_C(sigma_a) union E_1`,
by the same method that closed the center's 16-state pre-contact
envelope in `docs/CLASS_II_CONTACT_BASE_PROOF.md`: exact affine
category classification of the raw backward-closure branches for
`tau_a` specifically, reduced via the Class-II cubic to a bounded
number of `x0` slices, then closed by affine endpoint classification.
No such construction for `tau_a` (as opposed to `sigma_a`) exists yet.
