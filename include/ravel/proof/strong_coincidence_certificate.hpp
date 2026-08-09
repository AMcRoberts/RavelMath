#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/proof/strong_coincidence_pair_witness.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

enum class StrongCoincidenceStageResult {
    staged,
    reflection_disabled,
    inconclusive,
    failed,
};

// Runs the actual checker and records its bounded result.  No theorem claim
// is emitted here: the payload preserves the concrete substitution and the
// limits needed to reproduce exactly what was checked.
template <std::size_t d>
inline StrongCoincidenceStageResult stage_strong_coincidence_run(
    const std::array<std::vector<long long>, d>& images,
    long long max_depth = 20,
    long long max_word_len = 5'000'000,
    std::string description = {}) {
    const auto result = adelic::check_strong_coincidence<d>(images, max_depth, max_word_len);
    if (!mathlib::reflection::enabled()) return StrongCoincidenceStageResult::reflection_disabled;
    if (result.inconclusive) return StrongCoincidenceStageResult::inconclusive;
    if (!result.holds) return StrongCoincidenceStageResult::failed;

    mathlib::reflection::StrongCoincidenceRunCertificate node;
    node.images.assign(images.begin(), images.end());
    node.pair_resolution_depths = result.pair_resolution_depths;
    for (std::size_t i = 0; i < d; ++i) {
        for (std::size_t j = i + 1; j < d; ++j) {
            const auto witness = find_strong_coincidence_pair_witness(
                images, static_cast<long long>(i), static_cast<long long>(j),
                max_depth, max_word_len);
            if (!witness) return StrongCoincidenceStageResult::failed;
            node.pair_depths.push_back(witness->depth);
            const std::size_t pair_index =
                (i * (2 * d - i - 1)) / 2 + (j - i - 1);
            if (pair_index >= result.pair_resolution_depths.size() ||
                result.pair_resolution_depths[pair_index] != witness->depth)
                return StrongCoincidenceStageResult::failed;
            if (!stage_strong_coincidence_pair_witness(
                    images, static_cast<long long>(i), static_cast<long long>(j),
                    max_depth, max_word_len,
                    "strong coincidence pair (" + std::to_string(i) + "," +
                    std::to_string(j) + ")"))
                return StrongCoincidenceStageResult::failed;
        }
    }
    node.depth_reached = result.depth_reached;
    node.unresolved_pairs = result.unresolved_pairs;
    node.max_depth = max_depth;
    node.max_word_len = max_word_len;
    node.holds = result.holds;
    node.inconclusive = result.inconclusive;
    node.description = std::move(description);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, std::move(node));
    return StrongCoincidenceStageResult::staged;
}

} // namespace ravel::proof
