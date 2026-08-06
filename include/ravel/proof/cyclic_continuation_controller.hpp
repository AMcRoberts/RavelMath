#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

using CyclicState = std::vector<std::int64_t>;

struct CyclicControllerPlant {
    std::vector<CyclicState> states;
    std::map<std::pair<std::size_t, std::int64_t>,
             std::vector<std::size_t>> successors;
};

struct CyclicRunCertificate {
    std::string certificate_id;
    std::vector<std::int64_t> base_word;
    std::size_t initial_state = 0;
    std::vector<std::size_t> controller_states;
    bool transition_replay = false;
    bool closes = false;
    bool valid = false;
};

inline std::set<std::size_t> image_under_word(
    const CyclicControllerPlant& plant,
    const std::set<std::size_t>& initial,
    const std::vector<std::int64_t>& word) {
    std::set<std::size_t> current = initial;
    for (const auto input : word) {
        std::set<std::size_t> next;
        for (const auto state : current) {
            const auto it = plant.successors.find({state, input});
            if (it == plant.successors.end()) continue;
            next.insert(it->second.begin(), it->second.end());
        }
        current = std::move(next);
    }
    return current;
}

inline std::set<std::size_t> fixed_points_under_word(
    const CyclicControllerPlant& plant,
    const std::vector<std::int64_t>& word) {
    std::set<std::size_t> result;
    for (std::size_t state = 0; state < plant.states.size(); ++state) {
        const auto image = image_under_word(plant, {state}, word);
        if (image.contains(state)) result.insert(state);
    }
    return result;
}

inline CyclicRunCertificate synthesize_cyclic_run(
    const CyclicControllerPlant& plant,
    std::vector<std::int64_t> word,
    std::string certificate_id = {}) {
    CyclicRunCertificate cert;
    cert.certificate_id = std::move(certificate_id);
    cert.base_word = std::move(word);

    const auto fixed = fixed_points_under_word(plant, cert.base_word);
    if (fixed.empty()) return cert;

    cert.initial_state = *fixed.begin();
    cert.controller_states = {cert.initial_state};

    std::size_t current = cert.initial_state;
    for (const auto input : cert.base_word) {
        const auto it = plant.successors.find({current, input});
        if (it == plant.successors.end()) return cert;

        const auto& choices = it->second;
        if (choices.empty()) return cert;

        // Prefer a successor that can still close after the remaining suffix.
        std::size_t chosen = choices.front();
        cert.controller_states.push_back(chosen);
        current = chosen;
    }

    // A direct greedy trace may not close even though a fixed run exists.
    // Replay by dynamic programming over position/state and reconstruct.
    const std::size_t L = cert.base_word.size();
    std::vector<std::set<std::size_t>> reachable(L + 1);
    std::vector<std::map<std::size_t, std::size_t>> parent(L + 1);
    reachable[0].insert(cert.initial_state);
    for (std::size_t k = 0; k < L; ++k) {
        for (const auto state : reachable[k]) {
            const auto it = plant.successors.find({state, cert.base_word[k]});
            if (it == plant.successors.end()) continue;
            for (const auto next : it->second) {
                if (!parent[k + 1].contains(next))
                    parent[k + 1][next] = state;
                reachable[k + 1].insert(next);
            }
        }
    }

    if (!reachable[L].contains(cert.initial_state)) return cert;

    std::vector<std::size_t> path(L + 1);
    path[L] = cert.initial_state;
    for (std::size_t k = L; k > 0; --k)
        path[k - 1] = parent[k].at(path[k]);
    cert.controller_states = std::move(path);

    cert.transition_replay = true;
    for (std::size_t k = 0; k < L; ++k) {
        const auto it = plant.successors.find(
            {cert.controller_states[k], cert.base_word[k]});
        cert.transition_replay &=
            it != plant.successors.end() &&
            std::find(it->second.begin(), it->second.end(),
                      cert.controller_states[k + 1]) != it->second.end();
    }
    cert.closes =
        !cert.controller_states.empty() &&
        cert.controller_states.front() == cert.controller_states.back();
    cert.valid = cert.transition_replay && cert.closes;
    return cert;
}

}  // namespace ravel::proof
