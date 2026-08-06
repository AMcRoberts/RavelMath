# Executable derivation operations

The proof campaign may not close a task with theorem-specific raw Lean text.
A missing derivation is implemented as a reusable executable operation.

## Installed operation: piecewise matrix equality

`CampaignOperation::DerivePiecewiseMatrixEquality` consumes typed
`PiecewiseEntry` programs and a typed matrix transform. It verifies that every
branch condition belongs to the supported Presburger fragment, then executes a
three-stage proof program:

1. matrix extensionality;
2. normalization of the generated family and transform definitions;
3. arithmetic discharge of the normalized branch conditions.

The q-minor campaign invokes this operation. It contains no q-specific case
tree, hypothesis names, or raw proof payload. Unsupported branch languages fail
as `UnsupportedDerivationOperation` before serialization.

## Invariant

The Lean renderer accepts only structured proof steps produced by installed
operations. `LeanStepKind::Raw` no longer exists. Adding a new proof family
requires implementing the missing derivation operation, not manually performing
and embedding the derivation.

## Generic operation specifications (2026-08-03)

The campaign engine now separates theorem-family data from executable proof operations.
The campaign generator produces typed specifications for:

- lower-triangular support derivation;
- triangular determinant composition;
- erased-index piecewise equality;
- sparse cofactor recurrence;
- first-order recurrence closure;
- cofactor decomposition;
- polynomial normalization; and
- final theorem composition.

The executor contains no `qMatrix`, `rMatrix`, or `nbonacci` identifiers. It consumes the
typed specification and reflected family evidence, then constructs the Lean proof steps
associated with the operation. In particular, theorem specifications do not carry completed
proof-step vectors: the operation implementation supplies `ext/simp/omega`, cofactor
expansion, recurrence induction, normalization, or exact composition as appropriate.

The renderer also preserves definition insertion order. This is required because generated
definitions may refer to earlier generated definitions; alphabetical sorting previously placed
`qResidualCore` before `qResidualMinor` and would have produced an invalid Lean module.
Induction rendering now takes its base theorem, successor theorem, induction variable, and
hypothesis name from the typed operation rather than containing q-family names.
