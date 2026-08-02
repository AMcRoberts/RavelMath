# A family of families: shape, light, and near neighbors

This document turns the higher-dimensional projection metaphor into a
research organization. It is a program and vocabulary, not a theorem
that all unimodular Pisot substitutions lie in one connected moduli
space.

## Two levels that must not be conflated

For a primitive substitution `sigma`, the incidence matrix `M_sigma`
determines its Perron expansion and the stable/contracting splitting.
In the cut-and-project picture this is the **light**: the direction
along which the lattice is viewed and the linear contraction acting in
internal space.

The ordered words `sigma(j)` determine the prefix vectors used by the
graph-directed iterated function system. These digits determine the
placement and combinatorics of the Rauzy subtiles. This is the
**shape** presented to that light.

Consequently, substitutions with the same incidence matrix have the
same light but need not have the same window topology. The standard
example is Tribonacci versus flipped Tribonacci: the matrix agrees
while simple connectedness does not. Conversely, changing the matrix
usually changes both the light and the admissible prefix geometry.

This suggests a stratified object:

```text
matrix / number-field stratum (light)
└── word-order fiber over that matrix (shape)
    ├── language or conjugacy classes
    ├── coincidence and tiling chambers
    └── contact-boundary fingerprints
```

## A smallest useful neighborhood relation

`include/ravel/substitution_neighborhood.hpp` implements the first
exact edge relation: swap two adjacent unequal letters in one image.
This preserves every Parikh vector and hence preserves the incidence
matrix, characteristic polynomial, Perron root, and stable projection.
It changes only the ordered prefix data.

For the Class-II center

```text
0 -> 0^a 1 2
1 -> 0^a 2
2 -> 0
```

there are exactly three such neighbors for every `a>=1`: move the last
zero past `1`, reverse the terminal `12`, or move the last zero past
the terminal `2` in the second image. The enrolled
`substitution_neighborhood_test` checks this through `a=128` and
checks incidence equality exactly.

This adjacency is intentionally local and computationally cheap. It
does not claim dynamical equivalence. It supplies the edges on which
we can measure what changes.

## Quotients before comparison

Raw word edits overcount the meaningful space. Compare or quotient,
with explicit certificates, by:

1. simultaneous letter relabeling;
2. left/right substitution conjugacy and cyclic prefix transfer;
3. reversal or mirror when the question treats reflected windows as
   equivalent;
4. equality of generated language, which is stronger than equality
   of the incidence matrix;
5. return-substitution or phase-lift relations, recorded as maps rather
   than silently identifying differently presented systems.

Sellami's same-matrix intersection construction and the later results
on congruent Rauzy fractals make the language/conjugacy quotient
especially important: under the stated irreducible unimodular Pisot
hypotheses, equal languages give Rauzy fractals equal up to translation,
while equal matrices alone do not.

## Fingerprint at each vertex

The family graph should store a layered fingerprint rather than one
pass/fail label:

- incidence matrix, determinant, characteristic polynomial,
  discriminant, Perron root, and conjugate moduli;
- expanding and contracting eigenspaces;
- prefix-digit multiset and prefix automaton;
- primitivity, irreducibility, strong-coincidence status, and geometric
  property (F) status, including cutoff/inconclusive states;
- contact set, boundary size, recurrent SCC polynomials, and dominant
  spectral radius;
- balanced-pair core and any certified BP/contact correspondence;
- approximate window topology and symmetry, explicitly marked
  numerical until certified.

The first two rows describe the light. The remaining rows progressively
resolve the shape and its projected boundary.

## A family of presentations

There is another family besides the one obtained by varying
substitutions: the family of presentations of one underlying finite
local system. This is where the simple geometric picture meets the
abstract computational, dynamical, spectral, and arithmetic pictures.
A prototype translation chain is

\[
\text{window/lattice geometry}
\longrightarrow
\text{labelled transition system}
\longrightarrow
\{A_\rho\}_{\rho}
\longrightarrow
\{L_\rho\}_{\rho}
\longrightarrow
\text{vector geometry}.
\]

Here \(A_\rho\) is an adjacency or transfer operator twisted by a
character or representation \(\rho\), and \(L_\rho\) is a zeta,
Fredholm-determinant, or L-function-like spectral package. These arrows
are not automatically equivalences: each needs a ledger of what it
preserves and what it forgets.

For a processor-like example, address space is the base, the state or
contents at each address form a fiber, and instructions give labelled
transitions between fibers. A projected field placed on the addresses
supplies edge phases, weights, or transport maps--a discrete
connection. Products around instruction loops are then holonomies.
Twisting the transition operator by these data exposes the abstract
state relations to spectral methods.

An L-function alone does not generally recover ordinary geometry. It
typically retains cycle and eigenvalue data while forgetting
eigenvectors, localization, and sometimes labels; isospectral
non-isomorphic systems are the warning. A more faithful construction
keeps the whole twisted operator family, or localized
resolvents/Green functions, and forms diffusion, eigenvector, or
response coordinates. These turn states into vectors whose distances
represent a selected notion of dynamical or observational similarity.

| presentation | chiefly retains | may forget | natural wall |
|---|---|---|---|
| window/lattice | position, incidence, metric | internal state semantics | window face or cut crossing |
| labelled transition graph | reachability and instruction/state relations | ambient Euclidean metric | SCC or language change |
| twisted operators \(A_\rho\) | weighted paths, transport, loop holonomy | details invisible to the representations | rank or eigenspace change |
| \(L_\rho\), zeta, determinant | periodic-cycle and spectral data | eigenvectors, localization, some labels | zero/pole collision |
| spectral/vector embedding | selected dynamical similarities as geometry | information outside the chosen modes/probes | eigenspace collision or rank change |

Thus a family-of-families vertex can be refined from “a substitution
system” to
\((\text{system},\text{presentation},\text{probe})\). Edges may change
the system, or translate one system between presentations. The latter
edges are themselves a family of interest: a wall may look geometric
in one presentation, computational in another, and spectral in a
third.

The reconstruction question is:

> Which collection of probes makes the translation faithful enough to
> recover the finite local substrate, its labelled relations, or its
> geometric realization up to the equivalence we actually care about?

Equality of one untwisted L-function is weak evidence. Equality for a
sufficiently rich family of twists, together with localized response
data, is a plausible route to reconstruction. Finding the minimal
sufficient probe family is a concrete family-of-families problem.

## Walls and chambers

A “family” should mean a region on which a chosen fingerprint is
constant and has a transportable certificate. Potential walls include:

- a conjugate reaching modulus one (loss of Pisot contraction);
- determinant leaving `+/-1` (loss of the unit setting);
- a projected lattice or prefix point meeting a window boundary;
- a prefix collision or a new overlap/coincidence;
- appearance or disappearance of a contact edge;
- an SCC splitting, merging, or exchanging spectral dominance;
- failure or acquisition of strong coincidence or property (F);
- a change of **p-adic arithmetic stratum**, such as ramification
  index `e`, residue degree `f`, or automaton structure. This belongs
  in the broader fingerprint but is not a stepped-hyperplane wall:
  p-adic fields are unordered and do not decide real window signs.
  See `docs/STEPPED_HYPERPLANE_COMPARTMENTS_AND_WALLS.md`.

For a discrete word-order fiber, a wall is witnessed by an edge whose
endpoint fingerprints differ. For parametric matrix families, the wall
should be represented by an algebraic equality or certified root
interval, not inferred from a plot.

## Why connectedness is plausible but plural

There are at least three distinct notions of connectedness:

1. **combinatorial:** connected by incidence-preserving adjacent swaps;
2. **geometric:** joined by a continuous path of compact windows or
   Rauzy measures;
3. **dynamical:** joined while a property such as pure discrete
   spectrum remains invariant.

They must not be collapsed. Arnoux--Mizutani--Sellami prove continuous
dependence of generalized Rauzy fractals on an infinite sequence drawn
from a finite same-matrix substitution set. Gohlke--Mitchell--Rust--
Samuel provide a different interpolation: for random substitutions,
the Rauzy measure varies continuously with the probabilities, although
the support for nondegenerate probabilities is fixed by the random
substitution. These give rigorous forms of geometric interpolation.
They do not imply that coincidence, topology, or the contact graph is
constant along the interpolation.

Baake--Lenz show stability of pure point diffraction under a class of
equivariant local deformations of Delone dynamical systems. This is a
model for the kind of transport theorem worth seeking, but it is not
yet a theorem about the adjacent-swap graph defined here.

## First executable experiment

### Incidence columns already encode Perron-coordinate difference identities

One algebraic feature transports more broadly than the fixed-light fiber.
Let `v` be the left Perron covector used by the stepped-hyperplane
acceptance test, so `M^T v=beta v`. Subtracting coordinates `i,j` gives

```text
beta (v_i-v_j) = sum_k (M_ki-M_kj) v_k.
```

Thus any identity of the form "Perron coordinate times a coordinate
difference" is read directly from the corresponding incidence-column
difference. In the Class-II scaling `v=(b,c,1)` and `b=beta`, columns
0 and 1 differ by `(0,1,0)`, hence `b(b-c)=c`. At Tribonacci the same
acceptance-covector scaling gives difference `(0,1,-1)`, hence
`b(b-c)=c-1`.

This statement is not Pisot- or unimodularity-specific; those hypotheses
enter later, in the contracting geometry, lattice arithmetic, and window
positivity. `lean/perron_column_difference.lean` kernel-checks the arbitrary
finite-dimensional real-matrix theorem. The executable
`class_ii_identity_transport_probe` independently reduces the associated
quotient-ring products and requires exact agreement with the incidence
columns. This gives the family graph a useful division of labor: column
differences predict the linear identity at each matrix vertex; positivity
cones and ordered-prefix data decide whether that identity supports the same
geometric proof across an edge.

The first such positivity wall is already exact on the companion cubic
stratum. From the two coordinate eigen-equations, Lean proves
`A>=B>=1 -> b>c` and `B>=A+1 -> c>b`. The column-difference identity remains
valid on both sides; only its orientation reverses. Consequently the signed
`same_letter_H` strip is the transportable object, while an oriented
`0<height<width` certificate belongs to one chamber only.

Nor is the column rule confined to cubic strata. In the `n`-bonacci family,
consecutive columns differ by consecutive coordinate vectors, giving
`beta(v_i-v_{i+1})=v_{i+1}-v_{i+2}`. The familiar Tribonacci identity is the
terminal `c-1` case; Tetrabonacci continues with `c-d`. This supplies a
dimension-changing edge in the family-of-families map on which the same
algebraic mechanism transports literally, while the later window catalogue
and topology may still change.

That later finite window layer has now been crossed for the first three
nontrivial dimension steps. The explicit `item1_per_pair_check` target uses
exact `Q(beta)` arithmetic—not its retained floating diagnostics—to certify
every dominant-core face-pair maximum, its predicted sparse witness up to
signed mirror, and positive residual margin for
Tribonacci through n=7. In particular Tetrabonacci, Pentanacci, and
Hexanacci have core sizes 46, 108, and 210 and exact global worst margin
`2/beta-1 > 0`; Lean independently proves their coordinate order chains
through the first two nearest extensions. The finite n=3..7 certificate does
not by itself assert a uniform arbitrary-n core catalogue.

For each `2<=a<=A`:

1. take the Class-II center and its three fixed-light neighbors;
2. quotient obvious relabel/reversal/conjugacy duplicates;
3. run exact spectral classification once per matrix and reuse it for
   the whole fiber;
4. compute prefix, coincidence, property-(F), contact, SCC, and BP
   fingerprints with explicit resource caps;
5. label each edge by the first layer at which its endpoints differ.

The important efficiency rule is to cache matrix-level work above the
fiber. Same-matrix neighbors share the expensive algebraic root,
number-field, and projection calculations. Only ordered-prefix and
downstream graph objects need recomputation.

The first scientific question is modest and sharp: **which parts of
the exceptional Class-II affine-shell structure survive the three
smallest changes of shape under exactly the same light?** A stable
answer would identify a larger chamber. A failure would locate a
nearby wall and show which word-order feature the current theorem
actually uses.

## Family-of-families pilot (2026-07-28)

`tests/family_of_families_test.cpp` runs the adjacent-swap neighborhood
analysis across the `sigma_{a,b}` family at varying `(a, b)` and the
n-bonacci control.  The minimum-viable comparison:

- `sigma_{a,b}` for `a >= 1`: exactly **3 swap-neighbors**, regardless
  of `b` (tested for a ∈ {1, ..., 10} with b = 1 and a ∈ {1, ..., 6}
  with b = 2).  This matches the Class-II center claim that the
  fixed-light fiber has exactly 3 elements reachable by single
  adjacent swaps.
- `sigma_{0,1}`, `sigma_{0,2}`: 1 swap-neighbor each (no adjacent
  unequal pair in the leading image when `a=0`).
- n-bonacci for n ∈ {3, 4, 5, 6}: exactly **n-1 swap-neighbors**.
  So the count is linear in n: Tribonacci (n=3) gives 2,
  Tetrabonacci (n=4) gives 3, Pentabonacci (n=5) gives 4,
  Hexabonacci (n=6) gives 5.
- For every tested substitution, all swap-neighbors lie in the
  same Parikh-column fiber, confirming the substitution_neighborhood.hpp
  contract.

This is the minimum-viable fingerprint comparison.  Two structural
facts stand out:

1. **Class-II shape-invariant swap count**: σ_{a,1} and σ_{a,2} for
   a ≥ 1 all have exactly 3 swap-neighbors.  This is a "shape
   varies, light doesn't" structural invariant: the swap-graph
   density is uniform across the parametric Class-II family.
   A kernel-checkable theorem: `for all a >= 1, the swap-graph of
   sigma_{a,1} has exactly 3 nodes`.

2. **n-bonacci linear swap count**: the n-bonacci family has
   n-1 swap-neighbors.  This is a different parametric family
   with a different "swap-graph density" function of n.

These two findings suggest that the "shape varies" data in the
family graph (the swap-graph adjacency) is structured by the
incidence-matrix family index (a for Class-II, n for n-bonacci).
A larger pilot should explore which parts of the *finer* Class-II
fingerprints (contact size, dominant-core, recurrent SCC) are
also uniform in the family.

## Generalization to non-cube shapes (2026-07-28)

`math/include/math/pisot_numeration_topology.hpp` now exposes a
parameterized `bifix_code(r, max_len, alphabet_size = 3)`.  The
default `alphabet_size = 3` is the unit-cube case (alphabet
{0, 1, 2}, the standard Tribonacci / Class-II letters).  Setting
`alphabet_size = 4` (or larger) extends the bifix code to a
hypothetical k-letter Pisot substitution — for example, a
4-letter d=3 Pisot substitution with non-unit determinant (the
"hyperpyramid" case where the alphabet comes from a unit d-simplex
with d+1 vertices).

The Pisot number β is *unchanged* by the alphabet size (β comes
from the substitution's characteristic polynomial, not the tile
shape).  What changes with alphabet size:
- The bifix code (closure of the alphabet under concatenation
  up to length max_len) becomes the k-ary code for the corresponding
  polytope
- The "incident angle" in the higher dimension: a unit d-simplex
  has d(d+1)/2 distinct edge-slope classes (vs 2d for the unit
  cube).  For d=3, the cube has 6 edge slopes; the tetrahedron
  also has 6 edge slopes but with different geometry.  For d=4, the
  cube has 8 edge slopes; the 4-simplex has 10.
- The boundary of the projected generalized Rauzy fractal reflects
  the polytope's edge structure, not just the cube's.

The generalization to non-unit polytope shapes is the
"generalized Rauzy fractal" framework in the literature
(Minervino-Thuswaldner, Arnoux-Ito): the shape and the subdivision
rule together determine the structure, and the Pisot number is
fixed by the substitution.

### The acceptance-window half, done (2026-07-31)

`Substitution<d>` now carries `tile_faces` (`include/ravel/core.hpp`):
`d` face vectors `f_0..f_{d-1}`, one per letter, defaulting to the
standard basis `e_i` (so every existing caller is unaffected -- this
is a clean reparametrization of Eq 2.2, not a new geometric fact).
`in_H_sigma`/`in_H_sigma_exact` now test `0 <= <x,v> < <f_j,v>`
instead of the hardcoded `<x,v> < v[j]`; `set_tile_faces()` overrides
the default. `tests/tile_faces_test.cpp` checks this is bit-identical
to the old formula at every tested `(x,j)` for the default tile, and
that a genuinely non-cube tile changes acceptance in the expected
direction.

**What this does not yet cover, deliberately.** `faces.hpp`'s
face-intersection-dimension geometry (used by `search_D_cont`/
`is_in_D_cont` to find `D_cont`) is a *different*, unit-cube-specific
fact -- "face `[x,i]` extends by the unit interval `[x[k],x[k]+1)` in
every direction `k != i`" describes a cube face, not a
reparametrization of a formula that already covers arbitrary
polytopes. Hand-deriving the analogous fact for a general polytope
without literature grounding is exactly the mistake this project's own
`faces.py`/`faces.hpp` docstring already warns against. So
`search_D_cont`/`is_in_D_cont` now check `is_unit_cube_tile()` and
throw explicitly for any other tile, rather than silently computing a
wrong `D_cont`. Generalizing face-intersection to the simplex case
needs the actual Minervino-Thuswaldner/Arnoux-Ito construction read
properly first, not guessed.

## First pilot result

`app/class_ii_neighbor_probe.cpp` executes the first layer of that
experiment for `2<=a<=8`. Every run converged within the configured
pipeline bounds. The center is not locally constant under any of the
three edges:

| edit | observed contact size | observed boundary size |
|---|---:|---:|
| center | `14` | `20a+8` |
| move the last `0` past `1` in image 0 | `13` for `a=3..8` (`14` at `a=2`) | `42,65,86,106,126,146`; `20a-14` from `a=5` |
| reverse terminal `12` in image 0 | `14` | `20a-15` for `a=3..8` (`26` at `a=2`) |
| move the last `0` past `2` in image 1 | `25` | `20a+38` for `a=3..8` (`80` at `a=2`) |

These are finite computational observations, not extrapolated
theorems. They nevertheless answer the first structural question:
the center's exact contact and boundary fingerprints lie on a
word-order-sensitive ridge, not a locally constant same-matrix
chamber. More encouragingly, two neighboring rays immediately display
different affine boundary-size laws after the small `a=2` base case.
That is precisely the sort of higher organization the family graph was
meant to reveal: several nearby parametric families under one fixed
spectral light, separated by prefix/contact walls.

The next probe should replace the floating boundary eigenvalue column
with an exact certificate for *which* recurrent SCC is dominant and
attempt symbolic shell extraction.

The probe now computes the characteristic polynomial exactly after
selecting the dominant SCC numerically. For every tested `3<=a<=8`,
the three edits have respectively `a`, `a-1`, and `a+1` recurrent
components and dominant-core sizes `15`, `17`, and `39`. Their exact
coefficient data match the following interpolated formulas:

```text
P0(a,x) =
  x^15 -(a^2+2)x^13 +(2a^2+3a+2)x^11
       -(a^2+5a+3)x^9 +(2a+2)x^7

P1(a,x) =
  x^17 -(a^2+1)x^15 +a^2 x^13 -(a+2)x^12 +a x^10

P2(a,x) =
  x^39 -(a^2-1)x^37 -x^36 -(a^2+a+1)x^35
       -(2a^2+2a-1)x^34 -3a x^33 -(5a^2+2)x^32
       -(2a^2-3a+8)x^31 +(2a^2-3a+4)x^30
       +(4a^2-2a)x^29 -(3a^2+6a-5)x^28
       -(8a^2-3a+5)x^27 +(2a^2-2)x^26
       +(2a^2+2a-4)x^25.
```

`P1` also matches the exceptional `a=2` run; `P0` changes core size
there and `P2` does not follow the displayed formula there. At this
stage of the extraction, polynomial arithmetic was exact only for
each computed graph and the parameterized formulas were interpolation
conjectures. The finite-crossing and determinant-degree certificates
below subsequently promote them to universal catalogue identities.

The state-level extraction is now sharper. For every tested
`3<=a<=8`, each neighbor's selected dominant core has a literally
parameter-independent node set. The three explicit catalogues of
sizes `15,17,39` are library objects in
`include/ravel/class_ii_neighbor_family.hpp`, and the probe requires
freshly computed core-set equality.

With each catalogue sorted canonically, every adjacency entry tested
through `a=8` is affine in `a`:

```text
A_neighbor(a) = A_neighbor(3) + (a-3) D_neighbor.
```

The sparse supports contain respectively 41, 34, and 91 nonzero
affine entries. The probe constructs transitions independently from
the substitution at each parameter, verifies this matrix identity,
and verifies that its exact characteristic polynomial is `P0`, `P1`,
or `P2`. This was the finite derivation from which the universal
catalogue certificates below were constructed; complete-graph
dominance remains the separate open statement.

The sparse `(source,target,intercept,slope)` catalogues are now
explicit library objects. More importantly, the substitution images
have a bounded run representation: each image is at most four affine-
length constant-letter segments. `class_ii_neighbor_transition_weight`
intersects the corresponding prefix intervals exactly, without
expanding `0^a`, and therefore uses memory independent of `a`. It
agrees with independently expanded exact transitions for `3<=a<=8`
and with every sparse affine matrix through `a=128`.

The finite-crossing step is now certified exactly. Every branch in the
interval counter is controlled by a collected affine integer
expression. `class_ii_neighbor_affine_certificate` computes a point
strictly beyond every possible rational zero, exhausts the earlier
integer parameters, and checks two tail values; beyond that point both
the counter and candidate entry are affine, so two equal values force
identity on the whole tail. The three stable-tail bounds are
respectively `5,4,5`, requiring only `4,3,4` matrix evaluations.

Consequently the explicit 15-, 17-, and 39-state catalogue transition
matrices equal their sparse affine formulas for every integer `a>=3`.
This is a theorem about those displayed state catalogues, not yet a
theorem that they are the dominant recurrent cores of the complete
boundary graphs.

The displayed characteristic-polynomial formulas are now identities
as well. Since every matrix entry is affine in `a`, every coefficient
of `det(xI-A(a))` has degree at most the matrix size. Exact evaluation
at `n+1` distinct parameters therefore proves coefficient equality.
The certificate checks 16, 18, and 40 points for the matrices of sizes
15, 17, and 39 respectively. The only remaining universal claim in
this pilot is full-graph identification and spectral dominance.

The complete recurrent decompositions in the finite graph sweep also
have indexed shell spectra:

- neighbor 0 has two `x^2-1` components, then for
  `1<=t<=a-3` a ten-state component
  `x^6[x^4-((t+2)^2-2)x^2+1]`, then the 15-state core;
- neighbor 1 has, for `1<=t<=a-2`, a five-state component
  `x^3[x^2-t(t+2)]`, then the 17-state core;
- neighbor 2 has two `x^2-1` components and four-state components
  `x^2[x^2-k^2]` covering `2<=k<=a-1`, then the 39-state core.

`class_ii_neighbor_recurrent_component_states` is now the literal
indexed state family, and `class_ii_neighbor_probe` checks node-set,
matrix, and polynomial equality for every recurrent component in
`3<=a<=8`. The bounded prefix counter then goes beyond that sweep:
`class_ii_neighbor_regular_shell_certificate` partitions each
infinite `(parameter,slack)` cone into finitely many fixed-branch
cells. It proves the displayed 10-, 5-, and 4-state matrices and
polynomials for every admissible parameter. A separate
finite-crossing certificate proves all four exceptional two-state
swap matrices for every `a>=3`.

Thus the transition algebra of every proposed recurrent component is
universal. What remains finite is the complete-graph statement that
these literal catalogues occur and exhaust all recurrent states.

The third neighbor also has a newly exposed reduction. For every
tested `3<=a<=8`, its complete boundary node set is exactly

```text
center boundary graph union class_ii_neighbor2_center_extension(a).
```

The extension has thirty states: twenty-four fixed and six affine
terminal states. The union is literal, not an isomorphism inferred
from matching counts. A universal fixed-contact corona/Red induction
for this union would reduce the third-neighbor exhaustion theorem to
the center family plus a fixed-size correction.

The reduction persists at every tested corona layer, not only at the
fixed point. The neighbor layer literally contains the corresponding
center layer. Its set difference is:

- a 22-state initial signed-contact correction;
- a 25-state second-round correction;
- twenty-four fixed states plus the single affine tip
  `(2,-r,r,-1,0)` at every interior round;
- a 26-state penultimate correction;
- the thirty-state terminal correction above.

All five catalogue constructors are now explicit in
`class_ii_neighbor_family.hpp`, and the enrolled probe checks literal
layer-set equality for every `3<=a<=8`. The comparison trace uses
`CoronaEdgeArithmetic::exact_rational`, so its integral edge solves do
not depend on the rounded `solve_Mx_eq_rhs` shortcut. (The
stepped-hyperplane membership predicate is still the existing Perron
floating path; the Lean window lemmas are the route to removing that
last numerical boundary.) This is the right induction
object: the growing neighbor graph has become a known center layer
plus a bounded correction whose only interior motion is one affine
state. `lean/class_ii_neighbor2_extensions.lean` proves that the
terminal sextet has six distinct states, the penultimate pair has two,
the interior tips form an infinite injective family, and the moving
tip satisfies the signed-strip inequality under the center window
margin. A bounded raw-corona witness object now supplies all
twenty-five interior correction targets: twenty-four fixed
source/hop/target triples and the single affine tip propagation. It
uses only eighteen fixed center-interface states and constant memory.
The remaining propagation obligations are center-interface
membership and stepped-hyperplane validity; Red survival and exclusion
remain separate.

`lean/class_ii_neighbor_dominance.lean` kernel-checks the associated
candidate dominance inequalities. Neighbor 0's shell
root obeys `lambda+lambda^-1=t+2` and is below `a-1`, while its core
factor is negative at `a-1`. Neighbor 1's shells are below `a-1` and
its core factor is negative at `a`; neighbor 2's integer shell roots
are at most `a-1` and its core factor is negative at `a`.

Thus spectral comparison will be automatic once the indexed SCC
catalogues are proved to occur in and exhaust the full recurrent
graph. The Lean
file intentionally does not assert exhaustion or Perron-root
existence from a sign change; those logical bridges remain explicit.

## Low-priority singular-locus bridge

“Singularity” has two distinct precise meanings that may meet here.
For a cut-and-project scheme with star map `x -> x*`, window `W`, and
phason/window shift `t`, define

```text
Lambda_t = { pi_parallel(x) : x in L and x* + t in W }.
```

The model set is singular when some lattice image lies on the window
boundary:

```text
Sigma_W = { t : exists x in L, x* + t in boundary(W) }.
```

Crossing this locus changes point membership and is therefore a
literal candidate for a chamber wall in the family map. For a
polyhedral approximation it is a locally finite union of translated
faces/hyperplanes; for a Rauzy-fractal window it can inherit a fractal
boundary and should be represented by the contact graph or boundary
automaton rather than sampled pixels.

The 2025 plasmonic work uses optical phase singularities/topological
vortices. For a complex field `E`, a singularity is a zero
`E(r)=0`; an isolated zero has integer winding
`(1/(2*pi)) integral d arg(E)` around a small loop. The paper's
higher-dimensional charge vector retains several such integer charges
before their time-dependent two-dimensional projection. This is not
the same definition as a singular model set.

A possible bridge is to ask whether projection of a higher-dimensional
field creates or merges visible zeros precisely as its parameters
cross a discriminant, and whether the analogous cut-and-project
discriminant is `Sigma_W`. This is currently a low-priority analogy,
not a claimed relation between optical charge and contact-boundary
states.

## Literature boundary

The fixed-matrix fiber, balanced-block comparison, congruence under
language/conjugacy, and continuous same-matrix or random-substitution
interpolations come from the cited literature in
`refs/references.bib`. The shape/light terminology, stratified
fingerprint, adjacent-swap graph, and proposed Class-II pilot are this
project's research organization. They should be described as a
proposal, not as established moduli theory.
