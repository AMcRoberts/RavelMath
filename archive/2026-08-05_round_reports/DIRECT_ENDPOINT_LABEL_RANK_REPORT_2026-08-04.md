# Direct endpoint-label rank closure — 2026-08-04

## Result

Reverse Kahn elimination has been removed from the symbolic alternating-zero proof.
Every non-schedule candidate is now assigned rank `0`, `1`, or `2` directly from

```
(orientation, zero phase z, source endpoint label i, target endpoint label j).
```

The exact canonical prefix graph is retained only to replay-check that every edge
outside the predicted renewal schedules strictly lowers this formula rank.

## Odd dimensions

For negative orientation, rank two consists of

```
z=0: i=1, j>=3
z=1: i>=3, j=0.
```

Rank one consists of

```
z=0: i,j>0 and (i>=2 or j=1)
z>=2 even: i=0, j>=2
z>=2 odd:  i>=2, j=0.
```

The positive-orientation formula is the endpoint mirror:

```
rank 2:
  z=0: i>=3, j=1
  z=1: i=0, j>=3

rank 1:
  z=0: i,j>0 and (j>=2 or i=1)
  z>=2 even: i>=2, j=0
  z>=2 odd:  i=0, j>=2.
```

All other non-schedule states have rank zero.

## Even dimensions

There is no renewal schedule. For negative orientation:

```
rank 2: z=0, j=0, 1<=i<=D-2
rank 1: z=D-1 and either
        (i,j)=(0,0), or 1<=i<=D-3 and j>=1.
```

For positive orientation, exchange `i` and `j`.
All remaining candidates have rank zero.

## Evidence discipline

The formula is evaluated before graph traversal. The existing exact
`simple_forward_targets_exact` witnesses then check

```
rank(source) > rank(target)
```

for every edge whose target is not a predicted renewal state. A failure returns
a typed obstruction; there is no fallback to elimination or SCC discovery.

## Validation

The direct formula was instantiated and replay-checked in dimensions
`D=5,6,7,8,9`. In each case the maximum transient rank is exactly `2`.

Passing targets:

- `rejected_boundary_exhaustion_test`
- `corona_projection_test`
- `recurrent_family_exhaustion_real_test`

## Next seam

The remaining bounded operation is candidate enumeration over endpoint labels.
The next universalization step is to derive the valid-candidate inequalities and
transition cases symbolically from the n-bonacci parent grammar, so the proof no
longer loops over all `2D^3` candidates even as a validation surface.
