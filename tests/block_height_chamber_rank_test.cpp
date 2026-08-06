#include <cassert>
#include <cstdint>
#include <deque>
#include <iostream>
#include <unordered_set>
#include <vector>
#include "ravel/proof/block_height_shell_rank.hpp"
#include "ravel/proof/generated_n4_block_height_chamber_rank.hpp"
using namespace ravel::proof;

static std::string chamber(const ShellState& x) {
    std::string s;
    for (auto v : x) s.push_back(v < 0 ? '-' : v > 0 ? '+' : '0');
    return s;
}

static std::array<std::int64_t, 10> feature_vector(const ShellState& x) {
    const auto f = derive_block_height_features(x);
    std::int64_t level = 0, signed_sum = 0, maxv = x.front(), minv = x.front();
    for (auto v : x) {
        level += std::llabs(v);
        signed_sum += v;
        maxv = std::max(maxv, v);
        minv = std::min(minv, v);
    }
    return {level, f.forcing_support, f.forcing_zero_count, f.forcing_gcd,
            f.moment_variation, f.moment_energy, f.adjacent_energy,
            signed_sum, maxv, minv};
}

static __int128 rank_value(const ShellState& x) {
    const auto it = n4_block_height_chamber_rank().find(chamber(x));
    assert(it != n4_block_height_chamber_rank().end());
    const auto f = feature_vector(x);
    __int128 out = it->second.c[10];
    for (std::size_t i = 0; i < f.size(); ++i)
        out += static_cast<__int128>(it->second.c[i]) * f[i];
    return out;
}

int main() {
    std::size_t checks = 0;
    __int128 minimum_gain = static_cast<__int128>(1) << 120;
    constexpr std::size_t n = 4;
    for (std::int64_t bound = 2; bound <= 6; ++bound) {
        std::vector<ShellState> shell;
        std::unordered_set<ShellState, ShellStateHash> shell_set;
        ShellState state(n, 0);
        enumerate_box_states_rec(n, bound, 0, state, [&](const ShellState& x) {
            if (shell_radius(x) == bound) {
                shell.push_back(x);
                shell_set.insert(x);
            }
        });
        for (const auto& source : shell) {
            std::deque<ShellState> queue;
            std::unordered_set<ShellState, ShellStateHash> seen;
            queue.push_back(source);
            seen.insert(source);
            while (!queue.empty()) {
                auto current = std::move(queue.front());
                queue.pop_front();
                for (auto target : bounded_carry_successors(current, bound)) {
                    if (shell_set.contains(target)) {
                        const auto gain = rank_value(target) - rank_value(source);
                        assert(gain > 0);
                        minimum_gain = std::min(minimum_gain, gain);
                        ++checks;
                    } else if (seen.insert(target).second) {
                        queue.push_back(std::move(target));
                    }
                }
            }
        }
    }
    assert(checks == 156832);
    std::cout << "block-height chamber rank n=4 PASS checks=" << checks
              << " minimum_gain=" << static_cast<long long>(minimum_gain) << "\n";
}
