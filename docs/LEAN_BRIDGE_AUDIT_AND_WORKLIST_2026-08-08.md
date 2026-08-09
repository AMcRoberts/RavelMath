# The LEAN Bridge: full flat-vs-generative audit and worklist

Written 2026-08-08, in response to AM's direct instruction to read the entire
RavelMathPub source tree (this audit does not touch `ToStdMathLib/` at all —
that is a separate Apache-2.0-licensed project with no relationship to this
Unlicense one) and catalogue every proof as flat or generative, then build a
worklist to pull everything under the generative regime AM is calling **the
LEAN Bridge**: C++ code that computes a result, self-validates it meets a
certificate's conditions at the exact runtime point that validation is true,
records that certificate into a reflection trace, and *only then* is a Lean
proof wired to consume it and emit mechanically. The Barge-Diamond
certificate (`include/ravel/proof/barge_diamond_certificate.hpp`,
`lean/barge_diamond_lattice_line.lean`) is the canonical model: a tight,
certificate-shaped general lemma (37 lines) that takes exactly the data a
runtime check naturally produces as hypotheses, proven once, then
instantiated per concrete case by a renderer reading a live trace.

**Every fact below was checked directly against the current tree this
session** (`grep`/`wc`/reading source), not recalled from memory or from
prior documents' claims. Where a claim in an older doc (e.g.
`REFLECTION_RETROFIT_PLAN.md`) turned out to still hold, that's noted; where
this audit found something sharper or different, this document is
authoritative.

## Definitions used throughout

- **Generative (LEAN Bridge)**: a C++ function computes concrete data, checks
  it satisfies the certificate's conditions, and (only on success) calls
  `mathlib::reflection::record(...)` to add a typed node to an active trace;
  a `render_*_instances` function in `reflective_lean_renderer.hpp` (or the
  analogous path in `proof_campaign_engine.hpp`) reads that trace and emits
  Lean text mechanically; the emitted text has at some point been
  kernel-checked against the real Lean/Mathlib environment.
- **Flat**: a `.lean` file (or part of one) written by hand with no C++
  computation behind it at all -- whether it lives in `lean/` (the "library
  tier") or, worse, in `lean/generated/` despite that name.
- A general lemma is **not automatically flat** for being hand-proven --
  Barge-Diamond's own general lemma is hand-proven. It is flat when it is a
  large, independent mathematical development authored with no reference to
  what any C++ certificate actually produces, or when nothing ever calls it
  from a live trace.

## Part 1 -- what genuinely exists as generative infrastructure

Two real, live pipelines exist. Both were verified by finding actual
`record()` call sites and tracing them through to a renderer that is wired
into the module's output function.

### Pipeline A: `mathlib::reflection` / `reflective_lean_renderer.hpp`

`render_reflective_lean_module` (the dispatcher, `reflective_lean_renderer.hpp:1930`)
calls exactly 24 `render_*_instances` functions plus one inline block gated
on `has_r_matrix_proof`. Every one of these 24 was confirmed to have a live
`record()` call site somewhere in `include/ravel/proof/*.hpp` or in the
domain math headers (`include/ravel/class_ii_*.hpp`) -- 16 files call
`mathlib::reflection::record(...)` directly (13 under `proof/`, 3 under the
domain-math directory itself). This is real: Finding 42 (Barge-Diamond),
Finding 35/26 (period rotation), Finding 17/38 (constant first/last letter),
Finding 39/41 (zero-run, first/last-letter orbit), Finding 27 (leftmost
loop), Finding 32 (depressed cubic), Finding 23's Cayley-Hamilton sub-fact,
Finding 29's instance-level ordering, and sixteen separate Class-II
connections (shell round, fixed table, D_cont-in-pre-contact, round1-raw27,
neighbor-D-support, terminal shell, six-vertex-graduation, terminal sextet,
penultimate pair, interior tip, global round phase, both-fixed-affine) are
all genuinely wired this way.

### Pipeline B: `proof_campaign_engine.hpp` (the n-bonacci determinant thread)

Separate, older, and narrower in scope: `app/nbonacci_charmpoly_proof_general.cpp`
drives `mathlib::nbonacci_r_matrix`/`nbonacci_q_matrix` through a
`ScopedTrace`, and `ravel::proof::CampaignGenerator`/`ProofCampaignExecutor`
mechanically render `out/nbonacci_reflective_trace.lean`. This is the
pipeline the EraseIndex-retirement work earlier this session fixed --
confirmed as of this session: zero errors, zero `sorry`, both C++ regression
tests pass. This is the *only* thread with a matching `theorem_capabilities.tsv`
entry marked `kernel-checked-export` (all 14 such entries point at one file,
`lean/generated/nbonacci_universal_n.kernel_checked.lean` -- see Part 3).

## Part 2 -- structural gaps even inside the genuinely generative pipeline

These are real weaknesses in Pipeline A that should be fixed regardless of
any specific finding, because they undermine the "generative" guarantee for
*everything* built on this machinery, including future work:

1. **No automated kernel-check.** Every one of the 28 test files that
   exercise a `render_*_instances` function writes its output to `/tmp/*.lean`
   and stops -- the test itself only asserts on *substring content*
   (`lean.find("...") != npos`), never actually invokes `lake env lean` on
   the result. A renderer could silently start emitting Lean text that fails
   to kernel-check (a Mathlib API rename, a typo in a new instance) and no
   test would catch it. `scripts/safe_lean_check.sh` exists and is
   documented, but is invoked manually, not from any test or Makefile
   default target.
2. **`make lean-check` is not part of `make all`.** `all: build data apps
   tests` -- `lean-check` (57 `lake env lean` invocations) is a separate,
   manually-invoked target. A regression in any of the 57 files it covers
   would not be caught by a normal build.
3. **`lean-check` itself has gaps.** 22 of the 48 `lean/*.lean` files are not
   in the `lean-check` target at all (listed in Part 3) -- never even
   manually kernel-checked via the Makefile, whatever their individual
   history.
4. **The `/tmp` output is never persisted.** Only 2 of the 28 renderer-driven
   tests write to a path under `lean/generated/` at all
   (`realized_first_return_completeness.lean`,
   `characteristic_minor_maps_direct.lean`). The `lean/generated/*.lean`
   files that DO correspond to the 24 live renderers (batch files like
   `barge_diamond_batch.lean`, `cayley_hamilton_cubic_batch.lean`, the
   `class_ii_*_batch.lean` family) are committed snapshots that someone
   manually copied out of `/tmp` at some point -- there is no mechanism
   that keeps them in sync with the renderer's *current* output if the
   renderer or the C++ computation changes later.

## Part 3 -- the flat inventory (verified directly, not estimated)

### `config/theorem_capabilities.tsv` -- the project's own self-reported catalogue

252 rows, one per catalogued theorem declaration across all of `lean/`.
Status column breakdown: **230 `lean-declaration`** (self-reported as *not*
verified-generative by this catalogue), **14 `kernel-checked-export`** (all
14 in one file, `lean/generated/nbonacci_universal_n.kernel_checked.lean` --
Pipeline B, see above), **8 `draft`**. This catalogue itself is a
query/documentation index over the `lean/` tree (`ravel::proof::truth`
namespace, `theorem_capability_machine.hpp`) -- it does not compute or
generate anything; it does not make a theorem generative merely by listing
it. It is also stale: none of Pipeline A's 24 renderer-backed findings
(Barge-Diamond onward, all built after this catalogue's last update) appear
in it at all.

### `lean/*.lean` (48 files, the "library tier") -- reference-count against the C++ tree

19 of 48 have **zero** references anywhere in `include/`, `app/`, or `math/`
-- no certificate, no renderer, not even a docstring pointer. These are
standalone mathematical developments with no C++ side at all:

```
bp_correction_determinant          (has 1 sorry -- also backlog item #4)
class_ii_neighbor_dominance
continuation_controller_family_algebra
coupled_automaton_characterization
cycle_charpoly_campaign_validation
cyclic_controller_pumping
defect_spliced_covering_tube
family_of_families_adjacent_swap_counts
free_involution_perron_core         (has 1 sorry -- active backlog item #3)
graph_cycle_charpoly_factor_validation
graph_cycle_core_seam_validation
playground_recurrent_family_exhaustion
predicted_core_scc_exhaustion
rMatrix_proof_attempt               (name suggests an abandoned/superseded attempt)
radial_translation_defect
sturm_sequence_root_counting        (see Part 4 -- currently the active thread)
universal_dominance_phase_rank_transport
universal_dominance_shell_return_validation
universal_shell_pumping_proof
```

A further ~9 files have only 1-2 references, some of which turned out on
inspection to be incidental (a string key in `theorem_capability_machine.hpp`'s
catalogue, e.g. `perron_column_difference`) rather than real wiring -- treat
the "0 references" list above as the confirmed-flat floor, not the ceiling;
several of the 1-2-reference files likely belong in the same category and
were not individually re-verified line-by-line this pass.

22 of the 48 `lean/*.lean` files are absent from `make lean-check` entirely
(never even manually kernel-checked via the Makefile):

```
continuation_controller_family_algebra   coupled_automaton_characterization
covering_translation_tube                cycle_charpoly_campaign_validation
cyclic_controller_pumping                defect_corrected_radial_transport
defect_spliced_covering_tube             family_of_families_adjacent_swap_counts
free_involution_perron_existence_draft   graph_cycle_charpoly_factor_validation
graph_cycle_core_seam_validation         maximum_shell_maximality_contradiction
playground_recurrent_family_exhaustion   predicted_core_scc_exhaustion
predicted_core_scc_identification        rMatrix_proof_attempt
radial_translation_defect                symbolic_radius_one_controller
uniform_radius_one_synthesis             universal_dominance_phase_rank_transport
universal_dominance_shell_return_validation   universal_shell_pumping_proof
```

(Overlap with the zero-reference list above is real and expected -- a file
nothing references is also unlikely to be in the manual check list.)

### `lean/generated/*.lean` (62 files) -- despite the directory name

Confirmed by direct name-match against Pipeline A's 24 live renderers: about
22 files correspond to a currently-live renderer (the `*_batch.lean` family:
`barge_diamond_*`, `cayley_hamilton_cubic_batch`, `class_ii_*_batch`,
`constant_first/last_letter_batch`, `depressed_cubic_not_pisot_batch`,
`first/last_letter_orbit_batch`, `leftmost_loop_batch`, `period_rotation_batch`,
`pisot_root_ordering_batch`, `zero_run_same_chain_batch`). These are
genuinely renderer-derived content (see Part 2's caveats about staleness
risk since nothing keeps them in sync automatically).

The remaining ~35 files do not match any current renderer name and were
already flagged by `REFLECTION_RETROFIT_PLAN.md`'s own prior audit as
**"previously: entirely hand-authored per-finding text, not mechanically
rendered, despite the directory name"** -- that finding is confirmed still
accurate for these specific files:

```
adjacent_competitor_macro_profile        coefficient_profile_parity_obstruction
concrete_operator_qr_spectral_closure    condition_f_joint_qr_playground
cyclic_splice_compactness                cyclic_splice_completion
cyclotomic_obstruction                   finite_positive_grammar_majorant
first_return_obligations_discharged      first_return_residual_induction
generalized_multinacci_admissible_subgrammar
generalized_multinacci_general_m         generalized_multinacci_general_m_intertwiner
generalized_multinacci_primitive_intertwiner
generalized_multinacci_signed_renewal_twist
generalized_multinacci_symbolic_embedding
generic_residual_formula                 monotone_profile_corridor_closure
multi_compartment_closure_validation     norm_weighted_qr_majorant
property_f_zero_walk                     quartic_fourth_generator_quotient
reflective_boundary_grammar              shared_polynomial_closure_validation
shift_branch_three_generator_continuation
sign_flux_rank_playground_2026-08-04     supergolden_three_generator_intertwiner
third_smallest_pisot_parry_factorization twisted_bellman_transport
twisted_dimension_extension
```

`nbonacci_universal_n.kernel_checked.lean` and `characteristic_minor_maps_direct.lean`
/ `realized_first_return_completeness.lean` are the exceptions with real,
identifiable producers (Pipeline B and two directly-verified test writes,
respectively) -- these three are not flat, everything else in this list is
unconfirmed and should be assumed flat until checked.

### `include/ravel/proof/*.hpp` (186 headers) -- what they actually are

Only **13** headers under this directory have a live `record()` call
(3 more live in domain-math headers outside this directory, listed in Part
1). The other ~170 headers are a mix of: renderer helper functions
(`reflective_lean_renderer.hpp` itself), the `theorem_capability_machine.hpp`
catalogue machinery, domain mathematics with no proof-system involvement at
all, and headers matching `lean/*.lean` files 1:1 by name that are `#include`d
by OTHER headers as ordinary C++ helper code (e.g.
`defect_corrected_radial_transport.hpp`, `predicted_core_scc_identification.hpp`)
-- these are NOT reflection-engine wiring; they are unrelated C++ utilities
that happen to share a name with a flat `.lean` file. The "178 headers"
figure quoted in `REFLECTION_RETROFIT_PLAN.md`'s "Current state" section
significantly overstates live reflective infrastructure; this audit's 13+3=16
producer-file count is the real number.

## Part 4 -- the active thread: Sturm/Finding 30

`lean/sturm_sequence_root_counting.lean` (1008 lines) is confirmed flat by
every measure above: zero C++ references, a genuinely independent
mathematical development (sign-change counting, quasi-Sturm/Sturm sequence
hierarchy, local constancy, root-counting induction) written with no
reference to what any C++ certificate produces. It should **not** be
extended further by hand (the in-progress attempt to hand-port a canonical
PRS-construction proof into it, and then to duplicate Mathlib-PR content
into a new `Ravel.Polynomial.Sturm` file, were both wrong moves, corrected
mid-session -- see the conversation this document originated from).

The right shape, modeled exactly on Barge-Diamond: `math/bezout.hpp`'s
`ext_gcd` and `poly_z.hpp`'s `divmod` already compute the raw ingredients a
PRS certificate needs (Bézout coefficients, pseudo-division quotient/remainder).
What's missing is a small, certificate-shaped bridge -- own, independent,
Unlicense-authored, sized like Barge-Diamond's 37 lines, not like the 1008-line
abstract file -- whose hypotheses are exactly what a runtime Sturm-chain
computation naturally produces (a positive-scaled recurrence per step, a
Bézout identity, a nonzero terminal constant), proven once, then a C++
`stage_sturm_certificate(...)`-style function (mirroring
`stage_cayley_hamilton_cubic`'s shape: compute, self-validate, record only on
success) plus a `render_sturm_certificate_instances` renderer wired into the
dispatcher. This must be developed independently -- RavelMathPub cannot
import or copy from `ToStdMathLib/`'s Apache-2.0 `CertifiedSturmChain` (see
the licensing correction this document follows from); a fresh, from-scratch,
Unlicense-owned proof of the same *kind* of bridge is the correct target,
built the same way Barge-Diamond's lemma was: shaped by what the C++
actually needs to hand it, not derived independently and wired up after.

## Part 5 -- worklist, priority order

**2026-08-08, later the same day -- item 1 (the persistence half) done.**
Explicitly NOT automating the Lean invocation itself: AM's correction mid-session
was that the kernel-check step stays a deliberate action run by whichever
session happens to have a Lean toolchain available (this one did; most don't)
-- no `std::system`/subprocess call to `lake`/`lean` belongs inside a C++ test
binary or in `make all`. What *was* fixed: all 28 renderer-backed tests that
wrote their output to scattered `/tmp/*.lean` paths (asserted on by substring
match only, never persisted) now write directly to their canonical
`lean/generated/*_batch.lean` path -- the exact file `make lean-check` already
checks. All 28 also got proper `.PHONY` Makefile targets (previously
build-by-hand-with-`g++`-only) and were added to `TESTS_DEFAULT`, so
`make tests` regenerates every one of them automatically. Verified end-to-end
this session: `make tests` (regenerate) then `make lean-check` (kernel-check,
run manually) both pass clean, exit 0, zero errors -- the full arc (cert ->
self-validating check -> `record()` -> renderer -> canonical committed file ->
kernel-check) now closes without any stale-snapshot risk. Remaining from the
original item 1: `lean-check` is still not part of `make all` (left alone
deliberately -- making it a default-build dependency would break `make all`
on any machine without the Lean toolchain, which per AM is most of the time)
and still doesn't cover the 22 `lean/*.lean` files listed in Part 3 -- both
still open, listed below as item 1.

1. **Extend `lean-check`'s coverage** (Part 3): add the 22 currently-absent
   `lean/*.lean` files to the `lean-check` target so a future manual run
   actually covers them -- do NOT add `lean-check` itself to `make all` (see
   the note above; the toolchain isn't reliably present).
2. **Sturm/Finding 30** (Part 4): build the from-scratch, certificate-shaped
   bridge lemma (own Unlicense authorship, Barge-Diamond-sized), the
   `stage_*` C++ certificate function using `ext_gcd`/`divmod`, the payload,
   and the renderer -- validated first on a small concrete polynomial,
   kernel-checked, before attempting a real Pisot charpoly instance.
3. **Persist Pipeline A's `/tmp` outputs properly** (Part 2, item 4): either
   commit fresh copies into `lean/generated/` as part of the fix in item 1,
   or accept `/tmp` as intentionally ephemeral and rely entirely on the new
   automated kernel-check instead -- pick one and state it, rather than the
   current silent partial-persistence.
4. **Triage the 19 zero-reference `lean/*.lean` files** (Part 3): for each,
   decide fold-into-a-certificate (if a plausible C++ side exists or could be
   built), archive (if superseded/abandoned -- `rMatrix_proof_attempt.lean`
   is a strong candidate given its name), or flag as intentionally-abstract
   infrastructure with a documented reason it has no certificate (rare, and
   must be justified, not assumed).
5. **Triage the ~35 flat `lean/generated/*.lean` files** (Part 3): same
   three-way decision, but higher priority than item 4 since these are
   actively mislabeled by directory name -- either build the missing C++
   side and make them real, or move them out of `lean/generated/` into
   `lean/` (or archive) so the directory name stops lying about their
   status.
6. **Refresh or retire `config/theorem_capabilities.tsv`** (Part 3): either
   wire it to be regenerated from the real trace/renderer state (making its
   `kernel-checked-export` status meaningful and current), or stop treating
   it as authoritative documentation anywhere it's cited, since it is
   currently 230/252 self-reported non-generative and silently missing every
   Pipeline-A finding.
7. **`include/ravel/proof/*.hpp` cleanup** (Part 3): the ~170 non-producer
   headers should be classified (renderer helper / catalogue machinery /
   unrelated C++ utility / dead code) so future audits don't have to
   re-derive this; not urgent for correctness, but the "178 headers" framing
   in `REFLECTION_RETROFIT_PLAN.md` should be corrected to cite the real
   16-producer-file count instead, to stop overstating live infrastructure.

Items 1-3 are the load-bearing fixes (they protect correctness of what
already exists and unblock the active Sturm thread); items 4-7 are the
comprehensive cleanup to make "flat" actually rare across the whole tree,
not just absent from the newest work.

## 2026-08-08, continuation session: item 4 substantially completed

Worked through every zero-C++-reference `lean/*.lean` file found by grepping
each basename against `include/ravel/proof/*.hpp` and `tests/*.cpp`. Six
files converted to genuinely generative (new certificate + self-validating
C++ check + `record()` + renderer + positive/negative-control test + kernel
check, exactly the established pattern):

- `universal_shell_pumping_proof.lean` -- archived as fully superseded by
  `realized_first_return_completeness.hpp`'s strictly more general content
  (same `iterate_strict_shell_lift`/`StrictShellPump`, generalized to a
  higher universe); its OWN generative replacement is
  `strict_shell_pump_reflection_test.cpp`, wiring the pre-existing but
  previously Lean-disconnected `strict_shell_pump.hpp` engine
  (`certify_strict_shell_pump`) to a concrete two-point `StrictShellPump`
  instance.
- `realized_first_return_completeness.hpp`'s own emission -- NOT
  data-gated (still static text; the residual-formula/word-language layer
  has no concrete C++ backing anywhere in the tree and instantiating it
  honestly would require solving the actual open n-bonacci carry-family
  closure theorem, not a quick certificate). But its Lean content had a
  genuine, previously-undetected kernel-check FAILURE (`AcceptedPathPumps`
  elaborated at sort `Type` instead of `Prop` because its conclusion was
  the `Type`-sorted `StrictShellPumpWitness` structure) -- fixed to a
  proper existential Prop, confirmed this file had apparently never
  actually been run through `lake env lean` before this session despite
  living at a canonical `lean/generated/` path. Now wired into
  `TESTS_DEFAULT`/`lean-check`/sorry-scan (it wasn't before).
- `playground_recurrent_family_exhaustion.lean` -- wired to
  `recurrent_family_exhaustion.hpp`'s real caller
  (`recurrent_family_exhaustion_real_test.cpp`, genuine Tarjan SCC
  extraction + classification over a concrete corona-truth graph, never
  itself connected to reflection). New
  `recurrent_family_exhaustion_reflection_test.cpp` requires
  `out/corona_truth_n6.bin`; added a Makefile rule generating it via
  `./out/adjacent_competitor_transport --legacy-corona 6` (fast, <1s;
  the default `projected` mode is NOT used here since it hits an unrelated
  crash further down its own pipeline for n=4, irrelevant to this fix).
- `predicted_core_scc_exhaustion.lean` -- wired to
  `predicted_core_scc_identification.hpp`'s `certify_predicted_core_scc`
  (real Tarjan SCC + exact node/edge/predecessor-table count checks,
  previously a pure C++ unit test with no Lean persistence). Its general
  lemma's OWN proof also had a genuine, previously-undetected kernel-check
  failure (`Relation.ReflTransGen.tail`'s constructor arity assumed an
  older Mathlib convention where the start point `a` was itself a
  per-case bound variable; current Mathlib fixes `a` as an outer
  parameter) plus a `by_contra`-before-`induction` motive-generalization
  bug (the induction hypothesis needs `x ∉ Core` threaded as part of the
  motive, not fixed beforehand) -- both fixed; this file, too, had
  apparently never been kernel-checked before.
- `coupled_automaton_characterization.lean` -- its only prior C++
  reference (`coupled_winning_predicate.hpp`) merely evaluated a
  predicate tree and never checked the actual win-condition obligations
  (`hinit`/`hstep`/`haccept`) the theorem requires. Built a new
  `certify_synthesized_winning_predicate` that exhaustively replays all
  three over a finite state/input space, and a renderer instantiating the
  theorem at a concrete `Fin n`-state system via `decide` (required
  switching the emitted `Win`/`Init`/`Accept`/`Step` predicates from
  pattern-matched `True`/`False` `Prop`s to `Bool`-valued `def`s wrapped
  in `abbrev ... := ...B ... = true`, since bare `Prop`-valued pattern
  matches don't get automatic `Decidable` instances found by `decide`).
- `radial_translation_defect.lean` -- wired to the pre-existing
  `certify_translation_defect` (`radial_translation_defect.hpp`, exact
  integer matrix/vector arithmetic, previously a pure C++ unit test).
  Its general lemma's proof also had a latent `ring`-after-already-closed
  `simp` error (`No goals to be solved`) -- fixed by dropping the
  redundant trailing tactic, in both the flat original and the renderer's
  copy.

Two latent, previously-undetected Lean kernel-check bugs were found and
fixed purely as a side effect of finally running `lake env lean` on content
that had apparently never been checked before, despite living in the
committed tree (one in `realized_first_return_completeness.hpp`'s emission,
two more in the freshly-written `predicted_core_scc_exhaustion.lean`
instantiation, one in `radial_translation_defect.lean` itself) -- concrete
evidence for why "flat, never-generated, never-kernel-checked" content is
risky even when it looks superficially plausible.

**Correction (same session, later):** the first pass above wrote off five
more files as "open research" too quickly -- AM correctly pushed back:
these are *proven* (kernel-checked, no `sorry`) theorems, so nothing about
them is actually unsolved; "no concrete instantiation found yet" had been
conflated with "requires solving open mathematics." Re-investigated with
more care:

- `defect_spliced_covering_tube.lean` -- WRONG call. `certify_defect_spliced_tube`
  (`covering_translation_tube.hpp`) already independently verifies the
  translation and transported recurrences; only the base path's own
  recurrence wasn't checked, so `stage_defect_splice_steps` was added to
  close that gap. Built `nbonacciAHom`/`nbonacciForcingHom` (the n-bonacci
  carry step as a genuine `AddMonoidHom`, additive for *any* dimension via
  `Matrix.mulVec_add`/`add_smul`, proved once) and instantiated
  `defect_splice_step` per certified transition, cross-checked against the
  C++-computed successor states via `decide`. Kernel-checked clean.
- `universal_dominance_phase_rank_transport.lean` /
  `universal_dominance_shell_return_validation.lean` -- WRONG call.
  `validate_shell_return_certificate` (`universal_dominance_campaign.hpp`)
  already independently verifies exactly the `hout` hypothesis
  (`certificate_closed`) these theorems need. The theorems are
  non-existence results (no integer rank can make a closed finite relation
  strict), so the honest instantiation keeps `rank`/`level` universally
  quantified and supplies the concrete, C++-verified *closed relation*
  itself -- reusing the same 3-cycle "closed" case
  `universal_dominance_campaign_test.cpp` already demonstrates empirically.
  All four theorems in both files instantiated at that relation. This pass
  also caught two more latent bugs in never-before-checked proof text
  (`Set.Nonempty.exists_isMaxOn` doesn't exist in this Mathlib -- fixed to
  `Set.exists_max_image`; a stray `not_lt_of_ge` misuse -- fixed to a plain
  `omega` close), in both the renderer's copy and the original flat file.
- `cyclic_controller_pumping.lean` -- WRONG call on the framing (right
  call that the *closure* theorems are out of reach, wrong to call the
  whole file infeasible). `certify_strict_shell_pump` already computes
  exactly the closing base/translation/transported cycle data this file's
  `StrictShellPumpWitness` structure and its own docstring ask for
  ("the concrete n-bonacci campaign must instantiate this proposition by
  producing a... run") -- extended `render_strict_shell_pump_instances` to
  also construct a concrete `StrictShellPumpWitness` value at the same
  radius data already staged for `universal_shell_pumping_proof.lean`'s
  replacement. What's genuinely NOT instantiated (and correctly so) is
  `CyclicStrictShellPump`/`cyclicStrictShellPump_of_complete_family`'s full
  closure -- that needs a family complete under repeated pumping, which
  really is the open n-bonacci carry-family closure problem itself, not a
  quick certificate. The file was never in `lean-check` before; now is
  (kernel-checks clean).
- `continuation_controller_family_algebra.lean` and
  `nbonacci_margin_catalogue.lean` -- re-checked and this determination
  holds: both are purely definitional (`rfl`/`induction`-provable, no free
  numeric parameters, nothing to compute or validate against). Neither was
  ever in `lean-check` before; both now are (kernel-check clean).

`universal_dominance_phase_rank_transport.lean` and
`universal_dominance_shell_return_validation.lean` were archived (their
generative replacement is `universal_dominance_closed_relation_batch.lean`,
covering all four theorems from both files); `defect_spliced_covering_tube.lean`
was archived similarly. `cyclic_controller_pumping.lean`,
`continuation_controller_family_algebra.lean`, and
`nbonacci_margin_catalogue.lean` were NOT archived -- they remain the
authoritative source for content that isn't (and, for the first two, can't
yet be) fully instantiated, matching how `class_ii_neighbor_dominance.lean`
was already handled earlier in this document.

Net effect: of the 19 zero-reference files flagged in item 4, 9 are now
fully or partially generative and kernel-checked (6 fully archived +
replaced, 3 partially instantiated and kept as the authoritative source),
2 remain genuinely open research (the FULL closure forms of the shell-pump
and controller-family theorems -- not the general lemmas themselves, which
are proven, but their application to an actual complete, self-closing
n-bonacci family), and the remaining 5 are pre-existing, separately-tracked
threads (Sturm, free-involution, bp-correction) already covered elsewhere
in this document. Item 5 (the ~35 flat `lean/generated/*.lean` catalogue)
and items 6-7 remain open.

## 2026-08-08, same session, later: correction after reviewing the full audit history

The "2 remain genuinely open research" framing directly above needs a
real correction, not a footnote. `cyclic_controller_pumping.lean`,
`universal_dominance_shell_return_validation.lean`, and
`universal_dominance_phase_rank_transport.lean` all belong to the
shell-rank/carry-bound proof route for the flagship universal n-bonacci
boundary-dominance theorem (`rho(G_B(n))=rho(predicted_core(n))` for
every `n>=3`) -- and the research record marks that route as **explicitly
refuted and abandoned** on 2026-08-05 ("universal maximum-shell exclusion
refuted": an infinite triangular-wave counterfamily falsifies the route's
own shell-two induction target; "Do not resume attempts to prove
universal acyclicity of maximum-shell first-return graphs"). The theorem
itself is NOT open research -- it closed the same day, via a completely
different route (the canonical Q/R parent-prefix split), Lean
kernel-checked (`lean/generated/condition_f_joint_qr_playground.lean`,
independently reconfirmed in this environment this session) and now
wired into the reflection pipeline
(`condition_f_joint_dominance_reflection_test.cpp`). Full account:
`docs/NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md`.

This session also found and fixed a genuine, previously-undetected
kernel-check bug in `finite_positive_grammar_majorant.lean` -- the
general theorem behind the Q/R (2-generator), plastic/supergolden
(3-generator), and the found 4/5-generator witnesses (`docs/
DIRECTION_AND_OPEN_THREADS.md` Item A) -- while wiring it into
`make lean-check`'s permanent coverage for the first time
(`finite_positive_grammar_majorant_reflection_test.cpp`).

What this changes about the "2 remain genuinely open" files specifically:
`cyclic_controller_pumping.lean`'s `StrictShellPumpWitness` construction
(this session's earlier work) and `universal_dominance_*`'s closed-relation
instantiation remain valid, real, kernel-checked mathematics -- they are
just no longer load-bearing for the theorem the audit's earlier framing
implied they were working toward. They stand on their own as general
infrastructure, same register as Barge-Diamond. What genuinely IS open is
narrower than "the universal dominance theorem": whether an unbounded
family requiring arbitrarily many primitive generators exists at all
(`general_generator_theorem.hpp`'s own flagged, not-yet-constructed
extension) -- a different, harder question about the general (non-n-bonacci)
Pisot substitution case, not this file's target.

## 2026-08-08, same session, later still: item 5's `lean/generated/` catalogue,
## first real pass, plus two more genuine bugs found and one dead file archived

Extended `lean-check`'s coverage to the 24 files under `lean/generated/`
that had never been checked at all (confirmed by diffing every filename
against `lean-check`'s own `lake env lean` invocation list). Running it
surfaced real bugs in 8 of them -- Mathlib API drift (`∑ i in s` deprecated
for `∑ i ∈ s`; `OrderedAddCommMonoid` split into `[AddCommMonoid]
[PartialOrder] [IsOrderedAddMonoid]`; `Rat.norm_cast_real` needed instead
of `norm_num` alone), genuine proof gaps (`Finset.sum_comm` alone doesn't
close a relabelling identity; a missing `Function.Injective f` hypothesis
without which `labelled_subgraph_intertwiner` is FALSE, not merely
unproved -- multiple sources can share a target under a non-injective
map, breaking a single pointwise bound), and one structurally serious bug
in the generator itself, not just its output:
`symbolic_residual_induction.hpp`'s `render_residual_induction_lean`
emitted `def pre | d, q => ⟨match ..., by native_decide⟩` with `d`/`q`
already lambda-bound before `native_decide` ran on the proof obligation --
`native_decide`/`decide` cannot evaluate a goal containing free variables,
so this 2446-line, 604-state file (real, regeneratable via
`make first_return_completeness_prover`, not hand-authored) had
apparently never actually kernel-checked despite living at a canonical
`lean/generated/` path since 2026-08-04. Fixed the EMITTER (not just the
output) to prove the bound once as a closed, universally-quantified
`native_decide` lemma (`preRaw_lt : ∀ d q, preRaw d q < residualCount`)
and apply it per call site -- regenerated, kernel-checks clean.

One file, `sign_flux_rank_playground_2026-08-04.lean`, was archived
rather than fixed: zero C++ references anywhere (confirmed by content
grep, not filename pattern alone), and the project record shows it was built
specifically for the "sign-flux chamber rank" / "boundary-sensitive
sparse flux rank" search -- the shell-rank/carry-bound approach project record
2026-08-05 records as explicitly refuted by a triangular-wave
counterexample ("Do not resume attempts to prove universal acyclicity of
maximum-shell first-return graphs"). Distinguished from the other
newly-checked files precisely: several of those (`adjacent_competitor_macro_profile.lean`,
the `cyclic_splice_*` pair) also trace back to the shell-rank/carry-bound
era's earlier "graded core descent" sub-approach, but that sub-approach
was merely SUPERSEDED by the simpler Condition-F route, never shown
false by a counterexample the way the chamber-rank hypothesis was --
kept as legitimate, kernel-checked general infrastructure, same register
as Barge-Diamond, not archived.

Remaining from item 5: ~50 further `lean/generated/*.lean` files not yet
individually triaged this pass (most already have a working C++
producer per the earlier `_batch.lean` naming convention and were
excluded from this pass's candidate list on that basis, but that
exclusion was by grep match on filename-as-substring, not independently
re-verified content-by-content).

## 2026-08-08, continued -- lean/generated orphan sweep (21 files with zero generative backing)

Following the item-5 triage round noted above, a deeper pass checked every
`lean/generated/*.lean` file NOT covered by a `_batch.lean`-suffix writer
test for whether ANY test actually writes it to disk. This surfaced 21
files previously miscounted as "checked, no fix needed" that in fact had
**zero** C++ backing whatsoever -- no `stage_*`/`record()` call, no
`ofstream` write, sometimes no test at all. All 21 were individually
resolved this pass:

**Wired to genuine Pipeline A generation this pass (18 files, all
kernel-check clean, zero errors):**

```
third_smallest_pisot_parry_factorization_batch.lean   (new; flat original archived)
multi_compartment_closure_validation.lean              (real machinery, missing ofstream)
generalized_multinacci_admissible_subgrammar.lean
generalized_multinacci_general_m.lean
generalized_multinacci_general_m_intertwiner.lean
generalized_multinacci_primitive_intertwiner.lean
generalized_multinacci_signed_renewal_twist.lean
generalized_multinacci_symbolic_embedding.lean
monotone_profile_corridor_closure.lean
norm_weighted_qr_majorant.lean
plastic_three_generator_intertwiner.lean
supergolden_three_generator_intertwiner.lean
shift_branch_three_generator_continuation.lean
shared_polynomial_closure_validation.lean              (real machinery, missing ofstream)
coefficient_profile_parity_obstruction.lean
cyclotomic_obstruction.lean
cyclic_splice_compactness.lean
cyclic_splice_completion.lean
```

Two GENUINE kernel-check bugs found and fixed while wiring these (not just
API drift -- real defects that would have failed `lake env lean` the first
time anyone actually ran them):

1. `norm_weighted_qr_majorant.lean`: `norm_qr_word_majorant` and
   `contractive_qr_word_majorant` were missing `[NormOneClass α]` (same
   class of bug as `finite_positive_grammar_majorant.lean`, fixed earlier
   this session) -- `‖(1:α)‖ ≤ 1` is not guaranteed by plain `NormedRing`.
2. `shared_polynomial_closure.hpp`'s `geometric_recurrence_lean_declaration`
   emitter (a real, pre-existing Pipeline-B generator used by both
   `multi_compartment_closure_validation` and `shared_polynomial_closure`
   test families) emitted `def ... : ℕ → Polynomial ℤ` without
   `noncomputable`, which fails to compile since `Polynomial.instOne` is
   itself noncomputable. Fixed at the source (the emitter), not just the
   generated output, so every future run of `shared_polynomial_closure_test`
   now emits correct text.

**Consciously kept as legitimate general infrastructure (4 files, NOT
flat-proof violations)** -- fully abstract, unconditionally kernel-checked
theorems with no free numeric parameter to instantiate, matching the
Barge-Diamond precedent ("a general, certificate-shaped lemma is not flat
merely for lacking a concrete instance"), and in three of the four cases
directly supporting other kept infrastructure from the same era
(`cyclic_splice_compactness`/`cyclic_splice_completion`, both now
genuinely wired above):

```
property_f_zero_walk.lean            -- decided earlier this pass
adjacent_competitor_macro_profile.lean
concrete_operator_qr_spectral_closure.lean
first_return_obligations_discharged.lean
```

`first_return_obligations_discharged.lean` has a real, matching C++
function (`ravel::proof::discharge_first_return_obligations`, exercised by
`tests/first_return_obligation_discharge_test.cpp`) but that test depends
on external JSON artifacts (`out/cover_tube_defect_splice/n{3,4}_M{2,3}.json`)
not present in this checkout and not regenerated by any build target found
this pass -- wiring it to the Lean file would require first tracking down
or regenerating that JSON, out of scope for this pass. The Lean content
itself is fully general/abstract regardless, so leaving it unwired does not
violate the flat-proof bar; it is flagged here as a legitimate follow-up
if the JSON generator is ever located.

All 18 newly-wired files' writer tests are added to (or confirmed already
present in) `make lean-check`'s invocation list and the sorry-scan `rg`
file list, and all have `TEST_BIN_*`/target Makefile entries (several
pre-existing test files turned out to have NO build target at all despite
sitting at canonical paths -- `generalized_multinacci_general_m_test`,
`generalized_multinacci_general_m_intertwiner_test`,
`generalized_multinacci_signed_renewal_twist_test`,
`monotone_profile_corridor_closure_test`,
`plastic_three_generator_intertwiner_test`,
`supergolden_three_generator_intertwiner_test`,
`shift_branch_three_generator_continuation_test`,
`coefficient_profile_parity_obstruction_test` -- all added). Two
pre-existing build rules (`cyclic_splice_compactness_test`,
`cyclic_splice_completion_test`) were also missing `-Imath/include`/
`-Imath/include/mini-gmp`/`math/out/libmath.a` needed once they started
using `math/proof_reflection.hpp`; fixed.

**Correction to this doc's own earlier text**: the "General infrastructure
for fully abstract lemmas" pattern introduced this pass
(`mathlib::reflection::GeneralInfraSweepConfirmedCertificate` /
`confirm_general_infra_sweep`) is a NEW, lighter-weight staging mechanism
for the specific case where a lemma is already unconditionally proven for
every instance a C++ sweep could check -- it deliberately does NOT
fabricate per-instance numeric corollaries for facts that don't have
natural numeric witnesses (rejected, mid-pass, an early draft that would
have emitted a misleading `boundary_expanded_states ≤ universal_expanded_states`
claim for `generalized_multinacci_primitive_intertwiner` before checking
the real data showed that inequality is FALSE in general -- the correct,
data-verified relationship `mapped_phase_states ≤ boundary_expanded_states`
was used instead, confirmed true across all 21 sampled (D,m) pairs before
being committed to the renderer).

An earlier draft of the `cyclic_splice_completion_lemma_lean()` renderer
text was written from theorem-name memory (via
`LEAN_THEOREM_INVENTORY_2026-08-04.md`'s name-only listing) rather than by
reading the actual file, and did NOT match -- caught and corrected before
being kernel-checked or committed as a "verbatim reproduction" claim. This
is exactly the failure mode the project's standing "read the actual
source, don't take shortcuts on reference" instruction targets; flagging
it here as a reminder that even internal doc summaries are not a
substitute for reading the file being reproduced.

**Net effect**: as of this pass, zero `lean/generated/*.lean` files have
no discoverable, real, kernel-check-clean C++ backing (either direct
Pipeline A staging or the lighter general-infra confirmation), except the
one genuinely-open item (`first_return_obligations_discharged.lean`'s
JSON dependency) and the (already-decided-in-a-prior-pass) `lean/*.lean`
root-directory files, which were out of scope for this specific sweep and
remain a separate, larger, not-yet-fully-retriaged body of work (48 files,
~22 of them absent from `lean-check` per the earlier count in this doc).
