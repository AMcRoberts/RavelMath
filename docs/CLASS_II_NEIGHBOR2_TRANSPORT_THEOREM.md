# Class-II neighbor-2 bounded-correction transport theorem

## Statement under construction

Let

```text
sigma_a:
  0 -> 0^a 1 2
  1 -> 0^a 2
  2 -> 0

tau_a:
  0 -> 0^a 1 2
  1 -> 0^(a-1) 2 0
  2 -> 0
```

for `a>=3`. These substitutions have the same incidence matrix and
therefore the same Perron direction and stepped-hyperplane geometry.
Write `B_r(a)` for the literature-correct fixed-contact corona layer
of `sigma_a`, and `T_r(a)` for that of `tau_a`.

The candidate transport theorem is

```text
T_r(a) = B_r(a) union E_r(a),
```

where `E_r(a)` is the bounded correction returned by
`class_ii_neighbor2_layer_extension(a,r)`:

- `E_1` is a fixed mirror-closed 22-state signed-contact correction;
- `E_2` is a fixed 25-state correction;
- for `3<=r<a-1`, `E_r` is a fixed 24-state set plus
  `(2,(-r,r,-1),0)`;
- `E_(a-1)` is a 26-state penultimate correction;
- for `r>=a`, `E_r` is the 30-state terminal correction.

The `a=3` collision is handled by the dispatcher: round two uses
`E_2`, and round three is already terminal.

## Evidence already stronger than a size law

The enrolled `class_ii_neighbor_probe` constructs both complete
fixed-contact traces independently for every `3<=a<=8`. It requires:

1. literal center-layer containment at every round;
2. literal equality of the set difference with `E_r(a)`;
3. literal equality with
   `class_ii_neighbor2_layer_candidate(a,r,B_r(a))`;
4. exact rational incidence solves for all Red edges.

The final consequence is the tested identity

```text
G_B(tau_a) = G_B(sigma_a)
             union class_ii_neighbor2_center_extension(a),
```

and hence the observed boundary law `20a+38`. This remains finite
evidence because stepped-hyperplane membership and the parametric
corona/Red induction have not both been proved universally.

## Lemma queue

The proof should be split at the actual logical interfaces.

### 1. Signed-contact base

Prove for every `a>=3` that the reduced contact set of `tau_a` has
signed form

```text
plus_minus_C(tau_a)
  = plus_minus_C(sigma_a) union E_1.
```

`class_ii_neighbor2_signed_contact_set` is the candidate object.
`E_1` is mirror closed, so it consists of eleven signed pairs.

**Progress (2026-07-31).** The *unsigned* half of this lemma is now
closed for `a>=3`: `tau_a`'s own raw backward-closure of its `D_cont`
seeds is proved (not just checked) to self-close to exactly 27 states
and Red-prune to exactly the 25-state `center_states union new_states`
(`new_states` being the unsigned half of `E_1`), universally, not at
sampled `a`. `class_ii_neighbor2_signed_contact_set()` is verified
*exactly* equal to the mirror-closure of that unsigned 25-survivor
set, which is definitionally
`plus_minus_C(sigma_a) union E_1` (mirror-closing `center_states`
gives `plus_minus_C(sigma_a)`'s 28 states; mirror-closing `new_states`
gives `E_1`'s 22). See
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s Round 1 sections
(dated 2026-07-31) for the full derivation, the two bugs and one
overclaim caught along the way, and the precise remaining gap: whether
this definitional identity is also the complete *closure* answer this
lemma means, as opposed to merely the correct construction, is not yet
resolved -- checked directly, mirroring `D_cont`'s seeds and rerunning
the closure does *not* reproduce the mirror of the unsigned survivors,
so the lemma's truth (if it is one) rests on the corona framework's
`same_letter_H` convention, not on closure symmetry.

### 2. Window validity

The center and neighbor use the same `H_sigma`. Lean now proves
`same_letter_H`'s open-strip inequality for every state in the fixed
24-state correction, uniformly for all `a>=3`. The proof derives the
coarse Perron bound `beta<2a` directly from the Class-II cubic, turns
that into `3<c` and `1/2<beta-c<1`, and discharges the finite endpoint
table by exact linear arithmetic. The only moving interior state has
height

```text
-r(beta-c)-1,
```

so its lower strip inequality is exactly

```text
r(beta-c)+1 < c.
```

This follows from the existing center window bound at `q=r+1`.
`lean/class_ii_neighbor2_extensions.lean` proves the required margin,
the conditional strip implication, affine-tip injectivity, and the
fixed 24-state table. Thus interior correction window validity is
closed. The two penultimate affine states and terminal affine sextet
were the endpoint window cases. They are now closed too. Lean first
reduces the six terminal roles and extra penultimate state to
`c-3<(a-2)(beta-c)` and `(a-2)(beta-c)+2<c`, then derives both strict
margins from the cubic. The useful cancellation is
`(beta-a-1)(beta^2+beta)=1`. The first penultimate state follows from
the same margins. Therefore window validity for the entire interior,
penultimate, and terminal correction catalogue is kernel-checked.

### 3. Corona propagation

Assume `T_r=B_r union E_r`. Enumerate the bounded contact-hop witness
table and prove

```text
C-Corona_tau(T_r) = candidate_pre_red_(r+1).
```

The only unbounded coordinate propagates by the fixed center hop
`[0,(-1,1,0),0]`:

```text
(2,(-(r-1),r-1,-1),0)
  + [0,(-1,1,0),0]
= (2,(-r,r,-1),0).
```

Lean already kernel-checks this affine identity. The other witnesses
belong to finite fixed catalogues.

The raw composition part is now executable without constructing a
growing word or graph.
`class_ii_neighbor2_interior_composition_witnesses(r)` searches only a
fixed eighteen-state center interface, the previous bounded
correction, and the fifty signed contact hops. It returns exactly
twenty-five witnesses in constant memory. The associated certificate
shows that twenty-four witnesses are literally fixed and the remaining
one is the affine tip plus `[0,(-1,1,0),0]`; checks through `r=128`
guard the implementation. Thus exact color matching and lattice
addition are closed universally by the displayed catalogues.

What remains in this lemma is no longer an unstructured corona
calculation: prove the eighteen interface states persist in the center
layer. All destination-side `same_letter_H` predicates for the
interior and endpoint correction catalogues are now kernel-checked.

**Base-round note (2026-07-31).** For the base rounds specifically
(round 2's own `T_1 -> T_2` step, distinct from the `r`-indexed stable
machinery above), `app/class_ii_round2_ccorona_stability.cpp` confirms
`T_1` for `tau_a`'s own construction literally *is* the fifty signed
contact hops referenced above (`T_1 = +/-C`, the same 50-state
`class_ii_neighbor2_signed_contact_set()` object, checked equal, not
assumed) -- and calling `c_corona(T_1, +/-C)` directly gives an
identical 195-candidate raw composition at `a=6,7,8,15`. This is a
checked starting point for round 2's own reverse-inclusion obligation,
not a proof of it; whether those 195 candidates' window validity
already falls under the kernel-checked cases above has not been
matched category-by-category. See
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s "Round 2" sections.

The center-interface occurrence problem now has a bounded raw-corona
object too. The eighteen interface states split into parameter-free
base rounds of sizes `8+6+4`. Six auxiliary proof states close the
witness table: thirteen round-one support states lie in the explicit
signed center contact set, seven round-two support targets have exact
color/lattice witnesses, and the four round-three interface targets
do likewise. The remaining base-support obligations are
the universal center contact-set base theorem and Red retention; raw
occurrence no longer requires a parameter sweep. Stepped-hyperplane
validity is now closed as well: Lean checks all twenty-four support
endpoints (the thirteen round-one sources, seven round-two targets,
and four round-three targets) from the cubic Perron-gap bounds.

The same bounded construction now reaches both endpoint transitions.
For `a>=5`, the interior-to-penultimate routine supplies all 26 raw
composition witnesses using the center interface, the center
`(a-2)` shell, and the previous correction. The
penultimate-to-terminal routine supplies all 30 witnesses using the
center `(a-1)` shell. Its affine certificate aligns catalogue roles
rather than incidental set order and proves 24 fixed plus six affine
target formulas from three exact parameters. The small `a=3,4`
collisions remain finite base cases.

### 4. Red survival and exclusion

Prove every node in `B_(r+1) union E_(r+1)` has a retained outgoing
edge, then prove every other corona candidate is removed by the Red
fixed point. Keep these as separate lemmas: survival does not imply
exhaustion.

Correction survival is now closed conditionally on center-layer
survival. Every state in `E_2`, every interior correction, both
penultimate affine states, and all six terminal affine states has an
explicit simple-forward successor in `B_r union E_r`. The fixed
twenty-four-state map closes through seven center anchors; the moving
tip has exactly one edge by symbolic prefix arithmetic; finite-crossing
certificates prove the fixed, penultimate, and terminal edge families
have positive multiplicity throughout their domains. The finite probe
checks literal source/target membership at every layer through `a=8`.
Round one is the signed-contact input and undergoes no Red step.

The center-interface support needed by the raw corona proof now has
its own unconditional bounded Red object modulo the center contact-set
base: a twelve-state round-two functional graph and a nine-state
round-three graph (plus the isolated `a=3` form). Exact positive
successor multiplicities put every support state on a cycle or a path
to one.

Exclusion remains open, but its literal candidate grammar is now
implemented. `red_trace` records the exact simultaneous sink ranks,
so a trace carries both the surviving graph and a partition of every
excluded pre-Red node. For stable interior round `r>=4`,
`class_ii_neighbor2_interior_pruning_ranks(r)` consists of a
`48r-8` first rank, a fixed 19-state second rank, and a fixed
9-state third rank. The first rank is generated by a 184-state
round-four seed, a four-state moving frontier, and one new 48-state
affine cell per subsequent round.

```text
48r-8, 19, 9.
```

The penultimate and terminal correction catalogues are explicit small
edits of the same grammar and have ranks `48r-15,25,9` and
`48r-31,41,11`. Literal equality with the computed pruning sets now
passes for every applicable layer through `a=8`; a wider diagnostic
sweep through `a=10` also found the entire interior rank sets
independent of the ambient parameter. These comparisons are still
bounded evidence. The universal work is to prove (i) the catalogue is
the whole fixed-contact corona difference and (ii) every listed rank
is exactly the sink set after the preceding ranks are removed.

The repeated terminal fixed-point round now has a literal six-rank
catalogue too:

```text
48a-5, 46, 22, 15, 10, 4.
```

It is an explicit affine edit of the terminal grammar and matches the
complete computed ranks through `a=10`. There is no remaining
catalogue-discovery step at the endpoint; the next step is the
universal corona-exhaustion and ranked-edge proof.

The first universal exclusion block is now closed. For every `a>=7`
and every pair of indexed parameters `5<=q,s<=a-2`, each of the 48
states in pruning cell `q` has zero simple-forward weight to every
state type that can occur in a stable pre-Red layer: the 68-state
center base, all twenty-state center shells, the fixed and moving
neighbor correction, the four-state pruning frontier, every pruning
cell, and the fixed pruning ranks. The certificate expands neither
substitution words nor graphs. It translates each possible
prefix-pair contribution into integer affine constraints and uses
exact Fourier--Motzkin elimination to prove all 14,032 systems
infeasible. Thus every growing 48-state cell belongs universally to
rank one.

The rest of the stable ranked-edge theorem is now closed too. Exact
affine infeasibility proves that the 180 fixed seed-core states and
four moving frontier states have zero outgoing weight to the complete
stable pre-Red grammar, so the whole `48r-8` first rank is universal.
The fixed 19-state second rank has no edge after rank one is removed;
the fixed 9-state third rank has no edge after ranks one and two are
removed. Explicit universally weight-one edges from every rank-two
state into rank one and every rank-three state into rank two prove
that they do not disappear earlier. Hence, conditional only on the
raw corona being exactly the proposed grammar, the stable
`48r-8,19,9` Red reduction is universal. The raw-corona premise is now
discharged below. Endpoint ranks remain.

The inclusion half of stable raw-corona equality is now universal
algebraically. The center layer itself has been promoted to an
explicit 68-state base plus twenty-state indexed shells. On the
neighbor side, all 304 fixed pre-Red target roles compose from 113
fixed previous-layer roles and 53 contact/identity hops. All 73
affine target roles compose coefficientwise from a fixed source or
the preceding shell/tip role. These are exact lattice/color
identities, not a parameter sweep. A bounded full-set reconstruction
through `a=32` guards the assembly.

The reverse inclusion is now universal too.
`class_ii_neighbor2_stable_window_exhaustion_certificate` enumerates
all 835 fixed and 353 affine source-plus-contact roles. It proves all
304 fixed and 73 affine displayed targets window-valid; classifies 125
affine compositions as shifted displayed families, including 160
finite lower-end instances; and proves the remaining 442 fixed and
155 affine roles universally outside the open stepped window. There
are no unresolved roles. The exact elimination uses only
`a>=7`, `a<c<a+1`, `2/3<d<1`, and
`c-3<e<c-2`, where `c=a+1/beta`, `d=beta-c`, and `e=(a-2)d`.
Lean now derives the new `d>2/3` bound from the cubic via the coarse
root estimate `beta<3a/2`; the other margins were already
kernel-checked. Stable raw-corona equality, and therefore the complete
stable `48r-8,19,9` Red theorem, is closed.

The penultimate ambient grammar introduces no new state set. A
partition-transport certificate checks that it is exactly the
hypothetical stable round-`a-1` grammar: six affine states move from
rank one to rank two and one affine state moves from rank one into the
correction survivor set. The remaining penultimate obligation is only
the changed rank-edge relation, not a second corona-exhaustion theorem.
That relation is now closed. Exact endpoint elimination covers the
new target slice at index `a-1`, including a one-variable integer
solver for a rational-relaxation false positive. All changed rank-two
and rank-three pair groups have zero unresolved branches, and six
affine weight-one predecessor edges prevent the promoted states from
vanishing in rank one. The exceptional `a=7` collision is checked
directly. Thus the full penultimate Red ranks
`48a-63,25,9` are universal for every `a>=7`.

### 5. Terminal fixed point

Prove the penultimate-to-terminal correction and then

```text
Red(C-Corona_tau(B_a union E_a)) = B_a union E_a.
```

Together with the center terminal theorem, this yields the universal
boundary cardinality `20a+38`.

Raw terminal inclusion, window validity, correction survival, and the
literal repeated-terminal deletion grammar are now closed as
catalogue objects. The genuine remaining terminal work is proving
that this grammar exhausts the corona and has the asserted ranked
no-out-edge relation for every parameter. That proof gives equality
after one further fixed-point round.

The correct terminal comparison object is now explicit. If the stable
shell grammar is fictitiously continued through `q=a`, the true
terminal pre-Red grammar differs by only two removed and eight added
affine roles. The bounded edit certificate regenerates these roles
across three consecutive parameters. This is a structural reduction,
not yet the universal terminal corona or ranked-edge theorem; the
independent interior-shell index must remain present in that proof.

### 6. Recurrent exhaustion and dominance

Identify the recurrent SCCs with the literal indexed catalogues.
Their transition matrices, characteristic polynomials, and
irreducibility are already universal. Once exhaustion is proved, the
Lean shell bounds and core sign comparisons complete the spectral
ordering, subject only to stating the standard Perron/characteristic-
polynomial bridge explicitly.

## Research significance

This is a first exact candidate transport law along an edge of the
same-incidence “shape” fiber. It is stronger than two affine size
formulas: it says one complete contact-boundary construction embeds
layerwise in its adjacent word-order neighbor, with a uniformly
bounded defect. If proved, it gives a concrete local notion of
connectedness between substitution families under the same spectral
light.
