# Generic proof-operations status

The proof campaign executor has been separated from the n-bonacci theorem data.

Implemented reusable operations:

- lower-triangular support derivation from reflected entry-rule evidence;
- determinant composition from a triangularity artifact;
- piecewise erased-index matrix equality;
- sparse cofactor recurrence construction;
- first-order recurrence closure;
- multi-minor cofactor decomposition;
- polynomial normalization; and
- final theorem composition.

The theorem campaign generator supplies typed specifications containing family identifiers,
theorem signatures, matrix transforms, definitions, and relevant lemma names. The executor
constructs the proof program for each operation. The executor body contains no `qMatrix`,
`rMatrix`, or `nbonacci` identifiers.

Manual static audit performed:

- removed theorem-specific induction names from the renderer;
- confirmed operation specifications no longer carry completed proof-step vectors;
- preserved generated-definition dependency order instead of sorting definitions by name;
- confirmed `qResidualMinor` is emitted before `qResidualCore`;
- confirmed every campaign dependency closes before its consumer executes;
- confirmed no raw Lean escape hatch exists;
- inspected matrix dimensions and erased-index transforms along the universal-n chain.

Native validation completed:

- `qmatrix_typed_ir_test`
- `proof_reflection_test`
- `proof_campaign_engine_test`
- `nbonacci_charmpoly_proof_general`

The generated campaign reports all universal-n tasks closed. Lean kernel checking remains
explicitly deferred until the complete matching Lean toolchain artifacts are supplied.
