#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/reflective_collatz_weight.hpp"

namespace ravel::proof {

// A recursive equitable grammar derived from a nonnegative adjacency matrix.
// The matrix refines its own vertices by their weighted incoming/outgoing
// behaviour.  Stable colors are mathematical roles: vertices in one role
// have identical transition totals to every other role.
struct ReflectiveWeightGrammar {
    std::vector<std::size_t> role_of;
    std::vector<std::vector<std::size_t>> members;
    std::vector<std::vector<long long>> role_matrix;
    ReflectiveCollatzWeight role_weight;
    bool equitable = false;
    bool lifted_replay = false;
    std::size_t refinement_rounds = 0;

    [[nodiscard]] std::size_t roles() const { return members.size(); }
};

namespace detail {

inline std::vector<std::size_t> canonicalize_signatures(
    const std::vector<std::vector<long long>>& signatures) {
    std::map<std::vector<long long>, std::size_t> ids;
    std::vector<std::size_t> colors(signatures.size());
    for (std::size_t i = 0; i < signatures.size(); ++i) {
        auto [it, inserted] = ids.emplace(signatures[i], ids.size());
        (void)inserted;
        colors[i] = it->second;
    }
    return colors;
}

inline std::size_t color_count(const std::vector<std::size_t>& colors) {
    return colors.empty() ? 0 : 1 + *std::max_element(colors.begin(), colors.end());
}

}  // namespace detail

inline ReflectiveWeightGrammar derive_reflective_weight_grammar(
    const std::vector<std::vector<long long>>& matrix,
    std::size_t weight_iterations = 80) {
    if (matrix.empty())
        throw std::invalid_argument("reflective weight grammar: empty matrix");
    const auto n = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != n)
            throw std::invalid_argument("reflective weight grammar: nonsquare matrix");
        for (const auto x : row)
            if (x < 0)
                throw std::invalid_argument("reflective weight grammar: negative entry");
    }

    // Seed only with local invariants that are themselves derived from the
    // adjacency action.  No component indices or learned weights enter here.
    std::vector<std::vector<long long>> seed(n);
    for (std::size_t i = 0; i < n; ++i) {
        long long out_sum = 0, in_sum = 0, out_support = 0, in_support = 0;
        for (std::size_t j = 0; j < n; ++j) {
            out_sum += matrix[i][j];
            in_sum += matrix[j][i];
            out_support += matrix[i][j] != 0;
            in_support += matrix[j][i] != 0;
        }
        seed[i] = {matrix[i][i], out_sum, in_sum, out_support, in_support};
    }
    auto colors = detail::canonicalize_signatures(seed);

    ReflectiveWeightGrammar out;
    for (;;) {
        const auto classes = detail::color_count(colors);
        std::vector<std::vector<long long>> signatures(n);
        for (std::size_t i = 0; i < n; ++i) {
            auto& sig = signatures[i];
            sig.reserve(1 + 2 * classes);
            sig.push_back(static_cast<long long>(colors[i]));
            for (std::size_t c = 0; c < classes; ++c) {
                long long outgoing = 0, incoming = 0;
                for (std::size_t j = 0; j < n; ++j) {
                    if (colors[j] == c) {
                        outgoing += matrix[i][j];
                        incoming += matrix[j][i];
                    }
                }
                sig.push_back(outgoing);
                sig.push_back(incoming);
            }
        }
        auto next = detail::canonicalize_signatures(signatures);
        ++out.refinement_rounds;
        if (next == colors) break;
        colors = std::move(next);
    }

    out.role_of = colors;
    const auto classes = detail::color_count(colors);
    out.members.assign(classes, {});
    for (std::size_t i = 0; i < n; ++i) out.members[colors[i]].push_back(i);
    out.role_matrix.assign(classes, std::vector<long long>(classes, 0));

    out.equitable = true;
    for (std::size_t c = 0; c < classes; ++c) {
        const auto representative = out.members[c].front();
        for (std::size_t d = 0; d < classes; ++d) {
            long long expected = 0;
            for (const auto j : out.members[d]) expected += matrix[representative][j];
            out.role_matrix[c][d] = expected;
            for (const auto i : out.members[c]) {
                long long actual = 0;
                for (const auto j : out.members[d]) actual += matrix[i][j];
                if (actual != expected) out.equitable = false;
            }
        }
    }

    out.role_weight = derive_reflective_collatz_weight(
        out.role_matrix, weight_iterations);

    // Lift the grammar weight and replay its exact Collatz bracket on the
    // original matrix.  Equitability should make the two actions commute.
    std::vector<mathlib::BigInt> lifted;
    lifted.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        lifted.push_back(out.role_weight.weight[out.role_of[i]]);
    const auto replay = mathlib::collatz_step_exact(matrix, lifted);
    out.lifted_replay = out.equitable &&
        mathlib::cmp(replay.bracket.lo, out.role_weight.bracket.lo) == 0 &&
        mathlib::cmp(replay.bracket.hi, out.role_weight.bracket.hi) == 0;
    return out;
}

}  // namespace ravel::proof
