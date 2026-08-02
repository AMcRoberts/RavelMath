# Lean formalization

Run `make lean-check` from the project root. The current workspace uses
Lean 4.32.1 and Mathlib 4.32.1 through the cached Lake environment at
`../LEAN/free_involution_perron/free_involution_perron`.

See `docs/THEOREM_STATUS.md` for exact statements and the project-wide
status vocabulary.

## Signed certificate

Every `*.lean` file in this directory carries an attribution header
(`Author: Ravel`, `Date: 2026-07-29`) and a per-file GPG detached
signature (`*.lean.sig`). The `signatures/` directory holds the
manifest of SHA-256 hashes, its GPG signature, its X.509 signature,
the GPG public key, and the self-signed X.509 certificate. The
matching private material lives in `~/claude/.ravel/keys/`. See
`signatures/README.md` for full verification instructions.

## Checked modules

- `free_involution_perron_core.lean` is kernel checked with no
  `sorry`. It proves involution invariance and quotient eigendescend
  under the explicit `PerronUnique` hypothesis.
- `return_contact_lift.lean` is kernel checked with no `sorry`. It
  proves graph projection and both prefix-occurrence commuting
  equations independently of the C++ representation.
- `bp_correction_determinant.lean` is kernel checked with no `sorry`.
  Its general block determinant lemma, permutation-index
  `main_reduction`, and conditional final corollary are proved. The
  substitution-specific ruler-sequence nilpotency input remains an
  explicit named hypothesis.
- `class_ii_affine_shells.lean` is kernel checked with no `sorry`. It
  proves the twenty affine shell formulas are injective within each
  round, disjoint between rounds, and have infinite parameterized
  range. It also proves a universal predecessor-plus-contact-hop
  identity for every shell kind, explicit signed-contact membership
  of every hop, and universal interior stepped-hyperplane endpoint
  validity from the Class-II cubic. It also checks the fixed
  `9 ⊂ 16 = 14 ⊔ 2` seed/pre-contact/contact catalogue shape and
  proves that exactly nine of the 33 integer face-contact candidates
  satisfy the Class-II Perron window for every `a>=2`. It now derives
  the sharp contact-range gap `2/3 < beta-(a+1/beta) < 1`, proves all
  fourteen contact states and both valid Red exclusions lie in the
  restricted window, and proves the raw category bounds force any
  restricted predecessor into the five integer slices
  `-2<=x0<=2`. Exact C++ affine classification closes the resulting
  255 cases and the `6a+21` prefix multiplicity with zero unresolved.
  The
  separate C++/Lean terminal
  certificates now close neighbor-2 terminal exhaustion and Red;
  full recurrent-catalogue occurrence/exhaustion remains open. A provisional
  survival table specialized from `a=8,q=5` was rejected by Lean:
  several targets used the accidental relation `a=2q-2`, so it was
  removed rather than recorded as a family theorem.
- `class_ii_global_round_partition.lean` is kernel checked with no
  `sorry`. For every `a>=7`, it proves the base, stable, penultimate,
  terminal, and repeated-terminal domains are a total disjoint phase
  selection for all legal rounds `1<=r<=a+1`. This closes the domain
  partition needed to stitch the existing local layer certificates;
  it does not itself assert their raw-corona equalities.
- `perron_column_difference.lean` is kernel checked with no `sorry`.
  In arbitrary finite dimension and over `ℝ`, it proves that subtracting
  two coordinates of `M^T v = beta v` is exactly the corresponding
  incidence-column difference evaluated against `v`. Its three-coordinate
  corollaries give the Class-II `b*(b-c)=c` and Tribonacci
  `b*(b-c)=c-1` forms once their column differences are supplied. The
  theorem needs no Pisot, unimodularity, primitivity, or positivity
  hypothesis; those enter only in the later geometric application. The same
  file also proves the companion-stratum orientation wall from the two
  coordinate eigen-equations: for positive coordinates and integer-style
  coefficient chambers, `A>=B>=1` forces `b>c`, while `B>=A+1` forces
  `c>b`. It also proves the dimension-independent local `n`-bonacci gap
  recursion and its Tetrabonacci first-gap specialization
  `b*(b-c)=c-d`.

## Incomplete extension

`free_involution_perron_existence_draft.lean` is a larger attempt to
derive Perron existence and uniqueness internally. It currently has
elaboration errors and explicit finite-dimensional-analysis holes. It
is not part of `make lean-check` and is not a proved result.

## Portability

For portability, add a pinned `lakefile.toml`, manifest, and
`lean-toolchain`. The current relative environment path reuses an
existing Mathlib cache and is not itself portable.
