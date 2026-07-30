# Stepped-hyperplane compartments and walls

Status: organizing definition and theorem map, audited 2026-07-29.

For a fixed substitution and Perron covector `v`, the signed stepped
hyperplane uses the half-open convention
`0 <= <x,v> < <e_i,v>`. A **wall** is equality in either boundary
test.  (The lower face belongs to the window; the upper face does
not.) A **compartment** is a connected parameter region on which every
relevant comparison outcome is constant. The accepted endpoint
catalogue and each transition-count formula are consequently stable
there.

This is narrower than a chamber in the full family graph. Prefix
collisions, SCC changes, and coincidence changes are additional
combinatorial or dynamical walls, not automatically
stepped-hyperplane walls.

## Class-II coordinates

For `sigma_{a,1}`, put `b=beta`, `c=a+1/beta`, and `d=b-c`, where
`beta^3=a beta^2+(a+1)beta+1`. The twenty interior endpoints reduce
to two active margins:

```text
m0(q) = c - q d
m1(q) = c - ((q-1)d + 1).
```

`lean/class_ii_affine_shells.lean` proves these margins, with the
basic coordinate order, imply all twenty endpoint tests. Its Class-II
specialization proves them for `a>=2` and `4<=q<a`. The whole interior
range is therefore one certified open compartment for endpoint
membership.

At `q=a`, terminal and penultimate catalogues introduce bounded affine
edits. These are the correct wall objects. Red-pruning rank is a
second partition over endpoint membership; a rank change requires a
graph-edge proof and does not follow merely from a shrinking margin.

The terminal endpoint retains two correlations that are lost if
`c`, `d`, and `e=(a-2)d` are bounded independently:

```text
c < d + e + 2,
e + 3 < c + d.
```

They decide the last four terminal roles. Both follow strictly from
the Perron cubic and are kernel-checked in
`class_ii_neighbor2_terminal_refined_margins`. They are margins inside
the same Archimedean compartment, not new p-adic or Red walls.

The fixed layer needs three further correlated margins,
`c>3d+e`, `e+2d+1>c`, and `c+1>e+2d`. They are likewise consequences
of the same cubic compartment, not new walls; treating `c,d,e` as
independent intervals merely loses enough correlation to leave seven
roles undecided.

## What the slope matrix establishes

For each explicit neighbor dominant-core catalogue,
`A(a)=A(3)+(a-3)D`. The catalogues contain `41,34,91` total edges;
only `12,5,24` entries of `D` are nonzero. Core sizes are `15,17,39`.
The Lean file checks these copied catalogue facts and the generic
affine identity. It does not derive the catalogues from substitution
transitions or prove that slope support is caused by a geometric wall.

## Archimedean versus non-Archimedean

Real window membership uses an ordered real embedding and exact
`Q(beta)` sign decisions. A p-adic field has no compatible order, so
p-adic digits cannot certify a real window sign.

Non-Archimedean factors remain relevant to adelic boundedness,
property (F), ramification, and automata. Changes in ramification index
or residue degree form an **arithmetic stratification**, not a fifth
stepped-hyperplane wall. A future theorem may relate the two
stratifications; analogy does not identify them.

## Current boundary

Closed for `a>=7`: universal interior and displayed correction
endpoint validity; stable raw-corona equality and stable Red ranks;
penultimate and terminal raw-corona equality and complete Red ranks;
and repeated-terminal raw-corona equality with all six Red ranks.

The center-contact base now has a three-stage fixed catalogue:

```text
9 D_cont seeds  ⊂  16 pre-contact states
                       -- Red rank 1 removes 2 -->  14 contact states.
```

Lean kernel-checks the finite 16 = 14 + 2 disjoint partition. The
seed window is universal as well. Integer face
geometry leaves exactly 33 candidates (13 displacement vectors) in
the fixed `[-2,2]^3` search box. Lean proves from the Class-II cubic
that, for every `a>=2`, exactly nine satisfy the restricted Perron
window; the only scalar order needed by the finite table is
`1 < a+1/beta < beta`. Those nine are exactly the displayed
`D_cont` catalogue.

The backward occurrence splits as `9+6+1`. Seven explicit
prefix-cut witnesses produce the six first-layer states and singleton
second layer; their cuts are affine expressions
`0,1,a-1,a,a+1`, and Lean proves all seven incidence/prefix lattice
identities for arbitrary `a`. Reverse inclusion is now a certified
finite sign-stability theorem, not a sweep. Writing `b=c+d`, every
raw height is

```text
(x0+x1)c + x0 d + x2.
```

Lean derives `2/3<d<1` from the cubic for every `a>=2` and proves that the raw category
bounds force restricted membership into the five slices
`-2<=x0<=2`. Exact affine endpoint classification closes all
`51*5=255` cases with zero unresolved and leaves exactly the fourteen
contact sources. Prefix-interval intersection gives the independent
multiplicity `6a+21`. The other two pre-contact states remain valid
window points but have no outgoing restricted edge, so their removal
is a Red wall, not a stepped-hyperplane wall.

Thus the universal 9/16/14 center-contact derivation is closed.

Open: universal occurrence and exhaustion of neighbor recurrent
catalogues, and a theorem connecting slope support to geometric wall
crossings.

The next core proof should preserve this separation and lift the now
closed endpoint theorems into the full occurrence/exhaustion theorem.
