# Generalized multinacci general-m intertwiner transport

## Result

For every `m >= 1`, the thick prefix contributes the exact symbolic cut
catalogue

`(p,q) in {0,...,m}^2`.

Its signed defect `delta=q-p` emits `Q` for `delta=0` and `R^|delta|`
otherwise.  Therefore any common positive intertwiner

`Qc P <= P Qk`, `Rc P <= P Rk`

propagates through every cut channel, every roof word with roof at most `m`,
and the complete scheduler

`S_m(Q,R)=(m+1)Q + sum_{d=1}^m 2(m+1-d)R^d`.

The sign of `delta` is a separate side voltage and does not alter the positive
word inequality.

## Implementation

Added reusable derivation operation:

`derive_generalized_multinacci_general_m_intertwiner`.

The operation receives the concrete competitor/core Q/R matrices and a common
rectangular intertwiner.  It checks the two primitive inequalities, constructs
all roof powers exactly, evaluates both scheduler matrices, and checks the
full polynomial inequality entrywise.

## Validation

The executable regression checks every `m=1,...,128`, including the exact
catalogue size `(m+1)^2`, all `m+1` roof words, and the scheduler coefficient
sum `(m+1)^2`.  Invalid primitive intertwiners and `m=0` are rejected.

## Boundary of the result

This closes the renewal/suspension transport uniformly in `m`.  It does not by
itself prove that every concrete generalized-multinacci contact-boundary state
belongs to a uniform closed-form catalogue in `D`; the existing concrete graph
generator still supplies and verifies that admissibility refinement.  The
remaining structural task is a uniform-in-`D` description of the admissible
boundary states or an exact certificate theorem accepting the generated
finite catalogue.
