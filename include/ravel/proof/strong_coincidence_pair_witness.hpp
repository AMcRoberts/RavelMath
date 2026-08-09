#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "math/proof_reflection.hpp"

namespace ravel::proof {

template <std::size_t d>
struct StrongCoincidencePairWitness {
    long long first_letter = 0;
    long long second_letter = 0;
    long long depth = 0;
    long long common_letter = 0;
    long long first_position = 0;
    long long second_position = 0;
    std::array<long long, d> first_prefix{};
    std::array<long long, d> second_prefix{};
    std::array<long long, d> first_suffix{};
    std::array<long long, d> second_suffix{};
    bool prefix_match = false;
    bool suffix_match = false;
};

template <std::size_t d>
inline std::optional<StrongCoincidencePairWitness<d>> find_strong_coincidence_pair_witness(
    const std::array<std::vector<long long>, d>& images,
    long long first_letter, long long second_letter,
    long long max_depth = 20, long long max_word_len = 5'000'000) {
    std::vector<long long> first = images[static_cast<std::size_t>(first_letter)];
    std::vector<long long> second = images[static_cast<std::size_t>(second_letter)];
    for (long long depth = 1; depth <= max_depth; ++depth) {
        if (first.size() > static_cast<std::size_t>(max_word_len) ||
            second.size() > static_cast<std::size_t>(max_word_len)) return std::nullopt;
        std::array<long long, d> total_first{}, total_second{};
        for (const auto letter : first) ++total_first[static_cast<std::size_t>(letter)];
        for (const auto letter : second) ++total_second[static_cast<std::size_t>(letter)];
        std::array<long long, d> prefix_first{};
        for (std::size_t i = 0; i < first.size(); ++i) {
            const long long a = first[i];
            std::array<long long, d> prefix_second{};
            for (std::size_t j = 0; j < second.size(); ++j) {
                const long long b = second[j];
                if (a == b) {
                    std::array<long long, d> suffix_first{}, suffix_second{};
                    for (std::size_t k = 0; k < d; ++k) {
                        suffix_first[k] = total_first[k] - prefix_first[k] -
                            (static_cast<long long>(k) == a ? 1 : 0);
                        suffix_second[k] = total_second[k] - prefix_second[k] -
                            (static_cast<long long>(k) == b ? 1 : 0);
                    }
                    const bool prefix_match = prefix_first == prefix_second;
                    const bool suffix_match = suffix_first == suffix_second;
                    if (prefix_match || suffix_match) {
                        StrongCoincidencePairWitness<d> out;
                        out.first_letter = first_letter;
                        out.second_letter = second_letter;
                        out.depth = depth;
                        out.common_letter = a;
                        out.first_position = static_cast<long long>(i);
                        out.second_position = static_cast<long long>(j);
                        out.first_prefix = prefix_first;
                        out.second_prefix = prefix_second;
                        out.first_suffix = suffix_first;
                        out.second_suffix = suffix_second;
                        out.prefix_match = prefix_match;
                        out.suffix_match = suffix_match;
                        return out;
                    }
                }
                ++prefix_second[static_cast<std::size_t>(b)];
            }
            ++prefix_first[static_cast<std::size_t>(a)];
        }
        if (depth == max_depth) break;
        first = adelic::apply_substitution<d>(images, first);
        second = adelic::apply_substitution<d>(images, second);
    }
    return std::nullopt;
}

template <std::size_t d>
inline bool stage_strong_coincidence_pair_witness(
    const std::array<std::vector<long long>, d>& images,
    long long first_letter, long long second_letter,
    long long max_depth = 20, long long max_word_len = 5'000'000,
    std::string description = {}) {
    const auto witness = find_strong_coincidence_pair_witness(
        images, first_letter, second_letter, max_depth, max_word_len);
    if (!witness || !mathlib::reflection::enabled()) return false;
    mathlib::reflection::StrongCoincidencePairWitnessCertificate node;
    node.first_letter = witness->first_letter;
    node.second_letter = witness->second_letter;
    node.depth = witness->depth;
    node.common_letter = witness->common_letter;
    node.first_position = witness->first_position;
    node.second_position = witness->second_position;
    node.first_prefix.assign(witness->first_prefix.begin(), witness->first_prefix.end());
    node.second_prefix.assign(witness->second_prefix.begin(), witness->second_prefix.end());
    node.first_suffix.assign(witness->first_suffix.begin(), witness->first_suffix.end());
    node.second_suffix.assign(witness->second_suffix.begin(), witness->second_suffix.end());
    node.prefix_match = witness->prefix_match;
    node.suffix_match = witness->suffix_match;
    node.description = std::move(description);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, std::move(node));
    return true;
}

} // namespace ravel::proof
