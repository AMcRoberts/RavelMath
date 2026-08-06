# Condition-F Joint Q/R Comparison Closure

## Attribution

AM identified the root-cause strategy: compare the current problem with the
prior automatic universal-n proof and treat the new complexity as the same
underlying theorem with phase, fibre, holonomy, multiplicity, and matrix-pair
twists.  This led directly to preserving the canonical Q/R pair and proving
its joint order before summing the matrices.

## Theorem

Let `(Q_D,R_D)` be the canonical Condition-F parent-prefix pair in dimension
`D`, with `Q` the balanced-prefix generator and `R` the one-sided-prefix-defect
generator.

At the minimal alphabet `D=2`, the parent-role graph has one recurrent SCC.
The predicted core pair is therefore the whole recurrent pair, and the identity
matrix gives the simultaneous comparison

    Q_2 I = I Q_2,
    R_2 I = I R_2.

Under `D -> D+1`, all old roles and channels persist.  Every new role belongs
to the finite-depth terminal-parent boundary.  Eliminating that boundary
applies the same nonnegative noncommutative polynomial in `Q` and `R` to the
full quotient pair and to the core pair.

If

    Q_C P <= P Q_K,
    R_C P <= P R_K,

then every Q/R word satisfies the corresponding inequality, and therefore so
does every nonnegative polynomial in those words.  Thus the identity base
comparison propagates to every dimension.

Consequently the predicted core is spectrally maximal in the untwisted finite
Condition-F quotient.  Voltage/Fourier domination transports the result to all
twisted sectors, and the terminal/branching reduction transports it to the
literal carry system.

## New operations

- `derive_joint_pair_rank_one_dominance`
- `derive_condition_f_joint_pair_comparison`
- universal-dominance composition overload for the joint-pair theorem

The rank-one operation is retained for nontrivial finite pair comparisons.  The
universal base comparison itself uses the exact identity intertwiner, because
its two inequalities are equalities rather than a strict Collatz separation.

## Validation

- joint pair identity and nontrivial rank-one tests pass;
- canonical boundary substitution passes through dimension 256;
- universal pair reduction passes through dimension 128;
- joint comparison induction passes through dimension 128;
- universal dominance reduction accepts the resulting core-maximality theorem.

The remaining project step is Lean emission/kernel checking of the pair-word
monotonicity, dimension induction, and final sandwich.
