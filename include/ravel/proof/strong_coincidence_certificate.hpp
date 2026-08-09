#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/proof/strong_coincidence_pair_witness.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

enum class StrongCoincidenceStageResult {
    staged,
    reflection_disabled,
    inconclusive,
    failed,
};

enum class StrongCoincidencePrefixClosureStageResult {
    staged,
    reflection_disabled,
    inconclusive,
    unsupported,
};

// Stage a closed prefix-half landmark closure.  This is an independent
// finite route: it never emits a full strong-coincidence claim, and refuses
// to serialize a cutoff or an unsupported deterministic-chain shape.
template <std::size_t d>
inline StrongCoincidencePrefixClosureStageResult stage_strong_coincidence_prefix_closure(
    const std::array<std::vector<long long>, d>& images,
    long long max_depth = 20,
    std::size_t outcome_budget = 1'000'000,
    std::string description = {}) {
    std::array<std::array<long long, d>, d> matrix{};
    for (std::size_t column = 0; column < d; ++column)
        for (long long letter : images[column]) {
            if (letter < 0 || static_cast<std::size_t>(letter) >= d)
                return StrongCoincidencePrefixClosureStageResult::unsupported;
            ++matrix[static_cast<std::size_t>(letter)][column];
        }
    PrefixClosureCoincidenceResult result;
    try {
        result = check_prefix_coincidence_closure<d>(images, matrix, max_depth,
                                                     outcome_budget);
    } catch (const std::invalid_argument&) {
        return StrongCoincidencePrefixClosureStageResult::unsupported;
    } catch (const std::overflow_error&) {
        return StrongCoincidencePrefixClosureStageResult::inconclusive;
    }
    if (!result.holds || result.inconclusive)
        return StrongCoincidencePrefixClosureStageResult::inconclusive;
    if (!mathlib::reflection::enabled())
        return StrongCoincidencePrefixClosureStageResult::reflection_disabled;

    mathlib::reflection::StrongCoincidencePrefixClosureCertificate node;
    node.images.assign(images.begin(), images.end());
    node.pair_resolution_depths = result.pair_resolution_depths;
    node.pair_terminal_letters = result.pair_terminal_letters;
    node.pair_vectors = result.pair_vectors;
    node.depth_reached = result.depth_reached;
    node.max_depth = max_depth;
    node.outcome_budget = static_cast<long long>(outcome_budget);
    node.unresolved_pairs = result.unresolved_pairs;
    node.holds = result.holds;
    node.inconclusive = result.inconclusive;
    node.description = std::move(description);
    node.matrix.reserve(d * d);
    for (const auto& row : matrix)
        for (long long value : row) node.matrix.push_back(value);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication,
                                std::move(node));
    return StrongCoincidencePrefixClosureStageResult::staged;
}

enum class StrongCoincidenceClosureStageResult {
    staged,
    reflection_disabled,
    inconclusive,
    unsupported,
};

template <std::size_t d>
inline StrongCoincidenceClosureStageResult stage_strong_coincidence_closure(
    const std::array<std::vector<long long>, d>& images,
    long long max_depth = 20,
    std::size_t outcome_budget = 1'000'000,
    std::string description = {}) {
    std::array<std::array<long long, d>, d> matrix{};
    for (std::size_t column = 0; column < d; ++column)
        for (long long letter : images[column]) {
            if (letter < 0 || static_cast<std::size_t>(letter) >= d)
                return StrongCoincidenceClosureStageResult::unsupported;
            ++matrix[static_cast<std::size_t>(letter)][column];
        }
    ClosureStrongCoincidenceResult result;
    try {
        result = check_strong_coincidence_closure<d>(images, matrix, max_depth,
                                                     outcome_budget);
    } catch (const std::invalid_argument&) {
        return StrongCoincidenceClosureStageResult::unsupported;
    } catch (const std::overflow_error&) {
        return StrongCoincidenceClosureStageResult::inconclusive;
    }
    if (!result.holds || result.inconclusive)
        return StrongCoincidenceClosureStageResult::inconclusive;
    if (!mathlib::reflection::enabled())
        return StrongCoincidenceClosureStageResult::reflection_disabled;

    mathlib::reflection::StrongCoincidenceClosureCertificate node;
    node.images.assign(images.begin(), images.end());
    node.pair_resolution_depths = result.pair_resolution_depths;
    node.pair_terminal_letters = result.pair_terminal_letters;
    node.pair_vectors = result.pair_vectors;
    node.pair_from_suffix = result.pair_from_suffix;
    node.depth_reached = result.depth_reached;
    node.max_depth = max_depth;
    node.outcome_budget = static_cast<long long>(outcome_budget);
    node.unresolved_pairs = result.unresolved_pairs;
    node.holds = result.holds;
    node.inconclusive = result.inconclusive;
    node.description = std::move(description);
    node.matrix.reserve(d * d);
    for (const auto& row : matrix)
        for (long long value : row) node.matrix.push_back(value);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication,
                                std::move(node));
    return StrongCoincidenceClosureStageResult::staged;
}

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
