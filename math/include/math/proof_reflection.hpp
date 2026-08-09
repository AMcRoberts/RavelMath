#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace mathlib::reflection {

using NodeId = std::size_t;
inline constexpr NodeId no_node = static_cast<NodeId>(-1);

enum class NodeKind {
    Object,
    Observation,
    Pivot,
    Reduction,
    Certificate,
    LemmaApplication,
    Obligation,
    Result,
};

struct DimensionExpr {
    std::string lean;
    std::size_t concrete = 0;
};

enum class MatrixEntryRuleKind {
    Unknown,
    LowerBidiagonalXMinusOne,
    UpperBidiagonalWithTerminalRow,
};

enum class MatrixStructureKind {
    Unknown,
    LowerBidiagonal,
    UpperBidiagonalPlusTerminalRow,
};

// -----------------------------------------------------------------------------
// Typed piecewise entry intermediate representation.
//
// A piecewise matrix entry rule has the form
//
//   for each (i, j) in [0, rows) x [0, cols):
//     pick the first branch whose condition is satisfied;
//     emit the corresponding expression as the entry value.
//
// The branches, conditions, and expressions are typed closed variants so the
// proof-campaign engine can reason about the rule, render Lean definitions and
// proof goals, and validate finite regressions without touching arbitrary
// strings. The legacy `MatrixEntryRuleKind` and the human-readable
// `entry_rule` string remain for backward compatibility; the typed
// `PiecewiseEntry` below is the source of truth for new work.
// -----------------------------------------------------------------------------

// Symbolic dimension expression: e.g. `n`, `n - 1`, `n - 2`, `0`.
struct DimExpr {
    enum class Op { Const, Var, Sub };
    Op op = Op::Const;
    std::string var_name;       // for Var (e.g. "n")
    int64_t const_value = 0;    // for Const and the right-hand side of Sub
    std::vector<DimExpr> operands;  // for Sub: [left, right]

    static DimExpr lit(int64_t v);
    static DimExpr var(std::string name);
    static DimExpr sub(DimExpr a, DimExpr b);

    int64_t eval(int64_t n_var) const;     // evaluate at a concrete n
    std::string to_lean() const;            // render in Lean syntax
    bool is_constant() const noexcept { return op == Op::Const; }
    int64_t const_fold() const noexcept;    // only valid when is_constant()
};

// Symbolic index expression: refers to (i, j) coordinates and the
// dimension variable `n`. Examples: `i`, `j`, `i + 1`, `n - 2`.
struct IndexExpr {
    enum class Op { Lit, IVar, JVar, DimVar, Add, Sub };
    Op op = Op::Lit;
    int64_t literal = 0;
    std::string dim_var;        // for DimVar (e.g. "n")
    std::vector<IndexExpr> operands;  // for Add, Sub: [left, right]

    static IndexExpr lit(int64_t k);
    static IndexExpr ivar();
    static IndexExpr jvar();
    static IndexExpr dim(std::string name);
    static IndexExpr add(IndexExpr a, IndexExpr b);
    static IndexExpr sub(IndexExpr a, IndexExpr b);

    int64_t eval(int64_t i, int64_t j, int64_t n_var) const;
    std::string to_lean() const;
};

// Boolean condition over a single (i, j) entry, with the dimension
// variable n in scope. Examples: `i = j`, `i < j`, `i = n - 2`,
// `i = n - 2 AND j < n - 2`.
struct EntryCondition {
    enum class Op { True, Eq, Lt, And, Or, Not };
    Op op = Op::True;
    IndexExpr left, right;
    std::vector<EntryCondition> operands;  // for And, Or, Not

    static EntryCondition tru();
    static EntryCondition eq(IndexExpr a, IndexExpr b);
    static EntryCondition lt(IndexExpr a, IndexExpr b);
    static EntryCondition and_(EntryCondition a, EntryCondition b);
    static EntryCondition or_(EntryCondition a, EntryCondition b);
    static EntryCondition not_(EntryCondition a);

    bool eval(int64_t i, int64_t j, int64_t n_var) const;
    std::string to_lean() const;
};

// Polynomial entry value. At present we support the literals the
// existing n-bonacci families actually use: `0`, `1`, `-1`, `X`, and
// `X + 1`. `Const` is a free integer literal so future families (or
// mutation tests) can introduce other coefficients.
struct EntryExpression {
    enum class Op { Zero, One, Const, NegOne, VarX, OnePlusVarX, Add };
    Op op = Op::Zero;
    int64_t const_value = 0;     // for Const
    std::vector<EntryExpression> operands;  // for Add

    static EntryExpression zero();
    static EntryExpression one();
    static EntryExpression neg_one();
    static EntryExpression var_x();
    static EntryExpression one_plus_var_x();
    static EntryExpression constant(int64_t k);
    static EntryExpression add(EntryExpression a, EntryExpression b);

    int64_t eval_as_int(int64_t x_value) const;  // for finite regression
    std::string to_lean() const;
};

// One branch of the piecewise entry: if the condition holds, the entry
// value is the expression. Branches are tested in order; the first
// matching branch wins.
struct EntryBranch {
    EntryCondition condition;
    EntryExpression expression;
    std::string human_readable;   // optional, for debug reports

    static EntryBranch make(EntryCondition c, EntryExpression e,
                            std::string hr = {});
};

// A typed piecewise entry rule. Drives both finite evaluation
// (with concrete `n`) and Lean emission of the matrix family
// definition.
struct PiecewiseEntry {
    std::string family_id;
    DimExpr row_dim;
    DimExpr col_dim;
    std::vector<EntryBranch> branches;     // first-match wins
    MatrixEntryRuleKind kind = MatrixEntryRuleKind::Unknown;
    MatrixStructureKind structure = MatrixStructureKind::Unknown;

    int64_t eval_dim(int64_t n_var) const;  // both row and col dimensions
    // Evaluate a single (i, j) entry using the typed rule.
    EntryExpression eval(int64_t i, int64_t j, int64_t n_var) const;
    // Render the matrix family as a Lean `fun i j => ...` body.
    std::string to_lean() const;
    // Render the full type signature for the matrix family.
    std::string type_lean(const std::string& ring) const;
    // Render a closed Lean definition with given family name and binders.
    std::string definition_lean(const std::string& family_name,
                                std::vector<std::pair<std::string,std::string>> binders,
                                const std::string& ring) const;
    // Render only the right-hand side of a definition (`fun i j => ...`).
    std::string definition_lean_value() const;
};

struct MatrixFamily {
    std::string family_id;
    DimensionExpr rows;
    DimensionExpr columns;
    std::string entry_rule;
    std::string structure;
    MatrixEntryRuleKind entry_rule_kind = MatrixEntryRuleKind::Unknown;
    MatrixStructureKind structure_kind = MatrixStructureKind::Unknown;
    PiecewiseEntry piecewise;   // typed IR; empty `branches` if untyped
};

struct MatrixInstance {
    std::string family_id;
    std::size_t parameter_n = 0;
    std::size_t rows = 0;
    std::size_t columns = 0;
};

struct EraseIndexMap {
    NodeId source_matrix = no_node;
    // Symbolic dimensions and positions: required for parametric
    // minor transports (e.g. q_minor_reduction erases (0, 0) from
    // an n-dependent source matrix). The legacy concrete fields below
    // remain for finite regression and backward compatibility.
    DimExpr source_size_expr;
    DimExpr erased_row_expr;
    DimExpr erased_column_expr;
    std::size_t source_size = 0;
    std::size_t erased_row = 0;
    std::size_t erased_column = 0;
    std::string row_map_lean;
    std::string column_map_lean;
};

struct SparseSupportCertificate {
    NodeId matrix = no_node;
    std::size_t row = 0;
    std::size_t unique_column = 0;
};

struct TriangularityCertificate {
    NodeId matrix = no_node;
    bool lower = true;
    std::string diagonal_rule;
};

struct DeterminantIdentity {
    NodeId matrix = no_node;
    std::string identity;
};

struct LemmaApplication {
    std::string theorem_name;
    std::string conclusion;
};

struct ProofObligation {
    std::string obligation_id;
    std::string proposition;
    std::string blocked_by;
};

struct TextObservation {
    std::string operation;
    std::string subject;
    std::string detail;
};

// Records that a specific integer matrix M (n x n, row-major flat)
// instantiates the GENERAL, already-checked Lean lemma
// `RavelGenerated.irrational_eigenvalue_has_no_integer_eigenvector`
// (lean/barge_diamond_lattice_line.lean): for THIS M, no nonzero
// integer vector w can satisfy the eigenvector equation at any index
// i for an irrational eigenvalue beta. w, i, and the eigenvector
// equation itself stay universally quantified in the emitted
// corollary -- only M (and its dimension) are concrete, matching what
// Barge & Diamond's Theorem 1 proof actually needs (the fact applies
// to whatever hypothetical vector/index the pigeonhole argument
// produces, not a specific one fixed in advance).
struct IntegerEigenvectorNoWitness {
    long long n = 0;
    std::vector<long long> matrix_flat;  // n*n, row-major
    std::string charpoly_description;    // human-readable, for the trace report
};

// Records that a specific nonnegative integer matrix M (n x n,
// row-major flat) instantiates the GENERAL, already-checked Lean
// lemma `RavelGenerated.period_coloring_rotates_eigenvalue`
// (lean/period_rotation_forces_equal_modulus.lean): its support graph
// carries an explicit integer coloring `coloring` (length n) such
// that M[i][j] != 0 forces coloring[i] = coloring[j] + 1 (mod p) --
// verified exactly in C++ before this node is recorded, and
// re-verified by the emitted Lean corollary itself (via `omega` on
// the concrete finite case split), not merely asserted. `p` is the
// jump-size gcd this project calls `g` (Findings 25/26/35); p >= 2 is
// exactly the condition under which the rotated eigenvalue lam*zeta⁻¹
// (for any p-th root of unity zeta != 1) is guaranteed to differ from
// lam while sharing its modulus -- the fact that rules out a unique
// dominant (Pisot) eigenvalue. v, lam, and zeta stay universally
// quantified in the emitted corollary; only M, p, and the coloring
// are concrete.
struct PeriodRotationCertificate {
    long long n = 0;
    long long p = 0;                      // period divisor (g)
    std::vector<long long> matrix_flat;   // n*n, row-major
    std::vector<long long> coloring;      // length n, integer levels
    std::vector<long long> k_flat;        // n*n, row-major; k[i][j] with
                                           // coloring[i] = coloring[j] + 1 + k[i][j]*p
                                           // wherever matrix_flat[i][j] != 0 (0 filler elsewhere)
    std::string description;              // human-readable, for the trace report
};

// Records that a substitution's images all share the same first
// letter `constant_letter` -- the premise of the general, hand-proven,
// kernel-checked Lean theorem
// `RavelGenerated.constant_first_letter_forces_prefix_coincidence`
// (lean/constant_first_letter_forces_prefix_coincidence.lean, Finding
// 17): for ANY pair of images both starting with `constant_letter`,
// strong coincidence resolves at depth 1, unconditionally. `images`
// carries every image (not just a flag) so the renderer can emit one
// concrete per-pair corollary for every pair of letters.
struct ConstantFirstLetterCertificate {
    long long d = 0;
    long long constant_letter = 0;
    std::vector<std::vector<long long>> images;  // d images, images[i][0] == constant_letter
    std::string description;
};

// The exact dual of `ConstantFirstLetterCertificate`, for Finding 38
// and the SUFFIX half of `pair_has_coincidence`: instantiates
// `RavelGenerated.constant_last_letter_forces_suffix_coincidence`
// (lean/constant_last_letter_forces_suffix_coincidence.lean).
struct ConstantLastLetterCertificate {
    long long d = 0;
    long long constant_letter = 0;
    std::vector<std::vector<long long>> images;  // d images, images[i].back() == constant_letter
    std::string description;
};

// A concrete bounded run of the strong-coincidence checker.  This is finite
// evidence for the supplied substitution, not an unconditional theorem about
// all substitutions; the search limits and aggregate result remain explicit.
struct StrongCoincidenceRunCertificate {
    std::vector<std::vector<long long>> images;
    // Lexicographic pair profile from the finite classifier.  Entries are
    // exact resolution depths, or -1 for a pair unresolved at the cutoff.
    std::vector<long long> pair_resolution_depths;
    std::vector<long long> pair_depths;
    long long depth_reached = 0;
    long long unresolved_pairs = 0;
    long long max_depth = 0;
    long long max_word_len = 0;
    bool holds = false;
    bool inconclusive = false;
    std::string description;
};

struct StrongCoincidencePairWitnessCertificate {
    long long first_letter = 0;
    long long second_letter = 0;
    long long depth = 0;
    long long common_letter = 0;
    long long first_position = 0;
    long long second_position = 0;
    std::vector<long long> first_prefix;
    std::vector<long long> second_prefix;
    std::vector<long long> first_suffix;
    std::vector<long long> second_suffix;
    bool prefix_match = false;
    bool suffix_match = false;
    std::string description;
};

// Records a zero-run of length R (Finding 39/41): two chain-offset
// positions `s1_offset`, `s2_offset` inside the run reach the run's
// terminal position R after `k1`, `k2` steps respectively (both equal
// R - offset). Instantiates
// `RavelGenerated.same_chain_forces_coincidence`
// (lean/zero_run_same_chain_coincidence.lean) on the run's own local
// chain model (`Fin (R+1)`, `next := offset -> offset+1` truncated
// with a self-loop at R) -- an honest, self-contained model of the
// run's dynamics, not the full substitution's alphabet (see that
// file's header comment for why the full-alphabet hypothesis can't be
// satisfied by a genuine substitution's own sigma beyond the chain).
struct ZeroRunSameChainCertificate {
    long long run_length = 0;      // R
    long long s1_offset = 0;
    long long s2_offset = 0;
    std::string description;
};

// The GENERAL case of Finding 39/41 (not just the same-chain special
// case `ZeroRunSameChainCertificate` covers): the substitution's FULL
// image data (`sigma`), two letters `i`, `j`, and a verified collision
// depth `k` at which their `firstLetterMap` orbits agree
// (`sigma[i-iterate][0] == sigma[j-iterate][0]` after `k` iterations,
// checked exactly in C++ before recording). Instantiates
// `RavelGenerated.first_letter_orbit_collision_forces_coincidence`
// (lean/first_letter_orbit_coincidence.lean) -- covers mixed
// in-run/out-of-run and cross-run pairs the same-chain lemma cannot.
struct FirstLetterOrbitCertificate {
    long long d = 0;
    std::vector<std::vector<long long>> images;  // d images, all nonempty
    long long i = 0;
    long long j = 0;
    long long k = 0;   // verified collision depth
    std::string description;
};

// The exact dual of `FirstLetterOrbitCertificate` for the SUFFIX side:
// instantiates `RavelGenerated.last_letter_orbit_collision_forces_coincidence`
// (lean/last_letter_orbit_coincidence.lean).
struct LastLetterOrbitCertificate {
    long long d = 0;
    std::vector<std::vector<long long>> images;
    long long i = 0;
    long long j = 0;
    long long k = 0;
    std::string description;
};

// Finding 27's leftmost-loop corollary: junction `start` lies on its
// own leftmost-branch cycle of length `loop_length`. Instantiates
// `RavelGenerated.periodic_point_iterate_mul`
// (lean/periodic_point_repetition.lean) to certify, for `m = 0..max_m`,
// that gap `m * loop_length` coincides at exactly `K = m * loop_length`.
struct LeftmostLoopCertificate {
    long long start = 0;
    long long loop_length = 0;
    long long max_m = 0;
    std::string description;
};

// `family_closed_forms.hpp`'s `certify_class_ii_adjacent_swap_count`/
// `certify_nbonacci_adjacent_swap_count` already construct the substitution
// and independently count its adjacent-unequal-letter swap sites -- this
// carries the CONCRETE images (as they actually are, not a placeholder) plus
// the verified count, so the renderer can have Lean recompute the count from
// the same images via `decide` rather than just restating the number.
struct AdjacentSwapCountCertificate {
    std::string family;  // "class_ii" or "nbonacci"
    long long a = 0, b = 0, n = 0;
    std::vector<std::vector<long long>> images;
    long long count = 0;
    std::string description;
};

// `cycle_charpoly_campaign_validation.lean`'s `concreteCycleMatrix_charpoly`
// (X^(n+1)-1) combined with `graph_cycle_charpoly_factor_validation.lean`'s
// `feeder_cycle_charpoly_closed` give the full closed form for a directed
// (n+1)-cycle plus one transient feeder edge -- this carries the CONCRETE n
// and feeder target index (matching `graph_structural_campaign.hpp`'s
// `cycle_with_transient_feeder`/`certify_feeder_cycle_block`), so the
// renderer instantiates both lemmas directly rather than restating the
// closed form as an unchecked comment.
struct FeederCycleCharpolyCertificate {
    long long n = 0;
    long long feeder_target = 0;
    std::string description;
};

// docs/FAMILY_OF_FAMILIES.md's displayed regular-shell closed forms for
// the three Class-II adjacent-swap neighbors (neighbor 0: x^6*(x^4-
// ((t+2)^2-2)x^2+1); neighbor 1: x^3*(x^2-t(t+2)); neighbor 2: x^2*(x^2-
// k^2)) -- verified here for the FIRST time against the concrete compressed
// matrix (class_ii_neighbor_compressed_matrix_for_states) via exact integer
// Faddeev-LeVerrier (self-checked internally via Cayley-Hamilton), so the
// renderer can instantiate lean/class_ii_neighbor_dominance.lean's
// neighbor{0,1,2}_shell_below_* theorems (previously flat: zero C++
// consumer) at this CONCRETE (neighbor, a, t).
struct RegularShellCharpolyCertificate {
    long long neighbor = 0;
    long long a = 0;
    long long t = 0;  // the shell parameter (t for neighbors 0/1, k for 2)
    std::string description;
};

// Finding 32, generalized (NOT specific to sigma_{0,2}): a depressed
// cubic x^3+c*x+d with a verified sign-change bracket (lo,hi) around
// its real root -- carries the CONCRETE (c,d) so the renderer
// instantiates the general lemma
// `RavelGenerated.depressed_cubic_q_gt_one_iff_beta_lt_neg_d`
// (lean/depressed_cubic_complex_pair_modulus.lean) per polynomial,
// not a single hardcoded citation.
struct DepressedCubicNotPisotCertificate {
    long long c = 0;
    long long d = 0;
    long long lo = 0;   // exact integer bracket: cubic(lo) < 0 < cubic(hi)
    long long hi = 0;
    std::string description;
};

// Finding 9 (Class-II thread), generalized: the interior-shell table
// at a CONCRETE round `q` -- carries `q` so the renderer instantiates
// `shellNode_propagates`/`shellNode_injective_at_round`
// (lean/class_ii_affine_shells.lean) at that exact round via
// `decide` (concrete integer arithmetic, no reals needed), one
// corollary per round actually constructed -- not a static citation.
struct ClassIIShellRoundCertificate {
    long long q = 0;
    std::string description;
};

// One node: (left, x0, x1, x2, right) -- matches ClassIINode's fields.
struct ClassIINodeData {
    long long left = 0, x0 = 0, x1 = 0, x2 = 0, right = 0;
};

// A fixed-size finite table `class_ii_boundary_family.hpp` actually
// constructed (D_cont, pre-contact, or contact), carrying the EXACT
// concrete nodes -- not just a name. The renderer decides membership
// of THESE nodes in the appropriate `lean/class_ii_affine_shells.lean`
// Kind's range, so a divergence between what C++ built and the Lean
// table would make the kernel check legitimately fail, not silently
// query an unrelated pre-written fact.
struct ClassIIFixedTableCertificate {
    std::string table;   // "d_cont", "pre_contact", or "contact"
    std::vector<ClassIINodeData> nodes;
};

// `class_ii_terminal_shell(a)` at a CONCRETE parameter `a` -- carries
// `a` so the renderer instantiates `lean/class_ii_terminal_shells.lean`'s
// `terminalCrossColours_not_eq_interior_extremes` AT that exact `a`
// via `decide`.
struct ClassIITerminalShellCertificate {
    long long a = 0;
};

// The boundary-layer source/target index sets `class_ii_neighbor_
// d_boundary_source_indices(neighbor)`/`_target_indices(neighbor)`
// ACTUALLY computed (from the C++ affine-edge catalog's slope-nonzero
// entries) -- carries the CONCRETE sets, so the renderer decides them
// equal to `lean/class_ii_neighbor_d_support.lean`'s own
// `neighborXBoundarySource`/`neighborXBoundaryTarget` (re-derived
// there, independently, from the Lean-side edge catalog), not merely
// citing that file's theorem names.
struct ClassIINeighborDSupportCertificate {
    long long neighbor = 0;
    std::vector<long long> sources;
    std::vector<long long> targets;
};

// A concrete 3x3 integer matrix M (row-major, 9 entries) for which
// the C++ side has already verified, by exact integer arithmetic,
// that M^3 = M + I -- only recorded when that check passes. The
// renderer re-derives M^3 independently inside Lean (via Mathlib's
// own `Matrix` power, not by re-stating the C++'s numbers) and
// `decide`s the identity against these EXACT entries -- e.g. the
// sigma_{0,1} incidence matrix's Cayley-Hamilton-style relation used
// (but never previously backed by an executable/checkable artifact)
// in Finding 23's argument.
struct CayleyHamiltonCubicCertificate {
    std::array<long long, 9> matrix{};
    std::string description;
};

// Finding 29: the a-bonacci family's dominant root at n=3 is strictly
// less than at n=4, which is strictly less than a+1 -- carries the
// EXACT rational brackets `pisot_classify_3x3`/`_4x4` (Sturm-chain
// isolation, no floating point) actually certified, so the renderer
// instantiates the general ordering lemma with these concrete numbers
// rather than a floating-point midpoint.
struct PisotRootOrderingCertificate {
    long long a = 0;
    long long hi3_num = 0, hi3_den = 1;
    long long lo4_num = 0, lo4_den = 1;
    long long hi4_num = 0, hi4_den = 1;
};

// One canonical arbitrary-precision rational coefficient. Numerator and
// positive denominator are decimal strings so exact classifier output is not
// narrowed to a machine integer at the reflection boundary.
struct ExactRationalCoefficient {
    std::string numerator;
    std::string denominator;
};

using ExactRationalPolynomial = std::vector<ExactRationalCoefficient>;

// Finding 30: a complete exact-Q Sturm/PRS certificate for one polynomial
// actually accepted by `pisot_classify_degree_n`. The chain, recurrence
// quotients/scales, Bezout witnesses, classifier bracket, and endpoint
// variations are all concrete data; Lean independently checks them before
// applying `CertifiedSturmChain.count_roots_between`.
struct SturmChainCertificate {
    ExactRationalPolynomial polynomial;
    std::vector<ExactRationalPolynomial> chain;
    std::vector<ExactRationalPolynomial> quotients;
    std::vector<ExactRationalCoefficient> positive_scales;
    ExactRationalPolynomial bezout_u;
    ExactRationalPolynomial bezout_v;
    ExactRationalCoefficient bezout_constant;
    ExactRationalCoefficient bracket_lo;
    ExactRationalCoefficient bracket_hi;
    long long variations_lo = 0;
    long long variations_hi = 0;
    std::vector<long long> signs_lo;
    std::vector<long long> signs_hi;
    long long root_count = 0;
    bool classifier_is_pisot = false;
    long long classifier_real_inside = 0;
    std::string description;
};

// A closed finite run of the property-(F) graph checker.  This is deliberately
// a summary certificate, not a claim about the infinite translation set Γ:
// the graph construction and its bounds remain the mathematical inputs, while
// these counters preserve the exact result that the checker actually returned.
struct PropertyFFiniteRunCertificate {
    bool closure_reached = false;
    bool archimedean_bound_applied = false;
    bool extra_bound_applied = false;
    long long node_budget = 0;
    long long boundary_edges = 0;
    long long nodes_explored = 0;
    long long zero_nodes = 0;
    long long nonzero_nodes = 0;
    long long strongly_connected_components = 0;
    long long nonzero_cycle_components = 0;
    bool holds = false;
    std::string description;
};

// Exact finite graph export accompanying a PropertyFFiniteRunCertificate.
// `gamma_keys` are canonical Q(beta) strings supplied by the producing
// checker; Lean-side rendering checks the finite topology and partition data
// without pretending that a string key is a real-number proof.
struct PropertyFGraphCertificate {
    std::vector<std::string> characteristic_polynomial;
    std::vector<std::vector<ExactRationalCoefficient>> beta_inverse_matrix;
    std::vector<long long> scc_labels;
    std::vector<long long> scc_sizes;
    long long nonzero_cycle_components = 0;
    std::vector<std::string> gamma_keys;
    std::vector<std::vector<ExactRationalCoefficient>> gamma_coefficients;
    std::vector<long long> letters;
    std::vector<bool> zero_nodes;
    std::vector<std::vector<long long>> successors;
    std::vector<std::vector<std::vector<ExactRationalCoefficient>>> edge_digit_coefficients;
    std::string description;
};

struct PropertyFViolationCertificate {
    std::vector<long long> cycle_nodes;
    std::vector<std::pair<long long, long long>> cycle_edges;
    std::string description;
};

// `class_ii_neighbor2_penultimate_promoted_states(a)`/`_survivor_
// transfer(a)` at a CONCRETE `a` -- carries the CONCRETE 6 promoted
// nodes and the 1 transferred node, so the renderer decides them
// equal to `lean/class_ii_six_vertex_graduation.lean`'s own
// `promotedNodes`/`transferredNode` (functions of q = a-1, GENERAL in
// q, not a fixed table) evaluated at that exact q.
struct ClassIISixVertexGraduationCertificate {
    long long a = 0;
    std::array<ClassIINodeData, 6> promoted{};
    ClassIINodeData transferred{};
};

// `class_ii_neighbor2_terminal_affine_states(a)` at a CONCRETE `a` --
// carries the CONCRETE 6-node affine sextet, so the renderer decides
// it equal to `lean/class_ii_neighbor2_extensions.lean`'s own
// `neighbor2TerminalSextet` (a function of `a`, GENERAL, not a fixed
// table) evaluated at that exact `a`.
struct ClassIITerminalSextetCertificate {
    long long a = 0;
    std::array<ClassIINodeData, 6> nodes{};
};

// `class_ii_neighbor2_penultimate_pair(a)` at a CONCRETE `a` --
// carries the CONCRETE 2-node pair, so the renderer decides it equal
// to `lean/class_ii_neighbor2_extensions.lean`'s own
// `neighbor2PenultimatePair` (a function of `a`, GENERAL, not a fixed
// table) evaluated at that exact `a`.
struct ClassIIPenultimatePairCertificate {
    long long a = 0;
    std::array<ClassIINodeData, 2> nodes{};
};

// `class_ii_neighbor2_interior_tip(r)` at a CONCRETE `r` -- carries
// the CONCRETE node, so the renderer decides it equal to `lean/
// class_ii_neighbor2_extensions.lean`'s own `neighbor2InteriorTip`
// (a function of `r`, already proven injective/infinite-range there)
// evaluated at that exact `r`.
struct ClassIIInteriorTipCertificate {
    long long r = 0;
    ClassIINodeData node{};
};

// `class_ii_neighbor2_global_round_phase(a, round)` at CONCRETE
// (a, round) -- carries the CONCRETE phase index (0=base, 1=stable,
// 2=penultimate, 3=terminal, 4=repeated) the C++ selector actually
// returned, so the renderer decides it equal to `lean/class_ii_
// global_round_partition.lean`'s own `classIIGlobalRoundPhase`
// (a total, GENERAL-in-(a,r) decision procedure) evaluated at that
// exact (a, round).
struct ClassIIGlobalRoundPhaseCertificate {
    long long a = 0;
    long long round = 0;
    int phase = 0;
};

// One (CONST, slope, target, a_required) instance from `class_ii_
// both_fixed_affine_instances()` with `a_required < 7` -- carries the
// CONCRETE integers, so the renderer instantiates `lean/class_ii_
// round234_shape_closure.lean`'s already-proven general
// `affine_no_solution_at_or_above_threshold` (threshold=7) at exactly
// this data.
struct BothFixedAffineCertificate {
    long long const_ = 0;
    long long slope = 0;
    long long target = 0;
    long long a_required = 0;
};

// lean/universal_shell_pumping_proof.lean's consumer: a concrete
// n-bonacci carry-cycle pumping step, independently replayed and closed
// by ravel::proof::certify_strict_shell_pump (translation-cycle replay,
// affine-transport replay, face alignment, strict radius growth --
// nothing here is trusted, all re-derived from the raw states/digits).
// Carries the concrete (source_radius, lifted_radius) pair so the
// renderer instantiates a two-point concrete Cycle type at those exact
// values, rather than merely restating the abstract StrictShellPump
// definition.
struct StrictShellPumpInstanceCertificate {
    std::string certificate_id;
    unsigned long long source_radius = 0;
    unsigned long long lifted_radius = 0;
    bool face_aligned = false;
};

// lean/playground_recurrent_family_exhaustion.lean's consumer: an
// independently extracted, per-component classification of every
// recurrent SCC of a concrete corona-truth graph (Tarjan SCC extraction,
// per-component grade range, structural-family predicate replay -- see
// ravel::proof::derive_recurrent_family_exhaustion and its real caller,
// tests/recurrent_family_exhaustion_real_test.cpp). Carries the exact
// per-component family assignment so the renderer instantiates the
// general `recurrent_family_exhaustion` theorem's `classified` premise
// at this CONCRETE dimension's component list, not merely restate it.
struct RecurrentFamilyExhaustionCertificate {
    long long dimension = 0;
    std::vector<std::string> family_kinds;  // one per component, in order
    std::string description;
};

// lean/predicted_core_scc_exhaustion.lean's consumer:
// ravel::proof::certify_predicted_core_scc independently runs Tarjan's
// algorithm over the concrete predicted-core graph at this dimension and
// verifies it forms exactly one SCC (`direct_tarjan_one_scc`), together
// with exact node/edge/predecessor-table counts checked against the
// closed-form combinatorial formulas. Carries the exact node count so the
// renderer instantiates `exact_scc_of_stronglyConnected_noReturnAfterExit`
// at a concrete `Fin node_count` vertex type with `Core := Set.univ`
// (trivially satisfying `hnoReturn`), taking the graph's verified strong
// connectivity as documented C++-verified input data -- the same trust
// boundary PisotRootOrderingCertificate/RegularShellCharpolyCertificate
// already have for their own C++-verified facts.
struct PredictedCoreSccExhaustionCertificate {
    long long dimension = 0;
    long long node_count = 0;
    long long edge_count = 0;
    std::string description;
};

// lean/coupled_automaton_characterization.lean's consumer:
// ravel::proof::certify_synthesized_winning_predicate independently
// re-evaluates a predicate tree at every concrete state's feature value
// (not trusting any pre-labeled "Win" set) and exhaustively checks the
// hinit/hstep/haccept obligations `synthesized_winning_predicate_sound`
// requires over the full finite state/input space. Carries the exact
// finite state count, per-state Win value, and initial/accepting state
// sets so the renderer instantiates the theorem at a concrete
// `Fin state_count` state type via `decide`, not merely restate it.
// lean/radial_translation_defect.lean's consumer:
// ravel::proof::certify_translation_defect independently computes the
// translated block-dynamics output, the base-plus-linear-translation
// value, and their difference (the "same translation defect") from raw
// integer matrix/vector arithmetic -- nothing here is asserted. Carries
// the exact concrete (block, state, translation, forcing) data so the
// renderer applies the general `affine_block_same_translation_defect`
// theorem at this CONCRETE instance and independently checks (via
// `decide`) that the C++-computed defect vector matches `B *ᵥ t - t`.
struct RadialTranslationDefectCertificate {
    std::vector<std::vector<long long>> block;
    std::vector<long long> state;
    std::vector<long long> translation;
    std::vector<long long> forcing;
    std::vector<long long> same_translation_defect;
    std::string description;
};

// lean/defect_spliced_covering_tube.lean's consumer:
// ravel::proof::certify_defect_spliced_tube independently re-verifies the
// base path's own n-bonacci recurrence (not previously checked by that
// function -- added here so nothing is pre-trusted), the translation
// window's recurrence, and the transported (spliced) path's recurrence at
// the adjusted digit, all via raw integer arithmetic. Carries one concrete
// per-step (x, t, digit, defect, base_next, translation_next,
// transported_next) tuple so the renderer applies the general
// `defect_splice_step` algebraic identity at this CONCRETE dimension-`dim`
// step and independently checks (via `decide`) that both sides equal the
// C++-computed successor states.
struct DefectSpliceStepCertificate {
    long long dim = 0;
    std::vector<long long> x;
    std::vector<long long> t;
    long long digit = 0;
    long long defect = 0;
    std::vector<long long> base_next;
    std::vector<long long> translation_next;
    std::vector<long long> transported_next;
    std::string description;
};

// lean/universal_dominance_shell_return_validation.lean's and
// lean/universal_dominance_phase_rank_transport.lean's consumer:
// ravel::proof::universal_dominance::validate_shell_return_certificate
// independently verifies (from raw edge/state data, nothing pre-trusted)
// that every state in a concrete finite relation has an outgoing edge
// (`certificate_closed`, exactly `hout`). Carries the exact concrete edge
// set so the renderer instantiates `no_strict_rank_relation_closed` /
// `shell_empty_of_strict_first_return_rank` /
// `no_nonempty_shell_with_strict_first_return_rank` /
// `shell_empty_of_phase_rank_transport` at this CONCRETE closed relation,
// keeping the rank function itself universally quantified (matching the
// theorems' own generality) rather than fixing one -- the interesting
// content is that NO integer rank could make this specific, C++-verified
// closed relation strict, not a witness to one that does (none exists).
struct UniversalDominanceClosedRelationCertificate {
    long long state_count = 0;
    std::vector<std::array<long long, 2>> edges;
    std::string description;
};

// lean/generated/condition_f_joint_qr_playground.lean's consumer -- the
// actual closure of the flagship universal n-bonacci boundary-dominance
// theorem (rho(G_B(n)) = rho(predicted_core(n)) for every n >= 3), via
// the canonical Q/R (balanced/one-sided-defect parent-prefix) split, NOT
// the earlier, explicitly abandoned shell-rank/carry-bound route (see
// diary 2026-08-05 "universal maximum-shell exclusion refuted": a
// triangular-wave counterfamily refuted universal shell-two acyclicity;
// "Do not resume"). ravel::proof::derive_condition_f_joint_pair_comparison
// independently re-derives, from the canonical parent-role Q/R matrices,
// that the base alphabet (A=2) has exactly one recurrent SCC (so the
// core pair is the whole recurrent pair and the identity is a
// simultaneous intertwiner) and that every dimension extension through
// the target reduces to a finite-depth acyclic boundary substitution in
// the SAME two generators -- nothing here is pre-trusted. Carries the
// exact target dimension checked so the renderer records which concrete
// extent of the induction this run re-verified.
struct ConditionFJointDominanceCertificate {
    long long target_dimension = 0;
    long long base_roles = 0;
    long long base_scc_count = 0;
    std::string description;
};

// lean/generated/finite_positive_grammar_majorant.lean's consumer -- the
// GENERAL theorem behind the Q/R (2-generator), plastic/supergolden
// (3-generator), and the found 4/5-generator witnesses: an arbitrary,
// possibly-unboundedly-large finite generator alphabet, not fixed at
// two or three. ravel::proof::derive_finite_positive_grammar_majorant
// independently re-derives, from raw per-channel multiplicity/norm-bound
// data, one exact rational count matrix and one exact rational
// norm-weighted matrix per generator; this certificate carries per-
// generator scalar sums (independently re-summed from those matrices,
// not trusted from the aggregate `.proved` flag alone) so the renderer
// instantiates `norm_generator_word_majorant` at this CONCRETE
// generator alphabet via `Fin generator_count`.
struct FinitePositiveGrammarMajorantReflectionCertificate {
    long long base_vertices = 0;
    long long generator_count = 0;
    std::vector<long long> count_scalar_num;
    std::vector<long long> count_scalar_den;
    std::vector<long long> norm_scalar_num;
    std::vector<long long> norm_scalar_den;
    std::string description;
};

struct ThirdSmallestPisotParryFactorizationCertificate {
    std::vector<long long> minimal_polynomial;
    std::vector<long long> parry_polynomial;
    std::vector<long long> cyclotomic_factor;
    std::string description;
};

struct GeneralizedMultinacciAdmissibleSubgrammarReflectionCertificate {
    long long dimension = 0;
    long long multiplicity = 0;
    long long source_states = 0;
    long long witnessed_edges = 0;
    long long words_checked = 0;
    std::string description;
};

struct GeneralizedMultinacciGeneralMReflectionCertificate {
    long long multiplicity = 0;
    std::string description;
};

struct GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate {
    long long multiplicity = 0;
    long long symbolic_cut_states = 0;
    long long words_checked = 0;
    std::string description;
};

struct GeneralizedMultinacciPrimitiveIntertwinerReflectionCertificate {
    long long dimension = 0;
    long long multiplicity = 0;
    long long boundary_expanded_states = 0;
    long long universal_expanded_states = 0;
    long long universal_macro_edges = 0;
    long long mapped_phase_states = 0;
    std::string description;
};

struct GeneralizedMultinacciSignedRenewalTwistReflectionCertificate {
    long long dimension = 0;
    long long multiplicity = 0;
    long long maximum_return_time = 0;
    long long macro_edges = 0;
    std::string description;
};

struct GeneralizedMultinacciSymbolicEmbeddingReflectionCertificate {
    long long dimension = 0;
    long long multiplicity = 0;
    long long total_parent_occurrences = 0;
    long long universal_macro_edges = 0;
    std::string description;
};

struct MonotoneProfileCorridorClosureReflectionCertificate {
    long long dimension = 0;
    long long thick_parents = 0;
    std::string description;
};

struct NormWeightedQRMajorantReflectionCertificate {
    long long base_vertices = 0;
    long long replayed_words_count = 0;
    bool all_channels_contractive = false;
    bool expansive_channel_detected = false;
    std::string description;
};

struct ThreeGeneratorIntertwinerFamilyReflectionCertificate {
    std::string family;  // "plastic" or "supergolden"
    long long generator_count = 0;
    long long boundary_states = 0;
    long long boundary_edges = 0;
    long long universal_edges = 0;
    std::string description;
};

struct ShiftBranchThreeGeneratorContinuationReflectionCertificate {
    long long dimension = 0;
    long long parent_occurrences = 0;
    long long neutral_pair_count = 0;
    std::string description;
};

// A lightweight certificate for fully abstract, unconditionally-proven
// general lemmas whose associated C++ certificate independently sweeps and
// re-verifies concrete instances that ALL already fall under the general
// theorem's scope (so no further per-instance Lean corollary is needed --
// the general theorem already covers every case the sweep checks).
struct GeneralInfraSweepConfirmedCertificate {
    std::string subject;  // identifies which lemma family this confirms
    std::string description;
};

struct CoefficientProfileParityObstructionReflectionCertificate {
    long long dimension = 0;
    bool even_dimension = false;
    std::string description;
};

struct CyclotomicObstructionReflectionCertificate {
    std::vector<long long> coefficients;  // ascending, coeff(0) = constant term
    bool has_order_two = false;
    long long eval_at_minus_one = 0;
    std::string description;
};

struct CyclicSpliceCompactnessReflectionCertificate {
    long long state_count = 0;
    std::vector<std::vector<long long>> successors;
    std::vector<long long> orbit_states;
    std::string description;
};

struct CyclicSpliceCompletionReflectionCertificate {
    long long state_count = 0;
    std::vector<long long> deterministic_next;  // next[i] = successors[i].front()
    long long initial_state = 0;
    long long transient_laps = 0;
    long long period_laps = 0;
    std::string description;
};

struct WinningPredicateReflectionCertificate {
    std::string certificate_id;
    long long state_count = 0;
    std::vector<bool> win;             // one per state, in order
    std::vector<long long> init_states;
    std::vector<long long> accept_states;
    std::vector<std::array<long long, 3>> transitions;  // (from, input, to)
    std::vector<long long> inputs;
};

using Payload = std::variant<MatrixFamily, MatrixInstance, EraseIndexMap,
                             SparseSupportCertificate, TriangularityCertificate,
                             DeterminantIdentity, LemmaApplication, IntegerEigenvectorNoWitness,
                             PeriodRotationCertificate, ConstantFirstLetterCertificate,
                             ConstantLastLetterCertificate, StrongCoincidenceRunCertificate,
                             StrongCoincidencePairWitnessCertificate,
                             ZeroRunSameChainCertificate,
                             FirstLetterOrbitCertificate, LastLetterOrbitCertificate,
                             LeftmostLoopCertificate, DepressedCubicNotPisotCertificate,
                             ClassIIShellRoundCertificate, ClassIIFixedTableCertificate,
                             ClassIITerminalShellCertificate, ClassIINeighborDSupportCertificate,
                             CayleyHamiltonCubicCertificate, PisotRootOrderingCertificate,
                             SturmChainCertificate, PropertyFFiniteRunCertificate,
                             PropertyFGraphCertificate,
                             PropertyFViolationCertificate,
                             ClassIISixVertexGraduationCertificate, ClassIITerminalSextetCertificate,
                             ClassIIPenultimatePairCertificate, ClassIIInteriorTipCertificate,
                             ClassIIGlobalRoundPhaseCertificate, BothFixedAffineCertificate,
                             AdjacentSwapCountCertificate, FeederCycleCharpolyCertificate,
                             RegularShellCharpolyCertificate, StrictShellPumpInstanceCertificate,
                             RecurrentFamilyExhaustionCertificate,
                             PredictedCoreSccExhaustionCertificate,
                             WinningPredicateReflectionCertificate,
                             RadialTranslationDefectCertificate,
                             DefectSpliceStepCertificate,
                             UniversalDominanceClosedRelationCertificate,
                             ConditionFJointDominanceCertificate,
                             FinitePositiveGrammarMajorantReflectionCertificate,
                             ThirdSmallestPisotParryFactorizationCertificate,
                             GeneralizedMultinacciAdmissibleSubgrammarReflectionCertificate,
                             GeneralizedMultinacciGeneralMReflectionCertificate,
                             GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate,
                             GeneralizedMultinacciPrimitiveIntertwinerReflectionCertificate,
                             GeneralizedMultinacciSignedRenewalTwistReflectionCertificate,
                             GeneralizedMultinacciSymbolicEmbeddingReflectionCertificate,
                             MonotoneProfileCorridorClosureReflectionCertificate,
                             NormWeightedQRMajorantReflectionCertificate,
                             ThreeGeneratorIntertwinerFamilyReflectionCertificate,
                             ShiftBranchThreeGeneratorContinuationReflectionCertificate,
                             GeneralInfraSweepConfirmedCertificate,
                             CoefficientProfileParityObstructionReflectionCertificate,
                             CyclotomicObstructionReflectionCertificate,
                             CyclicSpliceCompactnessReflectionCertificate,
                             CyclicSpliceCompletionReflectionCertificate,
                             ProofObligation, TextObservation>;

struct Node {
    NodeKind kind;
    Payload payload;
    std::vector<NodeId> parents;
};

// =============================================================================
// Typed piecewise entry IR -- method implementations.
//
// Kept together (and not as a separate translation unit) so the math library
// remains header-only. Every method here is purely functional and depends only
// on the variant's own fields.
// =============================================================================

inline DimExpr DimExpr::lit(int64_t v) {
    DimExpr e; e.op = Op::Const; e.const_value = v; return e;
}

inline DimExpr DimExpr::var(std::string name) {
    DimExpr e; e.op = Op::Var; e.var_name = std::move(name); return e;
}

inline DimExpr DimExpr::sub(DimExpr a, DimExpr b) {
    DimExpr e; e.op = Op::Sub; e.operands = {std::move(a), std::move(b)}; return e;
}

inline int64_t DimExpr::eval(int64_t n_var) const {
    switch (op) {
        case Op::Const: return const_value;
        case Op::Var:   return n_var;
        case Op::Sub:   return operands[0].eval(n_var) - operands[1].eval(n_var);
    }
    return 0;
}

inline int64_t DimExpr::const_fold() const noexcept {
    return op == Op::Const ? const_value : 0;
}

inline std::string DimExpr::to_lean() const {
    switch (op) {
        case Op::Const: return std::to_string(const_value);
        case Op::Var:   return var_name;
        case Op::Sub:   return "(" + operands[0].to_lean() + " - " + operands[1].to_lean() + ")";
    }
    return "0";
}

inline IndexExpr IndexExpr::lit(int64_t k) {
    IndexExpr e; e.op = Op::Lit; e.literal = k; return e;
}

inline IndexExpr IndexExpr::ivar() {
    IndexExpr e; e.op = Op::IVar; return e;
}

inline IndexExpr IndexExpr::jvar() {
    IndexExpr e; e.op = Op::JVar; return e;
}

inline IndexExpr IndexExpr::dim(std::string name) {
    IndexExpr e; e.op = Op::DimVar; e.dim_var = std::move(name); return e;
}

inline IndexExpr IndexExpr::add(IndexExpr a, IndexExpr b) {
    IndexExpr e; e.op = Op::Add; e.operands = {std::move(a), std::move(b)}; return e;
}

inline IndexExpr IndexExpr::sub(IndexExpr a, IndexExpr b) {
    IndexExpr e; e.op = Op::Sub; e.operands = {std::move(a), std::move(b)}; return e;
}

inline int64_t IndexExpr::eval(int64_t i, int64_t j, int64_t n_var) const {
    switch (op) {
        case Op::Lit:    return literal;
        case Op::IVar:   return i;
        case Op::JVar:   return j;
        case Op::DimVar: return n_var;
        case Op::Add:    return operands[0].eval(i, j, n_var) + operands[1].eval(i, j, n_var);
        case Op::Sub:    return operands[0].eval(i, j, n_var) - operands[1].eval(i, j, n_var);
    }
    return 0;
}

inline std::string IndexExpr::to_lean() const {
    switch (op) {
        case Op::Lit:    return std::to_string(literal);
        case Op::IVar:   return "i.val";
        case Op::JVar:   return "j.val";
        case Op::DimVar: return dim_var;
        case Op::Add:    return "(" + operands[0].to_lean() + " + " + operands[1].to_lean() + ")";
        case Op::Sub:    return "(" + operands[0].to_lean() + " - " + operands[1].to_lean() + ")";
    }
    return "0";
}

inline EntryCondition EntryCondition::tru() {
    return {};
}

inline EntryCondition EntryCondition::eq(IndexExpr a, IndexExpr b) {
    EntryCondition c; c.op = Op::Eq; c.left = std::move(a); c.right = std::move(b); return c;
}

inline EntryCondition EntryCondition::lt(IndexExpr a, IndexExpr b) {
    EntryCondition c; c.op = Op::Lt; c.left = std::move(a); c.right = std::move(b); return c;
}

inline EntryCondition EntryCondition::and_(EntryCondition a, EntryCondition b) {
    EntryCondition c; c.op = Op::And; c.operands = {std::move(a), std::move(b)}; return c;
}

inline EntryCondition EntryCondition::or_(EntryCondition a, EntryCondition b) {
    EntryCondition c; c.op = Op::Or; c.operands = {std::move(a), std::move(b)}; return c;
}

inline EntryCondition EntryCondition::not_(EntryCondition a) {
    EntryCondition c; c.op = Op::Not; c.operands = {std::move(a)}; return c;
}

inline bool EntryCondition::eval(int64_t i, int64_t j, int64_t n_var) const {
    switch (op) {
        case Op::True: return true;
        case Op::Eq:   return left.eval(i, j, n_var) == right.eval(i, j, n_var);
        case Op::Lt:   return left.eval(i, j, n_var) <  right.eval(i, j, n_var);
        case Op::And:  return operands[0].eval(i, j, n_var) && operands[1].eval(i, j, n_var);
        case Op::Or:   return operands[0].eval(i, j, n_var) || operands[1].eval(i, j, n_var);
        case Op::Not:  return !operands[0].eval(i, j, n_var);
    }
    return false;
}

inline std::string EntryCondition::to_lean() const {
    switch (op) {
        case Op::True: return "True";
        case Op::Eq:   return "(" + left.to_lean() + " = " + right.to_lean() + ")";
        case Op::Lt:   return "(" + left.to_lean() + " < " + right.to_lean() + ")";
        case Op::And:  return "(" + operands[0].to_lean() + " ∧ " + operands[1].to_lean() + ")";
        case Op::Or:   return "(" + operands[0].to_lean() + " ∨ " + operands[1].to_lean() + ")";
        case Op::Not:  return "¬" + operands[0].to_lean();
    }
    return "True";
}

inline EntryExpression EntryExpression::zero() {
    return {};
}

inline EntryExpression EntryExpression::one() {
    EntryExpression e; e.op = Op::One; return e;
}

inline EntryExpression EntryExpression::neg_one() {
    EntryExpression e; e.op = Op::NegOne; return e;
}

inline EntryExpression EntryExpression::var_x() {
    EntryExpression e; e.op = Op::VarX; return e;
}

inline EntryExpression EntryExpression::one_plus_var_x() {
    EntryExpression e; e.op = Op::OnePlusVarX; return e;
}

inline EntryExpression EntryExpression::constant(int64_t k) {
    EntryExpression e; e.op = Op::Const; e.const_value = k; return e;
}

inline EntryExpression EntryExpression::add(EntryExpression a, EntryExpression b) {
    EntryExpression e; e.op = Op::Add; e.operands = {std::move(a), std::move(b)}; return e;
}

inline int64_t EntryExpression::eval_as_int(int64_t x_value) const {
    switch (op) {
        case Op::Zero:       return 0;
        case Op::One:        return 1;
        case Op::NegOne:     return -1;
        case Op::Const:      return const_value;
        case Op::VarX:       return x_value;
        case Op::OnePlusVarX: return 1 + x_value;
        case Op::Add:        return operands[0].eval_as_int(x_value) + operands[1].eval_as_int(x_value);
    }
    return 0;
}

inline std::string EntryExpression::to_lean() const {
    switch (op) {
        case Op::Zero:        return "(0 : Polynomial ℤ)";
        case Op::One:         return "(Polynomial.C 1 : Polynomial ℤ)";
        case Op::NegOne:      return "(Polynomial.C (-1) : Polynomial ℤ)";
        case Op::Const:       return "(Polynomial.C " + std::to_string(const_value) + " : Polynomial ℤ)";
        case Op::VarX:        return "(Polynomial.X : Polynomial ℤ)";
        case Op::OnePlusVarX: return "(Polynomial.C 1 + Polynomial.X : Polynomial ℤ)";
        case Op::Add:         return "(" + operands[0].to_lean() + " + " + operands[1].to_lean() + ")";
    }
    return "(0 : Polynomial ℤ)";
}

inline EntryBranch EntryBranch::make(EntryCondition c, EntryExpression e, std::string hr) {
    EntryBranch b;
    b.condition = std::move(c);
    b.expression = std::move(e);
    b.human_readable = std::move(hr);
    return b;
}

inline int64_t PiecewiseEntry::eval_dim(int64_t n_var) const {
    // Both dimensions agree for every family we currently model.
    return row_dim.eval(n_var);
}

inline EntryExpression PiecewiseEntry::eval(int64_t i, int64_t j, int64_t n_var) const {
    for (const auto& branch : branches) {
        if (branch.condition.eval(i, j, n_var)) return branch.expression;
    }
    return EntryExpression::zero();
}

inline std::string PiecewiseEntry::to_lean() const {
    if (branches.empty()) return "0";
    // Encode as nested `if ... then ... else ...`.
    // Branches are tested in declared order; the first match wins.
    // We build the chain from the bottom up: start with the last branch
    // (treated as the fallthrough) and prepend each earlier branch.
    std::string body = branches.back().expression.to_lean();
    for (std::size_t k = branches.size(); k-- > 1; ) {
        const auto& branch = branches[k - 1];
        body = "if " + branch.condition.to_lean() + " then " +
               branch.expression.to_lean() + " else " + body;
    }
    return body;
}

inline std::string PiecewiseEntry::type_lean(const std::string& ring) const {
    return "Matrix (Fin (" + row_dim.to_lean() + ")) (Fin (" + col_dim.to_lean() + ")) " + ring;
}

inline std::string PiecewiseEntry::definition_lean(
        const std::string& family_name,
        std::vector<std::pair<std::string,std::string>> binders,
        const std::string& ring) const {
    std::string out = "noncomputable def " + family_name;
    for (const auto& [name, type] : binders) out += " (" + name + " : " + type + ")";
    out += " :\n    " + type_lean(ring) + " :=\n  fun i j => " + to_lean();
    return out;
}

// Convenience renderer that produces only the right-hand side of a
// `noncomputable def ... := ...` declaration: the body of `fun i j => ...`.
// Useful when a caller is constructing a `LeanDefinition` value and
// already supplies the type and binders.
inline std::string PiecewiseEntry::definition_lean_value() const {
    return "fun i j => " + to_lean();
}

// =============================================================================

inline std::string payload_name(const Payload& payload) {
    return std::visit([](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, MatrixFamily>) return "matrix.family";
        else if constexpr (std::is_same_v<T, MatrixInstance>) return "matrix.instance";
        else if constexpr (std::is_same_v<T, EraseIndexMap>) return "matrix.erase_index_map";
        else if constexpr (std::is_same_v<T, SparseSupportCertificate>) return "matrix.sparse_support";
        else if constexpr (std::is_same_v<T, TriangularityCertificate>) return "matrix.triangularity";
        else if constexpr (std::is_same_v<T, DeterminantIdentity>) return "matrix.determinant_identity";
        else if constexpr (std::is_same_v<T, LemmaApplication>) return "lean.lemma_application";
        else if constexpr (std::is_same_v<T, IntegerEigenvectorNoWitness>) return "lean.integer_eigenvector_no_witness";
        else if constexpr (std::is_same_v<T, PeriodRotationCertificate>) return "lean.period_rotation_certificate";
        else if constexpr (std::is_same_v<T, ConstantFirstLetterCertificate>) return "lean.constant_first_letter_certificate";
        else if constexpr (std::is_same_v<T, ConstantLastLetterCertificate>) return "lean.constant_last_letter_certificate";
        else if constexpr (std::is_same_v<T, StrongCoincidenceRunCertificate>) return "lean.strong_coincidence_run_certificate";
        else if constexpr (std::is_same_v<T, StrongCoincidencePairWitnessCertificate>) return "lean.strong_coincidence_pair_witness_certificate";
        else if constexpr (std::is_same_v<T, ZeroRunSameChainCertificate>) return "lean.zero_run_same_chain_certificate";
        else if constexpr (std::is_same_v<T, FirstLetterOrbitCertificate>) return "lean.first_letter_orbit_certificate";
        else if constexpr (std::is_same_v<T, LastLetterOrbitCertificate>) return "lean.last_letter_orbit_certificate";
        else if constexpr (std::is_same_v<T, LeftmostLoopCertificate>) return "lean.leftmost_loop_certificate";
        else if constexpr (std::is_same_v<T, DepressedCubicNotPisotCertificate>) return "lean.depressed_cubic_not_pisot_certificate";
        else if constexpr (std::is_same_v<T, ClassIIShellRoundCertificate>) return "lean.class_ii_shell_round_certificate";
        else if constexpr (std::is_same_v<T, ClassIIFixedTableCertificate>) return "lean.class_ii_fixed_table_certificate";
        else if constexpr (std::is_same_v<T, ClassIITerminalShellCertificate>) return "lean.class_ii_terminal_shell_certificate";
        else if constexpr (std::is_same_v<T, ClassIINeighborDSupportCertificate>) return "lean.class_ii_neighbor_d_support_certificate";
        else if constexpr (std::is_same_v<T, CayleyHamiltonCubicCertificate>) return "lean.cayley_hamilton_cubic_certificate";
        else if constexpr (std::is_same_v<T, PisotRootOrderingCertificate>) return "lean.pisot_root_ordering_certificate";
        else if constexpr (std::is_same_v<T, SturmChainCertificate>) return "lean.sturm_chain_certificate";
        else if constexpr (std::is_same_v<T, PropertyFFiniteRunCertificate>) return "lean.property_f_finite_run_certificate";
        else if constexpr (std::is_same_v<T, PropertyFGraphCertificate>) return "lean.property_f_graph_certificate";
        else if constexpr (std::is_same_v<T, PropertyFViolationCertificate>) return "lean.property_f_violation_certificate";
        else if constexpr (std::is_same_v<T, ClassIISixVertexGraduationCertificate>) return "lean.class_ii_six_vertex_graduation_certificate";
        else if constexpr (std::is_same_v<T, ClassIITerminalSextetCertificate>) return "lean.class_ii_terminal_sextet_certificate";
        else if constexpr (std::is_same_v<T, ClassIIPenultimatePairCertificate>) return "lean.class_ii_penultimate_pair_certificate";
        else if constexpr (std::is_same_v<T, ClassIIInteriorTipCertificate>) return "lean.class_ii_interior_tip_certificate";
        else if constexpr (std::is_same_v<T, ClassIIGlobalRoundPhaseCertificate>) return "lean.class_ii_global_round_phase_certificate";
        else if constexpr (std::is_same_v<T, BothFixedAffineCertificate>) return "lean.both_fixed_affine_certificate";
        else if constexpr (std::is_same_v<T, AdjacentSwapCountCertificate>) return "lean.adjacent_swap_count_certificate";
        else if constexpr (std::is_same_v<T, FeederCycleCharpolyCertificate>) return "lean.feeder_cycle_charpoly_certificate";
        else if constexpr (std::is_same_v<T, RegularShellCharpolyCertificate>) return "lean.regular_shell_charpoly_certificate";
        else if constexpr (std::is_same_v<T, StrictShellPumpInstanceCertificate>) return "lean.strict_shell_pump_instance_certificate";
        else if constexpr (std::is_same_v<T, RecurrentFamilyExhaustionCertificate>) return "lean.recurrent_family_exhaustion_certificate";
        else if constexpr (std::is_same_v<T, PredictedCoreSccExhaustionCertificate>) return "lean.predicted_core_scc_exhaustion_certificate";
        else if constexpr (std::is_same_v<T, WinningPredicateReflectionCertificate>) return "lean.winning_predicate_certificate";
        else if constexpr (std::is_same_v<T, RadialTranslationDefectCertificate>) return "lean.radial_translation_defect_certificate";
        else if constexpr (std::is_same_v<T, DefectSpliceStepCertificate>) return "lean.defect_splice_step_certificate";
        else if constexpr (std::is_same_v<T, UniversalDominanceClosedRelationCertificate>) return "lean.universal_dominance_closed_relation_certificate";
        else if constexpr (std::is_same_v<T, ConditionFJointDominanceCertificate>) return "lean.condition_f_joint_dominance_certificate";
        else if constexpr (std::is_same_v<T, FinitePositiveGrammarMajorantReflectionCertificate>) return "lean.finite_positive_grammar_majorant_certificate";
        else if constexpr (std::is_same_v<T, ThirdSmallestPisotParryFactorizationCertificate>) return "lean.third_smallest_pisot_parry_factorization_certificate";
        else if constexpr (std::is_same_v<T, GeneralizedMultinacciAdmissibleSubgrammarReflectionCertificate>) return "lean.generalized_multinacci_admissible_subgrammar_certificate";
        else if constexpr (std::is_same_v<T, GeneralizedMultinacciGeneralMReflectionCertificate>) return "lean.generalized_multinacci_general_m_certificate";
        else if constexpr (std::is_same_v<T, GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate>) return "lean.generalized_multinacci_general_m_intertwiner_certificate";
        else if constexpr (std::is_same_v<T, GeneralizedMultinacciPrimitiveIntertwinerReflectionCertificate>) return "lean.generalized_multinacci_primitive_intertwiner_certificate";
        else if constexpr (std::is_same_v<T, GeneralizedMultinacciSignedRenewalTwistReflectionCertificate>) return "lean.generalized_multinacci_signed_renewal_twist_certificate";
        else if constexpr (std::is_same_v<T, GeneralizedMultinacciSymbolicEmbeddingReflectionCertificate>) return "lean.generalized_multinacci_symbolic_embedding_certificate";
        else if constexpr (std::is_same_v<T, MonotoneProfileCorridorClosureReflectionCertificate>) return "lean.monotone_profile_corridor_closure_certificate";
        else if constexpr (std::is_same_v<T, NormWeightedQRMajorantReflectionCertificate>) return "lean.norm_weighted_qr_majorant_certificate";
        else if constexpr (std::is_same_v<T, ThreeGeneratorIntertwinerFamilyReflectionCertificate>) return "lean.three_generator_intertwiner_family_certificate";
        else if constexpr (std::is_same_v<T, ShiftBranchThreeGeneratorContinuationReflectionCertificate>) return "lean.shift_branch_three_generator_continuation_certificate";
        else if constexpr (std::is_same_v<T, GeneralInfraSweepConfirmedCertificate>) return "lean.general_infra_sweep_confirmed_certificate";
        else if constexpr (std::is_same_v<T, CoefficientProfileParityObstructionReflectionCertificate>) return "lean.coefficient_profile_parity_obstruction_certificate";
        else if constexpr (std::is_same_v<T, CyclotomicObstructionReflectionCertificate>) return "lean.cyclotomic_obstruction_certificate";
        else if constexpr (std::is_same_v<T, CyclicSpliceCompactnessReflectionCertificate>) return "lean.cyclic_splice_compactness_certificate";
        else if constexpr (std::is_same_v<T, CyclicSpliceCompletionReflectionCertificate>) return "lean.cyclic_splice_completion_certificate";
        else if constexpr (std::is_same_v<T, ProofObligation>) return "proof.obligation";
        else return value.operation;
    }, payload);
}

inline std::string payload_detail(const Payload& payload) {
    return std::visit([](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        std::ostringstream out;
        if constexpr (std::is_same_v<T, MatrixFamily>) {
            out << value.family_id << " : " << value.rows.lean << " x " << value.columns.lean
                << ", entry=" << value.entry_rule << ", structure=" << value.structure;
        } else if constexpr (std::is_same_v<T, MatrixInstance>) {
            out << value.family_id << "(n=" << value.parameter_n << ") = "
                << value.rows << "x" << value.columns;
        } else if constexpr (std::is_same_v<T, EraseIndexMap>) {
            out << "delete (" << value.erased_row << ',' << value.erased_column << ") from "
                << value.source_size << "x" << value.source_size
                << "; row=" << value.row_map_lean << "; col=" << value.column_map_lean;
            if (!value.erased_row_expr.is_constant() || value.erased_row_expr.const_fold() != static_cast<int64_t>(value.erased_row)
                || !value.source_size_expr.is_constant() || value.source_size_expr.const_fold() != static_cast<int64_t>(value.source_size)) {
                out << " [sym: source=" << value.source_size_expr.to_lean()
                    << ", row=" << value.erased_row_expr.to_lean()
                    << ", col=" << value.erased_column_expr.to_lean() << "]";
            }
        } else if constexpr (std::is_same_v<T, SparseSupportCertificate>) {
            out << "row " << value.row << " has unique support at column " << value.unique_column;
        } else if constexpr (std::is_same_v<T, TriangularityCertificate>) {
            out << (value.lower ? "lower" : "upper") << " triangular; diagonal=" << value.diagonal_rule;
        } else if constexpr (std::is_same_v<T, DeterminantIdentity>) {
            out << value.identity;
        } else if constexpr (std::is_same_v<T, LemmaApplication>) {
            out << value.theorem_name << " proves " << value.conclusion;
        } else if constexpr (std::is_same_v<T, IntegerEigenvectorNoWitness>) {
            out << "n=" << value.n << " M=" << value.charpoly_description
                << " -- instantiates irrational_eigenvalue_has_no_integer_eigenvector";
        } else if constexpr (std::is_same_v<T, PeriodRotationCertificate>) {
            out << "n=" << value.n << " p=" << value.p << " " << value.description
                << " -- instantiates period_coloring_rotates_eigenvalue";
        } else if constexpr (std::is_same_v<T, ConstantFirstLetterCertificate>) {
            out << "d=" << value.d << " c=" << value.constant_letter << " " << value.description
                << " -- instantiates constant_first_letter_forces_prefix_coincidence";
        } else if constexpr (std::is_same_v<T, ConstantLastLetterCertificate>) {
            out << "d=" << value.d << " c=" << value.constant_letter << " " << value.description
                << " -- instantiates constant_last_letter_forces_suffix_coincidence";
        } else if constexpr (std::is_same_v<T, ZeroRunSameChainCertificate>) {
            out << "R=" << value.run_length << " offsets(" << value.s1_offset << "," << value.s2_offset
                << ") " << value.description << " -- instantiates same_chain_forces_coincidence";
        } else if constexpr (std::is_same_v<T, FirstLetterOrbitCertificate>) {
            out << "d=" << value.d << " i=" << value.i << " j=" << value.j << " k=" << value.k
                << " " << value.description << " -- instantiates first_letter_orbit_collision_forces_coincidence";
        } else if constexpr (std::is_same_v<T, LastLetterOrbitCertificate>) {
            out << "d=" << value.d << " i=" << value.i << " j=" << value.j << " k=" << value.k
                << " " << value.description << " -- instantiates last_letter_orbit_collision_forces_coincidence";
        } else if constexpr (std::is_same_v<T, LeftmostLoopCertificate>) {
            out << "start=" << value.start << " L=" << value.loop_length << " max_m=" << value.max_m
                << " " << value.description << " -- instantiates periodic_point_iterate_mul";
        } else if constexpr (std::is_same_v<T, DepressedCubicNotPisotCertificate>) {
            out << "c=" << value.c << " d=" << value.d << " bracket=(" << value.lo << "," << value.hi
                << ") " << value.description << " -- instantiates depressed_cubic_q_gt_one_iff_beta_lt_neg_d";
        } else if constexpr (std::is_same_v<T, ClassIIShellRoundCertificate>) {
            out << "q=" << value.q << " " << value.description
                << " -- instantiates shellNode_propagates/shellNode_injective_at_round";
        } else if constexpr (std::is_same_v<T, ClassIIFixedTableCertificate>) {
            out << value.table << " table, " << value.nodes.size() << " concrete nodes";
        } else if constexpr (std::is_same_v<T, ClassIITerminalShellCertificate>) {
            out << "a=" << value.a << " -- instantiates terminalCrossColours_not_eq_interior_extremes";
        } else if constexpr (std::is_same_v<T, ClassIINeighborDSupportCertificate>) {
            out << "neighbor=" << value.neighbor << " sources=" << value.sources.size()
                << " targets=" << value.targets.size()
                << " -- instantiates neighborXBoundarySource/TargetG";
        } else if constexpr (std::is_same_v<T, CayleyHamiltonCubicCertificate>) {
            out << value.description << " -- M^3 = M + I, verified by exact integer arithmetic";
        } else if constexpr (std::is_same_v<T, PisotRootOrderingCertificate>) {
            out << "a=" << value.a << " -- instantiates pisot_root_strictly_between";
        } else if constexpr (std::is_same_v<T, SturmChainCertificate>) {
            out << value.description << " -- chain=" << value.chain.size()
                << " V(lo)=" << value.variations_lo << " V(hi)=" << value.variations_hi
                << " roots=" << value.root_count
                << " classifier_pisot=" << (value.classifier_is_pisot ? "true" : "false");
        } else if constexpr (std::is_same_v<T, PropertyFFiniteRunCertificate>) {
            out << value.description << " -- nodes=" << value.nodes_explored
                << " zero=" << value.zero_nodes << " nonzero=" << value.nonzero_nodes
                << " scc=" << value.strongly_connected_components
                << " nonzero_cycles=" << value.nonzero_cycle_components
                << " holds=" << (value.holds ? "true" : "false");
        } else if constexpr (std::is_same_v<T, StrongCoincidenceRunCertificate>) {
            out << value.description << " -- depth=" << value.depth_reached
                << " unresolved=" << value.unresolved_pairs
                << " holds=" << (value.holds ? "true" : "false")
                << " inconclusive=" << (value.inconclusive ? "true" : "false");
        } else if constexpr (std::is_same_v<T, StrongCoincidencePairWitnessCertificate>) {
            out << value.description << " -- pair=(" << value.first_letter << ","
                << value.second_letter << ") depth=" << value.depth
                << " common=" << value.common_letter;
        } else if constexpr (std::is_same_v<T, PropertyFGraphCertificate>) {
            std::size_t edges = 0;
            for (const auto& row : value.successors) edges += row.size();
            out << value.description << " -- nodes=" << value.gamma_keys.size()
                << " edges=" << edges;
        } else if constexpr (std::is_same_v<T, PropertyFViolationCertificate>) {
            out << value.description << " -- cycle_nodes=" << value.cycle_nodes.size()
                << " cycle_edges=" << value.cycle_edges.size();
        } else if constexpr (std::is_same_v<T, ClassIISixVertexGraduationCertificate>) {
            out << "a=" << value.a << " -- instantiates promotedNodes/transferredNode at q=" << (value.a - 1);
        } else if constexpr (std::is_same_v<T, ClassIITerminalSextetCertificate>) {
            out << "a=" << value.a << " -- instantiates neighbor2TerminalSextet";
        } else if constexpr (std::is_same_v<T, ClassIIPenultimatePairCertificate>) {
            out << "a=" << value.a << " -- instantiates neighbor2PenultimatePair";
        } else if constexpr (std::is_same_v<T, ClassIIInteriorTipCertificate>) {
            out << "r=" << value.r << " -- instantiates neighbor2InteriorTip";
        } else if constexpr (std::is_same_v<T, ClassIIGlobalRoundPhaseCertificate>) {
            out << "a=" << value.a << " round=" << value.round << " phase=" << value.phase
                << " -- instantiates classIIGlobalRoundPhase";
        } else if constexpr (std::is_same_v<T, BothFixedAffineCertificate>) {
            out << "const=" << value.const_ << " slope=" << value.slope
                << " target=" << value.target << " a_required=" << value.a_required
                << " -- instantiates affine_no_solution_at_or_above_threshold";
        } else if constexpr (std::is_same_v<T, AdjacentSwapCountCertificate>) {
            out << value.family << " count=" << value.count << " " << value.description
                << " -- instantiates adjacentUnequalCount";
        } else if constexpr (std::is_same_v<T, FeederCycleCharpolyCertificate>) {
            out << "n=" << value.n << " feeder_target=" << value.feeder_target << " " << value.description
                << " -- instantiates feeder_cycle_charpoly_closed + concreteCycleMatrix_charpoly";
        } else if constexpr (std::is_same_v<T, RegularShellCharpolyCertificate>) {
            out << "neighbor=" << value.neighbor << " a=" << value.a << " t=" << value.t << " "
                << value.description << " -- instantiates neighbor_shell_below_*";
        } else if constexpr (std::is_same_v<T, StrictShellPumpInstanceCertificate>) {
            out << value.certificate_id << " radius " << value.source_radius << " -> "
                << value.lifted_radius << " face_aligned=" << (value.face_aligned ? "true" : "false")
                << " -- instantiates StrictShellPump";
        } else if constexpr (std::is_same_v<T, RecurrentFamilyExhaustionCertificate>) {
            out << "dimension=" << value.dimension << " components=" << value.family_kinds.size()
                << " " << value.description << " -- instantiates recurrent_family_exhaustion";
        } else if constexpr (std::is_same_v<T, PredictedCoreSccExhaustionCertificate>) {
            out << "dimension=" << value.dimension << " nodes=" << value.node_count
                << " edges=" << value.edge_count << " " << value.description
                << " -- instantiates exact_scc_of_stronglyConnected_noReturnAfterExit";
        } else if constexpr (std::is_same_v<T, WinningPredicateReflectionCertificate>) {
            out << value.certificate_id << " states=" << value.state_count
                << " -- instantiates synthesized_winning_predicate_sound";
        } else if constexpr (std::is_same_v<T, RadialTranslationDefectCertificate>) {
            out << "dim=" << value.state.size() << " " << value.description
                << " -- instantiates affine_block_same_translation_defect";
        } else if constexpr (std::is_same_v<T, DefectSpliceStepCertificate>) {
            out << "dim=" << value.dim << " " << value.description
                << " -- instantiates defect_splice_step";
        } else if constexpr (std::is_same_v<T, UniversalDominanceClosedRelationCertificate>) {
            out << "states=" << value.state_count << " edges=" << value.edges.size() << " "
                << value.description << " -- instantiates no_strict_rank_relation_closed";
        } else if constexpr (std::is_same_v<T, ConditionFJointDominanceCertificate>) {
            out << "target_dimension=" << value.target_dimension << " base_scc_count="
                << value.base_scc_count << " " << value.description
                << " -- instantiates universal_dominance_sandwich";
        } else if constexpr (std::is_same_v<T, FinitePositiveGrammarMajorantReflectionCertificate>) {
            out << "generators=" << value.generator_count << " base_vertices=" << value.base_vertices
                << " " << value.description << " -- instantiates norm_generator_word_majorant";
        } else if constexpr (std::is_same_v<T, ThirdSmallestPisotParryFactorizationCertificate>) {
            out << "deg(minpoly)=" << (value.minimal_polynomial.empty() ? 0 : value.minimal_polynomial.size() - 1)
                << " " << value.description << " -- instantiates third_smallest_pisot_parry_factorization";
        } else if constexpr (std::is_same_v<T, GeneralizedMultinacciAdmissibleSubgrammarReflectionCertificate>) {
            out << "D=" << value.dimension << " m=" << value.multiplicity
                << " states=" << value.source_states << " edges=" << value.witnessed_edges
                << " words=" << value.words_checked << " " << value.description
                << " -- instantiates admissible_subgrammar_intertwines";
        } else if constexpr (std::is_same_v<T, GeneralizedMultinacciGeneralMReflectionCertificate>) {
            out << "m=" << value.multiplicity << " " << value.description
                << " -- instantiates schedulerCoefficient_zero/schedulerCoefficient_positive";
        } else if constexpr (std::is_same_v<T, GeneralizedMultinacciGeneralMIntertwinerReflectionCertificate>) {
            out << "m=" << value.multiplicity << " states=" << value.symbolic_cut_states
                << " words=" << value.words_checked << " " << value.description
                << " -- instantiates roof_word_intertwiner";
        } else if constexpr (std::is_same_v<T, GeneralizedMultinacciPrimitiveIntertwinerReflectionCertificate>) {
            out << "D=" << value.dimension << " m=" << value.multiplicity
                << " boundary_states=" << value.boundary_expanded_states
                << " universal_states=" << value.universal_expanded_states << " "
                << value.description << " -- instantiates labelled_subgraph_intertwiner";
        } else if constexpr (std::is_same_v<T, GeneralizedMultinacciSignedRenewalTwistReflectionCertificate>) {
            out << "D=" << value.dimension << " m=" << value.multiplicity
                << " roof=" << value.maximum_return_time << " " << value.description
                << " -- instantiates defect_roof_bounded";
        } else if constexpr (std::is_same_v<T, GeneralizedMultinacciSymbolicEmbeddingReflectionCertificate>) {
            out << "D=" << value.dimension << " m=" << value.multiplicity
                << " total_parents=" << value.total_parent_occurrences << " "
                << value.description << " -- instantiates deletion_only_subsum";
        } else if constexpr (std::is_same_v<T, MonotoneProfileCorridorClosureReflectionCertificate>) {
            out << "D=" << value.dimension << " k=" << value.thick_parents << " "
                << value.description << " -- instantiates corridor_extra_occurrences";
        } else if constexpr (std::is_same_v<T, NormWeightedQRMajorantReflectionCertificate>) {
            out << "base_vertices=" << value.base_vertices << " words=" << value.replayed_words_count
                << " contractive=" << (value.all_channels_contractive ? "true" : "false")
                << " expansive=" << (value.expansive_channel_detected ? "true" : "false") << " "
                << value.description << " -- instantiates norm_qr_word_majorant";
        } else if constexpr (std::is_same_v<T, ThreeGeneratorIntertwinerFamilyReflectionCertificate>) {
            out << value.family << " boundary_states=" << value.boundary_states
                << " boundary_edges=" << value.boundary_edges << " " << value.description
                << " -- instantiates " << value.family << "_word_intertwiner";
        } else if constexpr (std::is_same_v<T, ShiftBranchThreeGeneratorContinuationReflectionCertificate>) {
            out << "D=" << value.dimension << " parents=" << value.parent_occurrences << " "
                << value.description << " -- instantiates three_generator_word_induction";
        } else if constexpr (std::is_same_v<T, GeneralInfraSweepConfirmedCertificate>) {
            out << value.subject << " " << value.description;
        } else if constexpr (std::is_same_v<T, CoefficientProfileParityObstructionReflectionCertificate>) {
            out << "D=" << value.dimension << " even=" << (value.even_dimension ? "true" : "false")
                << " " << value.description
                << " -- instantiates nearest_left_profile_even_has_minus_one_root/nearest_left_profile_odd_value";
        } else if constexpr (std::is_same_v<T, CyclotomicObstructionReflectionCertificate>) {
            out << "deg=" << (value.coefficients.empty() ? 0 : value.coefficients.size() - 1)
                << " has_order_two=" << (value.has_order_two ? "true" : "false") << " "
                << value.description << " -- instantiates x_add_one_dvd_iff_eval_neg_one_zero";
        } else if constexpr (std::is_same_v<T, CyclicSpliceCompactnessReflectionCertificate>) {
            out << "state_count=" << value.state_count << " orbit_length=" << value.orbit_states.size()
                << " " << value.description << " -- instantiates ClosedOrbit";
        } else if constexpr (std::is_same_v<T, CyclicSpliceCompletionReflectionCertificate>) {
            out << "state_count=" << value.state_count << " transient=" << value.transient_laps
                << " period=" << value.period_laps << " " << value.description
                << " -- instantiates finite_serial_relation_has_repeated_orbit";
        } else if constexpr (std::is_same_v<T, ProofObligation>) {
            out << value.obligation_id << ": " << value.proposition;
            if (!value.blocked_by.empty()) out << " [blocked by " << value.blocked_by << ']';
        } else {
            out << value.subject;
            if (!value.detail.empty()) out << " -- " << value.detail;
        }
        return out.str();
    }, payload);
}

class Trace {
public:
    explicit Trace(std::string theorem_id = {}) : theorem_id_(std::move(theorem_id)) {}

    NodeId record(Node node) {
        for (const auto parent : node.parents) {
            if (parent != no_node && parent >= nodes_.size())
                throw std::logic_error("reflection node has a nonexistent parent");
        }
        nodes_.push_back(std::move(node));
        return nodes_.size() - 1;
    }

    const std::string& theorem_id() const noexcept { return theorem_id_; }
    const std::vector<Node>& nodes() const noexcept { return nodes_; }
    const std::vector<Node>& events() const noexcept { return nodes_; } // compatibility
    bool empty() const noexcept { return nodes_.empty(); }

    template <typename T>
    std::vector<std::pair<NodeId, const T*>> find() const {
        std::vector<std::pair<NodeId, const T*>> result;
        for (NodeId i = 0; i < nodes_.size(); ++i)
            if (const auto* value = std::get_if<T>(&nodes_[i].payload)) result.emplace_back(i, value);
        return result;
    }

    std::string debug_report() const {
        std::ostringstream out;
        out << "reflective proof graph: " << theorem_id_ << "\n";
        for (NodeId i = 0; i < nodes_.size(); ++i) {
            const auto& node = nodes_[i];
            out << '[' << i << "] " << payload_name(node.payload) << " :: "
                << payload_detail(node.payload);
            if (!node.parents.empty()) {
                out << " <-";
                for (auto p : node.parents) if (p != no_node) out << ' ' << p;
            }
            out << '\n';
        }
        return out.str();
    }

private:
    std::string theorem_id_;
    std::vector<Node> nodes_;
};

inline thread_local Trace* active_trace = nullptr;

class ScopedTrace {
public:
    explicit ScopedTrace(Trace* trace) : previous_(active_trace) { active_trace = trace; }
    ScopedTrace(const ScopedTrace&) = delete;
    ScopedTrace& operator=(const ScopedTrace&) = delete;
    ~ScopedTrace() { active_trace = previous_; }
private:
    Trace* previous_;
};

inline bool enabled() noexcept { return active_trace != nullptr; }

inline NodeId record(NodeKind kind, Payload payload, std::vector<NodeId> parents = {}) {
    if (!active_trace) return no_node;
    return active_trace->record({kind, std::move(payload), std::move(parents)});
}

inline NodeId observe(std::string operation, std::string subject,
                      std::string detail = {}, std::vector<NodeId> parents = {}) {
    return record(NodeKind::Observation,
                  TextObservation{std::move(operation), std::move(subject), std::move(detail)},
                  std::move(parents));
}

// Records a `GeneralInfraSweepConfirmedCertificate`: the caller has just
// independently re-verified (by direct assertion/exception, not merely a
// cached bool) a concrete C++ sweep whose every case already falls under
// a fully abstract, unconditionally kernel-checked Lean theorem. Callers
// must only invoke this AFTER the real check has passed -- there is no
// separate `.proved` gate here since the caller's own verification (an
// `assert`/`throw`-checked sweep) is the gate.
inline void confirm_general_infra_sweep(const std::string& subject, const std::string& description) {
    if (!enabled()) return;
    GeneralInfraSweepConfirmedCertificate node;
    node.subject = subject;
    node.description = description;
    record(NodeKind::LemmaApplication, node);
}

} // namespace mathlib::reflection
