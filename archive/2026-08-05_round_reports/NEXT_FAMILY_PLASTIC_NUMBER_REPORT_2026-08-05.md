# Next genuinely non-n-bonacci benchmark: plastic-number substitution

Use `a -> b`, `b -> c`, `c -> ab`, with incidence matrix `[[0,0,1],[1,0,1],[0,1,0]]` and characteristic polynomial `x^3-x-1`. Its Perron root is the plastic number, the smallest Pisot number. The rule is not a common zero-prefix/simple-Parry chain: two images have no common leading letter and the third branches as `ab`. Therefore the n-bonacci spine is not universal; it was the organizing spine of the simple-Parry corridor.

Next operation: run the generic parent-decomposition, balanced-pair, contact-boundary, finite-positive-grammar, and cyclotomic-sector pipeline on this rule without importing zero-run assumptions. Discover the minimal primitive generator alphabet and controller directly.
