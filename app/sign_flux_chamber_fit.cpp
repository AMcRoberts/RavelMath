#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ravel/proof/block_height_shell_rank.hpp"
#include "ravel/proof/sign_symmetric_chamber_rank.hpp"

using namespace ravel::proof;

namespace {

constexpr std::size_t obs_count = 11;
constexpr std::size_t flux_count = 28; // 1 + counts(3) + ends(6) + linear pairs(9) + cyclic pairs(9)
constexpr std::size_t variable_count = obs_count * flux_count;

struct Edge { ShellState source, target; };

int sign_index(std::int64_t x) { return x < 0 ? 0 : x == 0 ? 1 : 2; }

std::string chamber(const ShellState& x) {
    std::string out;
    out.reserve(x.size());
    for (auto v : x) out.push_back(v < 0 ? '-' : v > 0 ? '+' : '0');
    return out;
}

ShellState negate_state(ShellState x) {
    for (auto& v : x) v = -v;
    return x;
}

ShellState canonical_state(const ShellState& x) {
    const auto c = chamber(x);
    const auto nc = negate_sign_chamber(c);
    return nc < c ? negate_state(x) : x;
}

std::array<long double, obs_count> observables(const ShellState& raw) {
    const auto x = canonical_state(raw);
    const auto f = derive_block_height_features(x);
    std::int64_t level = 0, sum = 0, maxv = x.front(), minv = x.front();
    for (auto v : x) {
        level += std::llabs(v); sum += v;
        maxv = std::max(maxv, v); minv = std::min(minv, v);
    }
    return {static_cast<long double>(level),
            static_cast<long double>(f.forcing_support),
            static_cast<long double>(f.forcing_zero_count),
            static_cast<long double>(f.forcing_gcd),
            static_cast<long double>(f.moment_variation),
            static_cast<long double>(f.moment_energy),
            static_cast<long double>(f.adjacent_energy),
            static_cast<long double>(sum),
            static_cast<long double>(maxv),
            static_cast<long double>(minv),
            1.0L};
}

std::array<long double, flux_count> sign_flux(const ShellState& raw) {
    const auto x = canonical_state(raw);
    std::array<long double, flux_count> f{};
    f[0] = 1;
    for (auto v : x) f[1 + sign_index(v)] += 1;
    f[4 + sign_index(x.front())] = 1;
    f[7 + sign_index(x.back())] = 1;
    for (std::size_t i = 1; i < x.size(); ++i)
        f[10 + 3 * sign_index(x[i-1]) + sign_index(x[i])] += 1;
    for (std::size_t i = 0; i < x.size(); ++i) {
        const auto j = (i + 1) % x.size();
        f[19 + 3 * sign_index(x[i]) + sign_index(x[j])] += 1;
    }
    return f;
}

std::array<long double, variable_count> design(const ShellState& x) {
    const auto o = observables(x);
    const auto f = sign_flux(x);
    std::array<long double, variable_count> p{};
    for (std::size_t i = 0; i < flux_count; ++i)
        for (std::size_t j = 0; j < obs_count; ++j)
            p[i * obs_count + j] = f[i] * o[j];
    return p;
}

long double dot(const std::array<long double, variable_count>& a,
                const std::array<long double, variable_count>& b) {
    long double out = 0;
    for (std::size_t i = 0; i < variable_count; ++i) out += a[i] * b[i];
    return out;
}

std::vector<Edge> collect_edges() {
    constexpr std::size_t n = 4;
    std::vector<Edge> edges;
    for (std::int64_t bound = 2; bound <= 6; ++bound) {
        std::vector<ShellState> shell;
        std::unordered_set<ShellState, ShellStateHash> shell_set;
        ShellState state(n, 0);
        enumerate_box_states_rec(n, bound, 0, state, [&](const ShellState& x) {
            if (shell_radius(x) == bound) { shell.push_back(x); shell_set.insert(x); }
        });
        for (const auto& source : shell) {
            std::deque<ShellState> queue;
            std::unordered_set<ShellState, ShellStateHash> seen;
            queue.push_back(source); seen.insert(source);
            while (!queue.empty()) {
                auto current = std::move(queue.front()); queue.pop_front();
                for (auto target : bounded_carry_successors(current, bound)) {
                    if (shell_set.contains(target)) edges.push_back({source, std::move(target)});
                    else if (seen.insert(target).second) queue.push_back(std::move(target));
                }
            }
        }
    }
    return edges;
}

} // namespace

int main() {
    const auto edges = collect_edges();
    std::cerr << "edges=" << edges.size() << " variables=" << variable_count << "\n";
    if (edges.size() != 156832) return 2;

    std::vector<std::array<long double, variable_count>> source_phi, target_phi;
    source_phi.reserve(edges.size()); target_phi.reserve(edges.size());
    for (const auto& e : edges) { source_phi.push_back(design(e.source)); target_phi.push_back(design(e.target)); }

    std::array<long double, variable_count> w{};
    constexpr long double desired_margin = 1.0L;
    constexpr int epochs = 30;
    for (int epoch = 0; epoch < epochs; ++epoch) {
        std::size_t violations = 0;
        long double worst = std::numeric_limits<long double>::infinity();
        for (std::size_t k = 0; k < edges.size(); ++k) {
            long double margin = 0, norm2 = 0;
            for (std::size_t i = 0; i < variable_count; ++i) {
                const auto d = target_phi[k][i] - source_phi[k][i];
                margin += w[i] * d; norm2 += d * d;
            }
            worst = std::min(worst, margin);
            if (margin < desired_margin && norm2 > 0) {
                const auto step = (desired_margin - margin) / norm2;
                for (std::size_t i = 0; i < variable_count; ++i)
                    w[i] += step * (target_phi[k][i] - source_phi[k][i]);
                ++violations;
            }
        }
        std::cerr << "epoch=" << epoch << " violations=" << violations << " preupdate_worst=" << (double)worst << "\n";
        if (!violations) break;
    }

    std::size_t bad = 0;
    std::map<std::pair<std::string,std::string>, std::size_t> bad_pairs;
    long double worst = std::numeric_limits<long double>::infinity();
    for (std::size_t k = 0; k < edges.size(); ++k) {
        long double margin = 0;
        for (std::size_t i = 0; i < variable_count; ++i)
            margin += w[i] * (target_phi[k][i] - source_phi[k][i]);
        worst = std::min(worst, margin);
        if (!(margin > 0)) { ++bad; ++bad_pairs[{chamber(edges[k].source), chamber(edges[k].target)}]; }
    }
    std::cout << "SIGN_FLUX_FIT edges=" << edges.size() << " variables=" << variable_count
              << " failures=" << bad << " minimum_margin=" << (double)worst << "\n";

    std::vector<std::pair<std::size_t,std::pair<std::string,std::string>>> bp;
    for (const auto& [p,c] : bad_pairs) bp.push_back({c,p});
    std::sort(bp.rbegin(), bp.rend());
    for (std::size_t i=0;i<std::min<std::size_t>(12,bp.size());++i)
        std::cout << "BADPAIR " << bp[i].second.first << "->" << bp[i].second.second << " count=" << bp[i].first << "\n";

    // Emit the largest coefficients for diagnosis.
    std::vector<std::pair<long double,std::size_t>> order;
    for (std::size_t i=0;i<variable_count;++i) order.push_back({std::fabs(w[i]),i});
    std::sort(order.rbegin(), order.rend());
    for (std::size_t k=0;k<std::min<std::size_t>(30,order.size());++k) {
        const auto i=order[k].second;
        std::cout << "W flux=" << (i/obs_count) << " obs=" << (i%obs_count)
                  << " value=" << (double)w[i] << "\n";
    }
    return bad ? 3 : 0;
}
