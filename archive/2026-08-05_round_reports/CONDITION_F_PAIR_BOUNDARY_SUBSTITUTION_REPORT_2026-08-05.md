# Condition-F Pair Boundary Substitution

The terminal-parent correction was derived without summing the canonical
Condition-F pair `(Q,R)`.

For the extension from dimension `D` to `D+1`, old parent-pair roles have both
coordinates below `D`; boundary roles contain the new terminal parent `D`.
Every first-return excursion from the old roles through the boundary and back
has length two or three and records a noncommutative word in `Q` and `R`, plus
the signed prefix-defect cocycle.

The exact formulas are:

- `8D-1` complete boundary excursions;
- `8D-5` excursions of length two;
- four excursions of length three;
- eleven signed word classes;
- every complete excursion has net prefix defect `0`, `+1`, or `-1`.

Thus the correction is an exact nonnegative noncommutative polynomial in the
pair, rather than a polynomial in `Q+R`.

The central algebraic theorem is now executable.  If a nonnegative rectangular
matrix `P` satisfies

```
Qc P <= P Qk
Rc P <= P Rk,
```

then for every word `W` in `Q,R`,

```
W(Qc,Rc) P <= P W(Qk,Rk).
```

Consequently the complete terminal-parent boundary polynomial preserves the
joint comparison term by term.  The `n -> n+1` recurrence therefore introduces
no new spectral inequality after a joint base-pair comparison has been found.

Implemented in:

- `include/ravel/proof/condition_f_pair_boundary_substitution.hpp`
- `tests/condition_f_pair_boundary_substitution_test.cpp`

The universal reduction now states the remaining problem correctly:

> derive the joint `Q/R` comparison on the base Condition-F pair; the exact
> terminal-parent substitution transports it to every larger dimension.

Validated dimensions: `D=2..256` for the exact excursion formulas.  Surrounding
Condition-F, component-maximality, voltage-Fourier, and universal-dominance
regressions remain passing.
