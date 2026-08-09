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
