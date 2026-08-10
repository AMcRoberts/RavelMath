# Property-F / sofic-extension bridge

This is the working contract for extending the finite-transport idea through
the family parameters (the analogue of extending the sofic construction
through `n` and `m`). It is a research contract, not yet a theorem.

## The common object

For a parameterized substitution `sigma_(n,m)`, construct one exact transport
object with these projections:

```text
base prefix automaton
        |
        +-- finite cocycle / return-phase lift
        |
        +-- exact Q(beta) displacement lift (Property-F graph)
        |
        +-- contact-boundary transport quotient
```

The projections must use the same prefix label. A contact edge and a prefix
edge are not allowed to be compared merely by numerical displacement.

## Certificate contract

Each `(n,m)` instance records:

1. the exact incidence matrix, reduced characteristic polynomial, and trusted
   p-adic bound;
2. strong-coincidence closure and its depth;
3. Property-F closure status and node/depth growth profile;
4. boundary-edge count, retained or elided explicitly;
5. recurrent-component classification: zero-only, mixed, or nonzero;
6. the finite return/cocycle lift and its SCC/holonomy profile;
7. the contact-to-adelic transport map and whether it preserves recurrence.

The scalar node count is diagnostic only. The theorem-facing invariant is:

```text
finite transport closure
and every recurrent component is zero-only.
```

## Parameter extension obligations

To extend through all `n` and `m`, the proof campaign has five reusable
obligations:

1. **Generator:** derive the substitution and prefix labels from `(n,m)`;
2. **Closure:** prove a uniform adelic bound or a well-founded rank for the
   nonzero displacement transport;
3. **Recurrence:** prove that the rank forbids mixed/nonzero SCCs, leaving only
   the allowed zero sector;
4. **Bridge:** show every contact transition has a transport witness and every
   recurrent contact cycle maps into the zero sector;
5. **Coincidence:** discharge strong coincidence by the existing generic
   closure engine.

The first three are the Property-F companion to the sofic extension. The
fourth is the actual bridge between the algebraic and geometric sides.

## General map: one skew product, three quotients

The sofic and transport corollaries are instances of one object. Let `B` be
the finite prefix/renewal base, `P` its finite phase lift, and `F` the carry
fiber (integer coefficient lattice in the unit case, adelic coefficient
lattice in the non-unit case). A transition has the form

```text
(b, p, gamma)  ->  (b', p', A gamma + delta(b,p,b'))
```

The sofic extension forgets `gamma`; the transport extension forgets `p`; the
Property-F graph retains both. The reusable proof map is:

1. prove the base language finite/regular;
2. prove the fiber cocycle has a finite quotient or a well-founded rank;
3. classify terminal fiber action as permutation/translation;
4. show every recurrent component projects to the zero sector.

Dimension adds a graded descent coordinate to `P`, multiplicity adds carry
colors to `F`, and a non-unit determinant adds valuation coordinates to `F`.
Non-unitness therefore changes the fiber, not the base or the recurrence
argument. The universal remaining lemma is a finite-cocycle/holonomy statement:
a recurrent loop with nonzero fiber holonomy must survive every quotient, while
Condition F makes the admissible zero-language quotient finite and terminal
fiber translations harmless permutations.

The implementation pairing is now clear as well. `check_property_f` already
has the exact step `gamma' = beta^(-1)(gamma + delta(prefix))`; in the unit case
`gamma` is the complete fiber. In the non-unit case the same step must be read
componentwise as

```text
(gamma_infty, (gamma_p)_p) ->
(A_infty gamma_infty + delta_infty,
 (A_p gamma_p + delta_p)_p),   p | (beta).
```

`make_combined_padic_bound` supplies the product-of-local-integrality predicate
for these extra coordinates. The missing explicit object is therefore not a
new theorem about the base: it is a typed local-fiber wrapper exposing the
same prefix label and cocycle simultaneously at every prime ideal.
That wrapper is now `adelic_cocycle_extension.hpp`: its unit specialization
has an empty local fiber, while its non-unit specialization requires a valid
prime-ideal descriptor for every local coordinate and verifies that the sofic
and transport forgetful projections commute.
It also factors the determinant and requires local coverage over every
rational prime in its support before accepting the non-unit specialization.
`derive_adelic_cocycle_extension_from_charpoly` now performs that wiring
against Dedekind factorization and constructs the combined local integrality
predicate; the non-unit regression uses the quadratic witness
`x^2-2x-2` at its prime `2`.

The first widening trunk is the monotone coefficient cone
`a_(d-1) >= ... >= a_0 > 0`. The certificate in
`monotone_coefficient_cone.hpp` simultaneously records the Condition-F
criterion, the Perron companion hypothesis, and the determinant split:
`a_0=1` is the empty-local-fiber unit case, while `a_0>1` is the non-unit case
whose prime divisors seed the valuation fibers. The generalized family is the
ray `(m,...,m,1)` inside this cone.

The current theorem boundary is precise. For every coefficient vector in this
cone, the beta-numeration Condition-F statement is uniform, including
non-units (`a_0>1`); the adelic representation has exactly the local fibers
supplied by the prime ideals above `a_0`. Beyond the cone, the missing theorem
is not Condition F itself, but proving that an arbitrary substitution's
contact cocycle is cohomologous to this canonical adelic extension and carries
no surviving nonzero holonomy.

## Canonical non-unit theorem route

The canonical part of that boundary is now executable in
`ravel/proof/canonical_nonunit_property_f.hpp`. For a terminating greedy
expansion whose digit vector is positive and nonincreasing, it checks:

1. the Frougny--Solomyak monotone Condition-F premise;
2. construction of the canonical Parry substitution and exact incidence
   characteristic polynomial identity;
3. Dedekind maximal-order factorization at every prime in the determinant;
4. the finite parent-role zero-kernel plus opposite-unit-cycle transport
   scheme;
5. the combined adelic bound and finite zero-expansion graph; and
6. absence of nonzero recurrent components.

This is the finite-expansion form of the paper's Proposition 9.4: the exact
graph closure is the unique-finite-expansion check in the non-unit
representation space. The quadratic canonical family `x^2-qx-q`, tested at
`q=2,3`, closes with 9 and 13 graph nodes respectively, and its parent-role
integer-transport scheme closes at the same finite cap. The route is a
theorem for this canonical monotone class; it deliberately does not promote
an arbitrary substitution with the same spectrum until the contact cocycle
cohomology/holonomy bridge is proved.

## Current evidence sheet

The fixed-spectrum fourth-generator family has nine exact instances. All close
strong coincidence and Property F, with no mixed or nonzero SCC. The broader
four-letter non-unit sample has 20 established cases and five remaining
inconclusives after boundary-sink elision. Focused profiles show:

- `rndW3_21`: closes at 6 nodes;
- `rndW3_25`: 200,002 interior nodes by depth 8, no partial nonzero back-edge;
- `rndW3_26`: 200,003 nodes by depth 52, 690,430 boundary edges, no partial
  nonzero back-edge.

These cases are not yet a uniform proof. They identify the missing theorem:
the transient transport growth must be separated from recurrence by a rank or
parameter-uniform adelic closure argument.

## Implementation order

1. Generalize the current family artifact to accept a parameterized neighbor
   enumerator and emit the certificate contract above.
2. Add a partial-cycle/rank probe for each unresolved `(n,m)` instance.
3. Build the contact-to-adelic edge simulation for one closed representative.
4. Promote that simulation to a parameterized transport lemma.
5. Only then attempt the all-`n,m` theorem and reflective certificate.

## First bridge result

The first representative edge-label bridge is now executable in
`property_f_contact_transport_bridge.hpp`. On the `x^3-2x^2-2` witness it
replays 1,215 contact-boundary edges and 3 distinct contact prefixes against
the exact prefix automaton. Every contact prefix is present in the adelic
label alphabet and its digit is recomputed in `Q(beta)`. This is label
compatibility, not yet a full state simulation; the next obligation is to
show that the contact edge map commutes with the adelic transport transition.
The certificate now discharges that local transition check too: all 1,215
edges replay as two prefix-automaton parent transitions, with five distinct
exact paired displacement labels. The same transition check now passes across
the first witness's radius-0/1/2 adjacent-ordering neighborhood: 1,215,
1,322, and 1,348 contact edges respectively, with 5, 7, and 9 paired labels.

The parent-role side is no longer canonical-only: the generic factory
`derive_parent_role_catalogue_from_substitution` builds the same labelled
forest directly from an arbitrary substitution. Its finite zero-kernel plus
opposite-unit-cycle transport scheme passes on all three radius-0/1/2
non-unit neighbors. This is the correct finite cohomology interface; it does
not yet make the bounded word cap uniform in the substitution family.

The exact `Q(beta)` refinement exposed an important correction to the proposed
uniform theorem. On the `first_100` ordering, the full digit-difference role
cocycle misses 54 of 81 ordered zero-kernel pairs even at word cap 64, while
the adelic Property-F graph still closes and has no nonzero recurrent SCC.
Thus arbitrary contact cocycles are not literally cohomologous to the
canonical pair-role cocycle. The final theorem must use a quotient/recurrence
invariant weaker than global zero-kernel equivalence; the failed stronger
hypothesis is now an executable negative regression, not an assumption.

## Generalized-multinacci sheet (2026-08-09)

The first shared parameter sheet is now executable for every concrete point
`(d,m)` with `d in {2,3,4}` and `1 <= m <= 6`. The test
`generalized_multinacci_property_f_transport_test` runs the existing symbolic
sofic scheduler certificate and the exact adelic Property-F transport on the
same instance. All 18 points close strong coincidence and Property F; every
recurrent component is zero-only, with no partial nonzero back-edge.

| d | m range | Property-F nodes (m=1 .. 6) | max observed depth |
|---|---|---|---:|
| 2 | 1..6 | 5, 4, 4, 4, 4, 4 | 2 |
| 3 | 1..6 | 33, 43, 54, 102, 229, 395 | 7 |
| 4 | 1..6 | 350, 912, 2,761, 7,386, 16,895, 33,837 | 17 |

The sofic side simultaneously certifies the uniform scheduler
`(m+1)Q + sum_{r=1}^m 2(m+1-r)R^r` and return roof `<= m`. This does not
yet prove the all-`d,m` adelic bound, but it establishes the intended common
contract on a nontrivial rectangle and gives a concrete growth law to attack:
for fixed `m`, transport size grows with `d`, while for fixed `d=4` it grows
rapidly but remains finite and cycle-safe.

## Cross-dimension transport law

The sofic and affine sides cross dimensions in different ways. The sofic
primitive grammar is a dimension-free renewal suspension: `m` determines the
prefix-phase scheduler, while changing `d` only deletes concrete parent or
boundary edges from the universal `Q/R^k` schema. The affine transport has a
graded tower instead. Its return blocks are indexed by `(height b, initial
carry k)`, with `0 <= b < d-1` and `0 <= k < m`, plus one terminal block.

- A `d -> d+1` lift preserves every old block, reinterprets the old terminal
  as the new zero-carry block at height `d-1`, and adds exactly `m` channels.
- An `m -> m+1` lift preserves every old block after relabelling each forced
  tail `m` as `m+1`, then adds one new carry color at each of the `d-1`
  nonterminal heights.

`generalized_multinacci_cross_dimension_test` checks these laws on 42
dimension lifts and 35 multiplicity lifts. This is the transport analogue of
the sofic cross-dimension suspension: the two axes are height and carry color,
not repeated copies of one fixed graph. The combinatorial lift does not by
itself transfer Property-F closure, because the beta-inverse coefficient
matrix changes with both parameters; it does transfer the finite return
language and the exact obligations that any rank proof must satisfy.

The exact affine reduction is now a reusable certificate in
`generalized_multinacci_affine_transport.hpp`: every prefix digit is
`delta(0^k)=k*beta`, so every Property-F step is
`gamma' = beta^(-1)*gamma + k`, with `0 <= k <= m`. Thus `m` changes the
carry alphabet, while `d` changes the reduction relation for `beta^(-1)`;
this separates the two parameter directions for the eventual rank proof.
The return-to-zero block certificate now makes the next quotient explicit:
there are exactly `(d-1)m+1` excursion channels, of lengths at most `d`;
each word is `[k,m,...,m]`, with one initial `k<m` (plus the terminal
`[0,m,...,m]` channel). Any future rank argument can therefore work on this
finite block alphabet instead of the raw zero-expansion graph.
The bridge test now replays every one of those channels against the concrete
prefix automaton, including the terminal-image exception; this is an exact
implementation cross-check for the block projection.
The companion `generalized_multinacci_block_affine.hpp` now turns each block
into an exact integer affine endpoint map on the `d`-coordinate coefficient
lattice. Its linear part is the appropriate power of
`beta^(-1) = beta^(d-1) - m(1+...+beta^(d-2))`, and its offset is the carried
initial digit. The affine test checks map shapes across the 42-point structural
sweep and replays all 19 `(4,6)` maps against exact `Q(beta)` arithmetic. This
is the concrete object
for the remaining rank proof: bound the reachable coefficient vectors under
these finitely many endpoint maps, rather than enumerate the raw graph.

There is now a theorem-level closure route for the whole generalized family.
The generalized-multinacci polynomial is a Pisot numeration polynomial with
finite carry alphabet bounded by `m`; therefore the CSY finite-alphabet
zero-language theorem supplies a finite Condition-F branching quotient. The
terminal transport fibres are permutation lifts, so quotienting them preserves
path growth. The executable certificate is
`derive_generalized_multinacci_branching_terminal_reduction(d,m)`; its scope is
the cited Condition-F/CSY theorem application, not a claim that every raw
bounded BFS must be small.

The Pisot premise follows from the primitive nonnegative companion matrix:
Perron-Frobenius makes the positive root the unique dominant root. The
polynomial is `x^d-m(x+...+x^(d-1))-1`, so its coefficient sequence is
`(m,...,m,1)`. The Frougny-Solomyak sufficient criterion
`a_(d-1) >= ... >= a_0 > 0` therefore applies uniformly. The constant term is
`-1`, so every member of this generalized family is unimodular, including
`m>1`.

## Why the long cases are long

The observed size is explained by three multiplicative effects, not by a new
recurrent holonomy: the block alphabet has `(d-1)m+1` channels, each state
lives in a `d`-coordinate power basis, and the secondary bound is inflated by
`1/(1-rho)` when the weakest conjugate modulus `rho` approaches one. For
reference, the exact root/bound diagnostics are:

| instance | `rho` | `1/(1-rho)` | observed result |
|---|---:|---:|---:|
| `(4,6)` | 0.8508 | 6.70 | 33,837 nodes, closed |
| `(5,3)` | 0.8757 | 8.04 | 792,212 nodes, closed |
| `(5,4)` | 0.8958 | 9.59 | 1,201,687 nodes, closed |
| `(5,5)` | 0.9118 | 11.34 | >2,000,000 nodes, inconclusive |
| `(6,1)` | 0.9062 | 10.66 | 188,545 nodes, closed |
| `(6,2)` | 0.9094 | 11.04 | 4,914,771 nodes, closed |
| `(6,3)` | 0.9260 | 13.52 | profile only; exact BFS deferred |
| `(7,2)` | 0.9396 | 16.55 | profile only; exact BFS deferred |
| `(8,1)` | 0.9472 | 18.93 | profile only; exact BFS deferred |

This predicts “large but harmless” transient growth: increasing `d` widens
the coefficient state space and weakens contraction, while increasing `m`
widens the carry alphabet. The current data show no nonzero recurrent signal;
the missing proof is a reachability rank on the block forest, not a search for
an additional cycle generator.
An optimized endpoint-only block experiment did not materially collapse the
large `(6,2)` search before the same practical limit. This is informative:
forced descent accounts for path depth, but the hard part is the cardinality
of distinct reachable coefficient vectors themselves.

An opt-in extended regression pushes beyond that rectangle
(`generalized_multinacci_property_f_transport_extended_test`). It closes
`(5,1)`, `(5,2)`, `(5,3)`, `(5,4)`, and `(6,1)` at 8,307, 67,007, 792,212,
1,201,687, and 188,545 nodes respectively. A larger exact run closes
`(6,2)` at 4,914,771 nodes (9,802,230 boundary edges), taking 12:56 and
peaking at 4.91 GB RSS. Thus the previously inconclusive 500,009-node probe
was a resource-bound observation, not an obstruction: this instance is a
mid-single-million, finite transport cloud. The large closed runs remain
expensive transient transport, not evidence of a recurrent obstruction.

The next fixed-degree point `(5,5)` reaches 2,000,001 nodes and 8,710,307
boundary edges at 6:29, using 1.67 GB RSS, and is intentionally left
inconclusive. The `generalized_multinacci_growth_profile_test` supplies the
cheap structural continuation of the sheet: it sweeps all 42 points with
`2 <= d <= 8` and `1 <= m <= 6`, checking the exact channel formula
`(d-1)m+1`, coefficient dimension `d`, and the Pisot condition `rho < 1`.
For the profile-only rows above this is scale evidence, not a Property-F
closure claim. The factors rise to 13.52 at `(6,3)`, 16.55 at `(7,2)`, and
18.93 at `(8,1)`, so an unstructured exact BFS would likely exceed the 10 GB
working envelope; the rank/cardinality proof is the correct next step there.
The companion `generalized_multinacci_structural_sweep_test` checks the other
finite obligations on the same 42 points: strong coincidence, the symbolic
general-`m` scheduler, and the return-block schema all pass through
`d=8,m=6`. Thus the profile-only rows are not extrapolated substitutions; they
are structurally valid members of the same sofic/transport family, with only
the expensive Property-F closure enumeration deferred.
The profile also exposes the forward-contraction carry-radius proxy
`m*rho/(1-rho)`: it is the radius outside which multiplication by `beta` in
the secondary embeddings dominates one carried digit. This separates the
contraction threshold from the subsequent count of admissible coefficient
vectors.
The profile now converts these embedding bounds through the inverse Vandermonde
matrix into a conservative coefficient-box proxy. Its base-10 box sizes are
about `10^6.82` for `(4,6)`, `10^9.59` for `(6,2)`, and `10^10.94` for `(6,3)`;
the exact reachable sets (33,837 and 4,914,771 in the first two cases) are
much sparser than those boxes. This is not a proof-quality count—the roots
and matrix inversion are floating-point diagnostics—but it gives the correct
resource scale and explains why the next dimensions should be ranked rather
than exhaustively enumerated.
The radius is not itself a state-count predictor: `(4,6)` has proxy radius
about 34.2 but only 33,837 closed states, whereas `(6,2)` has radius about
20.1 and 4,914,771 closed states. The latter is larger because the
six-coordinate lattice and its 11 return channels generate a much denser set
of distinct admissible vectors inside the contraction core. This is why the
uniform theorem must bound lattice cardinality/branching, not just the norm
radius.
There is a second reason no one-dimensional “contraction rank” is visible in
the coefficient coordinates: the constant term of the defining polynomial is
`-1`, so `beta` is a unit and every endpoint linear part is unimodular. The
maps preserve lattice volume; only the secondary embeddings contract in the
forward direction. The finite set is therefore created by admissibility and
boundary pruning, not by an ordinary contracting lattice map.
