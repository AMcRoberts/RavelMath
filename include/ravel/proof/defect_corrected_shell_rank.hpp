#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ravel/proof/defect_corrected_radial_transport.hpp"
#include "ravel/proof/phase_rank_transport.hpp"

namespace ravel::proof {

using ShellState = std::vector<std::int64_t>;

struct ShellStateHash {
    std::size_t operator()(const ShellState& x) const noexcept {
        std::size_t h = 1469598103934665603ULL;
        for (auto v : x) {
            h ^= static_cast<std::size_t>(v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
            h *= 1099511628211ULL;
        }
        return h;
    }
};

inline std::int64_t shell_level(const ShellState& x) {
    std::int64_t out = 0;
    for (auto v : x) out += std::llabs(v);
    return out;
}

inline std::int64_t shell_radius(const ShellState& x) {
    std::int64_t out = 0;
    for (auto v : x) out = std::max(out, static_cast<std::int64_t>(std::llabs(v)));
    return out;
}

inline std::vector<ShellState> bounded_carry_successors(const ShellState& x,
                                                         std::int64_t bound) {
    if (x.size() < 2) throw std::invalid_argument("dimension must be >=2");
    const auto tail = x.front() -
        std::accumulate(x.begin() + 1, x.end(), std::int64_t{0});
    std::vector<ShellState> out;
    out.reserve(3);
    for (auto digit : {-1LL, 0LL, 1LL}) {
        ShellState y(x.begin() + 1, x.end());
        y.push_back(tail + digit);
        bool inside = true;
        for (auto v : y) inside = inside && (-bound <= v && v <= bound);
        if (inside) out.push_back(std::move(y));
    }
    return out;
}

inline void enumerate_box_states_rec(std::size_t n, std::int64_t bound,
                                     std::size_t i, ShellState& state,
                                     const std::function<void(const ShellState&)>& f) {
    if (i == n) { f(state); return; }
    for (std::int64_t v = -bound; v <= bound; ++v) {
        state[i] = v;
        enumerate_box_states_rec(n, bound, i + 1, state, f);
    }
}

inline std::string defect_corrected_shell_phase(const ShellState& x) {
    const auto n = x.size();
    const auto profile = radial_defect_profile(x);
    std::vector<std::int64_t> mags(n);
    for (std::size_t i = 0; i < n; ++i) mags[i] = std::llabs(x[i]);
    const auto minimum = *std::min_element(mags.begin(), mags.end());

    std::ostringstream out;
    out << "s:";
    for (auto s : profile.signs) out << (s < 0 ? '-' : s > 0 ? '+' : '0');
    out << "|g:";
    for (std::size_t i = 0; i < n; ++i) {
        if (i) out << ',';
        out << (mags[i] - minimum);
    }
    out << "|r:" << (minimum % static_cast<std::int64_t>(n + 1));
    out << "|f:";
    for (std::size_t i = 0; i < profile.adjacent_flux.size(); ++i) {
        if (i) out << ',';
        out << profile.adjacent_flux[i];
    }
    out << "|b:" << profile.signed_imbalance;
    out << "|t:" << profile.terminal_flux;
    return out.str();
}

struct ShellFirstReturnInstance {
    std::size_t n = 0;
    std::int64_t bound = 0;
    std::size_t shell_states = 0;
    std::size_t first_return_edges = 0;
};

struct DefectCorrectedShellRankResult {
    PhaseRankTransportCertificate rank;
    std::vector<ShellFirstReturnInstance> instances;
    std::size_t raw_edges = 0;
};

inline DefectCorrectedShellRankResult derive_defect_corrected_shell_rank(
    std::size_t n, std::int64_t min_bound, std::int64_t max_bound) {
    if (n < 2 || min_bound < 1 || max_bound < min_bound)
        throw std::invalid_argument("invalid shell rank range");

    std::set<std::string> phases;
    std::map<std::pair<std::string,std::string>, std::int64_t> merged;
    std::vector<ShellFirstReturnInstance> instances;
    std::size_t raw_edges = 0;

    for (std::int64_t bound = min_bound; bound <= max_bound; ++bound) {
        std::vector<ShellState> shell;
        std::unordered_set<ShellState, ShellStateHash> shell_set;
        ShellState state(n, 0);
        enumerate_box_states_rec(n, bound, 0, state, [&](const ShellState& x) {
            if (shell_radius(x) == bound) {
                shell.push_back(x);
                shell_set.insert(x);
            }
        });

        std::size_t edge_count = 0;
        for (const auto& source : shell) {
            std::deque<ShellState> queue;
            std::unordered_set<ShellState, ShellStateHash> seen;
            queue.push_back(source);
            seen.insert(source);
            const auto ps = defect_corrected_shell_phase(source);
            phases.insert(ps);
            const auto ls = shell_level(source);

            while (!queue.empty()) {
                auto current = std::move(queue.front());
                queue.pop_front();
                for (auto target : bounded_carry_successors(current, bound)) {
                    if (shell_set.contains(target)) {
                        const auto pt = defect_corrected_shell_phase(target);
                        phases.insert(pt);
                        const auto weight = ls - shell_level(target) + 1;
                        auto key = std::make_pair(ps, pt);
                        auto it = merged.find(key);
                        if (it == merged.end()) merged.emplace(std::move(key), weight);
                        else it->second = std::max(it->second, weight);
                        ++edge_count;
                        ++raw_edges;
                    } else if (seen.insert(target).second) {
                        queue.push_back(std::move(target));
                    }
                }
            }
        }
        instances.push_back({n, bound, shell.size(), edge_count});
    }

    std::vector<std::string> phase_vec(phases.begin(), phases.end());
    std::vector<PhaseRankEdge> constraints;
    constraints.reserve(merged.size());
    for (const auto& [key, gain] : merged)
        constraints.push_back({key.first, key.second, gain});
    std::vector<std::string> covered;
    for (const auto& x : instances)
        covered.push_back("n=" + std::to_string(x.n) + ",M=" + std::to_string(x.bound));

    return {close_phase_rank_transport(
                "defect-corrected-shell-rank-n" + std::to_string(n),
                std::move(phase_vec), std::move(constraints), std::move(covered)),
            std::move(instances), raw_edges};
}

} // namespace ravel::proof
