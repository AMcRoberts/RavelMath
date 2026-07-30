# Recovery audit — 2026-07-29

This audit records verified discrepancies between claims, executable
checks, and theorem strength. It is not a replacement for
`docs/THEOREM_STATUS.md`.

## Audit rules

- A green executable is evidence only for conditions that affect its
  exit status.
- A Lean file containing `sorry` is not described as fully
  kernel-checked.
- A copied finite catalogue proves facts about that catalogue, not its
  equality with the C++ catalogue or its occurrence in the boundary
  graph.
- Numerical sweeps remain explicitly bounded.

## Findings

### A1 — Neighbor-D test contains non-asserting checks

Severity: high for validation language; low for the underlying affine
catalogue fact.

`tests/class_ii_neighbor_d_matrix_test.cpp` prints failures for the
expected nonzero count and the 0-1 property of `D`, but neither
condition contributes to the final `ok` or exit status.  The
q-expansion section computes values and discards them; it makes no
comparison and cannot fail.  Therefore the executable currently
checks only `certificate.affine_exact` and
`certificate.boundary_layer_exact`.

The affine identity itself is nearly tautological in this path:
`class_ii_neighbor_affine_matrix` and `class_ii_neighbor_d_matrix`
are both constructed from the same `AffineEdge::{intercept,slope}`
catalogue.  The test does not independently derive the matrices from
substitution transitions.

Targeted execution on 2026-07-29 passed, but the output phrase
“qexpansion gate consistent” overstates what was asserted.

Remediation: the expected nonzero counts, 0-1 slope condition, and
monotone decrease of the corrected q-expansion truncation error now
all contribute to `ok` and the exit status. The separate caveat
remains: these checks protect the explicit affine catalogue but do
not independently derive it from substitution transitions.

### A2 — Six-vertex Lean result is a finite shape check, not a rank theorem

Severity: high for theorem naming and documentation; no evidence that
the seven displayed states themselves are wrong.

`lean/class_ii_six_vertex_graduation.lean` defines six nodes and one
transferred node locally.  It proves:

- the displayed list always has length six;
- distinctness and disjointness at the finite values
  `q = 4..10, 20, 100`;
- `6 + 1 = 7`.

It does not connect those definitions to
`class_ii_neighbor2_penultimate_promoted_forms()`, the computed Red
ranks, corona occurrence, or the cubic margin identity.  It therefore
does not prove that any role graduates from one rank to another.
`TODAY.md` calling it a “closed-form kernel-checked theorem for the
six-vertex graduation” is too strong.  The more cautious wording in
the earlier cautious wording (“partial” and “at ten parameter values”)
is closer, though “kernel-checks the closed-form parametric shape” still
needs the cross-language catalogue connection.

The aggregate theorem also omits `q = 20, 100` even though separate
lemmas for those values exist.

Remediation: `six_vertex_catalogue_shape_universal` now proves the
distinctness, disjointness, and seven-role count for every `q>=4`.
The occurrence and Red-rank bridge remains open.

### A3 — `make lean-check` permits admitted proofs

Severity: high for interpreting the aggregate validation target.

The target completed successfully on 2026-07-29 while Lean warned
about declarations using `sorry` in:

- `lean/bp_correction_determinant.lean`;
- `lean/class_ii_terminal_shells.lean`.

The non-enrolled draft
`lean/free_involution_perron_existence_draft.lean` also contains three
explicit `sorry`s, but is already labelled as a draft.

At discovery time, “`make lean-check` passes” meant only that enrolled
files elaborated. After the remediation below, all enrolled files are
admission-free; the non-enrolled Perron-existence draft still contains
three deliberately marked `sorry`s.

Remediation: the terminal-shell `Nodup` hole is closed, and the
permutation-index reduction in `bp_correction_determinant.lean` is
now proved by factoring through the exact permutation inverse and
applying the block reduction. No enrolled Lean file contains `sorry`;
the ruler-sequence nilpotency input remains an explicit named
hypothesis rather than an admission.

### A4 — The CSY object is not a finite carry automaton

Severity: critical for the claimed CSY closure.

`CSYAutomaton::AutoState` contains the absolute word position.
`build()` merely explores a finite prefix tree up to caller-supplied
`max_pos`, discarding coefficient vectors outside caller-supplied
`bound_bits`. It formerly set `closed_at_bound_ = true`
unconditionally. That reporting bug is now repaired: a frontier
reaching `max_pos` reports truncation, since its zero-digit successor
necessarily exists at the next absolute position.

`acceptsWord()` independently recomputes the exact polynomial value;
for words beyond the artificial depth it bypasses the stored states
entirely. The validation therefore compares `pisotEval` with a thin
wrapper around `pisotEval`, not an independent automaton acceptance
path. The “pumping” test asserts only that the number of truncated
prefix states is positive. No pumping property is tested.

The exact `Q(beta)` power routines and their cross-checks may remain
useful, but the reading-list statement that the CSY automaton hookup
is closed is false.

### A5 — Rauzy unit-distance test has no test and misstates its object

Severity: critical.

`tests/rauzy_fractal_unit_distance_test.cpp` initializes
`exit_code=0` and never changes it. It always passes. Its congruence
set `a^2+b^2+c^2 = 0 (mod 5)` is generally not an additive sublattice
and therefore cannot have the claimed index five. The displayed map
retains three coordinates and is not the stated projection to a
two-dimensional contracting plane. Approximate Euclidean distance in
that coordinate transform is not an exact unit-distance predicate.

The file supplies neither a Rauzy-fractal comparison nor evidence for
an OpenAI/Sawin asymptotic claim. It should be removed from the
evidentiary surface until replaced by a mathematically defined object
with assertions.

### A6 — Unit-distance counter is sounder than its Sawin narrative

Severity: medium for the exact counter; high for interpretation.

The signed displacement enumeration, Jacobi count check, and
small-grid brute-force comparison are meaningful and affect the exit
status. The later “Sawin sanity gate” only prints data. Several
comments alternate between asymptotic pairs per point `r(c^2)/2` and
`r(c^2)/8`; the implemented limit is `r(c^2)/2`. Phase 3 uses
`k<=256` for a case whose reported minimum hosting width is 5009, so
it displays only zeros and cannot illustrate convergence.

The empirical phrase `k/min_k ≳ 200` is not a tested gate or theorem.
Retain the exact finite counter and hosting widths; remove the claimed
Sawin-regime conclusion unless a separately specified quantitative
criterion is proved.

### A7 — Q-expansion verifier concealed a wrong coefficient table

Severity: high.

`delta_qexpansion_verify` initialized `a_inv` to one rather than
`1/a`, returned a signed difference despite documenting an absolute
error, and no test asserted its result. After repairing the verifier,
the old coefficients diverged. Direct formal series substitution into
the Class-II cubic gives
`1,-1,2,-4,9,-23,63,-178,514,-1512,...`, not the documented
Schroeder-like positive tail. The header and executable check have
been corrected. The enrolled neighbor-D test and the complete project
suite now pass with the repaired coefficients.

### A8 — “Bounded LRU” does not bound the CSY call path in bytes

Severity: critical for memory claims.

`BetaPowerCache` bounds only its number of entries. Each exact
`QElem` grows with `n`, so neither its byte footprint nor the cost of
a miss is constant. More importantly, `pisotContextFor` retains a
thread-local `PisotContext` containing the unbounded vector
`beta_powers_cache`. `CSYAutomaton::build`, `acceptsWord`, and
`pisotEval` use that memoized path, bypassing the bounded LRU.

The matrix route avoids the growing vector but uses `O(d^2)` exact
matrix entries whose integer sizes grow with `n`, plus multiplication
temporaries. A sound interface must expose separate limits for cache
entry count, total stored coefficient bytes/bits, exponent/position,
and temporary-work budget. Exact arbitrary-`n` output cannot have a
position-independent memory bound because the output itself grows.

Remediation: `BetaPowerCache` now has a separate stored-coefficient
bit budget and declines to retain an individually oversized value
while still returning it exactly. The thread-local recurrence prefix
is now likewise limited to 64 entries and 1,048,576 coefficient bits,
with exact matrix fallback beyond either bound. This bounds retained
power coefficients, not matrix temporaries, exact result size, or the
number of distinct polynomial contexts in the thread-local map. See
`docs/CSY_MEMORY_EXACTNESS_AUDIT.md`.

### A9 — dangling temporary subobject in terminal target audit

Severity: high for runtime and exactness diagnostics.

`class_ii_neighbor2_terminal_new_survivor_targets` iterated
`class_ii_neighbor2_terminal_pruning_ranks(a)[0]` directly in a
range-for loop. Under the project's C++20 build, the selected set was
a subobject of a destroyed temporary vector. The observed
multi-minute “literal reconstruction” was undefined behavior, not
legitimate exact-computation cost. The same pattern was caught while
adding the post-rank-two helper.

Remediation: retain the rank vector in a named local before iterating
its set. The literal target comparison is now immediate. It also
exposed and corrected a separate two-role omission: the terminal
layer must use the twenty-role `class_ii_terminal_shell(a)` with its
two cross-colour replacements, not the ordinary twenty-role interior
shell. Literal and affine target sets
now agree (69 roles at `a=7,8`, with independent regeneration at
`a=9`).

### A10 — CSY topology test printed failures but always returned success

Severity: high for validation trust.

`pisot_numeration_topology_test` used an `EXPECT` macro that only
printed a failure and never updated the process status.  Four
Class-II homomorphism checks fail because the finite-patch
`group_add` is not the required bi-infinite carry operation, yet the
executable ended with the unconditional line `All tests passed.` The
same masking also applied to every supposedly ordinary assertion in
the file.

Remediation: ordinary `EXPECT` failures now increment a counter and
produce a nonzero exit status.  The four known unfinished
homomorphism checks use a separate `EXPECT_KNOWN_LIMITATION` path and
are reported as `XFAIL`. The rebuilt executable reports
`0 unexpected failures; 4 expected implementation limitations`.
This is honest test accounting; it does not repair the missing CSY
group law.

### A11 — family-of-families “verification” could only warn

Severity: medium for regression protection.

`family_of_families_test` says it verifies that every adjacent swap
preserves the incidence matrix, and it computes the corresponding
boolean, but `analyze_family` returned `void` and `main` returned zero
unconditionally. A broken fiber could only print `WARNING`.

Remediation: the per-family analysis now returns its result, `main`
aggregates every family, and a broken adjacent-swap fiber produces a
nonzero exit status. The rebuilt pilot remains green.

### A12 — “exact integer charpoly” used overflowing `long long`

Severity: critical for polynomial certificates.

UBSan reached `charpoly_int` through the neighbor theorem test and
reported signed overflow while forming matrix powers, traces, and
Newton sums. The public comments called this an exact integer
characteristic polynomial, but its intermediates were `long long`;
wraparound was undefined behavior even when cancellation left the
final displayed coefficients small.

Remediation: `charpoly_int` now delegates all arithmetic and the
Cayley--Hamilton self-check to mathlib's arbitrary-precision
`charpoly_faddeev_leverrier`, then converts the final descending
coefficients to `long long` with a checked conversion. It throws with
directions to use the `PolyZ` API if a true coefficient does not fit.
The rational-root evaluation used by the survey now has a separate
arbitrary-precision zero predicate; the legacy numeric return also
throws rather than overflowing. The neighbor certificate test and
survey test pass, and a fresh UBSan neighbor run reports no overflow.
The exact route makes the large neighbor test slower (about 33 seconds
locally) but removes a false exactness claim.

Residual boundary: many older consumers still store characteristic
polynomials as `vector<long long>`. They are now safe (checked) but
not arbitrary-precision at the API surface. Any workflow expected to
handle genuinely larger final coefficients should migrate its result
type to `PolyZ` rather than catching the overflow and silently
weakening the comparison.

## Still credible after this pass

- The copied affine catalogues in
  `lean/class_ii_neighbor_d_support.lean` have the stated finite
  cardinalities and support index sets; Lean checks those literal
  lists.
- The generic affine identity
  `A(a) = A(3) + (a-3)D` is universally valid for an `AffineEdge`
  because it is elementary ring arithmetic.
- The targeted C++ and Lean commands compile and run in the current
  workspace.  This says nothing beyond their actual assertions.

## Remaining audit queue

1. Compare each copied Lean catalogue against a generated,
   independently checkable serialization from C++.
2. Replace or remove the non-evidentiary Rauzy unit-distance test.
3. Implement the actual finite CSY quotient before restoring the
   automaton claim.
4. Migrate large-graph characteristic-polynomial consumers from the
   checked `vector<long long>` compatibility wrapper to `PolyZ`.
