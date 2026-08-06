# Reading list: universal n-bonacci proof campaign

## Read in order

1. `PROOF_SYSTEM_MANUAL.md`
2. `PROOF_SYSTEM_CONTRACTS.md`
3. `PROOF_SYSTEM_EXTENSION_GUIDE.md`
4. `NBONACCI_SHOOT_THE_MOON_RUNBOOK.md`
5. `THEOREM_STATUS.md`, universal-n section

Then inspect the implementation surface:

- `math/include/math/proof_reflection.hpp`
- `math/include/math/poly_matrix.hpp`
- `include/ravel/proof/proof_campaign_engine.hpp`
- `include/ravel/proof/reflective_lean_renderer.hpp`
- `include/ravel/proof/reflection_declaration.hpp`
- `app/nbonacci_charmpoly_proof_general.cpp`
- `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua`
- `lean/Ravel/Matrix/EraseIndex.lean`

## Current proof chain

The campaign must automatically execute:

```text
r-matrix lower-triangular support
→ r-matrix determinant
→ q-matrix erased-minor transport
→ q-matrix determinant recurrence
→ recurrence closed form
→ characteristic cofactor split
→ polynomial normalization
→ universal theorem composition
```

## Editing rule

The n-bonacci application and campaign specification may name n-bonacci objects. Generic executor operations may not.

When a step is missing, implement the reusable derivation operation. Do not insert a completed proof into C++, Lua, a test, or generated Lean.

## Current validation boundary

The universal-n campaign has completed a Lean kernel check. The exact checkpoint and hash are recorded in `NBONACCI_UNIVERSAL_N_KERNEL_CHECKED_2026-08-03.md`. Continue to keep `Closed`, `GeneratedUnchecked`, and `KernelChecked` distinct for all future campaigns.

## 2026-08-05 continuation notice

The universal n-bonacci campaign described above is complete and kernel checked. The active research program has moved through finite positive grammars, generalized-multinacci `Q/R`, plastic `Q/R/S`, quartic generator auditing, and the third-smallest Pisot canonical beta-substitution.

For all new sessions, continue with:

- `docs/READINGLIST.QRS_PISOT_CONTINUATION.md`
- `ROUND102_QUALITATIVE_PC_HANDOFF_2026-08-05.md`
- `DECISION_BASIS_DOCUMENTATION_2026-08-05.md`

The next target named there is the supergolden root of `x^3-x^2-1`.
