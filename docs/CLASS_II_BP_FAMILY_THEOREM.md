# The explicit Class-II balanced-pair matrix

This note proves the recurrent balanced-pair core and its
characteristic polynomial for the full family `sigma_{a,1}`, `a>=2`.
An exact sweep through `a=64` independently checks the symbolic proof.

The reduction method is classical. See
`LITERATURE_AUDIT_CLASS_II.md` for its sources and for the calibrated
claim that this specific state family and its two cubics are
project-derived but not yet exhaustively novelty-certified.

## The explicit matrix

For an integer `a >= 2`, put `r=a-1`. Order four proposed
balanced-pair states and their swaps as

`A,A*,B,B*,C,C*,D,D*`, where

- `A=(2 0^(a+1) 12 | 12 0^a 20)`;
- `B=(120 | 012)`;
- `C=(012 0^a | 2 0^(a+1) 1)`;
- `D=(2 0^a 1 | 12 0^a)`;
- `*` exchanges the two words.

Direct substitution and earliest-balanced-prefix reduction give the
candidate transition matrix

```text
0 1 0 0 1 0 0 r
1 0 0 0 0 1 r 0
1 0 0 0 0 0 0 0
0 1 0 0 0 0 0 0
0 0 1 0 0 1 r 0
0 0 0 1 1 0 0 r
0 0 0 0 1 0 0 r
0 0 0 0 0 1 r 0
```

`app/class_ii_bp_family_probe.cpp` constructs these words literally,
applies the library substitution and reduction, and independently
reconstructs this matrix.

## Theorem: this is the complete recurrent BP core

For every integer `a>=2`, the eight displayed states are exactly the
recurrent noncoincidence core reached by the balanced-pair algorithm
from `(01|10)`.

### Proof

Discarding unit coincidences, direct substitution and
earliest-balanced-prefix reduction give:

```text
I=(01|10) -> D*
A  -> A* + C  + (a-1)D*
A* -> A  + C* + (a-1)D
B  -> A
B* -> A*
C  -> B  + C* + (a-1)D
C* -> B* + C  + (a-1)D*
D  -> C  + (a-1)D*
D* -> C* + (a-1)D.
```

These identities follow by expanding
`sigma(0)=0^a12`, `sigma(1)=0^a2`, and `sigma(2)=0`,
then cutting whenever the two prefix Parikh vectors first agree. For
example, the initial image cuts into `a` copies of `(0|0)`, followed
by `D*=(12 0^a | 2 0^a 1)`, followed by `(2|2)`.
The same prefix-count scan gives the eight displayed rows of the
matrix; repeated `D` or `D*` chunks account for the coefficient
`a-1`.

Thus the initial state enters the displayed set after one step, and
the identities prove closure, so no other noncoincidence state is
reachable. For `a>=2`, all coefficients `a-1` are positive. The
displayed transitions connect every state to `A` and connect `A` to
every state (using the swap edges and the `B,C,D` chains), hence the
component is strongly connected. Therefore it is precisely the
recurrent core.

## Theorem: characteristic polynomial of the explicit matrix

For every `a >= 2`, the characteristic polynomial is

`x^2 [x^6-(a^2+1)x^4-2x^3+a^2 x^2-2a x+1]`.

### Proof

The word-swap involution exchanges adjacent indices. On its symmetric
and antisymmetric subspaces the matrix descends respectively to

```text
Q+ = [ 1 0 1  r ]     Q- = [ -1 0  1 -r ]
     [ 1 0 0  0 ]          [  1 0  0  0 ]
     [ 0 1 1  r ]          [  0 1 -1  r ]
     [ 0 0 1  r ]          [  0 0  1 -r ].
```

Expansion of the two `4 x 4` determinants gives

`chi_+(x)=x[x^3-(a+1)x^2+a x-1]`,

`chi_-(x)=x[x^3+(a+1)x^2+a x-1]`.

The involution decomposition is a direct sum, so the full
characteristic polynomial is their product. Writing
`u=x^3+a x-1`, that product is

`x^2 [u^2-(a+1)^2 x^4]`,

which expands to the stated polynomial.

This theorem concerns the displayed matrix and is independent of any
floating-point calculation.

## Independent exact finite check

For every integer `1 <= a <= 64`, exact BFS, recurrent-core extraction,
literal word-set comparison, and integer characteristic polynomials
show:

- for `a>=2`, the actual recurrent core is exactly the eight displayed
  states and has exactly the displayed matrix;
- for `a=1`, the `D,D*` orbit is absent, leaving six states, whose
  characteristic polynomial is the bracketed degree-six factor at
  `a=1`;
- both quotient polynomials match the formulas above whenever
  `a>=2`.

Reproduce with:

```sh
make class_ii_bp_family_probe
```

The `a=1` degeneration is deliberately kept separate: `a-1=0`, the
`D,D*` orbit is not recurrent, and the remaining six states have the
degree-six bracketed factor.
