# Delayed self-branch continuation

Define, for 2 <= D <= 127,

0 -> 1, 1 -> 2, ..., D-2 -> D-1, D-1 -> 0(D-1).

Its incidence characteristic polynomial is

x^D - x^(D-1) - 1.

The full parent-prefix catalogue is dimension-independent: {epsilon, 0}. Therefore the exact signed prefix defects are always exactly

0, +e0, -e0,

and the positive transport grammar is uniformly three-generator.

The quartic Pisot substitution is D=4. At D=5,

x^5-x^4-1 = (x^2-x+1)(x^3-x-1),

so the branch acquires a cyclotomic factor and recovers the plastic polynomial only as the remaining factor. This is a spectral obstruction, not a new positive generator.
