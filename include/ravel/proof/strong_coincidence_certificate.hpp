#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
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
