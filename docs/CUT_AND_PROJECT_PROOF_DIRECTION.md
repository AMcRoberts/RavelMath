# Cut-and-project interpretation of the Class-II shell proof

The stepped hyperplane in the substitution pipeline is not merely
terminological. Abelianized words live in `Z^3`; the Perron direction
defines an irrational cut; and projection to the complementary
contracting plane produces the Rauzy geometry. Membership in
`H_sigma` is therefore an acceptance-window condition before
projection.

Grimm's crystallographic overview describes model sets as slices of a
higher-dimensional periodic lattice selected by an internal-space
window and projected to physical space. Tsesses et al. provide a
modern physical example in which a two-dimensional plasmonic
quasicrystal retains conserved topological data naturally expressed
in four dimensions. The latter is conceptual precedent, not a direct
source for the Class-II contact theorem.

## Concrete payoff in the current proof

`lean/class_ii_affine_shells.lean` defines the higher-dimensional
height of a shell node against abstract Perron coordinates `(b,c,1)`.
Lean now proves all twenty signed stepped-hyperplane membership
conditions from only:

```text
4 <= q
c < b
1 < c
q(b-c) < c
(q-1)(b-c) + 1 < c.
```

The final inequality is the one-unit edge correction for a single
shell kind; it is exactly a window-boundary margin rather than an
unrelated twentieth case. This theorem is kernel checked without
`sorry`.

## Remaining family-specific algebra

For the Class-II Perron root `beta`, set

```text
b = beta
c = a + 1/beta.
```

Lean now derives the two window bounds uniformly for `4<=q<a` from

```text
beta^3 = a beta^2 + (a+1) beta + 1
```

and positivity/order of the Perron coordinates. The composite theorem
`class_ii_shell_endpoint_valid` therefore proves endpoint validity for
every interior shell kind and every admissible integer parameter,
assuming `beta` is the positive Class-II root with `beta>a`. No finite
sweep enters this result. It still does not prove Red survival,
universal derivation of the fourteen-state contact set, or terminal
exhaustion.

## Broader research question

After the Class-II induction, test whether recurrent boundary SCCs
admit a simpler invariant in internal space—window strata, a charge
vector, or a small homological class—whose projection explains the
integer shell Perron roots. This is motivated by higher-dimensional
inheritance in quasicrystals, but no such invariant is presently
claimed.

The next organizational layer is `FAMILY_OF_FAMILIES.md`. It separates
the incidence matrix and Perron splitting (the projection “light”)
from ordered prefix digits (the window “shape”), then defines exact
same-light near neighbors by adjacent unequal-letter swaps. This makes
it possible to ask whether the Class-II shell is stable under small
shape changes without recomputing or conflating the spectral geometry.
