# Stepped-face role junction — round 58

## Result

The residual-fiber seriality theorem from round 57 has been quotiented to the
actual stepped-hyperplane boundary roles.  For a first-return word, the active
source and target roles are signed coordinate faces `(coordinate, sign)`.
The new operation

    derive_stepped_face_role_relation(product, witness)

constructs the exact existential controller relation between those roles by
replaying the canonical radius-one controller plant over the word.

For every stored first-return segment in dimensions 3 and 4, the role relation
is bi-serial:

- every active source face has at least one active target face reachable by a
  legal controller path;
- every active target face has at least one active source face predecessor.

Validated corpus:

- n=3, M=2 and M=3: 1,146 / 1,146;
- n=4, M=2: 1,774 / 1,774;
- total: 2,920 / 2,920.

## Cyclic junction theorem at the role level

For a closed recurrent source lap, consecutive base endpoints are equal, so
the target role list of one segment is literally the source role list of the
next.  The operation

    derive_stepped_face_role_lap(segments)

composes the finite role relations around one lap and invokes the generic
finite cyclic-relation theorem.  This yields a positive cyclic sequence of
stepped-face roles.  Thus the face-junction obstruction itself is closed: a
hypothetical recurrent lap cannot fail merely because its consecutive active
faces are incompatible.

## Remaining distinction

A cyclic role choice is weaker than a closed controller-state orbit.  The
current data show why the distinction is real: the role quotient is bi-serial,
but an entire literal controller face is not generally mapped totally to a
literal target face.  Therefore the controller lift cannot be inferred from
role seriality alone.

The remaining local theorem is now exact:

> For the cyclic role choice supplied by the stepped-face role lap, the
> corresponding composition of exact controller-state relations contains a
> positive directed cycle.

All later operations are already complete:

1. controller-state cycle replay;
2. cyclic splice compactness;
3. strict shell pump;
4. contradiction with a maximum recurrent shell.

The next abstraction to test should use the linear residual formulas *inside a
fixed cyclic role choice*: derive a post-fixed residual subfiber for the
role-constrained one-lap predecessor operator.  This is narrower than the old
common-fiber conjecture and preserves the stepped-hyperplane geometry.
