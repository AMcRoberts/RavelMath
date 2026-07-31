# Theorem and certificate status

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
`sorry`. The implementation is `include/spectre/return_contact_lift.hpp`.
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
now universal; global catalogue occurrence and recurrent exhaustion
remain open. Once those facts are established, disjointness
immediately gives an unbounded lower bound for the boundary family.

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

For the four non-base phases, C++ now exposes named pre-Red, ordered
rank, and post-Red catalogue dispatchers. A structural sweep over 429
instances (`7<=a<=32`) checks rank containment/disjointness and exact
reconstruction of each pre-Red catalogue from the post-Red catalogue
plus its ranks. Base rounds are rejected by construction rather than
filled with an unproved guess.

Lean kernel-checks the candidate shell bounds and core-polynomial
signs. Universal membership of all catalogue states in the reduced
boundary graph, and exhaustion of every other recurrent state, remain
open. The result therefore must not yet be stated as universal
contact-boundary dominance.

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
