# Plastic-number three-generator grammar

For the substitution `a->b, b->c, c->ab`, the exact contact-boundary graph has three prefix-defect classes:

- `G0`: zero prefix difference;
- `G+`: right prefix exceeds left by `a`;
- `G-`: left prefix exceeds right by `a`.

The generated graph has 101 states and 125 edges, split exactly as `79 G0 + 28 G+ + 18 G-`.

The old involution `[i,x,j] -> [j,-x,i]` flips `G+` and `G-` on the 92 edges whose partners survive, with no wrong-class matches. But 33 edges have no involutive partner in the exact boundary core. Therefore `G+` and `G-` are not globally a single orientation-twisted positive generator after admissibility. This is the first concrete irreducible three-generator grammar found outside the simple-Parry/n-bonacci spine.

The next proof target is a three-generator common intertwiner and positive boundary-polynomial closure, using the already-built arbitrary finite-generator theorem.
