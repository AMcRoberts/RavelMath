# Symbolic Fiber Descriptor Derivation — 2026-08-04

## Result

The periodic grade-two boundary fibers are no longer discovered solely by finite equitable color refinement.

`describe_alternating_zero_fiber` derives a symbolic descriptor directly from each concrete corona state `[i,x,j]`:

- `x` has exactly one zero;
- every other coordinate is `+1` or `-1`;
- read cyclically beginning immediately after the zero, the nonzero coordinates alternate signs;
- `zero_position` is the phase;
- the sign immediately after the zero is the orientation.

Labels are deliberately not part of the fiber key. The real theta components contain phases with two distinct label pairs but the same displacement; replay proves those states have the same successor phase and multiplicity.

## Derived transition grammar

`derive_symbolic_alternating_zero_cycle` consumes a replay-validated boundary SCC and proves, from every concrete state and internal corona edge, that:

1. every state has a total symbolic descriptor;
2. one orientation is shared by the whole SCC;
3. every zero phase `z=0,...,D-1` occurs;
4. every internal transition preserves orientation and sends `z` to `z-1 mod D`;
5. all states in one phase agree on total witnessed multiplicity;
6. exactly one phase is doubled and every other phase has weight one;
7. the symbolic phases biject with the independently computed equitable classes.

Thus the period and block multiplicity are derived symbolically:

`p = D`, and `m = product_z w_z`.

For the real `n=7` boundary SCCs, the doubled phase is `z=2`, so `p=7`, `m=2`, and hence `A^7=2I`.

## Validation

Focused tests pass, including rejection of a malformed alternating-zero state.

The full projected `6 -> 7` run passes. Both nine-state SCCs report:

`macro_renewal=true state_level_renewal=true symbolic_fibers=true renewal=7/2`

## Remaining seam

The descriptor and transition law are still replay-derived on a finite SCC. The next universal step is to derive the same alternating-zero preservation and the unique doubled prefix role directly from the n-bonacci parent/prefix formulas under dimension transport, eliminating the need to first enumerate the SCC.
