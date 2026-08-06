# Symbolic controller characterization

The adjusted digit has been eliminated from the controller transition relation.

For a radius-one window `t`, base digit `d`, and proposed next tail `r`, define

    c(t) = t[0] - sum(t[1:]).

The proposed next state is `tail(t) ++ [r]`. It is a controller transition
exactly when:

    r in {-1,0,1}
    abs(r - c(t) + d) <= 1
    supNorm(tail(t) ++ [r]) = 1

The final clause excludes the all-zero window and is essential.

The analyzer:
- enumerates exact controller states for dimensions 2 through 7;
- performs reachable subset construction from every single source face;
- quotients subsets by global sign negation;
- searches for shortest counterexamples to acceptance for arbitrary digit
  words and arbitrary target faces;
- replays the complete exact first-return certificate corpus;
- records subset-cardinality and controller-center profiles.

Results:
- n=2: 8 concrete states, 34 reachable subset states through depth 6, 18 sign-quotient classes; arbitrary-language counterexample `{'reachable_states': 3, 'source_face': [0, -1], 'target_face': [0, 1], 'word': []}`.
- n=3: 26 concrete states, 465 reachable subset states through depth 6, 234 sign-quotient classes; arbitrary-language counterexample `{'reachable_states': 9, 'source_face': [0, -1], 'target_face': [0, 1], 'word': []}`.
- n=4: 80 concrete states, 1807 reachable subset states through depth 6, 907 sign-quotient classes; arbitrary-language counterexample `{'reachable_states': 27, 'source_face': [0, -1], 'target_face': [0, 1], 'word': []}`.
- n=5: 242 concrete states, 3894 reachable subset states through depth 6, 1952 sign-quotient classes; arbitrary-language counterexample `{'reachable_states': 81, 'source_face': [0, -1], 'target_face': [0, 1], 'word': []}`.
- n=6: 728 concrete states, 5411 reachable subset states through depth 6, 2713 sign-quotient classes; arbitrary-language counterexample `{'reachable_states': 243, 'source_face': [0, -1], 'target_face': [0, 1], 'word': []}`.
- n=7: 2186 concrete states, 6663 reachable subset states through depth 6, 3340 sign-quotient classes; arbitrary-language counterexample `{'reachable_states': 729, 'source_face': [0, -1], 'target_face': [0, 1], 'word': []}`.

The exact first-return corpus replay remains
`2920/2920` accepted.

The arbitrary-language theorem is false whenever a counterexample is reported.
The universal theorem must therefore exploit structural properties of exact
first-return words and their endpoint faces. It cannot quantify over unrelated
digit words and faces.

The next automated synthesis grammar should learn an inductive winning
predicate over:
- controller center `c(t)`;
- source and target face obligations;
- a symbolic summary of the remaining exact first-return word;
- global sign-negation symmetry;
- bounded local coordinate patterns.

The generated Lean module establishes the exact adjusted-digit elimination and
the trust boundary for synthesized winning certificates.
