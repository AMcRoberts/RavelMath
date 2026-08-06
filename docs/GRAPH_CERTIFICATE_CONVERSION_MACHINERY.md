# Graph certificate conversion machinery

This layer converts validated computational graph structure into the exact typed
premise consumed by Lean's matrix and characteristic-polynomial theorems.

The conversion is intentionally split across the trust boundary.

## Native certificate side

`graph_certificate_conversion.hpp` derives and independently verifies:

1. a bijection from the certificate's canonical vertex order;
2. both inverse laws for that order;
3. the simultaneously reindexed adjacency matrix;
4. the expected feeder/core block matrix;
5. entrywise equality of those matrices.

The resulting artifact records the premise shape

```text
Matrix.reindex e e adjacency = Matrix.fromBlocks 0 feeder 0 core
```

but does not declare it kernel checked.

## Lean conversion side

The emitted module provides reusable operations:

- `charpoly_transport_of_reindex_eq`;
- `charpoly_factor_of_reindex_feeder_core`;
- `charpoly_closed_of_reindex_cycle_core`.

They use the published Mathlib identities:

- `Matrix.charpoly_reindex`;
- `Matrix.charpoly_fromBlocks_zero₂₁`.

Thus a certificate-produced reindexing equality becomes

```text
adjacency.charpoly = X * core.charpoly.
```

Supplying the remaining core theorem

```text
core.charpoly = X^n - 1
```

closes the full graph polynomial as

```text
adjacency.charpoly = X * (X^n - 1).
```

The next operation to implement is a proof-producing standard-cycle core
backend, so the final premise is derived rather than supplied.
