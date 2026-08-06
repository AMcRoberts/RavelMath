#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "math/poly_z.hpp"
#include "math/proof_reflection.hpp"

namespace mathlib {

struct PolyMatrixZ {
    std::vector<std::vector<PolyZ>> entries;
    reflection::NodeId provenance = reflection::no_node;
    std::string family_id;
    std::size_t parameter_n = 0;

    PolyMatrixZ() = default;
    PolyMatrixZ(std::size_t rows, std::size_t columns, const PolyZ& fill = PolyZ(0))
        : entries(rows, std::vector<PolyZ>(columns, fill)) {}

    std::size_t size() const noexcept { return entries.size(); }
    auto begin() noexcept { return entries.begin(); }
    auto end() noexcept { return entries.end(); }
    auto begin() const noexcept { return entries.begin(); }
    auto end() const noexcept { return entries.end(); }
    std::vector<PolyZ>& operator[](std::size_t i) { return entries[i]; }
    const std::vector<PolyZ>& operator[](std::size_t i) const { return entries[i]; }
};

inline reflection::NodeId declare_matrix_family(
        const std::string& id, const std::string& dimension,
        std::size_t concrete_dimension, const std::string& entry_rule,
        const std::string& structure,
        reflection::MatrixEntryRuleKind entry_rule_kind = reflection::MatrixEntryRuleKind::Unknown,
        reflection::MatrixStructureKind structure_kind = reflection::MatrixStructureKind::Unknown,
        reflection::PiecewiseEntry piecewise = {}) {
    reflection::MatrixFamily family{id, {dimension, concrete_dimension}, {dimension, concrete_dimension},
                                    entry_rule, structure, entry_rule_kind, structure_kind,
                                    std::move(piecewise)};
    return reflection::record(reflection::NodeKind::Object, std::move(family));
}

inline PolyMatrixZ erase_row_column(const PolyMatrixZ& matrix,
                                    std::size_t row, std::size_t column) {
    const std::size_t n = matrix.size();
    if (row >= n || column >= n) throw std::out_of_range("erase_row_column");
    PolyMatrixZ minor(n - 1, n - 1);
    for (std::size_t r = 0, rr = 0; r < n; ++r) {
        if (r == row) continue;
        for (std::size_t c = 0, cc = 0; c < n; ++c) {
            if (c == column) continue;
            minor[rr][cc++] = matrix[r][c];
        }
        ++rr;
    }
    minor.family_id = matrix.family_id.empty() ? "anonymous.minor" : matrix.family_id + ".minor";
    minor.parameter_n = matrix.parameter_n;
    // The erased row/column are concrete integers here; their symbolic
    // expressions are the constant `0` (or whatever `row`/`column` actually
    // is). Callers that erase a parametric position construct their own
    // EraseIndexMap node with a non-constant `DimExpr` directly.
    reflection::EraseIndexMap node{
        matrix.provenance,
        reflection::DimExpr::lit(static_cast<int64_t>(n)),
        reflection::DimExpr::lit(static_cast<int64_t>(row)),
        reflection::DimExpr::lit(static_cast<int64_t>(column)),
        n, row, column,
        "Ravel.Matrix.EraseIndex.skip " + std::to_string(row),
        "Ravel.Matrix.EraseIndex.skip " + std::to_string(column),
    };
    minor.provenance = reflection::record(reflection::NodeKind::Reduction, std::move(node),
        matrix.provenance == reflection::no_node ? std::vector<reflection::NodeId>{}
                                                 : std::vector<reflection::NodeId>{matrix.provenance});
    return minor;
}

inline PolyZ det(const PolyMatrixZ& matrix) {
    const std::size_t n = matrix.size();
    for (const auto& row : matrix)
        if (row.size() != n) throw std::invalid_argument("det: matrix must be square");
    const auto enter = reflection::observe("matrix.det.enter",
        matrix.family_id.empty() ? std::to_string(n) + "x" + std::to_string(n) : matrix.family_id,
        "dimension=" + std::to_string(n),
        matrix.provenance == reflection::no_node ? std::vector<reflection::NodeId>{}
                                                 : std::vector<reflection::NodeId>{matrix.provenance});
    if (n == 0) return PolyZ(1);
    if (n == 1) return matrix[0][0];

    const PolyZ zero(0);
    std::size_t pivot_row = n;
    std::size_t pivot_col = n;
    for (std::size_t i = 0; i < n; ++i) {
        std::size_t nonzeros = 0;
        std::size_t candidate = n;
        for (std::size_t j = 0; j < n; ++j) {
            if (matrix[i][j] != zero) { ++nonzeros; candidate = j; }
        }
        if (nonzeros == 1) { pivot_row = i; pivot_col = candidate; break; }
    }

    if (pivot_row != n) {
        const auto support = reflection::record(reflection::NodeKind::Certificate,
            reflection::SparseSupportCertificate{matrix.provenance, pivot_row, pivot_col}, {enter});
        const PolyZ sign = ((pivot_row + pivot_col) % 2 == 0) ? PolyZ(1) : PolyZ(-1);
        const auto minor = erase_row_column(matrix, pivot_row, pivot_col);
        const PolyZ result = sign * matrix[pivot_row][pivot_col] * det(minor);
        reflection::record(reflection::NodeKind::Certificate,
            reflection::DeterminantIdentity{matrix.provenance,
                "det obtained by the certified singleton-row cofactor"}, {support, minor.provenance});
        return result;
    }

    PolyZ sum(0);
    for (std::size_t j = 0; j < n; ++j) {
        if (matrix[0][j] == zero) continue;
        const PolyZ sign = (j % 2 == 0) ? PolyZ(1) : PolyZ(-1);
        sum = sum + sign * matrix[0][j] * det(erase_row_column(matrix, 0, j));
    }
    reflection::record(reflection::NodeKind::Certificate,
        reflection::DeterminantIdentity{matrix.provenance, "det obtained by first-row cofactor expansion"},
        {enter});
    return sum;
}

// The shared typed piecewise entry for the n-bonacci q-matrix family.
//
//   q_matrix n is (n-1) x (n-1) with
//     q[i, i]   = X         for i < n-2
//     q[i, i+1] = -1        for i < n-2
//     q[n-2, j] = 1         for j < n-2
//     q[n-2, n-2] = X + 1   (terminal diagonal overrides the row pattern)
//
// The branches are ordered most-specific first so the first-match-wins
// semantics in `PiecewiseEntry::eval` reproduces the C++ loop above.
inline reflection::PiecewiseEntry nbonacci_q_piecewise_entry() {
    using reflection::IndexExpr;
    using reflection::EntryCondition;
    using reflection::EntryExpression;
    using reflection::EntryBranch;
    using reflection::DimExpr;

    const auto dim_n = DimExpr::var("n");
    const auto n_minus_2 = DimExpr::sub(dim_n, DimExpr::lit(2));
    const auto dim_n_as_index = IndexExpr::dim("n");
    const auto n_minus_2_as_index = IndexExpr::sub(dim_n_as_index, IndexExpr::lit(2));
    const auto i = IndexExpr::ivar();
    const auto j = IndexExpr::jvar();

    reflection::PiecewiseEntry entry;
    entry.family_id = "nbonacci.q";
    entry.row_dim = DimExpr::sub(dim_n, DimExpr::lit(1));
    entry.col_dim = entry.row_dim;
    entry.kind = reflection::MatrixEntryRuleKind::UpperBidiagonalWithTerminalRow;
    entry.structure = reflection::MatrixStructureKind::UpperBidiagonalPlusTerminalRow;

    // Branch 1: terminal row AND terminal column -> X + 1.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::and_(
            EntryCondition::eq(i, n_minus_2_as_index),
            EntryCondition::eq(j, n_minus_2_as_index)),
        EntryExpression::one_plus_var_x(),
        "terminal row & terminal col: X+1"));

    // Branch 2: terminal row AND j < n-2 -> 1.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::and_(
            EntryCondition::eq(i, n_minus_2_as_index),
            EntryCondition::lt(j, n_minus_2_as_index)),
        EntryExpression::one(),
        "terminal row & non-terminal col: 1"));

    // Branch 3: i < n-2 AND j = i + 1 (superdiagonal) -> -1.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::and_(
            EntryCondition::lt(i, n_minus_2_as_index),
            EntryCondition::eq(j, IndexExpr::add(i, IndexExpr::lit(1)))),
        EntryExpression::neg_one(),
        "superdiagonal: -1"));

    // Branch 4: i < n-2 AND j = i (diagonal) -> X.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::and_(
            EntryCondition::lt(i, n_minus_2_as_index),
            EntryCondition::eq(j, i)),
        EntryExpression::var_x(),
        "diagonal: X"));

    // Branch 5: fallthrough -> 0.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::tru(),
        EntryExpression::zero(),
        "fallthrough: 0"));

    return entry;
}

// The shared typed piecewise entry for the n-bonacci r-matrix family.
//
//   r_matrix n is (n-1) x (n-1) with
//     r[i, i]   = -1    for all i
//     r[i+1, i] = X     for i < n-2
// (lower bidiagonal with constant -1 diagonal and X subdiagonal).
inline reflection::PiecewiseEntry nbonacci_r_piecewise_entry() {
    using reflection::IndexExpr;
    using reflection::EntryCondition;
    using reflection::EntryExpression;
    using reflection::EntryBranch;
    using reflection::DimExpr;

    const auto i = IndexExpr::ivar();
    const auto j = IndexExpr::jvar();

    reflection::PiecewiseEntry entry;
    entry.family_id = "nbonacci.r";
    entry.row_dim = DimExpr::sub(DimExpr::var("n"), DimExpr::lit(1));
    entry.col_dim = entry.row_dim;
    entry.kind = reflection::MatrixEntryRuleKind::LowerBidiagonalXMinusOne;
    entry.structure = reflection::MatrixStructureKind::LowerBidiagonal;

    // Branch 1: i = j + 1 (subdiagonal) -> X.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::eq(i, IndexExpr::add(j, IndexExpr::lit(1))),
        EntryExpression::var_x(),
        "subdiagonal: X"));

    // Branch 2: i = j (diagonal) -> -1.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::eq(i, j),
        EntryExpression::neg_one(),
        "diagonal: -1"));

    // Branch 3: fallthrough -> 0.
    entry.branches.push_back(EntryBranch::make(
        EntryCondition::tru(),
        EntryExpression::zero(),
        "fallthrough: 0"));

    return entry;
}

inline PolyMatrixZ nbonacci_r_matrix(std::size_t n) {
    if (n < 2) throw std::invalid_argument("nbonacci_r_matrix: n must be >= 2");
    const auto family = declare_matrix_family("nbonacci.r", "n - 1", n - 1,
        "if j + 1 = i then X else if i = j then -1 else 0", "lower_bidiagonal",
        reflection::MatrixEntryRuleKind::LowerBidiagonalXMinusOne,
        reflection::MatrixStructureKind::LowerBidiagonal,
        nbonacci_r_piecewise_entry());
    PolyMatrixZ matrix(n - 1, n - 1);
    for (std::size_t i = 0; i + 1 < n - 1; ++i) {
        matrix[i][i] = PolyZ(-1);
        matrix[i + 1][i] = PolyZ({0, 1});
    }
    matrix[n - 2][n - 2] = PolyZ(-1);
    matrix.family_id = "nbonacci.r";
    matrix.parameter_n = n;
    matrix.provenance = reflection::record(reflection::NodeKind::Object,
        reflection::MatrixInstance{"nbonacci.r", n, n - 1, n - 1}, {family});
    const auto triangular = reflection::record(reflection::NodeKind::Certificate,
        reflection::TriangularityCertificate{matrix.provenance, true, "constant (-1)"},
        {matrix.provenance});
    (void)triangular; // consumed and discharged by the executable proof campaign engine
    return matrix;
}

inline PolyMatrixZ nbonacci_q_matrix(std::size_t n) {
    if (n < 2) throw std::invalid_argument("nbonacci_q_matrix: n must be >= 2");
    const auto family = declare_matrix_family("nbonacci.q", "n - 1", n - 1,
        "upper bidiagonal X/-1 with terminal row 1,...,1,1+X",
        "upper_bidiagonal_plus_terminal_row",
        reflection::MatrixEntryRuleKind::UpperBidiagonalWithTerminalRow,
        reflection::MatrixStructureKind::UpperBidiagonalPlusTerminalRow,
        nbonacci_q_piecewise_entry());
    PolyMatrixZ matrix(n - 1, n - 1);
    for (std::size_t i = 0; i + 1 < n - 1; ++i) {
        matrix[i][i] = PolyZ({0, 1});
        matrix[i][i + 1] = PolyZ(-1);
    }
    for (std::size_t j = 0; j + 1 < n - 1; ++j) matrix[n - 2][j] = PolyZ(1);
    matrix[n - 2][n - 2] = PolyZ({1, 1});
    matrix.family_id = "nbonacci.q";
    matrix.parameter_n = n;
    matrix.provenance = reflection::record(reflection::NodeKind::Object,
        reflection::MatrixInstance{"nbonacci.q", n, n - 1, n - 1}, {family});
    reflection::record(reflection::NodeKind::Obligation,
        reflection::ProofObligation{"q_minor_reduction",
            "the recurrence cofactor minor of qMatrix (n+1) is qMatrix n after erase-index transport",
            "typed terminal-row branches and symbolic erased-index positions"},
        {matrix.provenance});
    return matrix;
}

inline PolyZ geometric_sum_z(std::size_t n) {
    PolyZ result(0);
    PolyZ power(1);
    const PolyZ x({0, 1});
    for (std::size_t k = 0; k < n; ++k) {
        result = result + power;
        power = power * x;
    }
    reflection::observe("polynomial.geometric_sum", "sum[0," + std::to_string(n) + ")",
                        "constructed by successor recurrence");
    return result;
}

inline PolyZ nbonacci_charpoly_z(std::size_t n) {
    const PolyZ result = geometric_sum_z(n + 1) - PolyZ(2);
    reflection::observe("polynomial.nbonacci_charpoly", "n=" + std::to_string(n),
                        "X + ... + X^n - 1");
    return result;
}

} // namespace mathlib
