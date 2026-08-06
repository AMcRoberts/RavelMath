# Adjacent twisted renewal composition — 2026-08-04

## Result

The adjacent-dimensional proof stages are now composed by one reusable proof
operation rather than reported as independent PASS lines.

`derive_adjacent_twisted_renewal` lives in:

```text
include/ravel/proof/adjacent_twisted_renewal.hpp
```

It consumes independently derived evidence for:

1. complete competitor macro-path substitution;
2. exact competitor macro branch profiles;
3. total zero-append predicted-core shadow;
4. complete predicted-core macro-path substitution;
5. a total shared competitor/core role embedding;
6. the predicted-core Z/2 twisted extension;
7. Bellman profile commutation and strict renewal transport.

The operation returns the first named obstruction when a premise fails.  It
therefore satisfies the reflective proof rule: composition is executable proof
machinery, not a hand-written conjunction of log messages.

## Concrete replay

### n=4 -> n=5

Both competitor families pass the composite theorem.

```text
4-state family -> 44-state family
competitor edges: 4
competitor maximum macro path: 6
core maximum macro path: 14
competitor profiles checked: 4
core profiles checked: 46
inherited renewal block: 5
result: PASS

2-state family -> 18-state family
competitor edges: 2
competitor maximum macro path: 6
core maximum macro path: 14
competitor profiles checked: 2
core profiles checked: 46
inherited renewal block: 5
result: PASS
```

### n=5 -> n=6

Both competitor families pass the composite theorem using request-driven corona
images.

```text
44-state family -> 221-state family
competitor edges: 55
competitor maximum macro path: 18
core maximum macro path: 16
competitor profiles checked: 44
core profiles checked: 108
inherited renewal block: 8
result: PASS

18-state family -> 80-state family
competitor edges: 22
competitor maximum macro path: 21
core maximum macro path: 16
competitor profiles checked: 18
core profiles checked: 108
inherited renewal block: 8
result: PASS
```

No new spectral estimate is used.  The upper macro Bellman values are proved to
be the relabelled lower values, so the lower strict deficit is inherited.

## Self-explanatory runtime record

The adjacent driver now emits `ADJ_RENEWAL_EXPLAIN` with:

- core-shadow totality;
- core path completeness and maximum path;
- shared-role-map totality;
- twisted-core result;
- competitor and core profile counts;
- inherited renewal block;
- first obstruction, if any.

## n=6 -> n=7 boundary

The same projected driver was compiled for `6 -> 7`, but the run did not reach
its first family diagnostic within the execution fence.  This is not a failed
proof premise.  The next stable engineering target is to make the *lower*
`n=6` grade-family discovery request-driven as well, rather than constructing
all grade-at-most-two recurrent data before selecting a family.  Once a lower
family seed or formula is supplied, the upper request-driven machinery is
already in place.

## Trust status

C++ regression and real graph replay pass.  The emitted Lean playground is a
standalone abstract composition theorem; local Lean kernel checking is still
unavailable in this environment.
