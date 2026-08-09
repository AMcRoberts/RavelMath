# Reflective proof system contracts

These contracts are acceptance criteria, not suggestions.

## Architectural contracts

1. **The application is only a trigger.** It may select a problem and request outputs. It may not own determinant algorithms, matrix-family definitions, recurrence proofs, or theorem narration.
2. **The math library owns observation.** Structural facts must arise where the corresponding mathematical operation occurs.
3. **A plan is executable.** Every generated campaign operation must have an installed executor before `Go` is allowed.
4. **Implement the missing derivation operation.** Never manually perform a missing derivation and store its result as raw Lean, C++ strings, test fixtures, or theorem-specific handler code.
5. **The executor is theorem-neutral.** The generic executor must not contain `nbonacci`, `qMatrix`, `rMatrix`, or equivalent theorem-family names.
6. **Specifications contain data, not completed proofs.** They may supply definitions, transforms, signatures, and lemma identifiers. They may not supply prewritten proof-step vectors.
7. **No raw proof escape hatch.** Only typed proof operations may close a task.
8. **Only closed artifacts render.** Open goals, candidates, observations, and unsupported operations must be rejected before Lean serialization.
9. **Lean is the verifier.** C++ confidence checks must remain bounded engineering checks; do not recreate Lean in C++.
10. **Kernel status is explicit.** Internally closed, generated-unchecked, and kernel-checked are distinct states.
11. **No duplicated proof shapes.** If the same induction/case-split/calc structure is about to be hand-typed a second time under a different theorem name (even with different alphabet/variable names), stop and factor it into one proven-once general theorem plus mechanical corollaries, before writing the second copy — do not write the duplicate first and clean up later.
12. **A non-constructive existence step is an open item, not a closed one.** `Classical.choice`, `Classical.choose`, `by_contra` used to conjure an *existential witness* (as opposed to closing a decidable arithmetic contradiction), and non-computable `Set.Finite → Fintype` conversions do not satisfy this project's "we compute, therefore we know" standard when a computable alternative is available from data already in hand. Treat every occurrence as a TODO, not a completed proof, until either replaced with a computable witness or explicitly justified in writing as unavoidable.

### Anti-pattern reference (2026-08-08) — read before writing a new `_lemma_lean()` function

Two concrete failure modes were found and fixed in `include/ravel/proof/reflective_lean_renderer.hpp` on 2026-08-08, after having already shipped and kernel-checked. Read the violation and the fix before adding new proof text to this file, so the second discovery doesn't have to be as expensive as the first.

**Anti-pattern A — flat proof, duplicated under a new name (contract 11).** Before the fix, the exact same order-sandwich word induction was independently hand-typed four separate times, once per theorem name: `plastic_word_intertwiner`, `supergolden_word_intertwiner`, `three_generator_word_induction` (shift-branch), and `qr_word_intertwiner` (the flagship `condition_f_joint_qr` closure). Same for the power-sandwich induction inside `roof_word_intertwiner`, three separate copy-pasted "applyOnce/applyN/applyN_succ" blocks across `first_letter_orbit`/`last_letter_orbit`/`zero_run_same_chain`, and a duplicated `by_contra`/`iterate`/`omega` tail between `cyclic_splice_compactness` and `cyclic_splice_completion`. Kernel-checking a file does not catch this — every duplicate kernel-checked fine on its own; the anti-pattern is invisible to the type checker and only shows up as hand-authored proof *content* repeated under different names.
  - The fix, to imitate: `word_fold_intertwiner_lemma_lean()` (line ~1834) is now the single proof of the induction, stated maximally abstractly (no `Fintype`/`DecidableEq`, just a blanket `∀ g, ...` hypothesis) so every finite alphabet is a corollary, not a re-derivation. `compile_word_intertwiner_lemma()` (line ~2733) is a genuine proof-shape *compiler* — it takes a `WordIntertwinerSpec` (constructor list) and loops over it to assemble the case split, rather than storing N near-identical string literals. `pow_intertwiner_lemma_lean()` (line ~2277) is the analogous factoring for the power-sandwich shape. `recurrent_radius_le_one_of_pump_lemma_lean()` did the same for the `cyclic_splice_*` pair.
  - Before adding a new `_lemma_lean()` function: grep the file for `induction w with`, `induction k with`, `induction d with`, `by_contra`, and the specific hypothesis-signature shape (`mul_left_mono`/`mul_right_mono`/etc.) you're about to write. If something close already exists, extend or specialize it — do not write a fifth copy.

**Anti-pattern B — non-constructive existence treated as done (contract 12).** `universal_dominance_lemma_lean()`'s `shell_empty_of_strict_first_return_rank`, `no_nonempty_shell_with_strict_first_return_rank`, and `shell_empty_of_phase_rank_transport` (all around line 1660–1750) go through `Set.toFinite Shell |>.fintype` (line 1674) and `Shell.exists_max_image` (line 1744) — Mathlib's general `Set.Finite → Fintype` conversion is classical/non-computable, even though every actual call site in `render_universal_dominance_instances()` (line 1762) only ever instantiates `Shell := Set.univ` over a concrete `Fin n` — i.e. a case that was already finite and decidable and never needed the classical detour. This was shipped, kernel-checked, and treated as finished before the anti-pattern was named as a category to screen for.
  - A partial, already-built example of the *right* shape sits nearby in the same file: `cyclic_splice_completion`'s `chooseNext`/`orbit`/`finite_serial_relation_has_repeated_orbit` (line 3207+) use `Classical.choose`/`Classical.choice` (lines 3213, 3234) to assert a repeated state exists somewhere, without saying where — but `render_cyclic_splice_completion_instances()`'s per-node corollary bypasses that theorem entirely and instead states and `decide`-proves the *exact* repeat indices C++ actually found, as a direct `Nat.iterate` equality on a concrete, computable, `Fin n`-valued transition function built from the real relation data (`ravel::proof::stage_cyclic_splice_completion` in `include/ravel/proof/cyclic_splice_completion.hpp`). That concrete fact needed no `Classical.*` anywhere. The classical general theorem is still sitting in the same file unused by the live path — not yet replaced with a computable general version, which is the next step, not a finished one.
  - The working test for whether a `by_contra`/`Classical.*`/`Set.Finite`-based step is acceptable: does it close a *decidable* proposition (finite arithmetic, `Fintype`+`DecidableEq` case analysis — fine, e.g. `recurrent_radius_le_one_of_pump`'s `by_contra hnot` over `ℕ`, closed by `omega`)? Or does it assert an object exists (a witness, an index, a maximal element) that the surrounding proof or a downstream consumer would need to actually inspect, over a domain that is *already finite and decidable at every real call site* but stated abstractly enough to force Mathlib's classical fallback? If the latter, it is not done — restate the general theorem over `Fintype`+`DecidableEq` (or build the concrete instance directly from data, the way `cyclic_splice_completion`'s live corollary already does) so the witness is computed, not merely asserted to exist.
  - This audit only covered `reflective_lean_renderer.hpp`; the ~170 other
    `include/ravel/proof/*.hpp` headers and the root `lean/*.lean` files have
    not yet been screened for the same two anti-patterns.
  - **Priority ruling (AM, 2026-08-08, same day):** contract 12 applies at full strength, immediately, to anything reachable from `docs/COINCIDENCE_PROPERTY_F_CONTINUATION_ROADMAP.md`'s active strong-coincidence thread — that work exists to constructively *present* the substitutions the Pisot conjecture is about, not merely to prove existence, so a classical existence step there defeats the point even if it kernel-checks. Checked immediately: as of 2026-08-08 the coincidence/property-F critical path (`coincidence_closure.hpp`, `coincidence_converse_leftmost_loop.hpp`, `barge_diamond_certificate.hpp`, `property_f_unconditional.hpp`, and every coincidence/property-F `.lean` file, frozen and generated) has zero `Classical.*`/`by_contra`-existence/`Set.toFinite` hits — already clean, not yet exhaustively re-verified beyond grep. All three confirmed violations above (`universal_dominance`, `cyclic_splice_completion`, `predicted_core_scc_exhaustion`) trace to the superseded n-bonacci shell-rank/carry-bound thread and are explicitly *lower* priority by the same ruling — real debt, written up, but not worth session budget ahead of actual strong-coincidence progress.

## Derivation-operation contract

A derivation operation must declare:

- accepted typed inputs;
- structural preconditions;
- produced proof artifact;
- required dependencies;
- renderer operations it may emit;
- typed failure states.

Its implementation must construct the proof program from those inputs. It may not contain the target theorem's completed case tree.

## Static review contract

Before accepting an operation, manually inspect:

- dimension changes;
- index orientation and `Fin` transport;
- erased row and column maps;
- cofactor signs;
- branch coverage and branch exclusivity;
- definition dependency order;
- recurrence base and successor alignment;
- theorem and hypothesis names supplied to the renderer;
- absence of theorem-specific names in executor code.

Tests support the review but do not replace it. Lean remains the final authority.

## Failure contract

A missing capability must be reported directly. Do not downgrade it into plausible generated text. The report must name the missing operation or typed input and list the downstream blocked tasks.
