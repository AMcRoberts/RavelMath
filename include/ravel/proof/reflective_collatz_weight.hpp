#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/ball.hpp"

namespace ravel::proof {

// A positive integer Collatz--Wielandt weight derived by iterating the
// executable adjacency operator on the all-ones vector.  This is not a flat
// externally supplied certificate: the matrix acts on its own positive cone,
// producing both the weight and the exact coordinatewise rational bounds.
struct ReflectiveCollatzWeight {
    std::vector<mathlib::BigInt> weight;
    mathlib::Ball bracket;
    std::size_t iterations = 0;
    bool positive = false;
    bool replayed = false;
};

inline ReflectiveCollatzWeight derive_reflective_collatz_weight(
    const std::vector<std::vector<long long>>& matrix,
    std::size_t iterations = 80) {
    if (matrix.empty())
        throw std::invalid_argument("reflective Collatz weight: empty matrix");
    const auto n = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != n)
            throw std::invalid_argument("reflective Collatz weight: nonsquare matrix");
        for (const auto entry : row)
            if (entry < 0)
                throw std::invalid_argument("reflective Collatz weight: negative entry");
    }

    std::vector<mathlib::BigInt> current;
    current.reserve(n);
    for (std::size_t i = 0; i < n; ++i) current.emplace_back(1);

    ReflectiveCollatzWeight out;
    const auto rounds = std::max<std::size_t>(1, iterations);
    for (std::size_t round = 0; round < rounds; ++round) {
        const auto step = mathlib::collatz_step_exact(matrix, current);
        out.weight = current;
        out.bracket = step.bracket;
        out.iterations = round + 1;
        current = step.next_v;
    }

    out.positive = true;
    for (const auto& value : out.weight)
        out.positive = out.positive && mathlib::cmp_si(value, 0) > 0;

    // Replay the exact coordinate ratios from the derived weight.  Equality
    // with the stored bracket proves that no floating approximation entered
    // the operation.
    const auto replay = mathlib::collatz_step_exact(matrix, out.weight);
    out.replayed =
        mathlib::cmp(replay.bracket.lo, out.bracket.lo) == 0 &&
        mathlib::cmp(replay.bracket.hi, out.bracket.hi) == 0;
    return out;
}

struct ReflectivePerronDominance {
    ReflectiveCollatzWeight competitor;
    ReflectiveCollatzWeight core;
    bool strict = false;
    bool replayed = false;
};

// Derive two positive weights and compare their exact Collatz--Wielandt
// bounds.  If competitor.hi < core.lo, the competitor Perron root is strictly
// below the core root.  The proof object contains the vectors from which both
// inequalities were generated and replayed.
inline ReflectivePerronDominance derive_reflective_perron_dominance(
    const std::vector<std::vector<long long>>& competitor,
    const std::vector<std::vector<long long>>& core,
    std::size_t iterations = 80) {
    ReflectivePerronDominance out;
    out.competitor = derive_reflective_collatz_weight(competitor, iterations);
    out.core = derive_reflective_collatz_weight(core, iterations);
    out.strict = mathlib::cmp(
        out.competitor.bracket.hi, out.core.bracket.lo) < 0;
    out.replayed = out.competitor.replayed && out.core.replayed;
    return out;
}

}  // namespace ravel::proof
