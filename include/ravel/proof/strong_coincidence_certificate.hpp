#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <set>
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
    witness_rejected,
};

template <std::size_t d>
inline std::optional<std::pair<long long, long long>> find_closure_pair_occurrence(
    const std::array<std::vector<long long>, d>& images,
    long long first_letter, long long second_letter, long long depth,
    long long terminal_letter, const std::vector<long long>& target_vector,
    bool from_suffix, long long max_word_len = 5'000'000) {
    if (depth < 1 || terminal_letter < 0 || static_cast<std::size_t>(terminal_letter) >= d ||
        target_vector.size() != d)
        return std::nullopt;
    std::vector<long long> first = images[static_cast<std::size_t>(first_letter)];
    std::vector<long long> second = images[static_cast<std::size_t>(second_letter)];
    for (long long k = 1; k < depth; ++k) {
        if (first.size() > static_cast<std::size_t>(max_word_len) ||
            second.size() > static_cast<std::size_t>(max_word_len)) return std::nullopt;
        first = adelic::apply_substitution<d>(images, first);
        second = adelic::apply_substitution<d>(images, second);
    }
    if (first.size() > static_cast<std::size_t>(max_word_len) ||
        second.size() > static_cast<std::size_t>(max_word_len)) return std::nullopt;

    auto collect = [&](const std::vector<long long>& word) {
        std::set<std::vector<long long>> states;
        std::array<long long, d> total{};
        for (long long letter : word) ++total[static_cast<std::size_t>(letter)];
        std::array<long long, d> prefix{};
        for (long long letter : word) {
            if (letter == terminal_letter) {
                std::vector<long long> state(d);
                if (from_suffix) {
                    for (std::size_t i = 0; i < d; ++i)
                        state[i] = total[i] - prefix[i] -
                            (static_cast<long long>(i) == letter ? 1 : 0);
                } else {
                    state.assign(prefix.begin(), prefix.end());
                }
                states.insert(std::move(state));
            }
            ++prefix[static_cast<std::size_t>(letter)];
        }
        return states;
    };
    const auto first_states = collect(first);
    const auto second_states = collect(second);
    if (!first_states.count(target_vector) || !second_states.count(target_vector))
        return std::nullopt;
    auto find_position = [&](const std::vector<long long>& word) -> long long {
        std::array<long long, d> total{};
        for (const long long letter : word) ++total[static_cast<std::size_t>(letter)];
        std::array<long long, d> prefix{};
        for (std::size_t position = 0; position < word.size(); ++position) {
            const long long letter = word[position];
            if (letter == terminal_letter) {
                std::vector<long long> state(d);
                if (from_suffix) {
                    for (std::size_t i = 0; i < d; ++i)
                        state[i] = total[i] - prefix[i] -
                            (static_cast<long long>(i) == letter ? 1 : 0);
                } else {
                    state.assign(prefix.begin(), prefix.end());
                }
                if (state == target_vector) return static_cast<long long>(position);
            }
            ++prefix[static_cast<std::size_t>(letter)];
        }
        return -1;
    };
    const long long first_position = find_position(first);
    const long long second_position = find_position(second);
    if (first_position < 0 || second_position < 0) return std::nullopt;
    return std::make_pair(first_position, second_position);
}

template <std::size_t d>
inline bool validate_closure_pair_witness_against_words(
    const std::array<std::vector<long long>, d>& images,
    long long first_letter, long long second_letter, long long depth,
    long long terminal_letter, const std::vector<long long>& target_vector,
    bool from_suffix, long long max_word_len = 5'000'000) {
    return find_closure_pair_occurrence<d>(images, first_letter, second_letter, depth,
                                           terminal_letter, target_vector, from_suffix,
                                           max_word_len).has_value();
}

template <std::size_t d, typename Node>
inline void serialize_closure_edges(
    const std::array<std::vector<long long>, d>& images, Node& node) {
    node.edges.clear();
    for (const auto& edge : build_junction_graph<d>(images)) {
        typename Node::Edge serialized;
        serialized.from_junction = edge.from_junction;
        serialized.to_junction = edge.to_junction;
        serialized.jump_size = edge.jump_size;
        serialized.child_index = edge.child_index;
        serialized.landmark.assign(edge.landmark.begin(), edge.landmark.end());
        serialized.chain = edge.chain;
        node.edges.push_back(std::move(serialized));
    }
}

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
    if (result.pair_first_paths.size() != result.pair_resolution_depths.size() ||
        result.pair_second_paths.size() != result.pair_resolution_depths.size())
        return StrongCoincidencePrefixClosureStageResult::witness_rejected;
    const auto pair_index = [](std::size_t i, std::size_t j) {
        return (i * (2 * d - i - 1)) / 2 + (j - i - 1);
    };
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = i + 1; j < d; ++j) {
            const std::size_t index = pair_index(i, j);
            const auto occurrence = index < result.pair_resolution_depths.size()
                ? find_closure_pair_occurrence<d>(
                      images, static_cast<long long>(i), static_cast<long long>(j),
                      result.pair_resolution_depths[index],
                      result.pair_terminal_letters[index], result.pair_vectors[index], false)
                : std::nullopt;
            if (!occurrence)
                return StrongCoincidencePrefixClosureStageResult::witness_rejected;
        }
    if (!mathlib::reflection::enabled())
        return StrongCoincidencePrefixClosureStageResult::reflection_disabled;

    mathlib::reflection::StrongCoincidencePrefixClosureCertificate node;
    node.images.assign(images.begin(), images.end());
    serialize_closure_edges<d>(images, node);
    node.pair_resolution_depths = result.pair_resolution_depths;
    node.pair_terminal_letters = result.pair_terminal_letters;
    node.pair_vectors = result.pair_vectors;
    node.pair_first_paths = result.pair_first_paths;
    node.pair_second_paths = result.pair_second_paths;
    node.pair_first_positions.resize(result.pair_resolution_depths.size());
    node.pair_second_positions.resize(result.pair_resolution_depths.size());
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = i + 1; j < d; ++j) {
            const std::size_t index = pair_index(i, j);
            const auto occurrence = find_closure_pair_occurrence<d>(
                images, static_cast<long long>(i), static_cast<long long>(j),
                result.pair_resolution_depths[index], result.pair_terminal_letters[index],
                result.pair_vectors[index], false);
            node.pair_first_positions[index] = occurrence->first;
            node.pair_second_positions[index] = occurrence->second;
        }
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
    witness_rejected,
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
    if (result.pair_first_paths.size() != result.pair_resolution_depths.size() ||
        result.pair_second_paths.size() != result.pair_resolution_depths.size())
        return StrongCoincidenceClosureStageResult::witness_rejected;
    const auto pair_index = [](std::size_t i, std::size_t j) {
        return (i * (2 * d - i - 1)) / 2 + (j - i - 1);
    };
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = i + 1; j < d; ++j) {
            const std::size_t index = pair_index(i, j);
            const auto occurrence = index < result.pair_resolution_depths.size()
                ? find_closure_pair_occurrence<d>(
                      images, static_cast<long long>(i), static_cast<long long>(j),
                      result.pair_resolution_depths[index],
                      result.pair_terminal_letters[index], result.pair_vectors[index],
                      result.pair_from_suffix[index])
                : std::nullopt;
            if (!occurrence)
                return StrongCoincidenceClosureStageResult::witness_rejected;
        }
    if (!mathlib::reflection::enabled())
        return StrongCoincidenceClosureStageResult::reflection_disabled;

    mathlib::reflection::StrongCoincidenceClosureCertificate node;
    node.images.assign(images.begin(), images.end());
    serialize_closure_edges<d>(images, node);
    node.pair_resolution_depths = result.pair_resolution_depths;
    node.pair_terminal_letters = result.pair_terminal_letters;
    node.pair_vectors = result.pair_vectors;
    node.pair_from_suffix = result.pair_from_suffix;
    node.pair_first_paths = result.pair_first_paths;
    node.pair_second_paths = result.pair_second_paths;
    node.pair_first_positions.resize(result.pair_resolution_depths.size());
    node.pair_second_positions.resize(result.pair_resolution_depths.size());
    for (std::size_t i = 0; i < d; ++i)
        for (std::size_t j = i + 1; j < d; ++j) {
            const std::size_t index = pair_index(i, j);
            const auto occurrence = find_closure_pair_occurrence<d>(
                images, static_cast<long long>(i), static_cast<long long>(j),
                result.pair_resolution_depths[index], result.pair_terminal_letters[index],
                result.pair_vectors[index], result.pair_from_suffix[index]);
            node.pair_first_positions[index] = occurrence->first;
            node.pair_second_positions[index] = occurrence->second;
        }
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
