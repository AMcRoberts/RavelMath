# The universal n-bonacci boundary-dominance theorem is closed

**For every `n >= 3`: `rho(G_B(n)) = rho(predicted_core(n))`.**

This is the theorem `docs/UNIVERSAL_NBONACCI_DOMINANCE_CAMPAIGN.md` set out
to prove. It is closed -- not at the finite-n-checked tier, not at the
high-precision-numeric tier, but Lean kernel-checked, unconditionally in
`n`. This document is the current, authoritative status; the campaign doc
and the shell-rank/carry-bound documents listed below describe an earlier,
abandoned route to the same target and should be read as historical, not
current.

## What actually closed it

Not the shell-rank / carry-bound route (`NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`,
`NBONACCI_ARITHMETIC_DOMINANCE_THEOREM_N3_N7.md`,
`PARAMETRIC_SHELL_RANK_TRANSPORT_2026-08-03.md`). That route was explicitly
**refuted**: diary 2026-08-05, "universal maximum-shell exclusion refuted"
-- an infinite triangular-wave counterfamily (for every `M>=1, q>=1`,
dimension `n = q*(4M)+1` admits a closed period-`4M` shell-`M` carry cycle)
falsifies the shell-two induction target the whole route depended on. The
diary's own instruction: "Do not resume attempts to prove universal
acyclicity of maximum-shell first-return graphs."

What actually closed the theorem is the **canonical Q/R split**
(`docs/archive/2026-08-05_round_reports/CONDITION_F_CANONICAL_TWO_MATRIX_REPORT_2026-08-05.md`,
`CONDITION_F_JOINT_QR_COMPARISON_REPORT_2026-08-05.md`), attributed in the
diary to AM's root-cause insight (comparing this problem to the earlier
universal-n kernel proof and looking for one base cause whose extensions
appear as phase/holonomy/fibre/multiplicity/matrix-pair twists):

1. Every n-bonacci parent-prefix decomposition is either `[]` or `[0]`,
   so every transport edge's prefix defect `delta = |q|-|p|` lies in
   `{-1,0,+1}`. This forces exactly two canonical generators: `Q`
   (`delta=0`, balanced/bulk transport) and `R` (`|delta|=1`,
   one-sided/residual transport) -- the sign of `delta` is fibre
   voltage/holonomy, not a third matrix.
2. At the minimal alphabet (`D=2`), the parent-role graph has exactly one
   recurrent SCC, so the core pair is the whole recurrent pair and the
   identity matrix is trivially a simultaneous `Q`/`R` intertwiner.
3. Under `D -> D+1`, every old role/channel persists; every new role lies
   in a finite-depth terminal-parent boundary, eliminable by the same
   nonnegative noncommutative `Q`/`R` polynomial applied to both the
   quotient pair and the core pair.
4. Because Q/R-word intertwining is polynomial-monotone (`qr_word_intertwiner`
   / `qr_polynomial_intertwiner`), the identity base comparison propagates
   to every dimension by ordinary natural-number induction
   (`jointQR_dimension_induction`), giving spectral maximality of the
   predicted core in the untwisted finite Condition-F quotient. Voltage/
   Fourier domination transports this to all twisted sectors; the
   terminal/branching reduction transports it to the literal carry system.
   The final order-theoretic close is `universal_dominance_sandwich`.

## The checked artifact

`lean/generated/condition_f_joint_qr_playground.lean` --
`qr_word_intertwiner`, `qr_polynomial_intertwiner`,
`jointQR_dimension_induction`, `universal_dominance_sandwich`. Diary
2026-08-05 records AM confirming this passed the Lean kernel with no
messages at the time. **Independently reconfirmed in this environment,
this session (2026-08-08):** `lake env lean` on that file exits 0, zero
errors, zero `sorry`, against the pinned Lean 4.32.1 + Mathlib toolchain
this project's `make lean-check` uses.

The C++ side (`ravel::proof::derive_condition_f_joint_pair_comparison`,
`include/ravel/proof/condition_f_joint_pair_comparison.hpp`) independently
re-derives the base SCC count and the dimension-extension acyclicity from
raw parent-role data; its own test only checked `D=2..128` and never
emitted Lean. This session wired it into the reflection pipeline
(`tests/condition_f_joint_dominance_reflection_test.cpp`,
`lean/generated/condition_f_joint_dominance_batch.lean`), so the general
theorem's own file and a concrete C++-gated instance are now both in
`make lean-check`'s permanent coverage -- neither was before.

## What this means for the shell-rank/carry-bound-era files

The following describe the abandoned route and are historical, not
current status, though their content is not deleted (some pieces --
`radial_translation_defect.lean`, `covering_translation_tube.lean`,
`defect_spliced_covering_tube.lean`'s pointwise algebra, the
`no_strict_rank_relation_closed` pigeonhole lemma -- remain independently
true general infrastructure, just no longer load-bearing for *this*
theorem):

- `NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`
- `NBONACCI_ARITHMETIC_DOMINANCE_THEOREM_N3_N7.md`
- `PARAMETRIC_SHELL_RANK_TRANSPORT_2026-08-03.md`
- `UNIVERSAL_NBONACCI_DOMINANCE_CAMPAIGN.md` (names the four "genuinely
  open universal seams" this route needed; none of them were the route
  that actually closed it)
- `STRICT_SHELL_PUMP_IMPLEMENTATION_2026-08-04.md`
- `CYCLIC_CONTROLLER_PUMPING_2026-08-04.md`
- `UNIVERSAL_SHELL_PUMPING_PROOF_2026-08-04.md`
- `SOURCE_CONDITIONED_DEFECT_SPLICE_2026-08-03.md`
- `UNIFORM_RADIUS_ONE_SYNTHESIS_2026-08-04.md`
- `SYMBOLIC_CONTROLLER_CHARACTERIZATION_2026-08-04.md`
- `COUPLED_BACKWARD_TRANSFORMER_CHARACTERIZATION_2026-08-04.md`
- `RECURRENT_FAMILY_EXHAUSTION_2026-08-04.md` and
  `RECURRENT_FAMILY_EXHAUSTION_SEAM_STRATEGY.md` -- their finite (n=6)
  results stand; their stated dependency on an "all-n induction" /
  "universal carry-bound theorem" is moot, since the theorem closed via a
  route that never needed it.
- `PREDICTED_CORE_SCC_EXHAUSTION_2026-08-04.md` -- same: the n=3..7
  finite exhaustion result stands; its stated dependency on "the
  universal carry-bound theorem... placing every recurrent literal state
  inside the exact ternary arithmetic hull" is moot.

Each of these has been given a short banner pointing here. None were
deleted; several (`RECURRENT_FAMILY_EXHAUSTION_2026-08-04.md`,
`PREDICTED_CORE_SCC_EXHAUSTION_2026-08-04.md`,
`PREDICTED_CORE_SCC_IDENTIFICATION_2026-08-04.md`) still correctly
describe finite results this project's Lean bridge now depends on
(`recurrent_family_exhaustion_reflection_test.cpp`,
`predicted_core_scc_exhaustion_reflection_test.cpp`), just not the
"universal in n" framing their own text originally carried.

## Multi-generator generalization

The Q/R split is itself a specialization of a more general theorem:
`lean/generated/finite_positive_grammar_majorant.lean`'s
`generator_word_intertwiner` / `norm_generator_word_majorant` are stated
for an arbitrary, possibly-unboundedly-large finite (or infinite, for the
induction itself) generator alphabet `Γ`, not fixed at two. Concretely
witnessed generator counts elsewhere in this project range from 2 (Q/R,
n-bonacci) through 3 (plastic, supergolden, Q/R/S) to 5 (the found
`x^3-2x^2-2`, `x^3-x^2-2x-2`, `x^3-2x^2-x+1` witnesses,
`docs/DIRECTION_AND_OPEN_THREADS.md` Item A). Whether an unbounded family
requiring arbitrarily many generators exists is flagged in
`general_generator_theorem.hpp` as a genuine open extension, not yet
constructed. This session wired `finite_positive_grammar_majorant.lean`
into `make lean-check`'s permanent coverage for the first time
(`tests/finite_positive_grammar_majorant_reflection_test.cpp`), fixing a
genuine latent kernel-check bug found in the process (`‖(1:α)‖ ≤ 1` is not
guaranteed by plain `NormedRing`; needs `[NormOneClass α]`).
