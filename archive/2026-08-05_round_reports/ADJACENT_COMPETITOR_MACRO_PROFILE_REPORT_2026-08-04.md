# Adjacent-dimensional competitor macro-profile transport

## Target

Derive the competitor-family macro-edge profile embedding required by the
previously proved twisted Bellman transport theorem.

## Operation

`app/adjacent_competitor_transport.cpp` independently rebuilds the literal
corona graphs in dimensions `n` and `n+1`, extracts recurrent SCCs, identifies
the formula-defined predicted core, and treats all remaining recurrent SCCs as
competitor families.  A lower competitor state is transported by appending one
zero coordinate.  The operation then:

1. finds the unique upper recurrent competitor SCC containing every transported
   lower state;
2. replaces each lower edge by a shortest path between its transported
   endpoints inside that upper SCC;
3. derives the face-relative sign-role embedding from transported
   representatives;
4. proves that role transport is well-defined and injective;
5. reconstructs every lower outgoing branch profile and proves that its upper
   macro-edge profile is exactly the relabelled profile.

No spectral estimate or precomputed quotient is used.

## Exact n=4 -> n=5 replay

Both competitor families pass.

```
lower SCC 4 states -> upper SCC 44 states
lower roles 4 -> upper roles 44
lower edges 4
all 4 edges path-substituted
maximum path length 6
role embedding well-defined and injective
complete branch-profile preservation PASS

lower SCC 2 states -> upper SCC 18 states
lower roles 2 -> upper roles 18
lower edges 2
all 2 edges path-substituted
maximum path length 6
role embedding well-defined and injective
complete branch-profile preservation PASS
```

Thus the concrete competitor macro-profile premise required by
`derive_twisted_bellman_transport` is proved for `4 -> 5`.

## n=5 -> n=6 status

The same executable was run for `5 -> 6`, but the full literal n=6 corona/SCC
reconstruction did not complete within the twenty-minute command fence and
produced no comparison result.  This is an execution scalability obstruction,
not a failed transport witness.  The next engineering step is to feed the
operation the already persisted n=6 truth-machine graph or add a serialized
literal-graph cache, so adjacent transport does not rebuild the corona.

## Stable theorem boundary

The structural proof now has three separately replayed pieces:

- predicted-core twisted path/cocycle extension;
- generic twisted Bellman transport under exact macro-profile preservation;
- concrete competitor macro-profile preservation for `4 -> 5`.

The remaining universal seam is to derive the zero-append SCC-family inclusion
and bounded path substitution parametrically, then replay `5 -> 6` from the
cached literal truth graph.
