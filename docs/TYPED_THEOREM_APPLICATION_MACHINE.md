# Typed theorem application machine

The theorem capability machine now has a first proof-producing backend.

`typed_theorem_application.hpp` accepts a curated derivation rule plus named
Lean-typed hypotheses.  It refuses closure when a required premise is absent.
When all premises match, it emits an actual standalone Lean theorem application,
not merely a search result or proof-plan manifest.

The first enrolled backend is `charpoly-to-fast-power-reduction`:

- input: `hchar : A.charpoly = nbonacciCharpoly n`;
- corpus chain: `nbonacci_geomSum_of_charpoly` followed by
  `nbonacci_block_identity_scalar`;
- output: `A ^ (n + 1) = 2 * A - 1`.

The emitted file includes the supporting definitions and proofs from the
existing theorem corpus so it can be pasted into Lean Playground with only
`Mathlib.Tactic` available.

Trust boundary: C++ premise matching does not certify Lean.  The emitted file
must be kernel checked.  Failure to match a required typed premise is reported
as an open obligation and no theorem is emitted.
