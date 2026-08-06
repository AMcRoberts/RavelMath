# Uniform radius-one synthesis

The 2,920 source-conditioned certificates collapse to a shell-radius-independent
finite-state control problem.

The controller state is a translation window

    t ∈ {-1,0,1}^n,  ||t||∞ = 1.

For a base digit `d`, it selects an adjusted digit `d' ∈ {-1,0,1}` and uses the
forcing defect `q = d' - d`. The next translation state is the exact n-bonacci
forced update of `t` by `q`.

The controller input consists only of:

- the base digit word;
- the source boundary faces;
- the target boundary faces.

It does not inspect the shell radius `M` or any intermediate base coordinates.
For a genuine first-return path, the geometry is automatic:

- intermediate base states lie in the `(M-1)`-box;
- radius-one translations keep them inside the `M`-box, hence strictly inside
  the `(M+1)`-box;
- source and target face alignment place the translated endpoints on the
  `(M+1)` shell.

The generic dynamic-programming controller replayed every existing certificate:

- n=3, M=2: 298/298;
- n=3, M=3: 848/848;
- n=4, M=2: 1774/1774.

Total: 2920/2920.

This establishes a uniform synthesis *algorithm* and a radius-independent
correctness theorem. The remaining mathematical existence seam is to prove
that this finite-state controller always has an accepting path for every exact
maximum-shell first-return word, preferably uniformly in dimension `n`.
