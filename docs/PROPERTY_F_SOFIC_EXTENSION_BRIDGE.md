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
| `(6,1)` | 0.9062 | 10.66 | 188,545 nodes, closed |
| `(6,2)` | 0.9094 | 11.04 | >2,000,000 nodes, inconclusive |

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
`(5,1)`, `(5,2)`, `(5,3)`, and `(6,1)` at 8,307, 67,007, 792,212, and
188,545 nodes respectively. The `(6,2)` search reaches 500,009 nodes at
depth 14 without a partial nonzero back-edge; it is deliberately recorded as
inconclusive rather than promoted to a closure claim. The largest closed run
uses about 687 MB RSS and 1:57, so the observed growth is expensive transient
transport, not evidence of a recurrent obstruction.
