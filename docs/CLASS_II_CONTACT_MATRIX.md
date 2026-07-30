# The explicit Class-II contact-core matrix

For every exactly computed `sigma_{a,1}` case with `2<=a<=6`, the
dominant contact core consists of the same eleven triples, in
lexicographic order:

The closest literature precedent is Thuswaldner's 2006 parametric
contact graph for a different cubic family. See
`LITERATURE_AUDIT_CLASS_II.md`; that precedent does not contain this
matrix or quintic.

```text
[2,(-1, 1, 0),1]  [1,( 1,-1,-1),0]  [2,( 1, 0,-1),0]
[0,(-1, 1, 1),1]  [1,( 1,-1, 0),0]  [0,(-1, 1, 0),1]
[0,( 1,-1, 0),0]  [0,(-1, 1, 0),0]  [0,( 0,-1, 0),0]
[2,( 0, 1,-1),0]  [0,(-1, 0, 1),1].
```

In that order its observed transition matrix is the following
parametric matrix; omitted entries are zero:

```text
T[0,1]=T[1,10]=T[2,10]=T[3,2]=1
T[4,5]=T[4,7]=T[5,4]=T[5,6]=1
T[6,3]=a,   T[6,5]=T[6,7]=a-1
T[7,1]=a,   T[7,4]=T[7,6]=a-1
T[8,0]=1,   T[8,3]=a-1, T[8,5]=T[8,7]=a
T[9,8]=T[10,9]=1.
```

## Theorem: polynomial of the displayed matrix

For every integer `a`, direct determinant expansion gives

`charpoly(T_a)=x^6[x^5-a^2x^3-a(a+1)]`.

One compact way to perform the expansion is fraction-free elimination.
For a nonzero spectral parameter `z`, write `T v=zv` and put
`S=v5+v7`. The four unit chains give

```text
v1=z v0,  v10=z^2 v0,  v9=z^3 v0,  v8=z^4 v0,
v2=z v0,  v3=v0.
```

Rows 4–7 reduce to

`(z^2-a^2)S=a(z^2+a)v0`,

while row 8 gives

`z^5 v0=a(v0+S)`.

Eliminating `S` yields

`z^2[z^5-a^2z^3-a(a+1)]v0=0`.

Performing the same operations without dividing by `z`—equivalently,
fraction-free elimination on `zI-T`—contributes four further pivot
factors `z` from the unit chains. Hence

`det(zI-T)=z^6[z^5-a^2z^3-a(a+1)]`.

Both sides are monic of degree eleven, so no scalar factor is
undetermined. This is a theorem about the explicitly displayed integer matrix. The
six powers of `x` explain why the nine-state outgoing equitable
quotient observed by the driver has polynomial
`x^4[x^5-a^2x^3-a(a+1)]`: it removes two nilpotent directions without
changing the nonzero factor.

## Exact finite identification

`make class_ii_symmetry_probe` independently reconstructs the contact
boundary twice—report adjacency and direct labelled transitions—then
checks the dominant core, exact matrix, rigidity, quotient size, and
integer polynomial for every `2<=a<=6`. Any mismatch exits nonzero.

## Open family theorem

What remains to prove is that this fixed eleven-state component is the
dominant recurrent contact core for every integer `a>=2`. Closure of
the displayed component follows directly from the contact transition
formula. Universal dominance additionally requires controlling the
larger `G_B` produced by corona and reduction and excluding another
recurrent component with equal or larger Perron root.

## Research pattern from Thuswaldner (2006)

Thuswaldner's useful transferable insight is the family-proof
architecture, not his particular quartic. He starts from several
computed graphs, proposes a parameterized finite state set, proves
that it contains the geometric seeds, and then proves it is a fixed
point of the contact operator using a finite catalogue of symbolic
image identities.

For this project the architecture must be lifted from the smaller
contact graph to the corona-grown self-replicating boundary graph.
The first symbolic ingredient is already simple. The incidence matrix
is

```text
[a a 1]
[1 0 0]
[1 1 0],
```

and, if `beta` is its Perron root, the positive left eigenvector
normalized in the last coordinate is

`v=(beta, a+1/beta, 1)`.

For every `a>=1`,

`beta > a+1/beta > 1`.

Indeed, the second inequality is immediate, while
`beta^3=a beta^2+(a+1)beta+1` implies
`beta^2=a beta+(a+1)+1/beta>a beta+1`. These strict inequalities can
replace numerical stepped-hyperplane membership decisions by exact
symbolic cases, just as coordinate ordering does in Thuswaldner's
proof.

The proposed next object is a parameterized partition of the full
boundary graph into:

1. the fixed eleven-state component `K_a`;
2. affine-in-`a` state families feeding into `K_a`;
3. any remaining recurrent families, if they exist.

The computations suggest a compact target: for tested `a>=2`,
`|G_B|=20a+8`, while `K_a` always has eleven states. Thus one should
seek roughly twenty affine state families, not enumerate a growing
unstructured graph. For every family, prove exact `H_sigma`
membership, exact transition images, corona/reduction closure, and
either a decreasing rank to `K_a` or an exact Perron comparison.

If all states outside `K_a` admit such a decreasing rank, the result
will be stronger than dominance: `K_a` will be the only recurrent
component. This is a research direction and proof template, not yet
a theorem.

### Implemented literature step

The first part of that program is now executable.

- `algorithm2_trace` in `corona.hpp` records every pre-reduction and
  reduced corona layer, including labelled edges.
- `CoronaConnectorPolicy` makes a hidden semantic choice explicit.
  The literature uses fixed `±C` at every round; the historical loop
  used the evolving layer as its own connector set.
- Differential runs for `1<=a<=8` give the same final node set and
  dominant polynomial under both policies, so the earlier Class-II
  spectral conclusions survive.
- The production pipeline now uses literature-correct fixed `±C`,
  generating far fewer candidates before reduction.
- The fixed-contact sizes expose the induction: `A1=28`, `A2=47`,
  then `|A_r|=20r+8` through the terminal layer (with a small `a=2`
  degeneration).
- `class_ii_interior_shell(r)` in
  `class_ii_boundary_family.hpp` is the literal twenty-node affine
  template added at every interior round `4<=r<a`. The probe verifies
  exact set equality, not just cardinality, through `a=8`.

This is a finite certificate and an explicit candidate induction
object, not yet the universal induction. The remaining proof is now:

1. prove the base layers symbolically;
2. prove the twenty affine formulas propagate under fixed-`±C`
   corona/reduction;
3. prove the parameter-dependent terminal correction at `r=a`;
4. identify the recurrent shell components and compare their roots
   with the fixed eleven-state component.

The original hope that every non-core shell state was transient is
false. Exact SCC decomposition for `2<=a<=8` instead reveals a much
cleaner recurrent family:

- one 2-state SCC with characteristic polynomial `x^2-1`;
- for each integer `2<=k<a`, one 4-state SCC with polynomial
  `x^4-k^2x^2`;
- the eleven-state SCC with
  `x^6[x^5-a^2x^3-a(a+1)]`.

There are therefore exactly `a` recurrent SCCs in every tested
`a>=2` graph. The competing Perron roots are precisely
`1,2,...,a-1`. If this SCC family is proved symbolically, dominance
of the eleven-state component is immediate: for

`f_a(x)=x^5-a^2x^3-a(a+1)`,

one has `f_a(a)=-a(a+1)<0`, while `f_a(x)` tends to positive infinity.
The positive Perron root of the eleven-state component is therefore
strictly greater than `a`, hence greater than every competing root.

The universal theorem has consequently narrowed to a combinatorial
decomposition theorem for the base, interior, and terminal affine
shells. No delicate algebraic root separation is needed afterward.

### Explicit recurrent shell matrices

The competing components now have a closed form. For `1<=k<a`, put
`m=a-k+1`. For `k=1` the component is

```text
[0,(-a,a,0),1],  [1,(a,-a,0),0]
```

with matrix `[[0,1],[1,0]]`.

For `2<=k<a`, the four states in lexicographic order are

```text
[0,(-m,m,0),0], [0,(-m,m,0),1],
[0,( m,-m,0),0], [1,( m,-m,0),0].
```

Direct application of the simple forward-transition formula gives

```text
[0   0  k-1 k-1]
[0   0   1   1 ]
[k-1 k-1 0   0 ]
[1   1   0   0 ].
```

Its two pairs of identical columns make the rank at most two. On the
nonzero two-dimensional quotient the eigenvalues are `k` and `-k`;
hence its characteristic polynomial is

`x^2(x^2-k^2)=x^4-k^2x^2`.

These formulas are implemented by
`class_ii_recurrent_shell_component(a,k)` and
`class_ii_recurrent_shell_matrix(k)`, and literal transitions match
for every `1<=k<a` through `a=16` using exact algebraic-coordinate
edge construction. The default sweep is deliberately bounded because
those coordinate solves are much more expensive than evaluating the
closed matrix. The displayed-matrix polynomial calculation is valid
for every integer `k`; what remains finite rather than universal is
the claim that these and the eleven-state component exhaust all
recurrent states of the full boundary graph.

The whole tested fixed point also has a compact decomposition for
every `4<=a<=8`:

```text
G_B(a) = B_68
         union (union over 4<=r<a of interior_shell(r))
         union terminal_shell(a),
```

where `B_68` is the stable 68-state third corona layer. The unions are
disjoint, giving `68+20(a-4)+20=20a+8`. The driver checks equality of
the complete node sets, not only their sizes. A publication-grade
universal proof can therefore be organized around one fixed finite
base certificate and two affine shell formulas.

The purely affine part is now kernel checked in
`lean/class_ii_affine_shells.lean`: every interior round has twenty
distinct formulas, different rounds are disjoint, and their
parameterized range is infinite. The same file contains an explicit
twenty-entry predecessor table using only seven constant contact hops
and proves `source(q-1)+hop=target(q)` for arbitrary integer `q`.
The native probe separately confirms the full corona predicates for
these witnesses on its exact finite sample.

This is not yet a proof that all those states occur in `G_B(a)`. The
remaining bridge is no longer the affine addition. Lean proves all
seven hops belong to the explicit fourteen-state candidate `±C`; what
remains is proving that candidate is the contact set derived by the
algorithm for every `a`, plus stepped-hyperplane endpoint validity,
survival under Red, and terminal exhaustion. Afterward,
unbounded growth follows without further computation.
