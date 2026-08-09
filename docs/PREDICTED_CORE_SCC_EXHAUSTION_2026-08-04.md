# Predicted-core SCC exhaustion

> **Correction, 2026-08-08.** The finite result below (n=6, resp. n=3..7) stands and is now the basis of a kernel-checked reflection wiring (`recurrent_family_exhaustion_reflection_test.cpp` / `predicted_core_scc_exhaustion_reflection_test.cpp`). Its stated dependency on an "all-n induction" / "universal carry-bound theorem" is moot: the universal n-bonacci dominance theorem closed via a different route (the canonical Q/R split) that never needed this hull/carry-bound machinery. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md`.

The research record/provenance records a separate `1` versus `n+1` hop-distance dichotomy
from the older BP-core to literal `G_B` correspondence investigation.  It is
not the previous-alphabet shadow distance theorem and is not imported here.

## Correct exhaustion invariant

The formula-defined core is not forward-closed in the coefficient-one
arithmetic hull.  It has outgoing branches.  Those branches are transient with
respect to the core: no first edge leaving the core lands at a vertex that can
reach the core again.

Thus the exact criterion is:

1. predicted-core occurrence;
2. internal strong connectivity from the shadow induction;
3. no return after exit.

The arithmetic-hull backend computes the reverse basin of all core vertices by
one multi-source reverse BFS.  Every core-to-noncore edge is then checked: its
destination must lie outside that reverse basin.  This permits arbitrary
outgoing transient material while excluding every larger mutually reachable
component.

The emitted certificate reports:

    XOUT <all outgoing core boundary edges>
    XRETURNOUT 0
    XREVERSEOUTSIDE <noncore vertices that reach core>
    XSCC EXACT

`XREVERSEOUTSIDE` may be nonzero because incoming transient vertices can reach
the core.  The decisive value is `XRETURNOUT=0`.

The Lean theorem proves that an internally strongly connected core satisfying
this immediate-boundary no-return property is exactly the SCC of any core
anchor.

The finite exact audit covers the coefficient-one ternary arithmetic hull for
n=3..7.  The universal carry-bound theorem remains the bridge placing every
recurrent literal state inside that hull.
