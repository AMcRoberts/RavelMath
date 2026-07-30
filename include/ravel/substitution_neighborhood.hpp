#pragma once

#include <cstddef>
#include <cstdint>
#include <set>
#include <utility>
#include <vector>

namespace ravel {

using FiniteWord = std::vector<std::int8_t>;
using FiniteSubstitution = std::vector<FiniteWord>;

struct AdjacentSwapNeighbor {
    FiniteSubstitution substitution;
    std::size_t image = 0;
    std::size_t position = 0;
};

// The smallest word-level move that preserves every Parikh vector, hence
// preserves the incidence matrix and its expanding/contracting splitting.
// Equal-letter swaps are omitted and coincident resulting substitutions are
// deduplicated.
inline std::vector<AdjacentSwapNeighbor> adjacent_swap_neighbors(
        const FiniteSubstitution& substitution) {
    std::vector<AdjacentSwapNeighbor> result;
    std::set<FiniteSubstitution> seen;
    for (std::size_t image = 0; image < substitution.size(); ++image) {
        const auto& word = substitution[image];
        for (std::size_t position = 0; position + 1 < word.size(); ++position) {
            if (word[position] == word[position + 1]) continue;
            auto neighbor = substitution;
            std::swap(
                neighbor[image][position], neighbor[image][position + 1]);
            if (seen.insert(neighbor).second) {
                result.push_back(
                    {std::move(neighbor), image, position});
            }
        }
    }
    return result;
}

inline std::vector<std::vector<std::size_t>> parikh_columns(
        const FiniteSubstitution& substitution) {
    const std::size_t alphabet_size = substitution.size();
    std::vector<std::vector<std::size_t>> columns(
        alphabet_size, std::vector<std::size_t>(alphabet_size, 0));
    for (std::size_t image = 0; image < alphabet_size; ++image) {
        for (const auto letter : substitution[image]) {
            if (letter < 0
                    || static_cast<std::size_t>(letter) >= alphabet_size) {
                return {};
            }
            ++columns[image][static_cast<std::size_t>(letter)];
        }
    }
    return columns;
}

inline bool same_incidence_matrix(
        const FiniteSubstitution& left,
        const FiniteSubstitution& right) {
    const auto left_columns = parikh_columns(left);
    return !left_columns.empty() && left_columns == parikh_columns(right);
}

}  // namespace ravel
