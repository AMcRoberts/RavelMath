# Request-driven corona surface projection

Status: implemented and regression-tested in the live `RavelMathPub` tree.

## Purpose

The historical boundary pipeline built every candidate in every corona layer,
then reduced the full graph and only afterward selected the SCC or quotient a
consumer needed.  That is faithful but unnecessarily expensive for local proof
requests.  In dimension six the cost appeared even earlier: the old
`search_D_cont(subst, 2)` scanned the complete `[-2,2]^d` box before Algorithm
2 began.

The new default treats the contact/corona construction as an exact queryable
surface.  A consumer supplies seeds and a predicate describing the image it
needs.  The corona layer generates only that finite image, records every
rejected boundary candidate, and refuses to call a capped traversal complete.

Legacy full materialization remains available for differential verification:

```text
RAVEL_CORONA_MODE=legacy
```

or, for the adjacent competitor driver:

```text
./out/adjacent_competitor_transport 5 --legacy-corona
```

The default is `projected`.

## Owning sources

- `include/ravel/d_cont_check.hpp`
  - exact face-projected `D_cont` enumeration;
  - legacy box enumeration;
  - `RAVEL_D_CONT_MODE` / shared `RAVEL_CORONA_MODE` switch.
- `include/ravel/corona.hpp`
  - local corona composition and inverse composition;
  - request-driven `CoronaSurface`;
  - streaming `c_corona_projected`;
  - `algorithm2_projected_trace` and its rejected-boundary evidence;
  - default/legacy corona execution mode.
- `include/ravel/graph_divisor.hpp`
  - remains the owner of Tarjan SCC extraction and recurrent-component tests.
- `include/ravel/corona_projection.hpp`
  - compatibility layer for projections of already materialized graphs and
    old serialized truth graphs; it is no longer the primary architecture.

## Exact face-projection theorem for `D_cont`

For the unit-cube face model, intersect the origin face `[0,i]` with the
translated face `[x,j]`.

When `i != j`, an intersection of dimension `d-2` forces

```text
x_i in {-1,0}
x_j in {0,1}
x_k = 0 for every k not in {i,j}.
```

When `i == j`, it forces

```text
x_i = 0
exactly one k != i has x_k in {-1,1}
every other coordinate is zero.
```

Therefore the geometric candidate set is polynomial in `d`; scanning all
`(2b+1)^d` lattice vectors is unnecessary.  Every generated pattern is still
replayed through `is_in_D_cont`, including the exact `Q(beta)` strip test and
the independent face-intersection routine.

On the sigma-1 regression, projected enumeration equals the legacy box search
node-for-node.  For n-bonacci dimension six it reduces the contact-candidate
stage from more than five minutes to 45 candidates immediately, followed by a
145-node exact backward closure.

## Request-relative closure theorem

`CoronaSurface<D>::project(request)` starts from the supplied seeds and closes
under any requested combination of:

- simple forward transitions;
- independently derived simple backward transitions;
- local forward corona composition;
- inverse corona decomposition.

For every generated candidate:

- accepted nodes are inserted and expanded once;
- rejected nodes are retained in `rejected_boundary`;
- hitting a node or expansion cap marks the certificate incomplete.

If the frontier empties without a cap, `complete()` certifies closure under the
requested operations **relative to the request predicate**.  It does not claim
that the predicate itself is globally exhaustive.

## Projected Algorithm 2

`algorithm2_projected_trace` applies the request predicate during corona
composition, before insertion and before Red.  It preserves:

- initial signed-contact nodes admitted by the predicate;
- fixed-contact or evolving-layer connector semantics;
- fast-rounded or exact-rational simple-edge arithmetic;
- complete Red pruning ranks;
- every candidate rejected by the request boundary.

With the predicate `true`, the projected trace agrees exactly with legacy
Algorithm 2 on the sigma-1 reference test.

For a theorem-specific image, completeness has two layers:

1. the consumer first derives a bidirectionally closed transition image around
   its seeds;
2. Algorithm 2 is then projected to all signed-contact grade-one nodes plus
   that image.

Because the requested image is closed under both forward and backward simple
transitions, omitted same-grade states cannot enter or leave its SCC.  Red
survival of the selected recurrent SCC is witnessed internally by its own
outgoing edges.  Rejected corona candidates remain available for audit.

## Adjacent competitor result

The production default now runs without a whole n=6 corona graph:

```text
n=4 -> 5
  4-state family -> 44-state family: PASS
  2-state family -> 18-state family: PASS

n=5 -> 6
  44-state family -> 221-state family: PASS
  18-state family -> 80-state family: PASS
```

For n=5 -> 6:

```text
larger request image: 17,884 nodes
projected Algorithm-2 scope: 832 nodes
selected recurrent SCC: 221 nodes

smaller request image: 3,420 nodes
projected Algorithm-2 scope: 410 nodes
selected recurrent SCC: 80 nodes
```

The role embedding, path substitution, injectivity, and complete branch-profile
checks all pass.  The longest substituted paths are 18 and 21 respectively.

## Switch contract

Default behavior:

```text
RAVEL_CORONA_MODE unset/projected/surface/lazy
RAVEL_D_CONT_MODE unset/projected/faces
```

Legacy behavior:

```text
RAVEL_CORONA_MODE=legacy
RAVEL_D_CONT_MODE=legacy
```

An explicit command-line switch overrides the environment in
`adjacent_competitor_transport`.

## Trust boundary

This machinery supplies exact finite graph evidence and closure certificates.
It does not itself create a new Lean theorem.  Lean theorem status is unchanged;
see `THEOREM_STATUS.md` and `LEAN_THEOREM_INVENTORY_2026-08-04.md`.
