// return_substitution.hpp
//
// Induced return-word substitutions and their phase (tower) extension.
// A return word to marker m begins with m and ends immediately before
// the next occurrence of m.  For a recognizable marker-prefix
// substitution, images of return words factor exactly into return words.
//
// The ordinary derived substitution forgets position inside a return
// word.  ReturnPhaseSystem retains states (return_word, offset), producing
// a collared substitution that can carry the phase information missing
// from a quotient of the bare contact-boundary graph.

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "ravel/substitution.hpp"

namespace ravel {

using ReturnWord = std::vector<std::int8_t>;

struct ReturnPhaseState {
    std::size_t return_word = 0;
    std::size_t offset = 0;
};

struct ReturnSubstitution {
    std::int8_t marker = 0;
    std::vector<ReturnWord> words;
    // derived_images[r] is the factorization of sigma(words[r]) into
    // return-word indices.
    std::vector<std::vector<std::size_t>> derived_images;
};

struct ReturnPhaseSystem {
    ReturnSubstitution induced;
    std::vector<ReturnPhaseState> states;
    // phase_images[s] is the image of phase state s in phase-state
    // indices. It projects letter-for-letter to sigma(original_letter).
    std::vector<std::vector<std::size_t>> phase_images;

    std::vector<std::vector<long long>> incidence_matrix() const {
        const std::size_t n = states.size();
        std::vector<std::vector<long long>> matrix(
            n, std::vector<long long>(n, 0));
        for (std::size_t source = 0; source < n; ++source) {
            for (std::size_t destination : phase_images[source]) {
                ++matrix[destination][source];
            }
        }
        return matrix;
    }
};

inline std::vector<ReturnWord> discover_return_words(
        const SubstitutionRule& rule,
        std::int8_t marker,
        std::size_t orbit_cap = 1 << 20) {
    if (marker < 0
        || static_cast<std::size_t>(marker) >= rule.alphabet_size()) {
        throw std::invalid_argument("discover_return_words: marker out of range");
    }
    std::vector<std::int8_t> orbit{marker};
    std::vector<ReturnWord> words;
    std::map<ReturnWord, std::size_t> seen;
    std::size_t stable_rounds = 0;
    for (std::size_t round = 0; round < 64; ++round) {
        std::vector<std::size_t> markers;
        for (std::size_t i = 0; i < orbit.size(); ++i) {
            if (orbit[i] == marker) markers.push_back(i);
        }
        std::size_t before = words.size();
        for (std::size_t i = 0; i + 1 < markers.size(); ++i) {
            ReturnWord word(
                orbit.begin() + static_cast<std::ptrdiff_t>(markers[i]),
                orbit.begin() + static_cast<std::ptrdiff_t>(markers[i + 1]));
            if (!seen.count(word)) {
                seen[word] = words.size();
                words.push_back(std::move(word));
            }
        }
        if (words.size() == before && markers.size() >= 4) {
            ++stable_rounds;
            if (stable_rounds >= 2) break;
        } else {
            stable_rounds = 0;
        }
        if (orbit.size() >= orbit_cap) break;
        orbit = rule.apply_once(orbit);
        if (orbit.size() > orbit_cap) orbit.resize(orbit_cap);
    }
    if (words.empty()) {
        throw std::runtime_error("discover_return_words: no complete return word");
    }
    return words;
}

inline std::vector<std::size_t> factor_return_words(
        const ReturnWord& word,
        std::int8_t marker,
        const std::map<ReturnWord, std::size_t>& index) {
    if (word.empty() || word.front() != marker) {
        throw std::invalid_argument(
            "factor_return_words: word does not begin at marker");
    }
    std::vector<std::size_t> starts;
    for (std::size_t i = 0; i < word.size(); ++i) {
        if (word[i] == marker) starts.push_back(i);
    }
    starts.push_back(word.size());
    std::vector<std::size_t> factors;
    for (std::size_t i = 0; i + 1 < starts.size(); ++i) {
        ReturnWord factor(
            word.begin() + static_cast<std::ptrdiff_t>(starts[i]),
            word.begin() + static_cast<std::ptrdiff_t>(starts[i + 1]));
        auto it = index.find(factor);
        if (it == index.end()) {
            throw std::runtime_error(
                "factor_return_words: return-word set is not closed");
        }
        factors.push_back(it->second);
    }
    return factors;
}

inline ReturnSubstitution build_return_substitution(
        const SubstitutionRule& rule,
        std::int8_t marker,
        std::size_t orbit_cap = 1 << 20) {
    ReturnSubstitution result;
    result.marker = marker;
    result.words = discover_return_words(rule, marker, orbit_cap);
    std::map<ReturnWord, std::size_t> index;
    for (std::size_t i = 0; i < result.words.size(); ++i) {
        index[result.words[i]] = i;
    }
    result.derived_images.resize(result.words.size());
    for (std::size_t i = 0; i < result.words.size(); ++i) {
        ReturnWord image = rule.apply_once(result.words[i]);
        result.derived_images[i] =
            factor_return_words(image, marker, index);
    }
    return result;
}

inline ReturnPhaseSystem build_return_phase_system(
        const SubstitutionRule& rule,
        std::int8_t marker,
        std::size_t orbit_cap = 1 << 20) {
    ReturnPhaseSystem result;
    result.induced = build_return_substitution(rule, marker, orbit_cap);

    std::vector<std::vector<std::size_t>> state_index(
        result.induced.words.size());
    for (std::size_t r = 0; r < result.induced.words.size(); ++r) {
        state_index[r].resize(result.induced.words[r].size());
        for (std::size_t offset = 0;
             offset < result.induced.words[r].size(); ++offset) {
            state_index[r][offset] = result.states.size();
            result.states.push_back({r, offset});
        }
    }
    result.phase_images.resize(result.states.size());

    for (std::size_t r = 0; r < result.induced.words.size(); ++r) {
        std::vector<std::size_t> expanded_phase;
        for (std::size_t factor : result.induced.derived_images[r]) {
            for (std::size_t offset = 0;
                 offset < result.induced.words[factor].size(); ++offset) {
                expanded_phase.push_back(state_index[factor][offset]);
            }
        }
        std::size_t cursor = 0;
        for (std::size_t offset = 0;
             offset < result.induced.words[r].size(); ++offset) {
            std::size_t state = state_index[r][offset];
            std::size_t image_length = rule.image(
                static_cast<std::size_t>(
                    result.induced.words[r][offset])).size();
            if (cursor + image_length > expanded_phase.size()) {
                throw std::runtime_error(
                    "build_return_phase_system: inconsistent phase lift");
            }
            result.phase_images[state] = std::vector<std::size_t>(
                expanded_phase.begin()
                    + static_cast<std::ptrdiff_t>(cursor),
                expanded_phase.begin()
                    + static_cast<std::ptrdiff_t>(cursor + image_length));
            cursor += image_length;
        }
        if (cursor != expanded_phase.size()) {
            throw std::runtime_error(
                "build_return_phase_system: unused expanded phase");
        }
    }
    return result;
}

}  // namespace ravel
