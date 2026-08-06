# Third-smallest Pisot number: exact Q/R/S closure

Target minimal polynomial:

    m(x) = x^5 - x^4 - x^3 + x^2 - 1.

Its dominant real root is beta = 1.443268791270373... .  Exact greedy beta-expansion:

    d_beta(1) = 1001001.

Hence the canonical simple-Parry substitution is

    0 -> 01
    1 -> 2
    2 -> 3
    3 -> 04
    4 -> 5
    5 -> 6
    6 -> 0

and its Parry polynomial is

    P(x) = x^7 - x^6 - x^3 - 1
         = (x^2 + 1)(x^5 - x^4 - x^3 + x^2 - 1).

The factor x^2+1 = Phi_4 is a cyclotomic lift of the five-dimensional Pisot root into the seven-state beta substitution.

## Generator audit

The complete parent-prefix catalogue is exactly

    { epsilon, 0 }.

There are nine parent decompositions.  Ordered parent pairs therefore give 81 universal role edges on 7^2 = 49 ordered role states. Their exact prefix differences are only

    0, +e_0, -e_0,

with multiplicities

    53, 14, 14.

Thus every concrete boundary edge, in every exact admissible boundary subgraph of this substitution, is forced into the same three positive generator classes Q/R/S. Projection

    [i,x,j] -> (i,j)

has an exact parent-pair witness generatorwise by construction, so the finite-positive-grammar and norm-weighted operator-twist theorems apply whenever the concrete boundary graph is instantiated.

## Fourth/fifth-matrix verdict

Neither appears in the canonical beta-substitution.  A fourth or fifth raw matrix would have to refine parent roles or the neutral kernel without adding a new observable prefix defect, and therefore would not qualify under the project's irreducibility criterion.

The full seven-dimensional contact-boundary enumeration was attempted with enlarged caps but did not finish within the validation window.  This report does not invent boundary-state or edge counts.  The Q/R/S closure is symbolic and universal over whatever exact admissible boundary subgraph the contact algorithm returns.
