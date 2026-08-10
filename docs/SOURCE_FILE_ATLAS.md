# Source file atlas

Author: **Ravel**. Coordinator and project architect: **AM**.

This document accounts for the code-bearing files in the repository and
states what each one owns. It is an orientation map, not a theorem-status
ledger. For claim strength, use `THEOREM_STATUS.md`; for callable
mathematical objects, use `MATHEMATICAL_API.md`.

## Dependency shape

```text
math/include/math + mini-gmp
             |
             v
include/ravel -------- include/adelic
       |                       |
       +-----------+-----------+
                   v
              src/*.cpp
              /       \
          app/*.cpp   Lua native module
              \       /
               tests/*

Lean files formalize selected statements independently.
Python files are retained prototypes, fixtures, and visualization tools.
```

The reusable C++ namespaces are `mathlib`, `ravel`, and `adelic`.
Most `ravel` headers are header-oriented; `src/rauzy_fractal.cpp` and
`src/lua_bindings.cpp` contain the principal compiled implementations.

## Exact mathematics library

### Public headers

| File | Mathematical responsibility |
|---|---|
| `math/include/math/bigint.hpp` | RAII wrappers `BigInt` and `Rat` around mini-gmp integers and rationals; exact scalar arithmetic used by every higher tier. |
| `math/include/math/poly_z.hpp` | Dense polynomials in `Z[x]`: arithmetic, evaluation, pseudo-division, content, primitive part, and gcd. |
| `math/include/math/poly_q.hpp` | Dense polynomials in `Q[x]`: exact arithmetic, Euclidean division, evaluation, and gcd. |
| `math/include/math/mat_q.hpp` | Dynamic rational matrices with addition, multiplication, transpose, determinant, and inverse. |
| `math/include/math/bezout.hpp` | Extended gcd in `Z[x]` and `Q[x]`, reduction modulo a characteristic polynomial, and inversion in `Q(beta)`. |
| `math/include/math/qbeta.hpp` | `QElem` and `QBetaRing`, representing `Q[x]/(c(x))` in the power basis `1,beta,...,beta^(d-1)`. |
| `math/include/math/sturm.hpp` | Sturm chains, exact real-root counts and isolation, Cauchy bounds, and exact sign determination of `Q(beta)` elements. |
| `math/include/math/charpoly.hpp` | Exact Faddeev–LeVerrier characteristic polynomial with an integral-output and Cayley–Hamilton check. |
| `math/include/math/linalg_qbeta.hpp` | Linear solve, determinant, and left/right eigenvectors over `Q(beta)`, with exact verification. |
| `math/include/math/in_h_sigma.hpp` | Exact evaluation of the stepped-hyperplane predicate `0 <= <x,v> < v_j`, plus `in_h_sigma_general_bound` for an arbitrary bound vector (`0 <= <x,v> < <bound,v>`), the exact-arithmetic side of `core.hpp`'s tile-shape generalization. |
| `math/include/math/ball.hpp` | Closed rational intervals; exact interval arithmetic; Collatz–Wielandt Perron brackets and comparisons. |
| `math/include/math/bigfloat.hpp` | Tunable binary floating point `mantissa * 2^exponent` and bounded-precision Perron iteration. |
| `math/include/math/bigfloat_trig.hpp` | Trigonometric, inverse-trigonometric, hyperbolic, exponential, and logarithmic functions for `BigFloat`. |
| `math/include/math/perron_frobenius.hpp` | Irreducibility/primitivity tests and a composed exact Perron–Frobenius certificate. |
| `math/include/math/fft.hpp` | Power-of-two radix-2 FFT over `complex<double>` and exact small transforms over rational complex pairs. |
| `math/include/math/fft_strong.hpp` | Policy-driven fixed-size FFT wrapper with normalization and direction encoded by `FFTMode`. |
| `math/include/math/pisot_numeration_topology.hpp` | Pisot recurrences, finite patches, bifix extraction, group addition, and toroidal projection experiments. |
| `math/include/math/qexpansion.hpp` | Explicit asymptotic `Q`-expansions and margin checks used by Class-II estimates. |
| `math/include/math/poly_discriminant.hpp` | Arbitrary-precision polynomial discriminant (Sylvester matrix + Bareiss elimination, `BigInt` throughout) -- the fix for `include/adelic/maximal_order.hpp`'s `poly_discriminant_ll`, which silently overflows past degree 8 (confirmed, not hypothetical: a spurious "invariant violated" exception at degree 10, not just imprecision). |
| `math/include/math/primality.hpp` | Primality testing (`is_prime`, BPSW + deterministic Miller-Rabin via mini-gmp's `mpz_probab_prime_p`, no random state), prime enumeration (`next_prime`, `sieve_of_eratosthenes`), and the Legendre/Jacobi/Kronecker symbol (`kronecker_symbol`, Cohen Algorithm 1.4.10, written from scratch -- mini-gmp has none of these). Closes the confirmed gap blocking the Golod-Shafarevich class-field-tower criterion (Sawin's Lemma 11): enumerating `S_Q` and testing whether primes split in a quadratic subfield. |
| `math/include/mini-gmp/mini-gmp.h` | Vendored arbitrary-precision integer C API. |
| `math/include/mini-gmp/mini-mpq.h` | Project rational extension over mini-gmp. |
| `math/include/exact_pisot.h` | C ABI for exact cubic/quartic Pisot classification and generic real-root isolation. |

### Compiled sources, build, and tests

| File | Role |
|---|---|
| `math/src/mini-gmp.c` | Integer implementation backing `BigInt`. |
| `math/src/mini-mpq.c` | Rational implementation backing `Rat`. |
| `math/src/exact_pisot.c` | C implementation of Sturm-based Pisot classification and root isolation. |
| `math/Makefile` | Builds `libmath.a` and the math test binaries. |
| `math/TOOLS.md` | Tiered capability inventory and known work list. |
| `math/tests/test_common.hpp` | Shared test macros and helpers. |
| `math/tests/mathlib_test.cpp` | Aggregated compatibility/regression test for the exact library. |
| `math/tests/test_ball.cpp` | Rational intervals and Perron brackets. |
| `math/tests/test_bezout.cpp` | Polynomial Bezout identities and quotient-field inversion. |
| `math/tests/test_bigfloat.cpp` | Tunable-precision arithmetic and transcendental functions. |
| `math/tests/test_charpoly.cpp` | Exact characteristic polynomials and Cayley–Hamilton. |
| `math/tests/test_exploded.cpp` | Near-Salem regression cases requiring exact hyperplane signs. |
| `math/tests/test_fft.cpp` | Numeric, exact, inverse, and strong FFT modes. |
| `math/tests/test_in_h_sigma.cpp` | Exact stepped-hyperplane membership. |
| `math/tests/test_linalg_qbeta.cpp` | Exact linear algebra and left/right eigenvectors. |
| `math/tests/test_mat_q.cpp` | Rational matrix operations. |
| `math/tests/test_perron_frobenius.cpp` | Exact Perron–Frobenius certificates. |
| `math/tests/test_poly.cpp` | Integer/rational polynomial arithmetic. |
| `math/tests/test_primality.cpp` | Primality testing and the Kronecker symbol, including a full cross-check of `kronecker_symbol` against Euler's criterion for every odd prime <=97. |
| `math/tests/test_qbeta.cpp` | Quotient-ring arithmetic. |
| `math/tests/test_sturm.cpp` | Root counts, isolation, and algebraic sign tests. |

## Reflective proof system

For invocation and operating procedure, read `MATHEMATICAL_API.md` → `Reflective proof engine`. The table below records ownership only.

| File | Responsibility |
|---|---|
| `math/include/math/proof_reflection.hpp` | Optional scoped reflection, typed semantic nodes, provenance DAG, and structural certificates produced by ordinary math-library operations. |
| `math/include/math/poly_matrix.hpp` | Symbolic polynomial-matrix families, exact matrix operations, and the reflected structures used by campaign generation. |
| `include/ravel/proof/reflection_declaration.hpp` | Closed reader for the Lua reflection activation boundary. |
| `include/ravel/proof/proof_campaign_engine.hpp` | Campaign operation vocabulary, typed specifications, task graph, executor, closed proof artifacts, and campaign Lean serialization. |
| `include/ravel/proof/reflective_lean_renderer.hpp` | Legacy trace-oriented renderer; not the closure path for executable campaigns. |
| `app/nbonacci_charmpoly_proof_general.cpp` | Thin universal-`n` integration trigger; it must not own proof derivations. |
| `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua` | Lua activation and campaign-selection data. |
| `lean/Ravel/Matrix/EraseIndex.lean` | Shared Lean support for erased row/column index transport. |
| `scripts/safe_lean_check.sh` | Disposable-workspace Lean check that protects ridealong dependencies. |
| `tests/proof_reflection_test.cpp` | Reflection-path regression. |
| `tests/proof_campaign_engine_test.cpp` | Campaign scheduling and proof-operation regression. |
| `tests/qmatrix_typed_ir_test.cpp` | Typed matrix-family and erased-index-transform regression. |

## Spectral, substitution, and boundary library

### Core substitution and geometry headers

| File | Mathematical responsibility |
|---|---|
| `include/ravel/substitution.hpp` | Runtime `SubstitutionRule`, word iteration, incidence matrices, and binary correlation helpers. |
| `include/ravel/core.hpp` | Fixed-dimension `Substitution<d>`, affine nodes `ANode<d>`, Perron data, and exact/approximate `H_sigma` membership. |
| `include/ravel/spectral.hpp` | Perron root, secondary modulus, determinant, and separation bounds for 2x2, 3x3, and general matrices. `spectral_invariants_general`'s secondary-modulus (`beta2`) extraction uses a Rayleigh-Ritz step (project the Wielandt-deflated matrix onto its converged 2D invariant subspace, solve the resulting 2x2 eigenvalue problem exactly) rather than plain deflated power iteration -- the naive norm-ratio approach silently underestimated `beta2` whenever the dominant remaining eigenvalue was a complex-conjugate pair (found via the 4-letter Pisot survey; fixed and verified against an independent BigFloat root-finder, `math/tests/spectral_general_test.cpp`). |
| `include/ravel/spectral_dual.hpp` | Forward-mode automatic differentiation of the closed-form 3x3 spectral invariants. |
| `include/ravel/dual.hpp` | Generic first-order dual numbers and elementary operations. |
| `include/ravel/dual_format.hpp` | Exact-ball-aware presentation and rationality assessment of dual values. |
| `include/ravel/qbasis.hpp` | Compatibility layer for exact `Q(beta)` vectors and eigenvectors. |
| `include/ravel/qbeta_eigenvalue.hpp` | Memory-bounded exact-power iteration for a dominant eigenvalue in `Q(beta)`. |
| `include/ravel/rational_transcendentals.hpp` | Rational enclosures for pi, sine, and cosine. |
| `include/ravel/cyclotomic.hpp` | Arithmetic and planar embedding for the rank-four twelfth-cyclotomic coordinate ring. |
| `include/ravel/faces.hpp` | Cube-face intervals and intersection dimension tests. |
| `include/ravel/ambient_graph.hpp` | Parent decompositions, exact lattice solves, and type-1/type-2 ambient-graph edges. |
| `include/ravel/d_cont_check.hpp` | Exact face-projected construction and verification of `D_cont`; projected enumeration is default, with the historical box scan available in legacy mode. |
| `include/ravel/corona.hpp` | Simple nodes `SNode<d>`, signed contacts, local/inverse corona composition, request-driven `CoronaSurface`, projected Algorithm 2, and Red pruning. |
| `include/ravel/contact_boundary.hpp` | End-to-end `D_cont -> precontact -> C -> ±C -> G_B` computation and report. |
| `include/ravel/corona_projection.hpp` | Legacy compatibility views and serialization for already materialized corona graphs; new consumers should prefer `CoronaSurface` in `corona.hpp`. |
| `include/ravel/rauzy_fractal.hpp` | Contracting-space point generation and finite Rauzy-fractal diagnostics. |

### Symbolic dynamics and graph headers

| File | Mathematical responsibility |
|---|---|
| `include/ravel/balanced_pair.hpp` | Irreducible balanced-pair reduction, transition graph, certification, and noncoincidence spectral radius `rho_nc`. |
| `include/ravel/barge.hpp` | Barge-style substitution flags, exact integer characteristic polynomial, and simple Pisot classification helpers. |
| `include/ravel/graph_divisor.hpp` | Weighted digraphs, equitable partitions, quotient matrices, SCC extraction, and spectral comparison. |
| `include/ravel/gb_bp_hop_rule.hpp` | n-bonacci boundary/balanced-pair core construction and the predicted hop rule. |
| `include/ravel/involution_helpers.hpp` | Free-involution quotients, symmetric `G_B`/balanced-pair matrices, and exact factor checks. |
| `include/ravel/bp_dump_analysis.hpp` | Bounded sparse analysis of serialized balanced-pair core data. |
| `include/ravel/return_substitution.hpp` | Return-word discovery, factorization, induced substitution, and return-phase system. |
| `include/ravel/return_contact_lift.hpp` | Reachable product of a contact graph with a prefix/return-phase system. |
| `include/ravel/proof/finite_graph_correspondence.hpp` | Finite-to-one graph correspondences plus iterative escape-boundary, symbolic-height, and birth-round stratification certificates for live relations reaching terminal states. |
| `include/ravel/substitution_neighborhood.hpp` | Generic adjacent unequal-letter swaps and incidence-matrix/Parikh-fiber comparison. |
| `include/ravel/family_closed_forms.hpp` | Closed-form Class-II and n-bonacci adjacent-swap counts plus executable certificates against the generic generator. |
| `include/ravel/pisot_substitution_properties.hpp` | Frequencies, common factors/finals, dual substitution, return words, rotation data, complexity, and carry sequences. |
| `include/ravel/survey.hpp` | Enumerated/random substitution surveys, spectral classification, and Perron-frequency estimates. |

### Class-II family headers

| File | Mathematical responsibility |
|---|---|
| `include/ravel/class_ii_boundary_family.hpp` | Explicit Class-II contact candidates, affine interior shells, terminal shells, recurrent candidates, and witnesses. |
| `include/ravel/class_ii_neighbor_family.hpp` | Three fixed-light adjacent-swap families, symbolic affine prefixes, edge controls, and matrix catalogues. |
| `include/ravel/class_ii_neighbor2_pruning.hpp` | Five-phase global round decomposition and exact stable/terminal/repeated-terminal Red certificates for neighbor 2. |

### Finite dynamics and correlation headers

| File | Mathematical responsibility |
|---|---|
| `include/ravel/fibonacci_selection.hpp` | Exact-measure Fibonacci rotation windows and four-context correlation targets. |
| `include/ravel/fibonacci_finite.hpp` | Finite Fibonacci words, periodic Rule 30, and finite selection experiments. |
| `include/ravel/packed_binary_dynamics.hpp` | Packed periodic bit vectors, Rule 30, local windows, parity functionals, and correlation matrices. |
| `include/ravel/thermometer.hpp` | Substitution-sequence correlation diagnostic and finite-size convergence record. |
| `include/ravel/tilt.hpp` | Four-setting correlators and Hall–Branciard information-cost formulas. |

## Adelic library

| File | Mathematical responsibility |
|---|---|
| `include/adelic/padic.hpp` | Truncated `Z_p` arithmetic and totally ramified `Q_p` extensions. |
| `include/adelic/local_field.hpp` | Polynomial arithmetic over `Z/p^N Z`, Hensel lifting, unramified/residue-degree extensions, valuations, and local integrality. |
| `include/adelic/fp_poly.hpp` | `FpPoly`/`FpFactor` and the shared `F_p[x]` arithmetic primitives (add/sub/mul/divmod/eval/gcd), plus `Z[x] <-> F_p[x]` reduction/lift -- the common base `dedekind_factorization.hpp` and `fp_poly_factor.hpp` both depend on, extracted to avoid a circular include between them. |
| `include/adelic/fp_poly_factor.hpp` | General `F_p[x]` factoring, any degree, any prime: squarefree factorization (with the characteristic-p `f'=0` Frobenius-identity wrinkle), distinct-degree factorization, and Cantor-Zassenhaus equal-degree factorization (odd-`p` and `p=2` splitting polynomials both implemented). `dedekind_factorization.hpp`'s `factor_fp` is literally this file's `factor_fp_general` under that name. Also `is_irreducible_over_q_via_small_primes`: a rigorous sufficient condition for `Q[x]` irreducibility (single full-degree irreducible factor mod some tried prime `p`), generalizing the rational-root-theorem trick beyond degree 3; `tests/irreducibility_test.cpp` includes the degree-4 trap case `(x^2-2)(x^2-3)` (no rational roots, still reducible). |
| `include/adelic/dedekind_factorization.hpp` | Dedekind factorization of rational primes in `Q(beta)`, using `fp_poly_factor.hpp`'s general (any-degree) `F_p[x]` factoring -- an earlier, degree<=4-only, root-extraction-only version had a real bug for degree>=4 residuals that are a product of same-degree irreducibles, found and fixed (`tests/fp_poly_factor_test.cpp`). |
| `include/adelic/golod_shafarevich.hpp` | Verifies Sawin's Golod-Shafarevich inequality (arXiv 2605.20579, Lemma 11(4)/eq. 9) for a chosen `T`/`S_Q`: builds `Q = Q(sqrt(prod T))`, classifies each prime in `S_Q` as ramified/split/inert via the Kronecker symbol against `Q`'s fundamental discriminant, and checks `#T + #S_Q + #{split} + 1 <= (#T-1)^2/4`. Deliberately scoped to just this combinatorial inequality -- neither this nor Lemma 11 itself needs a computed class group (that's Lemma 9, used only inside Proposition 10's proof, not its formula). `tests/golod_shafarevich_test.cpp` reproduces the paper's own published `T`, `S_Q`, and the exact equality `36 <= 36` it reports. |
| `include/adelic/quadratic_class_group.hpp` | Class number AND group structure for imaginary quadratic fields `Q(sqrt(D))`, `D < 0`, via binary quadratic forms (Cohen Ch. 5): `qform_reduce`/`enumerate_reduced_forms`/`quadratic_class_number` (class number, direct search), plus `qform_compose` (the group law, via the ideal correspondence and `ideal_arithmetic.hpp`'s already-tested ideal multiplication rather than a hand-derived Gauss/Dirichlet composition formula), `qform_principal`, `qform_order`. Task #4 of the class-field-tower initiative, deliberately scoped to the imaginary-quadratic case rather than general-degree-n class groups (no LLL/Buchmann needed). `tests/quadratic_class_group_test.cpp` checks all nine Heegner numbers against an independently-constructed principal form; `qform_reduce` against `enumerate_reduced_forms`'s independent search (caught a real off-by-one bug in the reduction step); and `qform_compose` against the group axioms directly (identity, inverse, closure, commutativity, associativity, Lagrange) rather than a memorized structure table -- this caught a real sign/conjugation bug in the ideal-to-form extraction during development, and independently discovered (not asserted from memory) that D=-84 has a non-cyclic class group. |
| `include/adelic/sawin_exponent.hpp` | Verifies Sawin's Proposition 10 exponent formula (eq. 11, p. 12) directly: `delta` from `R`, `T`, and per-prime `(p, k(p), e(p))` in `S_Q`, computed with `math/bigfloat_trig.hpp`'s `bigfloat_log`/`bigfloat_exp` at 200-bit precision. `tests/sawin_exponent_test.cpp` reproduces the paper's own published numerator (3.8822...), denominator (275.055...), and `delta` (0.014114...) for its exact `T`/`S_Q`/`k`/`e` -- an independent computation, not Sage, giving Theorem 1's published exponent `1.014114` directly. |
| `include/adelic/sawin_lemma9.hpp` | Checks Sawin's Lemma 9 class-number bound (`h^-(K) <= 8*rd^2*(sqrt(rd)*log(rd)*e/(4*pi))^d`), specialized to `F=Q`, `K` imaginary quadratic (`d=1`), against `quadratic_class_group.hpp`'s own independently-computed class numbers. Closes the loose end `golod_shafarevich.hpp`'s header used to name -- Lemma 9 needed class-group machinery this project didn't have until this session. `tests/sawin_lemma9_test.cpp` checks the bound holds for the nine Heegner numbers plus several `h>1` discriminants. |
| `include/adelic/ideal_arithmetic.hpp` | Ideals as HNF lattices, ideal products/powers, norms, containment, and valuation cross-checks. |
| `include/adelic/maximal_order.hpp` | Pohst-Zassenhaus Round 2 `p`-maximal-order enlargement: a fast monogenic shortcut (`long long` and `BigInt` versions, the latter fixing a confirmed silent-overflow bug in the former -- not just imprecision, a spurious runtime exception) for round 1, plus a general, structure-constants-based method (`GeneralOrder`, Ore's Frobenius-method `p`-radical, Cohen Sec. 6.1.3) for round 2 onward, where the enlarged order may no longer have a single defining polynomial. `structure_constants_from_basis_change` closes the loop, letting a caller chain rounds to a fixed point; demonstrated end to end on Dedekind's own non-monogenic cubic. |
| `include/adelic/prefix_automaton.hpp` | Prefix-labelled substitution automaton and its finite digit alphabet. |
| `include/adelic/property_f_escape_rank.hpp` | Exact SCC-condensation boundary-layer rank for Property-(F) graphs; supplies the primary coordinate for lexicographic transport escape certificates. |
| `include/adelic/property_f_birth_round_grammar.hpp` | Finite birth-round/transport grammar derived from the exact Property-(F) SCC rank, including terminal layers, transport transitions, and shell-drop counts. |
| `include/adelic/property_f_class_ii_rank_spine.hpp` | Explicit `sigma_{a,1}` alternating affine rank-spine coefficients and a replay certificate for the observed height `2a+2` when `a>=4`; lower-bound witness only. |
| `include/adelic/property_f_class_ii_affine_tail.hpp` | Parameterized integer beta-inverse recurrence certificate for the Class-II affine tail and its digit schedule, independent of graph enumeration. |
| `include/adelic/property_f_class_ii_branch_census.hpp` | Separates the Class-II high-rank deterministic tail from the finite height-5 branching collar and checks the tail spine/edge census. |
| `include/adelic/property_f_class_ii_collar_grammar.hpp` | Records the finite height-transition language, digit labels, and no-tail-reentry contract for the Class-II collar. |
| `include/adelic/property_f_class_ii_tail_candidate_census.hpp` | Classifies every tail-state candidate edge and predecessor, including zero-prefix digit support, forced letter-1 return, and alternate-high rejection. |
| `include/adelic/property_f_class_ii_prefix_role_grammar.hpp` | Parameter-uniform word-level parent/prefix catalogue for the three-letter `sigma_{a,1}` substitution. |
| `include/adelic/property_f_class_ii_phase_strip.hpp` | Algebraic phase-strip invariant, the exact two-state quotient `q'=eta`, and the closed unique zero-prefix digit-forcing formula for the affine tail. |
| `include/adelic/property_f_class_ii_phase_absorption.hpp` | Graph-level check that off-strip high-rank states are absorbed by the fixed collar, except for the distinguished top zero seed; finite premise for the uniform upper-bound lemma. |
| `include/adelic/adelic_cocycle_extension.hpp` | Shared sofic/transport/adelic cocycle contract plus the non-unit valuation-fibre lemma: maximal local factorization, ideal-lattice agreement, and `sum(e f)=degree` partition checks. |
| `include/adelic/csy_carry_automaton.hpp` | Exact Pisot powers, CSY-style bounded prefix states, carry exploration, and memory accounting. |
| `include/adelic/coincidence_and_property_f.hpp` | Strong coincidence and geometric property-(F) checks, including archimedean and non-archimedean bounds. |
| `include/adelic/classify_adelic.hpp` | Composed tiling classifier and explicit verdict/report type. |

## Compiled implementations and language bridge

| File | Role |
|---|---|
| `src/rauzy_fractal.cpp` | Implements and explicitly instantiates `RauzyFractal<d>` point generation and diagnostics. |
| `src/lua_bindings.cpp` | Converts Lua tables to C++ objects and publishes the native `ravel.*` modules. |

## Research and diagnostic executables

Every file below contains `main()`. Build its Make target from the
repository root unless the file itself states otherwise.

### General algebra, spectra, and substitution geometry

| File | Purpose |
|---|---|
| `app/qbeta_eigenvalue.cpp` | CLI wrapper for dominant-eigenvalue computation in `Q(beta)`. |
| `app/rational_transcendentals_test.cpp` | Printable rational-enclosure checks for pi/sine/cosine. |
| `app/gkw_sturm_certify.cpp` | Exact rational characteristic polynomial plus Sturm certification for the GKW matrix problem. |
| `app/gkw_fully_rigorous.cpp` | Removes floating-point node/weight construction from the GKW certification pipeline. |
| `app/smooth_relaxation_search.cpp` | Continuous relaxation search around integer Pisot seed matrices; exploratory, not a certificate. |
| `app/hyperplane_progression.cpp` | Enumerates lattice points in `P_j={x:0<=<x,v><v_j}` across substitutions. |
| `app/thread_a4_extension_graph_probe.cpp` | Refines Thread A4's factor-complexity split into exact left/right/two-sided extension graphs through length 64 and a separately enlarged-prefix suffix-automaton stability check through length 500000 (35676949-symbol prefix); exposes persistent finite non-AR branching seams and guards against a measured finite-prefix artifact at length 55406. |
| `app/cylinder_measure.cpp` | Computes substitution cylinder measures using matrix growth rather than expanded words. |
| `app/tabulate_pisot_properties.cpp` | Prints the property bundle from `pisot_substitution_properties.hpp`. |
| `app/test_general_spectral.cpp` | Standalone diagnostic for general-dimension spectral invariants. |
| `app/verify_exploded_12.cpp` | Rechecks the twelve near-Salem contact cases with exact signs. |
| `app/validate_f2_bound.cpp` | Validates the finite F2 bound used by the corresponding research estimate. |

### Boundary graphs and balanced pairs

| File | Purpose |
|---|---|
| `app/contact_boundary_4x4.cpp` | Runs the four-letter contact pipeline from a matrix/rule and optionally certifies the boundary eigenvalue in `Q(beta)`. |
| `app/bp_dump_provenance.cpp` | Serializes expensive balanced-pair recurrent-core data for bounded downstream analysis. |
| `app/bp_full_core_det.cpp` | Studies the determinant of the full symmetric balanced-pair core. |
| `app/gb_bp_bijection_attempt.cpp` | Tests a periodic fixed-point construction for a node-level `G_B`/balanced-pair correspondence. |
| `app/gb_bp_crosscheck.cpp` | Cross-checks candidate node maps, hop distances, and quotient behavior. |
| `app/gb_bp_involution_check.cpp` | Checks the free involution on small n-bonacci boundary cores. |
| `app/gb_bp_involution_general_n.cpp` | Extends the involution check through the compiled dimension ceiling. |
| `app/gb_bp_matrix_equality.cpp` | Compares symmetric quotient characteristic polynomials directly. |
| `app/gb_bp_twisted_quotient_check.cpp` | Compares ordinary and signed/twisted involution quotients. |
| `app/test_bp_gb_divisor.cpp` | Exercises equitable-divisor comparison between balanced-pair and boundary graphs. |
| `app/return_contact_lift_probe.cpp` | Compares the recurrent return/contact lift with the balanced-pair graph. |
| `app/reaudit_gb_dominant_scc.cpp` | Recomputes survey rows using the Perron-dominant recurrent SCC. |

### n-bonacci structural probes

| File | Purpose |
|---|---|
| `app/item1_bfs_reachability_check.cpp` | Uses restricted forward/backward reachability to extend dominant-core checks. |
| `app/item1_conjecture_n6_check.cpp` | Tests whether Red asymmetry remains transient for Hexanacci. |
| `app/item1_cycle_dynamics_check.cpp` | Studies the affine contraction `t'=(t+delta)/beta` along core cycles. |
| `app/item1_cycle_dynamics_check_n7.cpp` | Septanacci extension of the cycle-dynamics check. |
| `app/item1_dominance_locality_check.cpp` | Refutes a purely local cycle criterion for spectral dominance. |
| `app/item1_equivariance_check.cpp` | Checks type-1 edge equivariance and the type-2/involution relation. |
| `app/item1_margin_trend_check.cpp` | Evaluates the closed margin `2/beta_n-1`. |
| `app/item1_per_pair_check.cpp` | Refines the margin check by ordered face pair. |
| `app/item1_spectral_dominance_check.cpp` | Connects recurrent SCC dominance with the geometric margin. |
| `app/n6_asymmetry_diagnosis.cpp` | Locates type-1/type-2 degree asymmetry at dimension six. |

### Class-II family probes

| File | Purpose |
|---|---|
| `app/class_ii_bp_family_probe.cpp` | Exact finite verification of the eight-state balanced-pair family and its characteristic polynomial. |
| `app/class_ii_corona_literature_probe.cpp` | Implements fixed-contact corona semantics and compares layer evolution. |
| `app/class_ii_neighbor_probe.cpp` | Surveys the three adjacent-swap neighbors sharing an incidence matrix. |
| `app/class_ii_symmetry_probe.cpp` | Enumerates exact automorphisms of the weighted recurrent core. |
| `app/class_ii_terminal_transport_probe.cpp` | Audits inherited terminal-layer transport and Red pruning. |
| `app/probe_a1_a2_unimodular.cpp` | Tests nilpotent cofactors and nearby unimodular families. |
| `app/class_ii_neighbor2_round1_window_certificate.cpp` | Round 1: closes the raw 27-state target's window validity abstractly (every `a`, not sampled). |
| `app/class_ii_neighbor2_round1_red_identity.cpp` | Round 1: checks Red's pruned pair equals the `D_cont`-seed pair, `a=3..60`. |
| `app/class_ii_neighbor2_round1_red_forward_check.cpp` | Round 1: dense/sparse numeric confirmation of the forward-image closed forms. |
| `app/class_ii_neighbor2_round1_red_symbolic.cpp` | Round 1: the symbolic Red-pruning proof for `a>=3`, derived from `tau_a`'s own fixed word forms. |
| `app/class_ii_neighbor2_round1_signed_relation.cpp` | Round 1: verifies the signed 50-state contact set is the unsigned survivors' exact mirror-closure (and that a naive mirror-and-rerun shortcut is false). |
| `app/class_ii_neighbor2_round1_union_hypothesis.cpp` | Round 1: proves the neighbor-2 signed contact set equals `class_ii_contact_set()` union its mirror union the initial reach-2 extension states -- a single, parameter-free identity holding for every `a` at once, enrolled as a permanent regression check (`tests/substitution_neighborhood_test.cpp`). |
| `app/class_ii_abstract_window_classify_regression.cpp` | Regression test for the extracted `class_ii_abstract_window_classify` header function against Round 1's own result. |
| `app/class_ii_round2_recon.cpp` | Round 2: confirms `E_1..E_4` sizes via the trusted `algorithm2_trace` pipeline. |
| `app/class_ii_round2_ccorona_stability.cpp` | Round 2: confirms `T_1 = ±C` and stable 195-candidate raw corona output across `a`. |
| `app/class_ii_round2_structure.cpp` | Round 2: confirms `T_2 = B_2 union E_2` exactly and the 195-to-72 Red-pruning split. |
| `app/class_ii_round2_red_exclusion_check.cpp` | Round 2: independent exact finite certificate for Red exclusion (123 pruned states, `simple_forward_targets_exact` vs `red_anode`'s ranks). |
| `app/class_ii_round2_pruned_states_a_independence_probe.cpp` | Round 2: confirms the 123 pruned states are the literal same coordinate set across `a`. |
| `app/class_ii_round3_structure.cpp` | Round 3: the same `T_3 = B_3 union E_3` identity and independent Red-exclusion certificate (163 pruned states). |
| `app/class_ii_round4_stable_bridge_check.cpp` | Round 4: checks whether the stable machinery's generic round>=5 source formula, evaluated at round 4, actually reproduces `A_4` from the independent ground-truth corona/Red pipeline -- the remaining named gap connecting the bounded round-4 grammar to the universal round>=5 theorem. |
| `app/class_ii_neighbor2_recurrent_exhaustion_check.cpp` | Recurrent-SCC exhaustion (`GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s "Recurrent exhaustion after layer equality"): partitions the real boundary graph by core/regular-shell/special-shell/transient and checks strong connectivity, escape edges, and absence of return edges against the real graph, not just the abstract catalogue matrix. |
| `app/class_ii_neighbor2_round_stratified_transient_check.cpp` | Recurrent-SCC exhaustion, items 3-4: builds a `birth_round(state)` assignment directly from the full (not just converged) corona trace, giving every state an "earlier"/"later" ordering needed to check escape edges and absence of return paths. |
| `app/class_ii_neighbor2_symmetric_seed_closure.cpp` | Tests whether the hardcoded 50-state signed contact set is the `backward_closure`/`red_anode` result of the symmetric seed `D_cont union mirror(D_cont)`, after ruling out `mirror(D_cont)` alone (which reproduces only 32 of the needed states). |

### Adelic and non-unit probes

| File | Purpose |
|---|---|
| `app/classify_adelic_tiling.cpp` | Runs the composed strong-coincidence/property-(F) classifier on named cases. |
| `app/adelic_boundary_spectral_radius.cpp` | Tests whether non-archimedean directions change boundary spectral conclusions. |
| `app/check_sigma12_coincidence.cpp` | Applies strong coincidence to two named unimodular substitutions. |
| `app/check_sigma12_property_f.cpp` | Adds property-(F) to the same cases. |
| `app/nonunit_family_sweep.cpp` | Systematic small non-unit Pisot family sweep. |
| `app/rnd13_factor_probe.cpp` | Dedekind factorization and ideal arithmetic for the rnd13 polynomial. |
| `app/rnd13_prefix_automaton_probe.cpp` | Builds the actual rnd13 prefix automaton and digit set. |
| `app/sweep_mismatches_property_f.cpp` | Unit-case mismatch sweep for coincidence versus property-(F). |
| `app/sweep_nonunit_property_f.cpp` | Non-unit counterpart with local-place bounds. |
| `app/sweep_nonunit_property_f_4letter.cpp` | Generalizes the non-unit sweep to 4-letter alphabets (`adelic::integer_determinant` + `is_irreducible_over_q_via_small_primes`). Surfaced and confirmed the `spectral_invariants_general` Pisot-classifier bug (1/7 verdicts before that fix, 6/7 after), then supplied the real `rndW3_5` regression for per-factor Hensel lifting: the deterministic batch is now 7/7 ESTABLISHED with zero skips. |

## Lean formalization

| File | Formal content |
|---|---|
| `lean/free_involution_perron_core.lean` | A free weighted-graph involution descends the Perron eigenproblem to the quotient, assuming the needed Perron vector. |
| `lean/free_involution_perron_existence_draft.lean` | Extended existence development; explicitly a draft and not enrolled because it contains `sorry`. |
| `lean/bp_correction_determinant.lean` | Block determinant reduction and determinant-neutral nilpotent correction, conditional on a named combinatorial input. |
| `lean/return_contact_lift.lean` | Abstract correctness of the labelled return/contact product construction. |
| `lean/class_ii_affine_shells.lean` | Injectivity, disjointness, and affine cardinality of the symbolic interior shell. |
| `lean/class_ii_terminal_shells.lean` | Symbolic terminal-shell shape and cardinality. |
| `lean/class_ii_neighbor2_extensions.lean` | Catalogue geometry for neighbor-2 terminal and repeated-terminal corrections. |
| `lean/class_ii_neighbor_dominance.lean` | Polynomial sign comparisons for candidate neighbor cores. |
| `lean/class_ii_balanced_pivot.lean` | Uniqueness of the Class-II balanced pivot from the Perron relations. |
| `lean/class_ii_neighbor_d_support.lean` | Affine matrix law `A(a)=A(3)+(a-3)D` and sparse support. |
| `lean/class_ii_six_vertex_graduation.lean` | Six-role penultimate-layer rank graduation. |
| `lean/class_ii_global_round_partition.lean` | Exhaustive, disjoint five-phase partition of legal global rounds. |
| `lean/README.md` | Enrollment boundary, dependency expectations, and proof status. |

## Lua package

### Runtime modules and deterministic data

| File | Role |
|---|---|
| `lua/lua_src/ravel/init.lua` | Composes the public Lua namespace and attaches the native backend. |
| `lua/lua_src/ravel/substitution_rule.lua` | Pure-Lua substitutions and balanced-pair helpers. |
| `lua/lua_src/ravel/lineage.lua` | Addresses, least common ancestors, and substitution-tree distance. |
| `lua/lua_src/ravel/coord_bfs.lua` | Breadth-first traversal of the coordinate transition graph. |
| `lua/lua_src/ravel/predict_dimension.lua` | Closed-form dimension estimates from spectral inputs. |
| `lua/lua_src/ravel/ravel.lua` | Pure-Lua Spectre geometry helpers. |
| `lua/lua_src/ravel/data/constants.lua` | Constants loaded from the native module. |
| `lua/lua_src/ravel/data/hex_kind.lua` | Symbolic names/indices for the nine hex metatile kinds. |
| `lua/lua_src/ravel/data/hex_vertices.lua` | Exact cyclotomic offsets of the unit hexagon. |
| `lua/lua_src/ravel/data/hex_outer_table.lua` | Deterministic outer-neighbor transition table. |
| `lua/lua_src/ravel/data/spectre_transitions.lua` | Per-kind Spectre transition table assembled from text fixtures. |

The `specmap_enriched/*.txt`, `spectre_transitions/*.txt`, and
`cluster_marks.json` files are data fixtures consumed by these modules;
they are not executable source.

### Lua scripts

| File | Purpose |
|---|---|
| `lua/scripts/run_lua_tests.lua` | Discovers and executes the enrolled Lua suites. |
| `lua/scripts/coord_bfs.lua` | Command-line coordinate walk. |
| `lua/scripts/parse_specmap.lua` | Converts parsed geometry text into enriched specmap records. |
| `lua/scripts/build_spectre_transition_table.lua` | Builds the deterministic transition table from local geometry inputs. |
| `lua/scripts/bp_hypothesis_checks.lua` | Bounded analysis of serialized balanced-pair data. |
| `lua/scripts/crunch_survey.lua` | Aggregates contact-boundary survey output. |
| `lua/scripts/pisot_search_4x4.lua` | Enumerates candidate four-letter Pisot substitutions. |
| `lua/scripts/probe_exploded.lua` | Rechecks near-Salem survey entries. |
| `lua/scripts/rnd13_rauzy.lua` | rnd13 Rauzy-fractal experiment. |
| `lua/scripts/survey_thread_a.lua` | Adds graph automorphism and spectral-gap features to survey rows. |
| `lua/scripts/dynamical_family.lua` | Packed deterministic family experiment at large site counts. |
| `lua/scripts/fibonacci_finite_family.lua` | Finite Fibonacci selection/correlation sweep. |
| `lua/scripts/fibonacci_selection_theorem.lua` | Exact-measure Fibonacci-window evaluation. |
| `lua/scripts/measurement_cost.lua` | Computes the four-setting information-cost diagnostic. |
| `lua/scripts/rule30_functional_search.lua` | Searches packed Rule-30 parity functionals. |
| `lua/scripts/run_cb_4x4_survey.sh` | Shell runner for batches of four-letter boundary cases. |
| `lua/scripts/run_qbeta_survey.sh` | Shell runner for exact boundary eigenvalue jobs. |

### Lua tests

| File | Coverage |
|---|---|
| `lua/lua_src/ravel/tests/runner.lua` | Minimal suite runner/assertion layer. |
| `test_cyclotomic.lua` | Twelfth-cyclotomic arithmetic. |
| `test_hex.lua` | Hex kinds, vertices, and outer table. |
| `test_transitions.lua` | General transition-table consistency. |
| `test_transitions_audit.lua` | Source-table audit identities. |
| `test_spectre_transitions.lua` | Per-kind Spectre transition records. |
| `test_coordinate.lua` | Coordinate BFS. |
| `test_tribonacci.lua` | Tribonacci substitution baseline. |
| `test_closure.lua` | Corrected versus naive closure. |
| `test_pisot.lua` | Named substitution classification and finite diagnostics. |
| `test_pisot_survey.lua` | Survey generation and classification. |
| `test_reference_sigma.lua` | Named sigma-family regression cases. |
| `test_predict_dimension.lua` | Dimension formulas and refusal conditions. |
| `test_gap_labels.lua` | Exact/recognized gap-label identities. |
| `test_contact_boundary.lua` | Native contact-boundary API and reports. |
| `test_general_algebraic.lua` | Characteristic polynomials and `Q(beta)` eigenvalues. |
| `test_return_phase.lua` | Return words and phase system. |
| `test_bp_dump_analysis.lua` | Sparse dump analysis and caps. |
| `test_fibonacci_selection.lua` | Exact and finite window selection. |
| `test_rauzy_fractal.lua` | Native point generation and caps. |
| `test_wide_survey.lua` | Wider random survey ranges. |

`lua/Makefile`, `lua/README.md`, and `lua/TODO_GENERALIZATION.md` define
the Lua build, usage overview, and engineering limits. `lua/tetrabonacci`
is a small matrix/rule input fixture.

## Python prototypes and visualization tools

Python is not the preferred implementation layer for new exact
machinery. These files remain useful as independent formulations,
finite experiments, and renderers.

### Spectre geometry and cellular dynamics

| File | Purpose |
|---|---|
| `python/ravel.py` | Basic polygon/affine-transform representation. |
| `python/spectre_gen.py` | Recursive Spectre tiling generator. |
| `python/spectre_gen_lineage.py` | Generator variant recording complete ancestor addresses. |
| `python/spectre_ca.py` | Reversible second-order CA on the Spectre adjacency graph. |
| `python/spectre_hex.py` | Conversion from Spectre tiles to hex metatiles. |
| `python/spectre_hex_ca.py` | Reversible CA on the hex-metatile graph. |
| `python/gen_1000.py` | Generates and serializes a medium tiling/adjacency fixture. |
| `python/gen_hex.py` | Converts the serialized tiling to hex data. |
| `python/render_1000.py` | Static polygon rendering. |
| `python/render_gif.py` | Substitution-growth animation. |
| `python/render_ca_gif.py` | Tile-graph CA animation. |
| `python/render_hex_ca.py` | Hex-graph CA rendering. |
| `python/debug_hex.py`, `python/debug_hex2.py` | Geometry diagnostics for hex grouping/adjacency. |

### Fibonacci, selection, and finite correlation prototypes

| File | Purpose |
|---|---|
| `python/fibmodel.py` | Shared Fibonacci rotation, target, and window definitions. |
| `python/ceiling_corrected.py` | Corrected ceiling/convergence experiment. |
| `python/discrete_stride_fresh.py` | Finite stride and support-disjointness experiment. |
| `python/check_natural_correlation.py` | Baseline correlation checks without retargeting. |
| `python/big_shot.py` | Large finite deterministic correlation run. |
| `python/tsirelson_family.py` | One-parameter Fibonacci-window family. |
| `python/tsirelson_family2.py` | Two-scale refinement/return-vector family. |
| `python/swap_step1.py` | Fourier-space correlator identity and closed-form tilt relation. |
| `python/swap_step2.py` | Fourier-support proof experiment for exact stride cancellation. |
| `python/second_2root2_construction.py` | Structurally distinct finite construction reaching the same four-setting target. |

### Information-cost and adversarial audits

| File | Purpose |
|---|---|
| `python/mi_certify.py` | Finite deterministic-profile optimization for an information-cost floor. |
| `python/slack_hunt.py` | Linear/nonlinear search over measurement-dependence slack. |
| `python/slack_close.py` | Closing numerical checks for hiddenness, MI, and refined stride. |
| `python/w1_complete.py` | Finite completion checks for the first work package. |
| `python/w1_fullfunction.py` | Full-grid correlation/cost experiment. |
| `python/w2_field.py` | Algebraic-field representation of a selected ratio. |
| `python/w2_exact.py` | Exact arithmetic redo and low-complexity scan. |
| `python/null_audit.py` | Adversarial test of integrality/termination claims. |
| `python/spectre_chsh.py` | Four-setting diagnostic on the reversible tile CA. |
| `python/run_chsh_big.py` | Batch runner for larger tile-CA samples. |
| `python/spectre_nomic_exclusion.py` | Conditional-selection experiment on the same local CA. |
| `python/ultrametric_chsh.py` | Correlations stratified by substitution-tree distance. |
| `python/ultrametric_orientation.py` | Lineage-derived orientation observable. |
| `python/ultrametric_scaling.py` | Finite-size scaling of lineage-stratified correlations. |
| `python/inside_view.py` | Embedded-observer/local-information demonstrations. |
| `python/verify_core.py` | Independent numerical comparison with a published analytic model. |

The `python/*results.json`, `.jsonl`, and `tiles.json` files are
deterministic result/input fixtures, not executable sources.

## Native test inventory

Each file is an independently runnable test binary enrolled by the
top-level `Makefile`.

| Test file(s) | Coverage |
|---|---|
| `ambient_graph_test.cpp`, `corona_test.cpp`, `d_cont_check_test.cpp`, `contact_boundary_test.cpp` | Ambient edges, corona/Red, contact digits, and full boundary construction. |
| `balanced_pair` coverage through `contact_boundary_test.cpp`, plus `bp_dump_analysis_test.cpp` | Balanced-pair radius and bounded serialized-core analysis. |
| `class_ii_boundary_family_test.cpp`, `class_ii_neighbor_d_matrix_test.cpp`, `substitution_neighborhood_test.cpp`, `family_of_families_test.cpp`, `family_closed_forms_test.cpp` | Class-II catalogues, affine matrices, adjacent-swap fibers, and family relations. |
| `graph_divisor_test.cpp`, `gb_bp_hop_rule_test.cpp`, `involution_helpers_test.cpp`, `weighted_digraph_file_test.cpp` | Quotients, SCCs, free involutions, hop rules, and graph parsing. |
| `return_substitution_test.cpp`, `return_contact_lift_test.cpp` | Return words, phase systems, and labelled product graph. |
| `dual_test.cpp`, `dual_format_test.cpp`, `spectral_dual_test.cpp`, `spectral_general_test.cpp` | Automatic differentiation, presentation, and spectral paths. |
| `test_qbasis.cpp`, `qbeta_eigenvalue_test.cpp`, `exact_pisot_test.cpp`, `test_12_exploded.cpp`, `validate_exploded.cpp` | Exact algebraic-number compatibility and Pisot regressions. |
| `classify_adelic_test.cpp`, `dedekind_factorization_test.cpp`, `ideal_arithmetic_test.cpp`, `maximal_order_test.cpp`, `padic_test.cpp`, `local_field_test.cpp`, `prefix_automaton_test.cpp`, `csy_carry_automaton_test.cpp`, `csy_finite_carry_automaton_test.cpp`, `coincidence_and_property_f_test.cpp` | Complete adelic stack. |
| `fibonacci_selection_test.cpp`, `fibonacci_finite_test.cpp`, `packed_binary_dynamics_test.cpp` | Exact-measure and packed finite dynamics. |
| `pisot_numeration_topology_test.cpp` | Pisot recurrence/group/topology experiment. |
| `rauzy_fractal_test.cpp`, `openai_unit_distance_test.cpp` | Rauzy generation and finite unit-distance experiments. (`rauzy_fractal_unit_distance_test.cpp` was removed 2026-07-29: non-evidentiary and mathematically wrong on its own terms -- see git history.) |
| `pisot_numeration_topology_test.cpp`, `survey_test.cpp` | Numeration topology and substitution surveys. |
| `lean_class_ii_catalogue_cross_check_test.cpp` | Cross-language catalogue cross-check between lean/*.lean literal catalogues and their C++ constructions. |

## Geometry inputs

`tatham_svg_parsed/` contains three text-table families for each of
the nine metatile kinds `D,F,G,J,L,P,S,X,Y`:

- `basehex_*.txt`: base-hex placement records;
- `hexmap_*.txt`: hex adjacency/child mapping;
- `specmap_*.txt`: Spectre-level placement records.

They are deterministic source data for `make data`; they do not contain
executable code.

## Where to extend

- Add general exact arithmetic under `math/include/math/`, with a paired
  `math/tests/test_*.cpp`.
- Add reusable substitution/boundary machinery under
  `include/ravel/`, with a paired native test.
- Add local-field/ideal machinery under `include/adelic/`.
- Add a one-purpose experiment under `app/`; do not put `main()` in
  `src/`.
- Add Lua orchestration under `lua/scripts/` and a test under
  `lua/lua_src/ravel/tests/`.
- Formalize a stable mathematical statement under `lean/`, and enroll
  it in `make lean-check` only when it is `sorry`-free.

### Adjacent twisted renewal composition

- `include/ravel/proof/adjacent_twisted_renewal.hpp`
  - reusable composite certificate joining competitor macro paths, predicted-core
    twisted transport, shared role embedding, Bellman profile commutation, and
    inherited strict renewal;
  - emits named obstruction evidence and standalone Lean theorem text.
- `app/adjacent_competitor_transport.cpp`
  - derives the concrete graph/path/profile premises from projected corona
    images and invokes the composite operation;
  - `ADJ_RENEWAL_EXPLAIN` is the audit-facing proof narrative.

### Recurrent-family exhaustion
- `include/ravel/proof/recurrent_family_exhaustion.hpp` — executable finite
  recurrent-SCC family classifier and the separate adjacent-dimensional
  induction boundary.
- `tests/recurrent_family_exhaustion_test.cpp` — proof-operation contract test.
- `tests/recurrent_family_exhaustion_real_test.cpp` — independent n=6
  truth-graph exhaustion replay.
- `docs/RECURRENT_FAMILY_EXHAUSTION_2026-08-04.md` — theorem status and the
  remaining rejected-boundary no-return seam.

## Generalized multinacci unit family

| File | Responsibility |
|---|---|
| `include/ravel/generalized_multinacci.hpp` | Reusable construction of the unimodular confluent-Parry/generalized-multinacci rule `sigma(i)=0^m(i+1)`, its exact integer polynomial, and a Perron-root approximation for pipeline metadata. |
| `app/generalized_multinacci_unit_probe.cpp` | Reuses the full balanced-pair/contact-boundary dominance pipeline on arbitrary `(D,m)` and reports the first `m=2` cases. |
| `tests/generalized_multinacci_family_test.cpp` | Verifies the `m=1` n-bonacci specialization, `m=2` rule, exact polynomial, and the silver-ratio base case. |
