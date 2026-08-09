# Cross-dimensional transport hypothesis

Status: strong, tested hypothesis; not yet a theorem.

## Statement

For the generalized-multinacci family

`sigma(i) = 0^m(i+1)` for `i<d-1`, and `sigma(d-1)=0`,

the sofic and affine transport systems form a two-axis graded suspension.

1. The sofic base is dimension-free.  The prefix-phase grammar depends only
   on `m`: equal cuts give `Q`, a defect of magnitude `r` gives `R^r`, and the
   sign is a side-twist.  A concrete dimension `d` contributes only a
   deletion/restriction of the universal parent-role graph.
2. The affine fiber is the return-block tower

   `C_(b,k) = [k,m,...,m]`  (`0<=b<d-1`, `0<=k<m`),

   together with the terminal block `C_*=[0,m,...,m]` of length `d`.
   Thus `d` is height and `m` is carry color.
3. The `d -> d+1` lift is a tower extension: all old blocks embed, the old
   terminal becomes the new zero-carry block at height `d-1`, and exactly `m`
   channels are added.
4. The `m -> m+1` lift is a fiber thickening: old blocks embed after replacing
   forced tails `m` by `m+1`, and one new carry color is added at every
   nonterminal height.

The conjectural Property-F consequence is that a nonzero recurrent component
cannot be created in an inherited fiber.  Any genuinely new obstruction must
live in the newly added top-height or new-color channels.  Therefore a
parameter-uniform proof should consist of a rank on the graded fiber, with
the sofic phase as a finite base and the affine endpoint maps as its cocycle.

## What a new dimension fiber can do

For a `d -> d+1` lift, the new height channels all have the same linear
endpoint part `A^d`, where `A` is multiplication by `beta^(-1)`. Their offsets
differ by `k A^(d-1)e_0`, so the carry color changes translation only. The new
terminal channel has linear part `A^(d+1)`: it is one extra forced-descent
step followed by the terminal carry. Thus the new dimension fiber adds affine
translates and one extra power of the existing linear generator; it does not
introduce a new matrix generator. The cross-dimension certificate checks these
identities in addition to the channel injection law.

## Evidence

- The symbolic dimension-free suspension certificate passes all 42 points with
  `2<=d<=8` and `1<=m<=6`.
- The exact dimension-lift certificate passes 42 `(d,m)->(d+1,m)` lifts.
- The exact multiplicity-lift certificate passes 35
  `(d,m)->(d,m+1)` lifts.
- Strong coincidence, the general-`m` scheduler, and the return-block schema
  pass on all 42 points.
- Exact Property-F closure is established on the original 18-point rectangle
  and on the measured higher-dimensional anchors, including `(6,2)` at
  4,914,771 states. No closed case has a nonzero recurrent component.
- The direct paired recurrence test covers 15 exact dimension lifts (`2->3`
  and `3->4` for all `m=1..6`, plus `4->5` for `m=1..3`). Every base/lift
  pair closes, has zero nonzero SCCs and zero partial nonzero back-edges, and
  adds exactly the predicted number of channels.

## What would falsify it

Any one of the following would invalidate the present hypothesis or force a
refinement:

- an inherited block acquires a nonzero SCC after a dimension or multiplicity
  lift;
- the old-channel injection or tail-relabel law fails for some parameter;
- a profile-only point exhibits a nonzero recurrent component before its
  closure search reaches the resource limit;
- endpoint-map rank cannot be made decreasing on the newly added channels even
  after adjoining the finite sofic phase.

The current tests establish the combinatorial lift laws, not the final rank
decrease. That is the next theorem-facing obligation; exhaustive enumeration
is no longer the informative cross-dimensional experiment.
