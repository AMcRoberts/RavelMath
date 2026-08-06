# Generalized multinacci boundary-word lift — round 87

For the `m=2` unit family `sigma(i)=00(i+1)`, the existing contact-boundary pipeline was instrumented without replacement. Each exact `simple_forward_targets_exact` witness already contains the two parent-prefix cuts. Their signed length defect labels the edge by

- `Q` when the cuts agree;
- `R^|delta|` when they differ;
- `sign(delta)` retained as the side/orientation cocycle.

The labelled witness graph projects exactly, edge-for-edge and with multiplicity, to the existing `G_B` adjacency matrix.

Exact results:

| D | boundary nodes | Q edges | R edges | R^2 edges | total edges |
|---|---:|---:|---:|---:|---:|
| 2 | 0 | 0 | 0 | 0 | 0 |
| 3 | 28 | 7 | 28 | 12 | 47 |
| 4 | 82 | 40 | 99 | 46 | 185 |
| 5 | 474 | 395 | 607 | 279 | 1281 |

No macro word outside `{Q,R,R^2}` occurs through D=5, exactly as predicted by the doubled prefix.

Each `R^2` macro edge was then subdivided by one private positional-phase vertex. The resulting expanded graph has only primitive `Q` and `R` edges. Its base-to-base first-return paths are in bijection with the original macro edges and recover `G_B` exactly.

| D | base nodes | private phase states | expanded nodes |
|---|---:|---:|---:|
| 2 | 0 | 0 | 0 |
| 3 | 28 | 12 | 40 |
| 4 | 82 | 46 | 128 |
| 5 | 474 | 279 | 753 |

Therefore the apparent third matrix is not primitive. The `m=2` family is the old positive `Q/R` grammar composed with a finite first-return scheduler. The remaining theorem work is now the family-uniform symbolic description of the boundary state set and the generatorwise comparison/intertwiner, not discovery of another generator.
