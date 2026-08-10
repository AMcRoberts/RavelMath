# Mathematical objects and invocation

Author: **Ravel**. Coordinator and project architect: **AM**.

This is the direct usage guide for the repository's mathematical
objects. It gives the representation, principal constructor or
operation, a minimal invocation, and the reference boundary.

All C++ examples assume:

```cpp
#include "math/..."
#include "ravel/..."
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


## Reflective proof engine

This is the public entry point for operating the self-observing proof system. Read this section before using the engine, then follow the linked manual and contracts.

### What the engine does

```text
Lua activation
→ ordinary exact math-library execution
→ semantic reflection trace
→ generated executable proof campaign
→ reusable derivation operations
→ dependency-closed proof artifacts
→ Lean serialization
→ one kernel check
```

The application is only a trigger. Mathematical definitions and observations belong in the math library; theorem data belongs in campaign specifications; reusable derivations belong in the executor; Lean verifies the finished result.

### Public locations

| Purpose | Location |
|---|---|
| Reflection trace, typed semantic nodes, scoped activation | `math/include/math/proof_reflection.hpp` |
| Symbolic polynomial-matrix families and reflected operations | `math/include/math/poly_matrix.hpp` |
| Lua activation declaration reader | `include/ravel/proof/reflection_declaration.hpp` |
| Campaign vocabulary, specifications, generator, executor, artifacts, renderer | `include/ravel/proof/proof_campaign_engine.hpp` |
| Legacy trace renderer; do not use for campaign closure | `include/ravel/proof/reflective_lean_renderer.hpp` |
| Universal-`n` integration trigger | `app/nbonacci_charmpoly_proof_general.cpp` |
| Lua activation schema | `lua/lua_src/ravel/proof/nbonacci_charpoly_schema.lua` |
| Shared erased-index Lean support | `lean/Ravel/Matrix/EraseIndex.lean` |
| Isolated Lean invocation | `scripts/safe_lean_check.sh` |

### Core API

Activate reflection around ordinary math-library work:

```cpp
#include "math/poly_matrix.hpp"
#include "math/proof_reflection.hpp"
#include "ravel/proof/proof_campaign_engine.hpp"
#include "ravel/proof/reflection_declaration.hpp"

auto declaration = ravel::proof::load_reflection_declaration(schema_path);
if (!declaration.enabled) throw std::runtime_error("reflection disabled");

mathlib::reflection::Trace trace("theorem.identifier");
{
    mathlib::reflection::ScopedTrace active(&trace);
    // Call ordinary exact math-library operations here.
}

ravel::proof::CampaignGenerator generator;
ravel::proof::ProofCampaignExecutor executor;
auto result = executor.run(generator.generate(trace), trace);

std::string report = result.report();
std::string lean = ravel::proof::render_closed_campaign_lean(result);
```

Principal public objects:

- `mathlib::reflection::Trace` — typed semantic provenance for one theorem campaign.
- `mathlib::reflection::ScopedTrace` — optional activation woven through ordinary math calls.
- `ravel::proof::CampaignOperation` — installed executable derivation vocabulary.
- `ravel::proof::CampaignTask` and `ProofCampaign` — dependency-linked executable plan.
- `ravel::proof::ProofCampaignExecutor::run(...)` — executes ready tasks until closure or a typed block.
- `ravel::proof::ClosedProofArtifact` — renderable definitions and theorems with no open goals.
- `ravel::proof::CampaignResult` — task states, diagnostics, artifacts, and `all_closed()`.
- `ravel::proof::render_closed_campaign_lean(...)` — serializes only closed artifacts.

Installed derivation operations are declared by `CampaignOperation`. They currently cover triangular support and determinant composition, piecewise erased-index equality, sparse cofactor recurrence, first-order recurrence closure, cofactor decomposition, polynomial normalization, and final theorem composition.

### Correct operating procedure

1. Enable reflection through the Lua declaration.
2. Run the ordinary symbolic problem through the math library; do not put proof logic in the trigger.
3. Generate the campaign from the resulting `Trace`.
4. Require every campaign operation to have an installed executor.
5. Run the executor to closure.
6. Inspect typed blocked states instead of editing generated Lean.
7. Render only dependency-closed artifacts.
8. Run `scripts/safe_lean_check.sh` when the matching Lean/Mathlib toolchain is complete.

`TaskState::Closed` means internally dependency-closed. It does **not** mean Lean accepted the module. Report kernel acceptance separately.

When a derivation is missing, implement the reusable derivation operation. Do not perform the derivation manually in the application, campaign data, C++ strings, tests, or generated Lean.

### Authoritative follow-on reading

Read these in order after this section:

1. `PROOF_SYSTEM_MANUAL.md` — operating workflow, outputs, and troubleshooting.
2. `PROOF_SYSTEM_CONTRACTS.md` — non-negotiable architecture and acceptance rules.
3. `PROOF_SYSTEM_EXTENSION_GUIDE.md` — how to add a missing reusable derivation operation.
4. `SAFE_LEAN_CHECK.md` — isolated kernel-check procedure.
5. The applicable campaign reading list, such as `READINGLIST.NBONACCI_SHOOT_THE_MOON.md`.

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
#include "ravel/substitution.hpp"

ravel::SubstitutionRule sigma({
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
#include "ravel/core.hpp"

std::array<std::vector<long long>, 3> images{
    std::vector<long long>{0,1},
    std::vector<long long>{0,2},
    std::vector<long long>{0}
};
ravel::Substitution<3> s(images, beta);
bool in_face_strip = s.in_H_sigma(x, j);
s.ensure_exact_qbeta();
bool exact = s.in_H_sigma_exact(x, j);
```

Use the exact method when a boundary comparison is mathematically
decisive or a floating tolerance is near zero.

### Spectral invariants

```cpp
#include "ravel/spectral.hpp"

auto inv3 = ravel::spectral_invariants_3x3(
    1, 1, 1,
    1, 0, 0,
    0, 1, 0);
auto invn = ravel::spectral_invariants_general(Mn);
```

The result contains the Perron root `beta`, secondary modulus `beta2`,
determinant, and the relevant separation/bound fields. These are
floating diagnostics unless paired with the exact classifier.

## Stepped hyperplanes, faces, and contact digits

An affine node is `[i,x,j]`, represented by `ANode<d>`, with lattice
translation `x in Z^d` and face labels `i,j`.

```cpp
ravel::ANode<3> n{i, x, j};
auto parents = ravel::parent_decompositions(s, n);
auto next = ravel::forward_edges(s, n);
```

The contact digit predicate combines hyperplane and face geometry:

```cpp
#include "ravel/d_cont_check.hpp"

bool contact = ravel::is_in_D_cont(s, candidate);
auto digits = ravel::search_D_cont(s, coordinate_bound);
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
#include "ravel/contact_boundary.hpp"

ravel::ContactBoundaryLimits limits;
auto report =
    ravel::compute_contact_boundary_from_subst<3>(
        rule, beta, beta2, 2, limits);
```

Or supply a verified `D_cont`:

```cpp
auto report =
    ravel::compute_contact_boundary<3>(
        rule, beta, beta2, d_cont, limits);
```

Important report fields include `contact_size`, `boundary_size`,
`converged`, `closure_stopped_early`, `bp_rho_nc`,
`boundary_eigenvalue`, `boundary_nodes`, and optionally `gb_matrix`.
A cap or stopped closure is a bounded result, not a theorem.

Low-level calls in `corona.hpp` expose `build_signed_contact_set`,
`c_corona`, simple forward/backward targets, and conversion from
`ANode` to `SNode`.

The default high-dimensional path is request-driven:

```cpp
ravel::CoronaSurface<6> surface(substitution, contact);
ravel::CoronaProjectionRequest<6> request;
request.seeds = embedded_states;
request.accept = relevant_state_predicate;
request.operations =
    static_cast<unsigned>(ravel::CoronaSurfaceOperation::simple_forward) |
    static_cast<unsigned>(ravel::CoronaSurfaceOperation::simple_backward);
auto image = surface.project(request);
if (!image.complete()) {
    // A cap or unfinished frontier is not a theorem.
}
```

For Algorithm 2 with filtering during corona generation:

```cpp
auto trace = ravel::algorithm2_projected_trace<6>(
    substitution, contact, predicate,
    ravel::CoronaConnectorPolicy::evolving_layer);
```

`trace.rejected_boundary` is proof-relevant audit data.  The projected mode is
the default.  Set `RAVEL_CORONA_MODE=legacy` or pass an explicit legacy mode to
use historical full materialization.  `search_D_cont` follows the same default
and uses exact face-pattern enumeration; `RAVEL_D_CONT_MODE=legacy` restores
the old box scan.  See `CORONA_SURFACE_PROJECTION.md`.

## Balanced pairs

For two words with equal abelianization, irreducible reduction splits
them at their first recurring equal prefix-sums. The transition graph
iterates substitution followed by this reduction.

```cpp
#include "ravel/balanced_pair.hpp"

auto cert = ravel::balanced_pair_certify(
    rule, max_pairs, max_word_length);
double rho = ravel::rho_nc(
    rule, max_pairs, max_word_length);
auto graph = ravel::balanced_pair_transition_graph(
    rule, max_pairs, max_word_length);
```

`rho_nc` is the Perron radius of the noncoincidence portion of the
balanced-pair automaton. It must not be identified with
`lambda(G_B)` unless a theorem or an exact per-case certificate supplies
that equality.

## Weighted graph quotients and involutions

```cpp
#include "ravel/graph_divisor.hpp"

ravel::WeightedDigraph g = ...;
auto p = ravel::coarsest_equitable_partition(g);
auto Q = ravel::quotient_matrix(g, p);
auto sccs = ravel::tarjan_scc(g);
auto [core, old_indices] =
    ravel::extract_dominant_recurrent_core(g);
```

`extract_recurrent_core` and `extract_dominant_recurrent_core` answer
different questions: largest node count versus greatest spectral
radius.

For the free involution mechanism:

```cpp
#include "ravel/involution_helpers.hpp"

auto gbq = ravel::compute_gb_sym_quotient<d>(report, subst);
auto bpq = ravel::compute_bp_sym_quotient(rule);
```

The Lean theorem in `free_involution_perron_core.lean` formalizes the
general quotient/Perron relationship.

## Return substitutions and labelled lifts

```cpp
#include "ravel/return_substitution.hpp"
#include "ravel/return_contact_lift.hpp"

auto words = ravel::discover_return_words(rule, marker, orbit_cap);
auto derived =
    ravel::build_return_substitution(rule, marker, orbit_cap);
auto phase =
    ravel::build_return_phase_system(rule, marker, orbit_cap);
ravel::ReturnContactLimits lift_limits;
auto lift = ravel::build_reachable_return_contact_lift(
    substitution, contact_states, phase, seeds, lift_limits);
```

The lift state is `(contact state, return phase)`. It retains prefix
information erased by the unlabelled contact graph.

For a relation-valued lift whose exceptional transitions are allowed to exit
through a finite terminal boundary, use the shared iterative certificate:

```cpp
#include "ravel/proof/finite_graph_correspondence.hpp"

auto escape = ravel::proof::derive_finite_escape_boundary_certificate(
    relation_adjacency, live_products, terminal_products);
// escape.acyclic, escape.every_live_vertex_reaches_terminal,
// escape.max_terminal_distance, and escape.terminal_distance are exact.

auto height = ravel::proof::derive_finite_escape_height_certificate(
    relation_adjacency, live_products, terminal_products, symbolic_height);
// height.proves_acyclic is exact when every nonterminal edge decreases the
// supplied rank and terminal vertices have no live outgoing edge.

auto ordered_height =
    ravel::proof::derive_finite_escape_lexicographic_height_certificate(
        relation_adjacency, live_products, terminal_products,
        boundary_layer, phase_residual);
// ordered_height separates primary-layer descent from fixed-layer
// phase/carry descent; it does not require a caller-chosen scalar weight.
```

This is an escape-boundary certificate, not a source-surjective graph-factor
claim. The height certificate checks a proposed symbolic rank; it does not
derive that rank. A total nonterminal factor must still be established
separately.

The adelic completion relation accepts an optional height callback with the
signature `height(lift_state, left_gamma_id, right_gamma_id)`. When supplied,
the relation certificate replays strict descent on its live product edges
without changing pruning. This is the intended place to falsify candidate
phase or boundary ranks; it is diagnostic input, not an implicit derivation.
It also accepts an optional pair-valued callback with the same arguments,
returning `(primary_layer, secondary_residual)`, and replays the ordered
contract directly. The pair-valued result is the authoritative form when a
scalar weighting could conceal a fixed-layer tie.

The exact primary boundary coordinate is derived from the Property-F graph's
SCC condensation:

```cpp
#include "adelic/property_f_escape_rank.hpp"
auto boundary_rank = adelic::derive_property_f_escape_rank(property_graph);
// boundary_rank.node_height[gamma_id] is the longest route to a sink SCC.
// boundary_rank.scc_partition_replayed independently checks that the
// supplied labels are the graph's maximal SCCs, not merely an acyclic merge.
// boundary_rank.edge_rank_strictly_decreasing replays that every cross-SCC
// node edge lowers the computed sink distance; the two counters expose any
// malformed/nondecreasing projection.
```

This operation validates the supplied SCC labels, constructs the acyclic
condensation, and computes the longest sink distance. A return-phase residual
can then be used as a bounded tie-breaker inside one boundary layer.

For a proposed shell/birth-round decomposition, the same correspondence header
provides a structural contract:

```cpp
auto strata = ravel::proof::derive_stratified_escape_certificate(
    adjacency, component_labels, recurrent_component, birth_round);
// strata.valid requires consistent component rounds, no recurrent return to
// an earlier transient round, and an escape from every transient round-group.
```

This checks the three local premises used by the Class-II round audit. It does
not discover `component_labels` or `birth_round`; those remain explicit inputs
so a future symbolic derivation cannot be hidden inside a verification loop.

## Class-II family

The Class-II family is:

```text
sigma_{a,b}: 0 -> 0^a 1,
             1 -> 0^b 2,
             2 -> 0
```

with the active parametric results concentrated on `b=1`.

```cpp
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/class_ii_neighbor2_pruning.hpp"

auto C = ravel::class_ii_contact_set();
auto shell = ravel::class_ii_interior_shell(round);
auto terminal = ravel::class_ii_terminal_shell(a);
auto matrix =
    ravel::class_ii_neighbor_compressed_matrix(neighbor, a);
```

The neighbor-2 pruning header exposes separate functions for stable,
penultimate, terminal, and repeated-terminal layers. Call the matching
phase function; do not extrapolate a stable formula through the terminal
edit. `class_ii_global_round_partition.lean` proves that the five phase
predicates are disjoint and exhaustive on the stated domain.

## Rauzy fractals

```cpp
#include "ravel/rauzy_fractal.hpp"

ravel::RauzyFractal<3> rf(images);
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

For a closed finite Property-(F) graph, the exact SCC rank can be exposed as
an auditable birth-round/transport grammar:

```cpp
#include "adelic/property_f_birth_round_grammar.hpp"

auto grammar = adelic::derive_property_f_birth_round_grammar(property_graph);
```

`grammar.layer_sizes` gives terminal-distance shells, while
`grammar.transitions` and `grammar.drop_histogram` separate internal
transport from strictly descending cross-SCC steps. Each transition also
carries a canonical coefficient-string label for its digit transport, so the
finite grammar can be compared with a symbolic branch catalogue. This is a
finite witness extractor; a non-unit theorem still needs explicit valuation
fibres.

`tests/property_f_class_ii_birth_round_test.cpp` replays the extractor over
the three-letter Class-II `sigma_{a,1}` family. Its default exact range is
`a=0..13` (including the non-AR control); larger resource-bounded probes are
opt-in and report their caps explicitly.

For `a>=4`, `include/adelic/property_f_class_ii_rank_spine.hpp` replays the
explicit alternating affine carry spine that realizes height `2*a+2`. It is
a verified lower-bound path and rank witness; bounding every other state is a
separate family proof obligation. The certificate also checks its exact digit
schedule: in the `[0,j,0]` coefficient notation the labels are
`1,a,(1,a-1),(2,a-2),...,(a-2,2),a-2,0,a-1,0`.

`include/adelic/property_f_class_ii_affine_tail.hpp` checks the same sequence
against the closed integer map
`(c0,c1,c2) -> (c1-(a+1)c0+j, c2-a*c0, c0)`, independently of any finite
graph. The graph spine certificate then checks that the generated Property-F
graph realizes this symbolic recurrence.
The family regression also sweeps this graph-independent recurrence through
`a=256`; only the graph closure remains resource-bounded.

`include/adelic/property_f_class_ii_branch_census.hpp` mechanically separates
the deterministic high-rank tail from the finite height-5 collar and checks
that, after internal SCC transport is removed, all tail-to-tail edges are
consecutive spine links. Its `valid` flag is an instance-level census
contract, not the uniform upper-bound theorem.

`include/adelic/property_f_class_ii_collar_grammar.hpp` records the collar's
height-transition language and digit labels. For the checked `a>=4` members,
the observed support is exactly
`1->0; 2->{0,1}; 3->{0,1,2}; 4->{1,2,3}; 5->{2,3,4}`;
there are no collar-to-tail edges or same-SCC collar transports. This is the
finite symbolic grammar to be lifted into a parameterized upper-bound lemma,
not that lemma itself.

### General number fields and the class-field-tower initiative

```cpp
#include "math/primality.hpp"

bool p = mathlib::is_prime(BigInt(97));
BigInt next = mathlib::next_prime(BigInt(100));
auto primes_up_to_200 = mathlib::sieve_of_eratosthenes(200);
int sym = mathlib::kronecker_symbol(BigInt(2), BigInt(7));  // Legendre/Jacobi/Kronecker
```

`is_prime` is BPSW plus deterministic Miller-Rabin (mini-gmp's own
`mpz_probab_prime_p`), fully reproducible, no random state anywhere.
`kronecker_symbol` handles any integers, not just odd positive moduli.

```cpp
#include "adelic/maximal_order.hpp"

auto round1 = adelic::enlarge_order_round2_bigint(charpoly_high_to_low, p);        // monogenic, fast, default
auto O = adelic::monogenic_structure_constants(charpoly_high_to_low);
auto round2 = adelic::enlarge_order_round2_bigint(O, p);                          // general, explicit overload
```

Pohst-Zassenhaus `p`-maximal-order enlargement: the monogenic overload
factors a single defining polynomial mod `p` (cheap, round-1-only,
since only `Z[beta]` is guaranteed monogenic); the `GeneralOrder`
overload uses Ore's Frobenius-map `p`-radical from an order's own
structure constants (more expensive, needed once an order may no
longer have a single generator, i.e. round 2 onward).
`structure_constants_from_basis_change` derives a new order's own
structure constants from an enlarged order's HNF basis, letting a
caller chain rounds.

```cpp
#include "adelic/quadratic_class_group.hpp"

long long h = adelic::quadratic_class_number(BigInt(-23));         // class number
QuadForm f3 = adelic::qform_compose(f1, f2, BigInt(-23));          // the group law
long long order = adelic::qform_order(f, BigInt(-23), h);
```

Class number AND class group structure for imaginary quadratic fields
`Q(sqrt(D))`, `D<0`, via binary quadratic forms. `qform_compose` uses
the ideal correspondence (`ideal_arithmetic.hpp`'s ideal
multiplication), not a hand-derived Gauss/Dirichlet composition
formula.

```cpp
#include "adelic/golod_shafarevich.hpp"
#include "adelic/sawin_exponent.hpp"
#include "adelic/sawin_lemma9.hpp"

auto check = adelic::check_golod_shafarevich(T, S_Q);               // Lemma 11
auto delta = adelic::compute_sawin_delta(prod_T, R, S_Q_terms);     // Proposition 10
bool holds = adelic::sawin_lemma9_holds(D);                         // Lemma 9 (base case)
```

Verification of Sawin's Golod-Shafarevich unit-distance construction
(arXiv 2605.20579): Lemma 11's inequality, Proposition 10's exponent
formula (reproduces the paper's own published `delta=0.014114`
exactly), and Lemma 9's class-number bound (specialized to `F=Q`, `K`
imaginary quadratic, the one case this project's own class-group code
can check directly).

## Finite Fibonacci and packed dynamics

```cpp
#include "ravel/fibonacci_selection.hpp"

auto target = ravel::quantum_chsh_targets();
ravel::FibonacciSelectionLimits limits;
auto exact = ravel::fibonacci_selection_exact(
    selection_stride, setting_stride, shift, target, limits);
```

The exact routine integrates interval/window lengths rather than
sampling sites.

```cpp
#include "ravel/packed_binary_dynamics.hpp"

auto bits = ravel::packed_fibonacci_word(N);
ravel::packed_rule30(bits, steps);
auto C = ravel::packed_parity_correlation_matrix(bits, settings);
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
local ravel = require("ravel").init(native)
```

### Core examples

```lua
local z = ravel.cyclo.make(1, 2, 0, -1)
local xy = ravel.cyclo.to_xy(z)

local inv = ravel.spectral.invariants_3x3({
  {1, 1, 1},
  {1, 0, 0},
  {0, 1, 0},
})

local sigma = {
  {0, 1},
  {0},
}
local bp = ravel.balanced_pair.certify(sigma)

local report = ravel.contact_boundary.from_subst(sigma)
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


### Family adjacent-swap closed forms

Owner: `include/ravel/family_closed_forms.hpp`.

- `class_ii_adjacent_swap_count_closed_form(a,b)` returns `3` in the
  nondegenerate chamber `a,b>=1`.
- `nbonacci_adjacent_swap_count_closed_form(n)` returns `n-1` for `n>=2`.
- `certify_class_ii_adjacent_swap_count(a,b)` and
  `certify_nbonacci_adjacent_swap_count(n)` independently construct the
  substitutions, invoke `adjacent_swap_neighbors`, compare the exact counts,
  and verify incidence preservation.

These are exact word-combinatorics operations, not spectral or topological
claims. The n-bonacci executable constructor uses `int8_t` letters and is
therefore limited to `n<=127`; the mathematical formula and Lean statement are
not dimension-limited.
