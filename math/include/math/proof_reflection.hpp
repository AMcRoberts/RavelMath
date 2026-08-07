#pragma once

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

using Payload = std::variant<MatrixFamily, MatrixInstance, EraseIndexMap,
                             SparseSupportCertificate, TriangularityCertificate,
                             DeterminantIdentity, LemmaApplication, IntegerEigenvectorNoWitness,
                             PeriodRotationCertificate, ConstantFirstLetterCertificate,
                             ConstantLastLetterCertificate, ZeroRunSameChainCertificate,
                             FirstLetterOrbitCertificate, LastLetterOrbitCertificate,
                             LeftmostLoopCertificate, DepressedCubicNotPisotCertificate,
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
        else if constexpr (std::is_same_v<T, ZeroRunSameChainCertificate>) return "lean.zero_run_same_chain_certificate";
        else if constexpr (std::is_same_v<T, FirstLetterOrbitCertificate>) return "lean.first_letter_orbit_certificate";
        else if constexpr (std::is_same_v<T, LastLetterOrbitCertificate>) return "lean.last_letter_orbit_certificate";
        else if constexpr (std::is_same_v<T, LeftmostLoopCertificate>) return "lean.leftmost_loop_certificate";
        else if constexpr (std::is_same_v<T, DepressedCubicNotPisotCertificate>) return "lean.depressed_cubic_not_pisot_certificate";
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

} // namespace mathlib::reflection
