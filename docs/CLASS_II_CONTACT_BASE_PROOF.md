# Class-II center-contact base proof

Status: closed exact certificate, 2026-07-29.

For `sigma_{a,1}`, the contact construction is

```text
9 D_cont seeds
  -- backward closure: +6, then +1 -->
16 pre-contact states
  -- one restricted-Red rank removes 2 -->
14 contact states.
```

## Seed stage

The face-intersection test is integer-only. In the fixed
`[-2,2]^3` box it leaves 33 colored candidates using 13 displacement
vectors. With Perron covector proportional to

```text
(b,c,1) = (beta, a + 1/beta, 1),
```

Lean proves from the Class-II cubic that
`1<c<b` and exactly nine of those 33 candidates satisfy
`0 <= <x,v> < v_j` for every `a>=2`. Those nine are
`class_ii_d_cont_set()`.

Artifacts:

- `class_ii_d_cont_face_candidates()` performs the fixed integer
  enumeration;
- `DContFaceCandidateKind` is the 33-case Lean catalogue;
- `class_ii_dCont_face_candidate_valid_iff` is the universal window
  theorem.

## Positive backward occurrence

The closure additions split as six first-layer states and one
second-layer state. Seven labelled witnesses use only prefix cuts

```text
0, 1, a-1, a, a+1.
```

`class_ii_pre_contact_backward_witnesses(a)` records their colors,
destinations, types, and cuts. The labelled backward implementation
projects exactly to the independent `backward_edges` implementation.
Lean theorem `preContactBackwardWitness_lattice_identity` proves the
seven incidence/prefix coordinate equations for arbitrary integer
`a`.

## Reverse inclusion and Red

Over all sixteen possible
destinations, the unfiltered backward envelope has exact observed
counts

```text
raw prefix branches = 18a^2 + 72a + 68
distinct raw nodes  = 38a + 54
valid branches      = 6a + 21
distinct valid nodes = 14.
```

The fourteen valid nodes are exactly the contact catalogue. Thus one
universal classification of the backward envelope will prove both:

1. closure has no states beyond the displayed sixteen;
2. the two non-contact pre-contact states have no outgoing restricted
   edge, while every contact state does, so restricted Red removes
   exactly those two in rank one.

The raw nodes use 51 fixed `(left,x1,x2,right)` categories; only `x0`
varies. `class_ii_pre_contact_backward_category_ranges()` records the
affine minimum and maximum `x0` in every category.
`class_ii_contact_backward_envelope_certificate()` now derives the
51 categories and all range extrema directly from the bounded
symbolic image segments. It proves dominance of each displayed
extremum for every `a>=1` and independently obtains
`18a²+72a+68` raw prefix branches. (The summed interval span is
`38a+55`; one category has a single unattained interior lattice
point, giving the literal distinct-node count `38a+54`.)
It also proves for every `a>=2` that all fourteen contact states are
covered by symbolic raw-prefix intervals, while the two valid
pre-contact states removed by Red are absent: their relevant category
ranges are the singletons `x0=a` and `x0=-a`, not their displayed
`x0=0`.

The reverse certificate is now complete. Lean derives
`2/3 < b-c < 1` from the cubic for every `a>=2` and proves that the category bounds
`x1∈[-2,2]`, `x2∈[-1,1]` force every restricted raw predecessor into
`x0∈[-2,2]`. The symbolic certificate then:

1. classifies all `51*5=255` affine `(category,x0)` cases, with zero
   unresolved;
2. checks actual symbolic prefix intervals rather than filling their
   convex hull, excluding the unattained trivial-self hole;
3. obtains exactly `class_ii_contact_set()` as the restricted raw
   predecessor set;
4. intersects affine prefix-position intervals in every surviving
   branch family, recovering `6a+21` with zero unresolved.

Lean theorem `contactNode_in_restrictedH` proves that all fourteen
displayed survivors are valid from `1<c<b` and `b-c<1`.
`class_ii_perron_gap_lt_one` derives the gap bound directly from the
Class-II cubic, and `class_ii_contactNode_valid` packages the resulting
universal validity theorem.
`contactRedExcludedNode_in_restrictedH` records the important converse
diagnostic: the two Red exclusions are also valid restricted states.
They are removed by rank-one graph pruning, not by the
stepped-hyperplane window. The symbolic raw predecessor classification
contains every contact state and neither exclusion, proving the
outdegree distinction.

Together with the seven positive `9+6+1` witnesses, this proves for
every `a>=2` that backward closure is exactly the sixteen-state
pre-contact catalogue and one Red rank leaves exactly the fourteen
contact states. No parameter sweep is used in this conclusion.
