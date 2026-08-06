# Coefficient-profile outward twists

## Scope

This round moves one integral coefficient beyond each solved endpoint of the
path from n-bonacci `(1,...,1)` to uniform Perry `(2,...,2,1)`.

* Nearest left-outside profile: `(1,...,1,0,1)`.  One internal coefficient is
  decreased by one, so the penultimate nonterminal image loses its sole
  zero-prefix.
* Interior control: `2^k 1^(D-k)`.
* Nearest right-outside profile: `(3,2,...,2,1)`.  One coefficient is increased
  by one, so the first parent class gains one renewal phase.

The implementation uses the canonical simple-Parry profile constructor
`simple_parry_profile_rule`; it does not special-case either endpoint.

## Exact transport result

Every tested boundary witness still emits only

    Q                 when the two prefix cuts agree,
    R^|q-p|           otherwise.

Thus both outward deformations retain the primitive positive alphabet `{Q,R}`.
The coefficient profile is a finite parent-controlled roof function.  Relative
to an endpoint, a positive coefficient change is a phase insertion and a
negative change is a phase deletion.

## New left-side phenomenon

For the nearest left profile

    t = (1,...,1,0,1),

one has

    p_D(x) = x^D - x^(D-1) - ... - x^2 - 1.

Exact alternating-sum evaluation proves

    p_D(-1) = 0  iff  D is even.

Hence every even-dimensional member has the cyclotomic factor `x+1`.  The
phase deletion has exposed a genuine Z/2 peripheral mode, so these members are
not Pisot.  This is the first deformation in the program that crosses the
Pisot boundary rather than merely changing a positive renewal scheduler.

The concrete graph data reflect the obstruction: at D=4 the left-outside
boundary catalogue expands to 462 states, compared with 113 for 4-bonacci.

Odd dimensions do not have this forced `-1` root.  Numerical root probes for
D=3 and D=5 place all non-Perron conjugates inside the unit circle, but this
round does not promote that numerical observation to a general odd-D theorem.

## Right-side phenomenon

The nearest right profile `(3,2,...,2,1)` remains monotone.  Its transport is a
single-parent phase insertion with roofs through 3.  Exact boundary probes for
D=3,4,5 use only `Q,R,R^2,R^3`; no new primitive generator appears.

This is a skew renewal twist rather than the global scheduler of uniform
Perry: the extra phase is selected by the parent class.

## Concrete probes

The exact output is in `ROUND94_PROFILE_PROBE_OUTPUT.txt`.  Highlights:

* D=3 left `(1,0,1)`: 20 states, words Q/R.
* D=4 left `(1,1,0,1)`: 462 states, words Q/R, with the exact x+1 obstruction.
* D=5 left `(1,1,1,0,1)`: 2851 states, words Q/R.
* D=3 right `(3,2,1)`: words Q/R/R^2/R^3.
* D=4 right `(3,2,2,1)`: words Q/R/R^2/R^3.
* D=5 right `(3,2,2,2,1)`: words Q/R/R^2/R^3.

## Interpretation

The coefficient profile acts as a signed integer-valued renewal cocycle over
the same primitive Q/R transport:

* insertion: add a scheduled R phase;
* deletion: remove a scheduled R phase;
* nonuniform profiles: select insertions/deletions by parent state.

But phase deletion can reveal roots of unity hidden by the positive endpoint.
The first such obstruction is the parity character at `-1`.  Therefore the
next general theory must track not only norm-weighted positive domination, but
also cyclotomic character twists created by coefficient deficits.

## References

The canonical finite-expansion/simple-Parry setting and finite-normalization
background are due to Frougny--Solomyak and the standard Parry-substitution
literature.  The uniform confluent endpoint is covered by Dombek--Masakova--
Vavra.  These references justify the established surrounding constructions;
the signed phase-insertion/deletion classification and the parity obstruction
identified here are project results.
