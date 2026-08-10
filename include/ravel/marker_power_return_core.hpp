// Finite marker-power return core for recognizable substitution experiments.
// This is an analysis certificate, not a claim that every substitution has a
// small power: it records the first tested power whose marker image begins at
// the marker and the recurrent structure of its return-phase lift.

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "ravel/return_substitution.hpp"

namespace ravel {

struct MarkerPowerReturnCoreCertificate {
    std::int8_t marker = 0;
    std::size_t power = 0;
    bool marker_proper = false;
    std::size_t return_words = 0;
    std::size_t phase_states = 0;
    std::size_t phase_sccs = 0;
    std::size_t largest_phase_scc = 0;
    bool single_recurrent_core = false;
    bool holds = false;
};

inline SubstitutionRule marker_power_rule(
        const SubstitutionRule& rule, std::size_t power) {
    std::vector<std::vector<std::int8_t>> images(rule.alphabet_size());
    for (std::size_t i = 0; i < images.size(); ++i)
        images[i] = {static_cast<std::int8_t>(i)};
    for (std::size_t k = 0; k < power; ++k)
        for (std::size_t i = 0; i < images.size(); ++i)
            images[i] = rule.apply_once(images[i]);
    return SubstitutionRule(images);
}

inline std::vector<std::size_t> marker_phase_scc_sizes(
        const ReturnPhaseSystem& phase) {
    const std::size_t n = phase.states.size();
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<bool> active(n, false);
    std::vector<std::size_t> sizes;
    int next = 0;
    std::function<void(std::size_t)> visit = [&](std::size_t v) {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v));
        active[v] = true;
        for (std::size_t w : phase.phase_images[v]) {
            if (index[w] < 0) {
                visit(w);
                low[v] = std::min(low[v], low[w]);
            } else if (active[w]) {
                low[v] = std::min(low[v], index[w]);
            }
        }
        if (low[v] != index[v]) return;
        std::size_t size = 0;
        while (true) {
            const std::size_t w = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            active[w] = false;
            ++size;
            if (w == v) break;
        }
        sizes.push_back(size);
    };
    for (std::size_t v = 0; v < n; ++v)
        if (index[v] < 0) visit(v);
    return sizes;
}

inline MarkerPowerReturnCoreCertificate analyze_marker_power_return_core(
        const SubstitutionRule& rule, std::int8_t marker,
        std::size_t max_power = 8, std::size_t orbit_cap = 1 << 18) {
    if (marker < 0
        || static_cast<std::size_t>(marker) >= rule.alphabet_size()) {
        throw std::invalid_argument("marker_power_return_core: marker out of range");
    }
    MarkerPowerReturnCoreCertificate out;
    out.marker = marker;
    for (std::size_t power = 1; power <= max_power; ++power) {
        const auto powered = marker_power_rule(rule, power);
        const auto& image = powered.image(static_cast<std::size_t>(marker));
        if (image.empty() || image.front() != marker) continue;
        out.power = power;
        out.marker_proper = true;
        const auto phase = build_return_phase_system(powered, marker, orbit_cap);
        out.return_words = phase.induced.words.size();
        out.phase_states = phase.states.size();
        const auto sizes = marker_phase_scc_sizes(phase);
        out.phase_sccs = sizes.size();
        for (std::size_t size : sizes)
            out.largest_phase_scc = std::max(out.largest_phase_scc, size);
        out.single_recurrent_core =
            sizes.size() == 1 && out.largest_phase_scc == out.phase_states;
        out.holds = out.single_recurrent_core;
        return out;
    }
    return out;
}

}  // namespace ravel
