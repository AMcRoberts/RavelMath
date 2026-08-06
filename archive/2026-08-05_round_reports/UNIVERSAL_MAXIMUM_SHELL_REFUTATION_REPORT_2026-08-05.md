# Universal maximum-shell exclusion — refuted and replaced

## Result

The obligation

> for every dimension `n>=3` and every shell radius `M>=2`, the shell first-return relation is acyclic

is false.

For every `M>=1` and every positive integer `q`, set

```
P = 4M,
n = qP + 1.
```

Let `w` be the period-`P` triangular wave

```
M, M-1, ..., -M, -M+1, ..., M-1.
```

It has zero period sum and adjacent differences in `{-1,+1}`.  Define the
`n`-coordinate state

```
x_k = (w_k, w_{k+1}, ..., w_{k+n-1}).
```

Because `n-1=qP`, the last `n-1` coordinates contain exactly `q` complete
zero-sum periods.  Therefore the homogeneous carry tail at `x_k` is `w_k`.
Choosing digit

```
d_k = w_{k+1} - w_k in {-1,+1}
```

gives exactly `x_{k+1}`.  After `P=4M` steps the state closes.  Every state
has shell radius `M`.

Thus there are recurrent shell cycles at arbitrarily large radii and
in infinitely many dimensions.

The smallest new counterexample to the previous finite range is

```
n=9, M=2, P=8.
```

One representative scalar period is

```
2, 1, 0, -1, -2, -1, 0, 1.
```

Its length-nine windows form an exact eight-cycle under the legal carry
digits.

## Engine changes

Added:

```
include/ravel/proof/triangular_wave_terminal_shell.hpp
```

with the reusable derivation operation

```
derive_triangular_wave_terminal_shell(n, M)
```

and the dimension helper

```
triangular_wave_terminal_radii(n).
```

The operation derives and replays:

- the zero-sum triangular wave;
- unit adjacent slope;
- the parameter identity `n=q*(4M)+1`;
- every state and digit;
- every carry transition, including closure;
- shell-radius equality;
- the permutation-cycle adjacency and its cycle spectral radius `1`.

The focused test covers 32 symbolic families over `M=1..8`, `q=1..4`, plus
the explicit `n=9,M=2` case and a rejected nonconforming dimension.

## Safety correction

The old composition operation

```
certify_parametric_maximum_shell
```

now rejects all purported universal premises.  Fabricated Boolean evidence can
no longer certify the refuted statement.

A corrected composition boundary was added:

```
certify_parametric_maximum_shell_partition
```

Its target is:

> classify every recurrent maximum-shell SCC into uniform recurrent shell
> families, and strictly rank only the residual first-return relation.

The campaign map now records the triangular-wave cycle theorem as an exact
certificate and replaces `carry.parametric_shell_rank` by
`carry.parametric_shell_partition`.

## Consequences

1. There is no universal ternary recurrent-hull theorem.
2. There is no universal exclusion of all shells `M>=2`.
3. A coefficient bound `B(n)` may still exist for each fixed `n`, but it must
   allow the recurrent shell radii permitted by the dimension arithmetic.
4. The universal dominance theorem may still hold because the displayed cycle
   itself has permutation spectral radius `1`; however, the SCC containing it
   must be classified exactly before that fact can be used spectrally.
5. The next correct obligation is to classify the full recurrent SCCs
   containing these triangular cycles and prove that every other shell SCC is
   either another uniform family or lies in a strictly ranked residual graph.

This is not an intermediate obstruction.  It is the final resolution of the
maximum-shell-exclusion obligation: the statement was false and has been
removed from the proof architecture.
