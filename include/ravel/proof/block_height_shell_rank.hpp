#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ravel/proof/defect_corrected_shell_rank.hpp"

namespace ravel::proof {

// Exact support function of the (n+1)-digit block forcing polytope.
// For a state x and block digits d_0,...,d_n in {-1,0,1},
//   <x,F(d)> = sum_r c_r(x) d_r,
// where F is the exact forcing vector from A^(n+1)=2A-I.
inline std::vector<std::int64_t> block_forcing_dual_coefficients(
    const ShellState& x) {
    if (x.size() < 2) throw std::invalid_argument("dimension must be >=2");
    const std::size_t n = x.size();
    std::vector<std::int64_t> c(n + 1, 0);
    c[0] = -x[0] + 2 * x[n - 1];
    for (std::size_t r = 1; r < n; ++r) c[r] = x[r - 1] - x[r];
    c[n] = x[n - 1];
    return c;
}

inline std::int64_t block_forcing_support(const ShellState& x) {
    const auto c = block_forcing_dual_coefficients(x);
    std::int64_t out = 0;
    for (auto v : c) out += std::llabs(v);
    return out;
}

inline ShellState homogeneous_carry_step(const ShellState& x) {
    const auto tail = x.front() -
        std::accumulate(x.begin() + 1, x.end(), std::int64_t{0});
    ShellState y(x.begin() + 1, x.end());
    y.push_back(tail);
    return y;
}

// A finite exact joint-height signature.  The sequence m_k=(A^k x)_0 is a
// Krylov coordinate system for the carry companion matrix and therefore
// contains the same information as the conjugate-height vector, but uses only
// integer arithmetic.  We retain transportable qualitative and residue data,
// not the full concrete tuple.
inline std::vector<std::int64_t> carry_krylov_moments(const ShellState& x) {
    ShellState y = x;
    std::vector<std::int64_t> moments;
    moments.reserve(x.size() + 1);
    for (std::size_t k = 0; k <= x.size(); ++k) {
        moments.push_back(y.front());
        y = homogeneous_carry_step(y);
    }
    return moments;
}

inline std::string sign_word(const std::vector<std::int64_t>& xs) {
    std::string out;
    out.reserve(xs.size());
    for (auto x : xs) out.push_back(x < 0 ? '-' : x > 0 ? '+' : '0');
    return out;
}

inline std::int64_t positive_mod(std::int64_t x, std::int64_t m) {
    auto r = x % m;
    return r < 0 ? r + m : r;
}

struct BlockHeightFeatures {
    std::vector<std::int64_t> forcing_coefficients;
    std::int64_t forcing_support = 0;
    std::int64_t forcing_zero_count = 0;
    std::int64_t forcing_gcd = 0;
    std::vector<std::int64_t> krylov_moments;
    std::int64_t moment_variation = 0;
    std::int64_t moment_energy = 0;
    std::int64_t adjacent_energy = 0;
};

inline BlockHeightFeatures derive_block_height_features(const ShellState& x) {
    BlockHeightFeatures f;
    f.forcing_coefficients = block_forcing_dual_coefficients(x);
    for (auto c : f.forcing_coefficients) {
        f.forcing_support += std::llabs(c);
        f.forcing_zero_count += (c == 0);
        f.forcing_gcd = std::gcd(f.forcing_gcd, std::llabs(c));
    }
    f.krylov_moments = carry_krylov_moments(x);
    for (std::size_t i = 0; i < f.krylov_moments.size(); ++i) {
        const auto m = f.krylov_moments[i];
        f.moment_energy += m * m;
        if (i) f.moment_variation += std::llabs(m - f.krylov_moments[i - 1]);
    }
    for (std::size_t i = 1; i < x.size(); ++i)
        f.adjacent_energy += x[i - 1] * x[i];
    return f;
}

inline std::string block_height_shell_phase(const ShellState& x,
                                             bool use_block,
                                             bool use_height) {
    std::ostringstream out;
    out << defect_corrected_shell_phase(x);
    const auto n = static_cast<std::int64_t>(x.size());
    const auto radius = shell_radius(x);
    const auto modulus = n + 1;
    const auto f = derive_block_height_features(x);
    if (use_block) {
        out << "|BF:s=" << sign_word(f.forcing_coefficients)
            << ",z=" << f.forcing_zero_count
            << ",g=" << positive_mod(f.forcing_gcd, modulus)
            << ",q=" << (radius ? f.forcing_support / radius : 0)
            << ",r=" << positive_mod(f.forcing_support, modulus);
    }
    if (use_height) {
        out << "|JH:s=" << sign_word(f.krylov_moments)
            << ",vq=" << (radius ? f.moment_variation / radius : 0)
            << ",vr=" << positive_mod(f.moment_variation, modulus)
            << ",eq=" << (radius ? f.moment_energy / (radius * radius) : 0)
            << ",er=" << positive_mod(f.moment_energy, modulus)
            << ",ar=" << positive_mod(f.adjacent_energy, modulus);
    }
    return out.str();
}

struct BlockHeightShellRankResult {
    PhaseRankTransportCertificate rank;
    std::vector<ShellFirstReturnInstance> instances;
    std::size_t raw_edges = 0;
    bool use_block = false;
    bool use_height = false;
};

inline BlockHeightShellRankResult derive_block_height_shell_rank(
    std::size_t n, std::int64_t min_bound, std::int64_t max_bound,
    bool use_block, bool use_height) {
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
            const auto ps = block_height_shell_phase(source, use_block, use_height);
            phases.insert(ps);
            const auto ls = shell_level(source);

            while (!queue.empty()) {
                auto current = std::move(queue.front());
                queue.pop_front();
                for (auto target : bounded_carry_successors(current, bound)) {
                    if (shell_set.contains(target)) {
                        const auto pt = block_height_shell_phase(target, use_block, use_height);
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

    std::string mode = use_block ? "block" : "plain";
    if (use_height) mode += "+height";
    return {close_phase_rank_transport(
                mode + "-shell-rank-n" + std::to_string(n),
                std::move(phase_vec), std::move(constraints), std::move(covered)),
            std::move(instances), raw_edges, use_block, use_height};
}

} // namespace ravel::proof
