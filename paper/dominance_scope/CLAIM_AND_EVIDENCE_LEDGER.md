# Claim and evidence ledger

This ledger prevents the paper from silently strengthening repository results.

| Claim | Status | Primary repository evidence |
|---|---|---|
| Universal determinant identity for the generated n-bonacci characteristic matrix, all `n >= 2` | Lean kernel checked | `docs/NBONACCI_UNIVERSAL_N_KERNEL_CHECKED_2026-08-03.md`; `lean/generated/nbonacci_universal_n.kernel_checked.lean` |
| Identification of the generated matrix family with an independently defined carry/substitution operator | Requires a bridge theorem or executable source-equality certificate | Scope boundary in the checkpoint above |
| Exact finite arithmetic dominance certificates in bounded dimensions | Executable exact certificates; dimensions and domains must be stated | `docs/NBONACCI_ARITHMETIC_DOMINANCE_THEOREM_N3_N7.md`; arithmetic reports and tests |
| Canonical Condition-F split into balanced `Q` and residual `R` generators | Executably derived and regression checked | `CONDITION_F_CANONICAL_TWO_MATRIX_REPORT_2026-08-05.md` |
| Twisted finite quotient is a channelized permutation skew product with exact reconstruction and equal row-path growth | Executably derived and regression checked | `CONDITION_F_TWISTED_COMPONENT_ASSEMBLY_REPORT_2026-08-05.md` |
| Q/R intertwining propagates through every word and every finite nonnegative word polynomial | Lean kernel checked | `lean/generated/condition_f_joint_qr_playground.lean`; `ROUND79_TWIST_EXTENSION_VALIDATION_2026-08-05.txt` |
| Natural-number induction and final order sandwich used by the Q/R campaign | Lean kernel checked as abstract interfaces | same Lean file and validation record |
| Concrete eleven-word terminal-parent boundary polynomial is exactly the generated Condition-F boundary | Executable derivation/regression evidence; cite concrete generator artifacts | Condition-F boundary reports and tests |
| Full literal-carry-system dominance follows from quotient, core comparison, and bridge reductions | Project theorem claim; paper must enumerate each bridge and its evidence | Condition-F closure reports, universal dominance campaign, test logs |
| Gauge invariance and products of finite permutation twists | Constructive finite-dimensional consequences; engine tests | `TWIST_EXTENSION_THEOREMS_2026-08-05.md` and round-79 tests |
| Contractive/unitary operator twists are dominated by the scalar channel-count system | Standard norm-majorization argument; not yet a dedicated kernel-checked full operator theorem | literature audit and twist report |
| Arbitrary expansive twists are dominated by a norm-weighted scalar augmentation | Proposed general theorem and implementation target, not presently derived by the current engine operation | `DOMINANCE_THEOREM_LITERATURE_AUDIT_2026-08-05.md`; current counterexample audit |
| Unweighted dominance fails for arbitrary expansive twists | Proven by explicit scalar one-loop example `[1]` versus `[2]` | exact elementary counterexample; current round-79 witness |

The manuscript uses the terms **kernel checked**, **executable certificate**, **regression checked**, **paper proof**, and **proposed theorem** distinctly.
