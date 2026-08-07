# Coincidence / property (F) continuation roadmap

Written 2026-08-07, as a bridge document for resuming this thread once
the reflection-retrofit work (currently active: Sturm's theorem port
in `lean/sturm_sequence_root_counting.lean`, then wiring it and the
remaining ~25 findings into the `mathlib::reflection` C++ emitters,
see `docs/REFLECTION_RETROFIT_PLAN.md`) is done. Written so a future
session — Claude Code or the ChatGPT chain — can pick this up without
re-deriving where things stand from `DIRECTION_AND_OPEN_THREADS.md`'s
full length.

## Where this sits relative to the two live threads

The project currently has two active research threads and one active
engineering thread:

1. **Item A / Item B** (`docs/DIRECTION_AND_OPEN_THREADS.md`) — the
   general unimodular and non-unimodular Pisot conjecture work. This
   is the mathematical frontier.
2. **The reflection retrofit** (`docs/REFLECTION_RETROFIT_PLAN.md`) —
   making every existing finding's Lean proof a mechanical emission
   from concrete C++ evidence rather than hand-authored text. This is
   the trust/engineering frontier, currently consuming session time.
3. **This document** — the specific sub-question of strong coincidence
   and property (F), which is where Item A and Item B's open cases
   actually bottom out once the easier obstructions are ruled out.

## What's already closed (as of 2026-08-06/07, verify against
`docs/PROPERTY_F_UNCONDITIONAL_KERNEL_CHECKED_2026-08-06.md` and
`docs/DIRECTION_AND_OPEN_THREADS.md` before trusting this as current)

- **Geometric property (F) is now unconditionally established**, not
  case-checked: `RavelGenerated.zeroWalk_eq_zero_iff`
  (`lean/generated/property_f_zero_walk.lean`, kernel-checked, zero
  `sorry`) proves the real-analysis core showing
  `adelic::check_property_f`'s `DOES_NOT_TILE_PROPERTY_F` path is
  unreachable for any Pisot substitution, given the standing
  Minervino–Thuswaldner model correspondence this project has relied
  on throughout. This means: **property (F) can never be the
  obstruction** to the Pisot conjecture in this framework anymore —
  the entire open question reduces to strong coincidence.
- The 13 confirmed unimodular `ρ_nc ≠ λ(G_B)` mismatches (σ₁, σ₂, +11
  random) all separately satisfy the classical Pisot conjecture
  (strong coincidence holds, property (F) holds) — that discrepancy is
  a refinement finding, not a counterexample to the classical
  conjecture. Settled, not open.
- Non-unimodular survey: 24/24 three-letter candidates ESTABLISHED
  (`app/sweep_nonunit_property_f.cpp`), all three known adelic bugs
  fixed. Four-letter survey (`sweep_nonunit_property_f_4letter.cpp`)
  found a real upstream bug in `spectral_invariants_general`'s
  Wielandt-deflation power iteration (underestimates the second
  eigenvalue's modulus when it's part of a dominant complex-conjugate
  pair) — check `docs/DIRECTION_AND_OPEN_THREADS.md` Item B for
  whether that's been fixed since 2026-08-01, since this document does
  not re-verify it.

## What's still open — the actual next targets

**Strong coincidence itself.** With property (F) closed off as an
obstruction, strong coincidence is now the *entire* remaining content
of the Pisot conjecture question in this framework. There is currently
no unconditional closure for it analogous to `zeroWalk_eq_zero_iff` —
check whether one has been attempted since this document was written
before assuming it's still open.

**Item A (unimodular, general case).** The n-bonacci structural proof
(free involution + exact polynomial divisibility + nilpotent cofactor)
is closed for n=2..7 but does NOT generalize past AR-exact substitutions
(Finding 5/6). Thread A3/A4 (AR-partial and non-AR cofactor mechanisms)
remain open with no known invariant — the originally proposed
beta-expansion-period invariant for non-AR was refuted. The
smooth-relaxation search (Thread A-relax/B2) is the standing plan for
generating new test candidates beyond random survey.

**Item B (non-unimodular).** Wider survey (4/5/6-letter, wider `|det|`,
wider `K_max`) is the concrete next step once the 4-letter blocker
above is confirmed fixed.

## Concrete pointers for resuming

- Full open-thread detail: `docs/DIRECTION_AND_OPEN_THREADS.md`
  (Item A starts ~line 30, Item B ~line 142 as of this writing — line
  numbers will drift, search for the header).
- Property (F) closure: `docs/PROPERTY_F_UNCONDITIONAL_KERNEL_CHECKED_2026-08-06.md`.
- Findings ledger (cite-by-number): `docs/FINDINGS_FOR_CITATION.md`.
- Prior unabsorbed exploration notes possibly relevant to this thread:
  the `NEXT_SESSION_CONDITION_F_*` and `NEXT_SESSION_*GRADE2*`/
  `*SHELL*` files audited (but not content-reviewed) in
  `.ravel/FLAT_FILE_ELIMINATION_ROADMAP.md` — read that audit's
  item 1 before assuming any of that content is either fresh or
  stale; nobody has actually opened those files against current
  findings yet.
- Lean coincidence infrastructure already in the library tier:
  `lean/constant_first_letter_forces_prefix_coincidence.lean`,
  `lean/first_letter_orbit_coincidence.lean`,
  `lean/zero_run_same_chain_coincidence.lean`,
  `lean/last_letter_orbit_coincidence.lean`,
  `lean/constant_last_letter_forces_suffix_coincidence.lean` — these
  are about symbolic-dynamics coincidence in the substitution/orbit
  sense used by the n-bonacci corridor's own proofs, not yet connected
  (as far as this document's author checked) to the strong-coincidence
  question above; worth a deliberate check for overlap before treating
  strong coincidence as needing entirely new machinery.

## Why this waits on the reflection retrofit

Per `docs/REFLECTION_RETROFIT_PLAN.md`'s corrected discipline (the
"citation pattern" mistake caught 2026-08-07): every new finding,
including whatever comes out of resuming this thread, needs to be
staged as concrete C++ evidence threaded into a typed trace, not a
hand-written Lean proof with a name-matched citation. Picking this
thread back up before the retrofit's patterns and tooling are solid
risks producing more of the exact citation-pattern debt that was just
caught and removed once already (property F's `zeroWalk` connection).
Finish the retrofit's emitter patterns first; resume this thread with
those patterns already load-bearing rather than retrofitting again
later.
