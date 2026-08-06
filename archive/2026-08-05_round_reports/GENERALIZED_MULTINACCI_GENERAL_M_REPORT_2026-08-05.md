# Generalized multinacci: symbolic general-m renewal theorem

## Result

For the unimodular generalized-multinacci substitution

\[
\sigma_{D,m}(i)=0^m(i+1),\qquad \sigma_{D,m}(D-1)=0,
\]

every ordered pair of parent-prefix cuts \(p,q\in\{0,\ldots,m\}\) has signed defect

\[
\delta=q-p.
\]

Its positive transport word is exactly

\[
\delta=0\Rightarrow Q,\qquad \delta\ne0\Rightarrow R^{|\delta|},
\]

and \(\operatorname{sgn}(\delta)\) is a separate side-voltage coordinate. Consequently, the universal unsigned prefix scheduler is

\[
S_m(Q,R)=(m+1)Q+\sum_{d=1}^{m}2(m+1-d)R^d.
\]

The coefficients are exact because there are \(m+1\) diagonal cut pairs and, for each \(d>0\), two orientations for each of the \(m+1-d\) possible intervals of length \(d\).

Boundary admissibility can delete scheduler channels but cannot introduce a new primitive generator, alter the word attached to a surviving channel, or create a roof greater than \(m\).

## Implementation

`include/ravel/proof/generalized_multinacci_general_m.hpp` derives this result symbolically from the exact prefix-phase construction. It also checks that a concrete contact-boundary signed-renewal certificate refines the symbolic scheduler.

`lean/generated/generalized_multinacci_general_m.lean` emits the abstract word, coefficient, and roof-bound statements.

## Exact validation

The symbolic construction was evaluated for every \(1\le m\le64\). This is a regression range, not the theorem boundary; the construction itself accepts arbitrary positive `size_t` multiplicity subject only to memory availability.

Concrete contact-boundary graphs were generated exactly for

\[
D=2,3,4,\qquad m=1,2,3,4,5,6.
\]

Every graph refined the symbolic scheduler. In all nonempty new cases the maximum observed roof equalled \(m\), demonstrating that the bound is sharp rather than an artifact of small multiplicity.

## Scope now closed

The transition from fixed \(m=2\) to general \(m\) no longer requires generator discovery. The primitive transport alphabet remains exactly \(\{Q,R\}\); varying \(m\) changes only the finite renewal scheduler and side voltage.

The remaining family theorem must transport the existing generatorwise intertwiner through the scheduler uniformly in dimension and identify the symbolic boundary-state comparison map for the generalized-multinacci contact system.
