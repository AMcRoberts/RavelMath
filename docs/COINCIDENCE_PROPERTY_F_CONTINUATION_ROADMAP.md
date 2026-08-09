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

- The earlier “property (F) is now unconditionally established”
  headline is withdrawn. `RavelGenerated.zeroWalk_eq_zero_iff`
  (`lean/generated/property_f_zero_walk.lean`, kernel-checked, zero
  `sorry`) proves the real-analysis lemma that a nonnegative walk from
  zero cannot return to zero after a nonempty prefix. It is the
  load-bearing lemma that rules out mixed zero/nonzero SCCs; it does
  **not** rule out cycles consisting entirely of nonzero nodes, which
  are the paper's actual obstruction (Lemma 9.8). The corrected
  `adelic::check_property_f` therefore remains a meaningful finite
  check, conditional on its certified closure and bounds.
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
later. The first post-retrofit property-(F) task is now concrete:
retain and reflect the per-run finite graph evidence (node classes,
SCC count, and nonzero-cycle count) instead of citing the zero-walk
lemma by name. `PropertyFResult` now exposes those counters, and the
historical Fibonacci/rnd13/x²−2x−2 regression covers them. This is
evidence for a particular closed run, not an unconditional theorem.
The next increment now exports the actual bounded adjacency and canonical
Q(beta) node keys through `PropertyFGraph`; the Fibonacci graph topology is
rendered and kernel-checked as a finite Lean artifact. The Lean bridge still
checks topology/partition data rather than pretending string keys prove
algebraic equality; a future increment can replace those opaque keys with a
typed Q(beta) representation.
The graph certificate now also carries each node's exact rational coefficient
vector in the power basis, and the Fibonacci artifact renders those vectors
as Lean `List (Int × Nat)` data. The remaining algebraic bridge is to export
the edge digit/prefix data and characteristic polynomial so Lean can check
the recurrence on every serialized edge.
