#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "math/ball.hpp"
#include "ravel/proof/paired_matrix_dominance.hpp"

namespace ravel::proof {

// The k-th path-count cone vector is M^k 1.  Coordinate i counts weighted
// directed walks of length k starting at i.  For M^T it counts walks ending at
// i.  Thus the positive cone weights used by the paired proof have a direct,
// dimension-independent grammar: extend every represented walk by one edge.
struct PathCountCone {
    std::vector<mathlib::BigInt> weight;
    std::vector<mathlib::BigInt> action;
    std::vector<mathlib::BigInt> primitive_next;
    mathlib::Ball bracket;
    std::size_t horizon = 0;
    bool positive = false;
    bool recurrence_replayed = false;
};

inline PathCountCone derive_path_count_cone(
    const std::vector<std::vector<long long>>& matrix,
    std::size_t horizon) {
    if (matrix.empty()) throw std::invalid_argument("path-count cone: empty matrix");
    const auto n = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != n) throw std::invalid_argument("path-count cone: nonsquare matrix");
        for (const auto x : row)
            if (x < 0) throw std::invalid_argument("path-count cone: negative entry");
    }
    std::vector<mathlib::BigInt> current(n, mathlib::BigInt(1));
    for (std::size_t k = 0; k < horizon; ++k)
        current = mathlib::collatz_step_exact(matrix, current).next_v;

    PathCountCone out;
    out.weight = current;
    out.action = exact_matrix_vector_product(matrix, out.weight);
    out.horizon = horizon;
    const auto exact = mathlib::collatz_step_exact(matrix, out.weight);
    out.primitive_next = exact.next_v;
    out.bracket = exact.bracket;
    // The primitive path-count recurrence is M w_k = g_k w_{k+1}.
    // Verify that every raw action coordinate has the same exact scale.
    out.recurrence_replayed = out.action.size() == out.primitive_next.size();
    mathlib::BigInt scale;
    bool have_scale = false;
    for (std::size_t i = 0; i < out.action.size() && out.recurrence_replayed; ++i) {
        if (mathlib::cmp_si(out.primitive_next[i], 0) <= 0) {
            out.recurrence_replayed = false;
            break;
        }
        mathlib::BigInt q, r;
        mpz_tdiv_qr(q.get(), r.get(), out.action[i].get(), out.primitive_next[i].get());
        if (mathlib::cmp_si(r, 0) != 0) { out.recurrence_replayed = false; break; }
        if (!have_scale) { mathlib::set(scale, q); have_scale = true; }
        else if (mathlib::cmp(scale, q) != 0) out.recurrence_replayed = false;
    }
    out.positive = true;
    for (const auto& x : out.weight)
        out.positive = out.positive && mathlib::cmp_si(x, 0) > 0;
    return out;
}

struct PathCountConePair {
    PathCountCone competitor_right;
    PathCountCone core_left;
    std::size_t first_separating_horizon = 0;
    bool separated = false;
    bool entrywise_intertwiner = false;
    bool replayed = false;
};

// Search for the first finite walk horizon at which the outgoing path-count
// cone of A is strictly below the incoming path-count cone of B.  At that
// horizon u=A^k1 and v=(B^T)^k1 generate the implicit rank-one matrix uv^T.
inline PathCountConePair derive_path_count_cone_pair(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::vector<long long>>& core,
    std::size_t max_horizon = 80) {
    PathCountConePair out;
    const auto bt = transpose_nonnegative_matrix(core);
    for (std::size_t k = 0; k <= max_horizon; ++k) {
        auto right = derive_path_count_cone(competitor, k);
        auto left = derive_path_count_cone(bt, k);
        if (mathlib::cmp(right.bracket.hi, left.bracket.lo) < 0) {
            out.competitor_right = std::move(right);
            out.core_left = std::move(left);
            out.first_separating_horizon = k;
            out.separated = true;
            break;
        }
    }
    if (!out.separated) return out;

    out.entrywise_intertwiner = true;
    for (std::size_t i = 0; i < out.competitor_right.weight.size(); ++i) {
        for (std::size_t j = 0; j < out.core_left.weight.size(); ++j) {
            mathlib::BigInt lhs, rhs;
            mathlib::mul(lhs, out.competitor_right.action[i], out.core_left.weight[j]);
            mathlib::mul(rhs, out.competitor_right.weight[i], out.core_left.action[j]);
            if (mathlib::cmp(lhs, rhs) >= 0) out.entrywise_intertwiner = false;
        }
    }
    out.replayed = out.separated && out.entrywise_intertwiner &&
                   out.competitor_right.positive && out.core_left.positive &&
                   out.competitor_right.recurrence_replayed &&
                   out.core_left.recurrence_replayed;
    return out;
}

} // namespace ravel::proof
