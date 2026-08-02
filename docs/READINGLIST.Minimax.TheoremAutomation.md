# Reading list: Minimax theorem automation

## Bootstrap instruction

On bootstrap, ask AM:

> “Should I resume `docs/READINGLIST.Minimax.TheoremAutomation.md`? The
> periodic carry-bound lemma has its coarse forcing bound formalized. The next
> concrete task is the exact chamber escape certificate.”

If AM says yes, follow this file in order. Do not restart old corona
enumerations and do not claim an arbitrary-`n` theorem from finite runs.

## Purpose and mathematical context

The immediate missing lemma is:

> If an integer sequence `a_t` is periodic and
> `d_t = -a_t + a_(t+1) + ... + a_(t+n)` belongs to `{-1,0,1}` for every
> `t`, then every `a_t` belongs to `{-1,0,1}`.

This is the periodic carry-bound lemma. It bridges the finite carry automaton
to the universal n-bonacci dominance theorem. Split it into:

1. exact `(n+1)`-block algebra;
2. a uniform bound on block forcing;
3. strict rank escape outside the ternary layer;
4. contradiction with periodicity.

Piece 1 is complete. This file is a practical skill-style handoff for
replaying it and continuing with pieces 2–4.

## Start safely

Run:

```sh
cd ~/claude/RavelMathPub
git status --short
```

Normally the only unrelated untracked context file is `automation.txt`.
Do not edit or delete it. Preserve other user changes; never run
`git reset --hard`.

All Python probes use a 10 GiB virtual-memory cap by default. Keep it. Do not
set `RAVEL_PROBE_MEMORY_MB=0` without explicit authorization.

## Read the project math before editing

Before changing a theorem or probe, read these in order:

1. `docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md` — current theorem
   obligations, evidence, and known negative results;
2. this file — execution order and acceptance gates;
3. `lean/nbonacci_margin_catalogue.lean` — kernel-checked algebra;
4. `tests/nbonacci_block_identity_test.cpp` — independent exact regression;
5. `python/nbonacci_block_forcing_probe.py` and
   `python/nbonacci_sign_chamber_probe.py` — finite automation;
6. the relevant Mathlib source, using `rg` before guessing lemma names.

Example library lookup:

```sh
MATHLIB=~/claude/LEAN/free_involution_perron/free_involution_perron/.lake/packages/mathlib/Mathlib
rg -n "det_succ_row|det_of_upperTriangular|sum_le_sum" "$MATHLIB"
```

Then inspect the surrounding declaration, not just the grep line:

```sh
sed -n '760,790p' "$MATHLIB/LinearAlgebra/Matrix/Determinant/Basic.lean"
```

This prevents wasting a work block on a guessed theorem name or the wrong
typeclass assumptions. Mathlib is part of the proof environment: if a
declaration is useful, record its exact name and assumptions in the project
document.

## Replay completed certificates

Run these in order:

```sh
cd ~/claude/RavelMathPub
make nbonacci_block_identity_test
./out/nbonacci_block_identity_test
make nbonacci_block_forcing_probe
make nbonacci_block_spectrum_probe
make lean-check
```

Expected important lines:

```text
nbonacci_block_identity_test: 858 checks, 0 failures
Enrolled Lean files are sorry-free.
```

The forcing probe checks every word through `n=10`, reports maximum
coordinate `4`, and observes sharp L1 values `2*n+2` for even `n` and
`2*n` for odd `n`. The spectrum probe is diagnostic only.

## Existing formal machinery

`lean/nbonacci_margin_catalogue.lean` contains:

- `nbonacci_block_identity_scalar`;
- `nbonacci_block_identity_matrix`;
- `nbonacci_geomSum_of_charpoly`;
- `nbonacci_block_identity_of_charpoly`;
- the explicit `inverseCarryMatrix`;
- executable finite geometric-sum checks for dimensions `2,...,8`.

The C++ regression independently checks the companion characteristic
polynomial, geometric sum, block power identity, and forcing formula for
`2 <= n <= 40`. Do not silently replace the universal characteristic
polynomial calculation with those finite checks.

## Completed subtask: Lean forcing bound

Prove the coordinate statement:

```text
-4 <= F_i(d_0,...,d_n) <= 4
```

under `-1 <= d_r <= 1`. Use this signed interval form before introducing
absolute values. Then prove `sum_i |F_i| <= 4*n`; the sharp
parity-dependent L1 formula is optional.

The definitions `nbonacciBlockDigit` and `nbonacciBlockForcing` and the
theorems `nbonacciBlockForcing_coord_bound` and
`nbonacciBlockForcing_l1_bound` now compile in the enrolled Lean file. The
coarse bound is the accepted one for the escape proof:
`-4 <= F_i <= 4` and `sum_i |F_i| <= 4*n`. The sharper parity-dependent bound
from the Python enumerator is optional.

The original implementation order, useful if this lemma must be reconstructed
on another substrate, was:

1. define a `Fin (n+1) -> Z` digit word;
2. define first, interior, and last coordinates with dependent `if`
   branches so the `Fin` bounds exist in the relevant branch;
3. prove each case with `omega`;
4. prove the L1 bound with `Finset.sum_le_sum` and `abs_le.2`;
5. add executable examples for `n=2,3`;
6. run `make lean-check` before committing.

The next live task is the exact chamber escape certificate below.

If `Fin` bookkeeping blocks progress, first prove the inequality over natural
indices with explicit bounds, then add a thin `Fin` conversion lemma.

## Next task: exact chamber escape certificate

The candidate rank is:

```text
R(x) = sum_i |x_i| + offset(chamber(x)).
```

The chamber is the exact sign/gap/residue chamber generated by
`python/nbonacci_sign_chamber_probe.py`, with modulus `n+1`. The offset is
an integer solution of strict difference constraints on transient edges.

Start small:

```sh
RAVEL_PROBE_MEMORY_MB=10240 python3 \
  python/nbonacci_sign_chamber_probe.py \
  --n=3 --bound=8 --mode=gaps-mod --modulus=auto --rank-base=sum
```

Extend one dimension or bound at a time. Record chamber count, largest
quotient SCC, rank feasibility, peak RSS, and the exact command. A
`MEMORY_LIMIT`/`INCONCLUSIVE` result is not a mathematical failure.

The promotion artifact must contain canonical chamber IDs, every block
transition, integer chamber offsets, and a check that every exterior edge
increases `R`. Generate one with `--emit-certificate=PATH`, then replay it
with `python3 python/nbonacci_chamber_certificate_check.py PATH`; the second
checker replays every recorded integer inequality (which also rules out any
positive difference-constraint cycle). Floating eigenvalues
are orientation only and must not enter the certificate.

To test whether a finite rank stabilizes as the box grows, compare two
already-emitted certificates without rebuilding either graph:

```sh
python3 python/nbonacci_chamber_stability.py \
  /tmp/n3_b8_rank.json /tmp/n3_b12_rank.json
```

This checks metadata, shared chamber names, translation-equivalence of the
canonical integer offsets, and whether the larger offsets still satisfy every
smaller weighted edge. `offset_translation=DIFF` is evidence against a
uniform chamber rank, not a checker failure; the edge-violation count is the
sound replay result. The Make target `nbonacci_chamber_stability` runs the
bounded n=3 example end to end.

For the stronger cross-box test, use `python/nbonacci_chamber_merge.py` (or
`make nbonacci_chamber_merge`). It identifies chambers by their symbolic name,
unions the weighted constraints from multiple boxes, and searches for one
common integer potential. The n=3 bounds 8 and 12 merge successfully, even
though their independently normalized longest-path offsets differ.
The same merged potential survives fresh bounds 16 and 32 (98,996 chambers
and 285,872 weighted edges at bound 32); bound 16 introduces a 162-node
quotient SCC, and both still pass the merged difference constraints. The
merge solver uses SCC-local propagation so this remains practical as the
certificate grows.
The same cross-box merge passes for `n=4`, bounds 8 and 10: 184,450 chambers
and 349,088 weighted edges. Extending once more to bound 12 gives 352,178
chambers and 701,448 weighted edges, again with one shared potential.
The existing certificates also merge at `n=5`, bounds 4 and 6 (371,198
chambers, 611,658 edges), and `n=6`, bounds 3 and 4 (531,186 chambers,
812,926 edges). These are promotion tests across dimensions, not yet a
uniform-in-`n` theorem.

The replay checker also enforces the exact involution `x ↦ -x`: every chamber
must have its sign-negated partner with the same offset. This is a finite
certificate reduction, not a numerical heuristic, and can be carried into
the formal exterior proof.

`python/nbonacci_rank_feature_search.py` is the next discovery gate. It solves
the recorded edge inequalities for small linear feature families. On the
current n=3, bound-8 certificate, sign, gap-linear, quadratic, and richer
low-degree families are all infeasible. This rules out several tempting
closed-form ranks without spending another graph-enumeration run. It is a
negative discovery result, not a universal impossibility theorem.

An abstraction audit is also informative: `sign`, `ordered`, and `gapcap`
quotients fail their strict-rank tests, while the exact `gaps` grammar passes.
For n=3, bounds 8 and 12, the residue-free exact-gap certificates merge over
4,964 chambers and 14,302 edges. Any universal template must retain this
unbounded gap information; finite sign/order summaries are too coarse.

Example:

```sh
RAVEL_PROBE_MEMORY_MB=10240 python3 \
  python/nbonacci_sign_chamber_probe.py \
  --n=3 --bound=8 --mode=gaps-mod --modulus=auto --rank-base=sum \
  --emit-certificate=/tmp/n3_b8_rank.json
python3 python/nbonacci_chamber_certificate_check.py /tmp/n3_b8_rank.json
```

The same path has passed one higher-dimensional stress point, `n=4,bound=8`:
82,194 chambers, 149,778 weighted edges, and a 9.2 MiB certificate replayed
successfully under the 10 GiB cap. Increase only one bound or dimension at a
time after recording the result. A controlled next-dimensional run also
passes at `n=5,bound=4`: 58,954 chambers, 96,824 weighted edges, and a 6.3 MiB
certificate; its quotient has no nontrivial SCCs.
A further controlled point passes at `n=6,bound=3`: 117,394 chambers,
178,978 weighted edges, and a 13 MiB replayable certificate, again with no
nontrivial quotient SCCs.
The `n=8,bound=2` shell also passes: 388,678 chambers, 523,824 weighted
edges, and a 41 MiB replayable certificate, with no nontrivial quotient SCCs.
Comparing `n=3` certificates at bounds 8 and 12 gives a useful negative
result: 3,798 of 4,388 shared chamber signatures change canonical offset.
Do not try to parameterize the theorem using only bounded residues; absolute
gap scale must remain visible in the rank argument.
Recording `floor(min_abs/(n+1))` as an additional chamber coordinate also
fails to stabilize the potential: at `n=3`, bounds 8 and 12 differ on 4,042
of 4,898 shared signatures. The scale dependence is in the exact gap geometry,
not a single radial quotient.
The capped vectorized raw-block test (`make nbonacci_block_l1_growth_probe`)
also confirms that raw `L1` is insufficient: minimum changes are `-15` at
`n=3,B=8` and `-13` at `n=4,B=6`. The chamber/coboundary correction is
mathematically necessary, not just an implementation choice.
The larger known passing shell `n=5,bound=6` also emits and replays
successfully: 371,198 chambers, 611,658 weighted edges, and a 41 MiB
certificate. Treat this as a stress result, not as a universal theorem.
The next shell `n=6,bound=4` also passes: 531,186 chambers, 812,926 weighted
edges, and a 57 MiB replayable certificate, with no nontrivial quotient SCCs.
The `n=7,bound=3` shell also emits and replays successfully: 822,844
chambers, 1,181,154 weighted edges, and an 87 MiB certificate, again with no
nontrivial quotient SCCs.

## Periodic contradiction

The finite telescoping endpoint is already kernel-checked as
`no_strict_rank_finite` in `lean/nbonacci_margin_catalogue.lean`: a strict
integer rank increase on a finite state set is impossible. Once the exterior
rank certificate is exact, instantiate that theorem on the exterior chamber
states. The orbit then remains in the ternary layer (or in the explicit fixed
hull, if the weaker theorem was proved).

## Downstream full-theorem checklist

After the carry lemma closes, audit:

1. core identification: grade-1 cyclic block equals the predicate in
   `include/ravel/nbonacci_margin_invariant.hpp`;
2. grade exhaustion: every other cyclic orbit lies in a higher-support grade
   or a terminal permutation component;
3. uniform dominance: each nonterminal grade has a positive intertwiner or
   subeigenvector certificate.

Use the existing dominance ledger, arithmetic-hull programs, and intertwiner
searches as certificate generators. They are not universal proofs until their
parameterized identities and boundary cases are checked.

## Tool-call examples

For a short check, call the shell tool synchronously:

```text
exec_command({
  cmd: "make nbonacci_block_forcing_probe",
  workdir: "/home/anonymous/claude/RavelMathPub",
  yield_time_ms: 30000
})
```

For a long probe, start it with the memory cap and a short initial wait,
then poll the returned session:

```text
exec_command({
  cmd: "RAVEL_PROBE_MEMORY_MB=10240 python3 python/nbonacci_sign_chamber_probe.py --n=4 --bound=15 --mode=gaps-mod --modulus=auto --rank-base=sum",
  workdir: "/home/anonymous/claude/RavelMathPub",
  yield_time_ms: 1000
})
write_stdin({ session_id: SESSION_ID, chars: "", yield_time_ms: 30000 })
```

If a process exceeds the agreed cap or starts scaling unexpectedly, stop it
by its recorded PID/session, record the outcome as `INCONCLUSIVE`, and do
not reinterpret it as a failed theorem.

For file edits, use `apply_patch`. After a coherent edit:

```sh
make lean-check
make nbonacci_block_identity_test
git diff --check
git status --short
git add <specific-files>
git commit -m "describe the proof seam"
```

## Documentation loop

Update `docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md` whenever a seam
changes. Add:

- the exact command;
- parameters and memory cap;
- PASS/FAIL/INCONCLUSIVE status;
- counts, bounds, or certificate hashes;
- what the result proves and what it does not prove;
- the next uncompleted obligation.

Do not update documentation with a guessed result. Run the command first and
copy the relevant output. Do not edit `automation.txt`; it is context, not
the durable theorem record.

## Handoff rules

Keep long probes under the 10 GiB cap and background runs likely to exceed five
minutes. Commit a coherent checkpoint after each proof seam. End each session
by naming the exact next uncompleted step. On bootstrap, ask about this file
before starting unrelated experiments.
