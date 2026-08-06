#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/reflective_collatz_weight.hpp"

namespace ravel::proof {

// A quotient matrix reflects over its own positive cone and records a
// canonical proof profile.  The profile is independent of vertex numbering:
// it retains the exact Collatz bracket and the multiset of normalized weight
// values, not the raw coordinate order.
struct ReflectiveWeightProfile {
    ReflectiveCollatzWeight proof;
    std::vector<std::pair<std::string, std::size_t>> normalized_histogram;
    bool permutation_component = false;
    bool replayed = false;
};

inline bool is_weighted_permutation_matrix(
    const std::vector<std::vector<long long>>& matrix) {
    if (matrix.empty()) return false;
    const auto n = matrix.size();
    std::vector<long long> incoming(n, 0);
    for (const auto& row : matrix) {
        if (row.size() != n) return false;
        long long outgoing = 0;
        for (std::size_t j = 0; j < n; ++j) {
            if (row[j] < 0) return false;
            outgoing += row[j];
            incoming[j] += row[j];
        }
        if (outgoing != 1) return false;
    }
    return std::all_of(incoming.begin(), incoming.end(),
                       [](long long x) { return x == 1; });
}

inline ReflectiveWeightProfile derive_reflective_weight_profile(
    const std::vector<std::vector<long long>>& matrix,
    std::size_t iterations = 80) {
    ReflectiveWeightProfile out;
    out.proof = derive_reflective_collatz_weight(matrix, iterations);
    out.permutation_component = is_weighted_permutation_matrix(matrix);

    // Normalize by the minimum positive coordinate.  This is canonical under
    // arbitrary vertex relabeling and sufficient for exact family matching.
    auto minimum = out.proof.weight.front();
    for (const auto& x : out.proof.weight)
        if (mathlib::cmp(x, minimum) < 0) minimum = x;

    std::map<std::string, std::size_t> histogram;
    for (const auto& x : out.proof.weight) {
        mathlib::Rat ratio;
        mathlib::set_num_den(ratio, x, minimum);
        ++histogram[mathlib::str(ratio)];
    }
    out.normalized_histogram.assign(histogram.begin(), histogram.end());
    out.replayed = out.proof.replayed && out.proof.positive;
    return out;
}

inline bool same_reflective_weight_family(
    const ReflectiveWeightProfile& a,
    const ReflectiveWeightProfile& b) {
    return a.permutation_component == b.permutation_component &&
           mathlib::cmp(a.proof.bracket.lo, b.proof.bracket.lo) == 0 &&
           mathlib::cmp(a.proof.bracket.hi, b.proof.bracket.hi) == 0 &&
           a.normalized_histogram == b.normalized_histogram;
}

struct ReflectiveWeightFamilyCorpus {
    std::vector<ReflectiveWeightProfile> profiles;
    std::vector<std::size_t> family_of;
    std::vector<std::vector<std::size_t>> members;
    bool replayed = false;
};

inline ReflectiveWeightFamilyCorpus derive_reflective_weight_family_corpus(
    const std::vector<std::vector<std::vector<long long>>>& matrices,
    std::size_t iterations = 80) {
    ReflectiveWeightFamilyCorpus out;
    out.replayed = true;
    for (const auto& matrix : matrices) {
        out.profiles.push_back(derive_reflective_weight_profile(matrix, iterations));
        out.replayed = out.replayed && out.profiles.back().replayed;
    }
    for (std::size_t i = 0; i < out.profiles.size(); ++i) {
        std::size_t family = out.members.size();
        for (std::size_t f = 0; f < out.members.size(); ++f) {
            if (same_reflective_weight_family(
                    out.profiles[i], out.profiles[out.members[f].front()])) {
                family = f;
                break;
            }
        }
        if (family == out.members.size()) out.members.push_back({});
        out.members[family].push_back(i);
        out.family_of.push_back(family);
    }
    return out;
}

}  // namespace ravel::proof
