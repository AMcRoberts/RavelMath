#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/ball.hpp"
#include "ravel/proof/reflective_collatz_weight.hpp"

namespace ravel::proof {

inline std::vector<std::vector<long long>> transpose_nonnegative_matrix(
    const std::vector<std::vector<long long>>& matrix) {
    if (matrix.empty())
        throw std::invalid_argument("paired matrix dominance: empty matrix");
    const auto n = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != n)
            throw std::invalid_argument("paired matrix dominance: nonsquare matrix");
        for (const auto value : row)
            if (value < 0)
                throw std::invalid_argument("paired matrix dominance: negative entry");
    }
    std::vector<std::vector<long long>> result(
        n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            result[j][i] = matrix[i][j];
    return result;
}

inline std::vector<mathlib::BigInt> exact_matrix_vector_product(
    const std::vector<std::vector<long long>>& matrix,
    const std::vector<mathlib::BigInt>& vector) {
    if (matrix.size() != vector.size())
        throw std::invalid_argument("paired matrix dominance: product size mismatch");
    std::vector<mathlib::BigInt> result;
    result.reserve(matrix.size());
    for (const auto& row : matrix) {
        if (row.size() != vector.size())
            throw std::invalid_argument("paired matrix dominance: product nonsquare");
        mathlib::BigInt sum(0);
        for (std::size_t j = 0; j < row.size(); ++j) {
            if (row[j] == 0) continue;
            mathlib::BigInt term;
            mathlib::mul_si(term, vector[j], row[j]);
            mathlib::BigInt next;
            mathlib::add(next, sum, term);
            sum = std::move(next);
        }
        result.push_back(std::move(sum));
    }
    return result;
}

// A and B are the competitor and core matrices.  The proof derives a positive
// right weight u for A and a positive left weight v for B (equivalently a
// right weight for B^T).  The implicit rectangular matrix P = u v^T then
// satisfies
//
//     A P < P B
//
// entrywise whenever max_i (Au)_i/u_i < min_j (B^T v)_j/v_j.  P is never
// materialized: every entry inequality is replayed exactly as
//
//     (Au)_i v_j < u_i (B^T v)_j.
//
// This is a genuine paired-matrix proof operation.  The matrices act on their
// own positive cones to derive the two factors of the intertwiner.
struct RankOnePairedDominance {
    ReflectiveCollatzWeight competitor_right;
    ReflectiveCollatzWeight core_left;
    std::vector<mathlib::BigInt> competitor_step;
    std::vector<mathlib::BigInt> core_left_step;
    std::size_t implicit_entries = 0;
    std::size_t strict_entries = 0;
    bool bracket_separated = false;
    bool entrywise_intertwiner = false;
    bool replayed = false;
};

inline RankOnePairedDominance derive_rank_one_paired_dominance(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::vector<long long>>& core,
    std::size_t iterations = 80) {
    RankOnePairedDominance out;
    out.competitor_right =
        derive_reflective_collatz_weight(competitor, iterations);
    const auto core_transpose = transpose_nonnegative_matrix(core);
    out.core_left =
        derive_reflective_collatz_weight(core_transpose, iterations);
    out.competitor_step = exact_matrix_vector_product(
        competitor, out.competitor_right.weight);
    out.core_left_step = exact_matrix_vector_product(
        core_transpose, out.core_left.weight);

    out.bracket_separated = mathlib::cmp(
        out.competitor_right.bracket.hi,
        out.core_left.bracket.lo) < 0;

    out.entrywise_intertwiner = true;
    out.implicit_entries =
        out.competitor_right.weight.size() * out.core_left.weight.size();
    for (std::size_t i = 0; i < out.competitor_right.weight.size(); ++i) {
        for (std::size_t j = 0; j < out.core_left.weight.size(); ++j) {
            mathlib::BigInt lhs, rhs;
            mathlib::mul(lhs, out.competitor_step[i], out.core_left.weight[j]);
            mathlib::mul(rhs, out.competitor_right.weight[i], out.core_left_step[j]);
            const auto comparison = mathlib::cmp(lhs, rhs);
            if (comparison >= 0) out.entrywise_intertwiner = false;
            else ++out.strict_entries;
        }
    }

    out.replayed = out.competitor_right.replayed &&
                   out.core_left.replayed &&
                   out.competitor_right.positive &&
                   out.core_left.positive &&
                   out.bracket_separated &&
                   out.entrywise_intertwiner &&
                   out.strict_entries == out.implicit_entries;
    return out;
}

}  // namespace ravel::proof
