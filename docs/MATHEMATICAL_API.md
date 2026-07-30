# Mathematical objects and invocation

Author: **Ravel**. Coordinator and project architect: **AM**.

This is the direct usage guide for the repository's mathematical
objects. It gives the representation, principal constructor or
operation, a minimal invocation, and the reference boundary.

All C++ examples assume:

```cpp
#include "math/..."
#include "spectre/..."
#include "adelic/..."
```

and are compiled through the repository `Makefile`, which supplies the
mini-gmp and math-library link flags. Header paths below are relative to
`math/include/` or `include/`.

## Exact scalars

### Integers and rationals

`mathlib::BigInt` represents `Z`; `mathlib::Rat` represents `Q`.
Both own their mini-gmp storage.

```cpp
#include "math/bigint.hpp"

using namespace mathlib;
BigInt z; set_si(z, 42);
Rat q; set_si(q, 7, 12);
Rat r = q * q;
```

Use these types whenever a later sign, equality, divisibility, or
characteristic-polynomial claim must be exact. Do not convert to
`double` before the decision.

### Rational intervals

`Ball{lo,hi}` is a closed exact interval `[lo,hi]`.

```cpp
#include "math/ball.hpp"

Ball a{Rat(1, 3), Rat(1, 2)};
Ball b = ball_sqrt(a);
bool contains = ball_contains(b, Rat(2, 3));
```

The interval functions use outward-safe rational endpoints. Perron
brackets in the same header implement Collatz–Wielandt bounds for
nonnegative integer matrices.

### Tunable binary floating point

`BigFloat` stores `mant * 2^exp` with an explicit precision budget.

```cpp
#include "math/bigfloat.hpp"
#include "math/bigfloat_trig.hpp"

BigFloat x = bigfloat_from_ll(1);
BigFloat y = bigfloat_sin(x);
BigFloat z = bigfloat_mul(x, y, 256);
```

This is a high-precision numerical tool, not an interval certificate.
Use `Ball` or an exact algebraic sign test when proof depends on the
result.

## Polynomials and roots

### `Z[x]` and `Q[x]`

Coefficient vectors are low-degree first:

```cpp
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"

PolyZ f({-1, -1, -1, 1});  // x^3 - x^2 - x - 1
PolyZ g = f * f;
BigInt value = eval(f, BigInt(2));

PolyQ fq(f);
auto qr = divmod(fq * fq, fq);
```

### Sturm sequencing and isolation

For a square-free integer polynomial `f`, a Sturm chain gives exact
root counts in rational intervals.

```cpp
#include "math/sturm.hpp"

auto chain = sturm_chain(f);
long long roots = sturm_root_count_rat(
    chain, Rat(1), Rat(2));
RootInterval beta_I =
    isolate_real_root_rat(chain, Rat(1), Rat(2), 128);
```

`sturm_root_count[_rat]` counts roots by sign-variation difference.
`isolate_real_root_rat` bisects until the interval meets its requested
bit tolerance. This is the principal implementation of the project's
"root extraction": it returns a certified isolating interval rather
than a floating approximation.

### Exact Pisot classification

The C ABI classifies the roots of an integer 3x3 or 4x4 incidence
matrix:

```cpp
#include "exact_pisot.h"

long long M[3][3] = {{1,1,1}, {1,0,0}, {0,1,0}};
pisot_info_t info{};
int ok = pisot_classify_3x3(M, &info);
pisot_info_clear(&info);
```

The classifier constructs the exact characteristic polynomial, isolates
real roots with Sturm sequences, and bounds complex conjugates. Its
verdict is stronger than a power-iteration estimate.

## Matrices and characteristic polynomials

### Rational matrices

```cpp
#include "math/mat_q.hpp"

MatQ A(2, 2);
A(0,0) = Rat(1); A(0,1) = Rat(1);
A(1,0) = Rat(1); A(1,1) = Rat(0);
Rat detA = determinant(A);
MatQ invA = inverse(A);
```

### Exact characteristic polynomial

```cpp
#include "math/charpoly.hpp"

std::vector<std::vector<long long>> M{
    {1,1,1}, {1,0,0}, {0,1,0}
};
PolyZ chi = charpoly_faddeev_leverrier(M);
```

`charpoly_faddeev_leverrier` checks that every rational division is
integral and verifies Cayley–Hamilton before returning.

## Algebraic numbers in `Q(beta)`

Let `c(beta)=0` be monic of degree `d`. `QBetaRing(c)` represents
`Q[x]/(c)`, and `QElem` stores a power-basis vector.

```cpp
#include "math/qbeta.hpp"
#include "math/bezout.hpp"

QBetaRing R(PolyZ({-1, -1, -1, 1}));
QElem beta = R.beta_k(1);
QElem a = R.add(R.one(), beta);
QElem a_inv = invert_in_qbeta(a, R).inverse;
QElem should_be_one = R.mul(a, a_inv);
```

To decide the sign of an algebraic element at the selected real root:

```cpp
#include "math/sturm.hpp"

RootInterval beta_I = isolate_beta(R);
int sign = qbeta_sign(a, R, beta_I);
```

The isolating interval identifies which conjugate is meant; the quotient
ring alone does not.

## Exact linear algebra over `Q(beta)`

```cpp
#include "math/linalg_qbeta.hpp"

EigenvectorResult rv = right_eigenvector_via_qbeta(M, R);
EigenvectorResult lv = left_eigenvector_via_qbeta(M, R);
bool right_ok = verify_right_eigenvector(rv.v, M, R);
bool left_ok  = verify_left_eigenvector(lv.v, M, R);
```

The left and right systems are distinct:

```text
M v = beta v
u^T M = beta u^T
```

Contact hyperplanes use the right/left convention stated by the owning
`Substitution<d>` method; do not substitute one eigenvector for the
other by transposing at the call site.

## Perron–Frobenius objects

For a nonnegative integer matrix:

```cpp
#include "math/perron_frobenius.hpp"

bool irreducible = is_irreducible(M);
bool primitive = is_primitive(M);
PerronFrobeniusCertificate cert =
    certify_perron_frobenius(M);
```

The composed certificate records:

1. graph irreducibility;
2. exact characteristic polynomial;
3. isolation of the largest real root;
4. an exact `Q(beta)` eigenvector;
5. exact positivity and eigenvector identities.

For a cheap exact comparison between two nonnegative matrices:

```cpp
auto comparison = compare_perron_roots_exact(A, B);
```

An `inconclusive` bracket is not equality; refine or use the full
algebraic path.

## FFT objects

The numeric transform accepts a fixed power-of-two array:

```cpp
#include "math/fft.hpp"

std::array<std::complex<double>, 8> x{};
x[0] = 1.0;
fft_inplace(x, false);
fft_inplace(x, true);
```

The exact small transform uses `ComplexRat`; the strong wrapper makes
direction/normalization explicit:

```cpp
#include "math/fft_strong.hpp"

std::array<std::complex<double>, 8> x{};
auto spectrum = fft_strong_double<8>(std::move(x));
```

These are batch transforms. The repository does not presently expose a
complete N-oscillator chirp tracker or sparse spectral root solver.
For frequency estimation beyond FFT bins, the available exact pieces
are the characteristic-polynomial, Sturm, and `Q(beta)` layers; a
Prony/matrix-pencil construction would be a new composed subsystem.

## Substitutions

### Runtime substitution

`SubstitutionRule` is used by word algorithms and surveys:

```cpp
#include "spectre/substitution.hpp"

spectre::SubstitutionRule sigma({
    {0,1},  // sigma(0)
    {0}     // sigma(1)
});
auto M = sigma.incidence_matrix();
auto w = sigma.expand(0, 12);
```

Words use zero-based signed integer letters.

### Fixed-dimension geometric substitution

`Substitution<d>` owns a compile-time-dimension rule plus Perron and
hyperplane data:

```cpp
#include "spectre/core.hpp"

std::array<std::vector<long long>, 3> images{
    std::vector<long long>{0,1},
    std::vector<long long>{0,2},
    std::vector<long long>{0}
};
spectre::Substitution<3> s(images, beta);
bool in_face_strip = s.in_H_sigma(x, j);
s.ensure_exact_qbeta();
bool exact = s.in_H_sigma_exact(x, j);
```

Use the exact method when a boundary comparison is mathematically
decisive or a floating tolerance is near zero.

### Spectral invariants

```cpp
#include "spectre/spectral.hpp"

auto inv3 = spectre::spectral_invariants_3x3(
    1, 1, 1,
    1, 0, 0,
    0, 1, 0);
auto invn = spectre::spectral_invariants_general(Mn);
```

The result contains the Perron root `beta`, secondary modulus `beta2`,
determinant, and the relevant separation/bound fields. These are
floating diagnostics unless paired with the exact classifier.

## Stepped hyperplanes, faces, and contact digits

An affine node is `[i,x,j]`, represented by `ANode<d>`, with lattice
translation `x in Z^d` and face labels `i,j`.

```cpp
spectre::ANode<3> n{i, x, j};
auto parents = spectre::parent_decompositions(s, n);
auto next = spectre::forward_edges(s, n);
```

The contact digit predicate combines hyperplane and face geometry:

```cpp
#include "spectre/d_cont_check.hpp"

bool contact = spectre::is_in_D_cont(s, candidate);
auto digits = spectre::search_D_cont(s, coordinate_bound);
```

Face intersections use `face_intersection_dim` and
`d_cont_face_intersection_dim` from `faces.hpp`.

## Corona and contact-boundary graph

`SNode<d>` is the simple-node form of `[i,x,j]`. The principal sets are:

```text
D_cont       finite contact digits/candidates
pre_contact  backward closure of D_cont
C            contact set after the geometric restrictions
±C           signed contact set
G_B          recurrent boundary graph obtained by corona + Red
```

Invoke the composed pipeline:

```cpp
#include "spectre/contact_boundary.hpp"

spectre::ContactBoundaryLimits limits;
auto report =
    spectre::compute_contact_boundary_from_subst<3>(
        rule, beta, beta2, 2, limits);
```

Or supply a verified `D_cont`:

```cpp
auto report =
    spectre::compute_contact_boundary<3>(
        rule, beta, beta2, d_cont, limits);
```

Important report fields include `contact_size`, `boundary_size`,
`converged`, `closure_stopped_early`, `bp_rho_nc`,
`boundary_eigenvalue`, `boundary_nodes`, and optionally `gb_matrix`.
A cap or stopped closure is a bounded result, not a theorem.

Low-level calls in `corona.hpp` expose `build_signed_contact_set`,
`c_corona`, simple forward/backward targets, and conversion from
`ANode` to `SNode`.

## Balanced pairs

For two words with equal abelianization, irreducible reduction splits
them at their first recurring equal prefix-sums. The transition graph
iterates substitution followed by this reduction.

```cpp
#include "spectre/balanced_pair.hpp"

auto cert = spectre::balanced_pair_certify(
    rule, max_pairs, max_word_length);
double rho = spectre::rho_nc(
    rule, max_pairs, max_word_length);
auto graph = spectre::balanced_pair_transition_graph(
    rule, max_pairs, max_word_length);
```

`rho_nc` is the Perron radius of the noncoincidence portion of the
balanced-pair automaton. It must not be identified with
`lambda(G_B)` unless a theorem or an exact per-case certificate supplies
that equality.

## Weighted graph quotients and involutions

```cpp
#include "spectre/graph_divisor.hpp"

spectre::WeightedDigraph g = ...;
auto p = spectre::coarsest_equitable_partition(g);
auto Q = spectre::quotient_matrix(g, p);
auto sccs = spectre::tarjan_scc(g);
auto [core, old_indices] =
    spectre::extract_dominant_recurrent_core(g);
```

`extract_recurrent_core` and `extract_dominant_recurrent_core` answer
different questions: largest node count versus greatest spectral
radius.

For the free involution mechanism:

```cpp
#include "spectre/involution_helpers.hpp"

auto gbq = spectre::compute_gb_sym_quotient<d>(report, subst);
auto bpq = spectre::compute_bp_sym_quotient(rule);
```

The Lean theorem in `free_involution_perron_core.lean` formalizes the
general quotient/Perron relationship.

## Return substitutions and labelled lifts

```cpp
#include "spectre/return_substitution.hpp"
#include "spectre/return_contact_lift.hpp"

auto words = spectre::discover_return_words(rule, marker, orbit_cap);
auto derived =
    spectre::build_return_substitution(rule, marker, orbit_cap);
auto phase =
    spectre::build_return_phase_system(rule, marker, orbit_cap);
spectre::ReturnContactLimits lift_limits;
auto lift = spectre::build_reachable_return_contact_lift(
    substitution, contact_states, phase, seeds, lift_limits);
```

The lift state is `(contact state, return phase)`. It retains prefix
information erased by the unlabelled contact graph.

## Class-II family

The Class-II family is:

```text
sigma_{a,b}: 0 -> 0^a 1,
             1 -> 0^b 2,
             2 -> 0
```

with the active parametric results concentrated on `b=1`.

```cpp
#include "spectre/class_ii_boundary_family.hpp"
#include "spectre/class_ii_neighbor_family.hpp"
#include "spectre/class_ii_neighbor2_pruning.hpp"

auto C = spectre::class_ii_contact_set();
auto shell = spectre::class_ii_interior_shell(round);
auto terminal = spectre::class_ii_terminal_shell(a);
auto matrix =
    spectre::class_ii_neighbor_compressed_matrix(neighbor, a);
```

The neighbor-2 pruning header exposes separate functions for stable,
penultimate, terminal, and repeated-terminal layers. Call the matching
phase function; do not extrapolate a stable formula through the terminal
edit. `class_ii_global_round_partition.lean` proves that the five phase
predicates are disjoint and exhaustive on the stated domain.

## Rauzy fractals

```cpp
#include "spectre/rauzy_fractal.hpp"

spectre::RauzyFractal<3> rf(images);
auto raw_points = rf.chaos_game(max_points, seed);
auto point = rf.project_2d(raw_points.front());
```

Generation expands substitution prefixes and projects abelianized
prefix positions into the contracting space. Point clouds and finite
distance counts are computational approximations; self-affine tiling
and boundary-dimension claims require the separate contact/graph
conditions described in `THEOREM_STATUS.md`.

## Pisot recurrence topology

```cpp
#include "math/pisot_numeration_topology.hpp"

auto r = mathlib::pisot_recurrence_tribonacci();
auto digits = mathlib::pisot_digit_sequence(r, count);
mathlib::Patch c = mathlib::group_add(r, a, b);
double theta = mathlib::toroidal_projection(r, c);
```

This module is an experimental finite implementation of recurrence,
bifix, group-addition, and toroidal-projection ideas. Its bounded checks
do not substitute for the hypotheses of the cited topology theorem.

## Adelic objects

### Rational-prime factorization

```cpp
#include "adelic/dedekind_factorization.hpp"

auto fac = adelic::factor_prime_in_qbeta(charpoly, p);
```

The result lists prime-ideal data `(p,g,e,f)` derived from factorization
modulo `p`, together with the maximality conditions used.

### Ideals

```cpp
#include "adelic/ideal_arithmetic.hpp"

auto I = adelic::ideal_from_prime(prime, degree, charpoly);
auto J = adelic::ideal_pow(I, exponent, charpoly);
bool same = adelic::ideal_equal(I, J);
```

Ideals are column lattices in Hermite normal form. Equality is exact
HNF equality after reduction.

### `p`-adic and local fields

```cpp
#include "adelic/padic.hpp"
#include "adelic/local_field.hpp"

auto x = adelic::zp_from_int(2, 3, 80);
auto y = adelic::zp_invert(x);
auto z = adelic::zp_mul(x, y);
```

Precision is finite and explicit. Check valuation/integrality and
whether the requested precision was sufficient before treating a local
calculation as decisive.

### Prefix and carry automata

```cpp
#include "adelic/prefix_automaton.hpp"
#include "adelic/csy_carry_automaton.hpp"

auto P = adelic::build_prefix_automaton<d>(words);
adelic::PisotContext ctx(poly, limits);
auto state = ctx.eval(digit_word);
```

The current carry exploration is bounded and tracks absolute prefix
positions. Exhausting a depth or memory cap returns a truncated result,
not a finite-automaton theorem.

### Coincidence, property-(F), and tiling classification

```cpp
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/classify_adelic.hpp"

auto sc = adelic::check_strong_coincidence<d>(words, limits);
auto pf = adelic::check_property_f(...);
auto verdict = adelic::classify_tiling<d>(words, options);
```

The composed classifier reports hypotheses, caps, and inconclusive
states separately. Use its `TilingVerdict`; do not infer a tiling
theorem from a single successful subcheck.

## Finite Fibonacci and packed dynamics

```cpp
#include "spectre/fibonacci_selection.hpp"

auto target = spectre::quantum_chsh_targets();
spectre::FibonacciSelectionLimits limits;
auto exact = spectre::fibonacci_selection_exact(
    selection_stride, setting_stride, shift, target, limits);
```

The exact routine integrates interval/window lengths rather than
sampling sites.

```cpp
#include "spectre/packed_binary_dynamics.hpp"

auto bits = spectre::packed_fibonacci_word(N);
spectre::packed_rule30(bits, steps);
auto C = spectre::packed_parity_correlation_matrix(bits, settings);
```

Packed routines are finite periodic experiments. Their caps and system
size belong in every reported result.

## Lua invocation

Build the native module:

```sh
make lua
```

Then from `lua/`:

```lua
package.cpath = "../out/?.so;" .. package.cpath
package.path = "lua_src/?.lua;lua_src/?/init.lua;" .. package.path

local native = require("spectre_native")
local spectre = require("spectre").init(native)
```

### Core examples

```lua
local z = spectre.cyclo.make(1, 2, 0, -1)
local xy = spectre.cyclo.to_xy(z)

local inv = spectre.spectral.invariants_3x3({
  {1, 1, 1},
  {1, 0, 0},
  {0, 1, 0},
})

local sigma = {
  {0, 1},
  {0},
}
local bp = spectre.balanced_pair.certify(sigma)

local report = spectre.contact_boundary.from_subst(sigma)
assert(report.converged and not report.closure_stopped_early)
```

The public submodules attached by `init` are:

```text
cyclo, constants, spectral, tilt, substitution, return_phase,
balanced_pair, fibonacci_selection, fibonacci_finite,
fibonacci_dynamics, thermometer, barge, survey,
contact_boundary, rauzy_fractal, d_cont_check,
predict_dimension, lineage, coord_bfs, spectre_transitions
```

Use the Lua tests as executable call examples; the mapping is listed in
`SOURCE_FILE_ATLAS.md`.

## Command-line invocation

The root `Makefile` is the stable entry point:

```sh
make                         # native library and default targets
make check                   # complete enrolled native + Lua checks
make lean-check              # enrolled sorry-free Lean modules
make tables                  # rebuild deterministic Lua geometry tables
make class_ii_bp_family_probe
make class_ii_corona_literature_probe
make class_ii_symmetry_probe
make class_ii_terminal_transport_probe
```

For any app whose target matches its stem:

```sh
make <target>
./out/<target> [arguments]
```

Read the file header before running exploratory drivers: several accept
explicit size/memory caps, and some are intentionally not enrolled in
`make check`.

## Lean invocation

```sh
make lean-check
```

The Make target runs each enrolled file in the configured Mathlib
environment and separately checks that enrolled files contain no
`sorry`. The existence draft is intentionally outside this enrollment.

## Reference map

The bibliography is `refs/references.bib`. The principal mathematical
connections are:

| Subject | Repository objects | References |
|---|---|---|
| Pisot substitutions and pure discrete spectrum | `balanced_pair.hpp`, `pisot_substitution_properties.hpp` | Sirvent–Solomyak; Barge; Bernat |
| Rauzy fractals and substitutions | `rauzy_fractal.hpp`, `core.hpp` | Arnoux–Ito; Thuswaldner |
| Contact sets, coronae, and boundary graphs | `ambient_graph.hpp`, `corona.hpp`, `contact_boundary.hpp` | Loridant–Thuswaldner–Zhang |
| Non-unit/adelic representation space | `include/adelic/*` | Minervino–Thuswaldner |
| Pisot numeration and topological groups | `pisot_numeration_topology.hpp`, `csy_carry_automaton.hpp` | Carton–Sudbery–Yassawi |
| Graph-directed boundary dimension | `contact_boundary.hpp`, `graph_divisor.hpp` | Mauldin–Williams |
| Common dynamics and same-matrix families | `substitution_neighborhood.hpp`, Class-II neighbor headers | Sellami; Arnoux–Mizutani–Sellami; Sellami–Sirvent |
| Aperiodic order and deformation | Spectre/Rauzy geometry layers | Grimm; Baake–Lenz |

Use `docs/LITERATURE_AUDIT_CLASS_II.md` for what the cited literature
does and does not establish for the Class-II family. Use
`docs/THEOREM_STATUS.md` before promoting any numerical or bounded
certificate to a general claim.
