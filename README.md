# RavelMathPub

Ravel is an exact-mathematics research library with a reflective proof-campaign engine.

Start with `docs/DOCUMENTATION_INDEX.md`.

For the proof system, the key rule is:

> When a derivation is missing, implement the reusable derivation operation. Do not hard-code the derivation or generated proof text.

The application triggers a problem, the math library records semantic structure, the campaign engine executes installed operations, and Lean checks the completed artifact.

Current formal status is recorded in `docs/THEOREM_STATUS.md`. Internal campaign closure is not the same as Lean kernel acceptance.
