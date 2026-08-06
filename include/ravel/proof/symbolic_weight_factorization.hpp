#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <vector>

#include "math/ball.hpp"

namespace ravel::proof {

struct SymbolicWeightFactorization {
    std::vector<std::size_t> role_of;
    std::vector<std::vector<std::size_t>> members;
    bool weight_constant_on_roles = false;
    std::size_t collisions = 0;

    [[nodiscard]] std::size_t roles() const { return members.size(); }
};

// Factor a derived positive weight through mathematical labels supplied by
// the graph implementation.  Equal labels are a conjectured symbolic role;
// exact BigInt equality decides whether the weight genuinely factors through
// that role grammar.  This operation never inserts the weight into the label.
inline SymbolicWeightFactorization derive_symbolic_weight_factorization(
    const std::vector<std::vector<long long>>& labels,
    const std::vector<mathlib::BigInt>& weight) {
    if (labels.size() != weight.size())
        throw std::invalid_argument("symbolic weight factorization: size mismatch");

    std::map<std::vector<long long>, std::size_t> ids;
    SymbolicWeightFactorization out;
    out.role_of.resize(labels.size());
    for (std::size_t i = 0; i < labels.size(); ++i) {
        auto [it, inserted] = ids.emplace(labels[i], ids.size());
        (void)inserted;
        out.role_of[i] = it->second;
    }
    out.members.assign(ids.size(), {});
    for (std::size_t i = 0; i < labels.size(); ++i)
        out.members[out.role_of[i]].push_back(i);

    out.weight_constant_on_roles = true;
    for (const auto& role : out.members) {
        const auto first = role.front();
        for (const auto i : role) {
            if (mathlib::cmp(weight[first], weight[i]) != 0) {
                out.weight_constant_on_roles = false;
                ++out.collisions;
            }
        }
    }
    return out;
}

}  // namespace ravel::proof

namespace ravel::proof {

struct RefinedSymbolicWeightGrammar : SymbolicWeightFactorization {
    std::size_t refinement_rounds = 0;
    bool equitable = false;
};

inline RefinedSymbolicWeightGrammar derive_refined_symbolic_weight_grammar(
    const std::vector<std::vector<long long>>& matrix,
    const std::vector<std::vector<long long>>& labels,
    const std::vector<mathlib::BigInt>& weight) {
    if (matrix.size() != labels.size() || labels.size() != weight.size())
        throw std::invalid_argument("refined symbolic weight grammar: size mismatch");
    const auto n = matrix.size();
    auto base = derive_symbolic_weight_factorization(labels, weight);
    std::vector<std::size_t> colors = base.role_of;
    RefinedSymbolicWeightGrammar out;

    for (;;) {
        const auto classes = colors.empty() ? 0 : 1 + *std::max_element(colors.begin(), colors.end());
        std::vector<std::vector<long long>> sig(n);
        for (std::size_t i = 0; i < n; ++i) {
            sig[i] = labels[i];
            sig[i].push_back(-9000001);
            sig[i].push_back(static_cast<long long>(colors[i]));
            for (std::size_t c = 0; c < classes; ++c) {
                long long out_sum = 0, in_sum = 0;
                for (std::size_t j = 0; j < n; ++j) if (colors[j] == c) {
                    out_sum += matrix[i][j];
                    in_sum += matrix[j][i];
                }
                sig[i].push_back(out_sum);
                sig[i].push_back(in_sum);
            }
        }
        std::map<std::vector<long long>, std::size_t> ids;
        std::vector<std::size_t> next(n);
        for (std::size_t i = 0; i < n; ++i) {
            auto [it, inserted] = ids.emplace(sig[i], ids.size());
            (void)inserted;
            next[i] = it->second;
        }
        ++out.refinement_rounds;
        colors.swap(next);

        out.role_of = colors;
        out.members.assign(ids.size(), {});
        for (std::size_t i = 0; i < n; ++i) out.members[colors[i]].push_back(i);
        out.weight_constant_on_roles = true;
        out.collisions = 0;
        for (const auto& role : out.members) {
            const auto first = role.front();
            for (const auto i : role) if (mathlib::cmp(weight[first], weight[i]) != 0) {
                out.weight_constant_on_roles = false;
                ++out.collisions;
            }
        }
        if (out.weight_constant_on_roles || colors == next || out.members.size() == n) break;
    }

    out.equitable = true;
    for (const auto& role : out.members) {
        const auto rep = role.front();
        for (std::size_t c = 0; c < out.members.size(); ++c) {
            long long expected = 0;
            for (const auto j : out.members[c]) expected += matrix[rep][j];
            for (const auto i : role) {
                long long actual = 0;
                for (const auto j : out.members[c]) actual += matrix[i][j];
                if (actual != expected) out.equitable = false;
            }
        }
    }
    return out;
}

} // namespace ravel::proof

namespace ravel::proof {

struct SymbolicRoleUpperBound {
    SymbolicWeightFactorization roles;
    std::vector<std::vector<long long>> upper_matrix;
    ReflectiveCollatzWeight upper_weight;
    bool transition_majorization_replayed = false;
};

// Build the least role-level matrix (for the supplied roles) that
// coordinatewise majorizes every concrete transition row.  For role c and
// target role d, B[c,d] is the maximum weighted number of d-successors of any
// concrete state in c.  Thus A * lift(w) <= lift(B*w) for every positive role
// weight w.  This is a symbolic subeigenvector proof, not an exact quotient.
inline SymbolicRoleUpperBound derive_symbolic_role_upper_bound(
    const std::vector<std::vector<long long>>& matrix,
    const std::vector<std::vector<long long>>& labels,
    std::size_t iterations = 80) {
    if (matrix.size() != labels.size())
        throw std::invalid_argument("symbolic role upper bound: size mismatch");
    const auto n = matrix.size();
    std::vector<mathlib::BigInt> dummy;
    dummy.reserve(n);
    for (std::size_t i = 0; i < n; ++i) dummy.emplace_back(1);

    SymbolicRoleUpperBound out;
    out.roles = derive_symbolic_weight_factorization(labels, dummy);
    const auto r = out.roles.roles();
    out.upper_matrix.assign(r, std::vector<long long>(r, 0));
    for (std::size_t c = 0; c < r; ++c) {
        for (const auto i : out.roles.members[c]) {
            std::vector<long long> row(r, 0);
            for (std::size_t j = 0; j < n; ++j)
                row[out.roles.role_of[j]] += matrix[i][j];
            for (std::size_t d = 0; d < r; ++d)
                out.upper_matrix[c][d] = std::max(out.upper_matrix[c][d], row[d]);
        }
    }
    out.upper_weight = derive_reflective_collatz_weight(out.upper_matrix, iterations);

    // Exact replay of A*lift(w) <= lift(B*w).
    out.transition_majorization_replayed = true;
    for (std::size_t i = 0; i < n; ++i) {
        mathlib::BigInt lhs(0), rhs(0);
        for (std::size_t j = 0; j < n; ++j) {
            if (matrix[i][j] != 0) {
                mathlib::BigInt term;
                mathlib::mul_si(term, out.upper_weight.weight[out.roles.role_of[j]], matrix[i][j]);
                mathlib::BigInt next;
                mathlib::add(next, lhs, term);
                lhs = std::move(next);
            }
        }
        const auto c = out.roles.role_of[i];
        for (std::size_t d = 0; d < r; ++d) {
            if (out.upper_matrix[c][d] != 0) {
                mathlib::BigInt term;
                mathlib::mul_si(term, out.upper_weight.weight[d], out.upper_matrix[c][d]);
                mathlib::BigInt next;
                mathlib::add(next, rhs, term);
                rhs = std::move(next);
            }
        }
        if (mathlib::cmp(lhs, rhs) > 0) out.transition_majorization_replayed = false;
    }
    return out;
}

struct SymbolicRoleDominance {
    SymbolicRoleUpperBound competitor;
    ReflectiveCollatzWeight core;
    bool strict = false;
    bool replayed = false;
};

inline SymbolicRoleDominance derive_symbolic_role_dominance(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::vector<long long>>& competitor_labels,
    const std::vector<std::vector<long long>>& core,
    std::size_t iterations = 80) {
    SymbolicRoleDominance out;
    out.competitor = derive_symbolic_role_upper_bound(
        competitor, competitor_labels, iterations);
    out.core = derive_reflective_collatz_weight(core, iterations);
    out.strict = mathlib::cmp(
        out.competitor.upper_weight.bracket.hi, out.core.bracket.lo) < 0;
    out.replayed = out.competitor.transition_majorization_replayed &&
                   out.competitor.upper_weight.replayed && out.core.replayed;
    return out;
}

} // namespace ravel::proof
