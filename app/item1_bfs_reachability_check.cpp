// item1_bfs_reachability_check.cpp
//
// Extends the item-1 maximality check (app/item1_per_pair_check.cpp)
// to values of n the full corona construction makes impractical, by
// exploiting a fact confirmed this session (see docs/RESEARCH_STATUS.md):
// the DOMINANT recurrent core is stable under embedding -- a plain
// forward+backward BFS through simple_forward_targets /
// simple_backward_targets (restricted only by is_valid_simple_node),
// starting from ANY single node known to be in the dominant core,
// recovers EXACTLY the same dominant recurrent core as the full
// corona/backward-closure/Red construction, regardless of how much
// further the BFS explores beyond it (checked stable at both a
// 20,000- and 60,000-node cap for Tetrabonacci, n=4: both give
// exactly the same 46-node dominant core as the real, corona-built
// G_B). This sidesteps the corona machinery's own cost (which grows
// fast enough that n=7 already needs backgrounding, per TECHNICAL NOTE).
//
// The seed used here is the CONJECTURED extremal node itself
// (`x = -e_1 + e_{n-1}` at label `(i=1, j=n-1)`), which the existing
// four-case derivation (docs/RESEARCH_STATUS.md, "the item-1
// investigation") already shows is validly reachable for every `n`
// via `simple_forward_targets`'s own decomposition structure -- so
// this isn't circular: we are not ASSUMING it's in the dominant core,
// we are independently RE-DISCOVERING which SCC it belongs to via
// Tarjan + Perron-root comparison on the BFS-explored neighborhood,
// exactly as `extract_dominant_recurrent_core` would from the full
// graph.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/item1_bfs_reachability_check.cpp math/out/libmath.a -o item1_bfs_check

#include <array>
#include <cmath>
#include <cstdio>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

double compute_beta(std::size_t n) {
    std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
    for (std::size_t c = 0; c + 1 < n; ++c) {
        M[0][c] += 1;
        M[c + 1][c] += 1;
    }
    M[0][n - 1] += 1;
    auto cls = classify_matrix_spectral(M);
    return cls.beta;
}

template <std::size_t D>
void check(std::size_t n, std::size_t bfs_cap) {
    double beta = compute_beta(n);
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);
    std::printf("=== n=%zu, beta=%.12f ===\n", n, beta);

    // Conjectured extremal seed: i=1, x = -e_1 + e_{n-1}, j = n-1.
    SNode<D> seed;
    seed.i = 1;
    std::array<long long, D> x{};
    x[1] = -1;
    x[n - 1] += 1;
    seed.x = x;
    seed.j = static_cast<long long>(n - 1);

    if (!is_valid_simple_node<D>(subst, seed)) {
        std::printf("  seed invalid -- aborting this n\n");
        return;
    }

    std::set<SNode<D>> bfs_nodes;
    std::queue<SNode<D>> q;
    bfs_nodes.insert(seed);
    q.push(seed);
    bool hit_cap = false;
    while (!q.empty() && bfs_nodes.size() < bfs_cap) {
        SNode<D> cur = q.front();
        q.pop();
        auto fwd = simple_forward_targets<D>(subst, cur);
        for (const auto& [dest, pq] : fwd) {
            (void)pq;
            if (bfs_nodes.size() >= bfs_cap) { hit_cap = true; break; }
            if (bfs_nodes.insert(dest).second) q.push(dest);
        }
        auto bwd = simple_backward_targets<D>(subst, cur);
        for (const auto& [pred, pq] : bwd) {
            (void)pq;
            if (bfs_nodes.size() >= bfs_cap) { hit_cap = true; break; }
            if (bfs_nodes.insert(pred).second) q.push(pred);
        }
    }
    std::printf("  BFS explored %zu nodes%s\n", bfs_nodes.size(),
                hit_cap ? " (HIT CAP -- results may be incomplete, raise bfs_cap)" : " (queue exhausted naturally)");

    std::vector<SNode<D>> nodes(bfs_nodes.begin(), bfs_nodes.end());
    std::map<SNode<D>, std::size_t> idx;
    for (std::size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;
    WeightedDigraph g(nodes.size());
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        auto fwd = simple_forward_targets<D>(subst, nodes[i]);
        for (const auto& [dest, pq] : fwd) {
            (void)pq;
            auto it = idx.find(dest);
            if (it != idx.end()) g.add_edge(i, it->second, 1);
        }
    }
    auto [dom_core, dom_idx] = extract_dominant_recurrent_core(g, 500);
    (void)dom_core;
    std::printf("  dominant recurrent core: %zu nodes\n", dom_idx.size());

    // Verify the seed itself really landed in the extracted dominant core
    // (sanity check the whole exercise isn't vacuous).
    std::size_t seed_u = idx.at(seed);
    bool seed_in_dom = false;
    for (auto u : dom_idx) if (u == seed_u) { seed_in_dom = true; break; }
    std::printf("  seed node in extracted dominant core? %s\n", seed_in_dom ? "YES" : "NO (problem!)");

    // Per-pair argmax check, exactly as item1_per_pair_check.cpp.
    std::vector<double> v(n);
    v[0] = 1.0;
    for (std::size_t c = 1; c < n; ++c) v[c] = beta * v[c - 1] - 1.0;

    std::map<std::pair<long long, long long>, double> best_t;
    for (auto u : dom_idx) {
        double t = subst.dot_v(nodes[u].x);
        auto key = std::make_pair(nodes[u].i, nodes[u].j);
        auto it = best_t.find(key);
        if (it == best_t.end() || std::abs(t) > std::abs(it->second)) best_t[key] = t;
    }

    int mismatches = 0;
    for (auto& [key, t] : best_t) {
        auto [i, j] = key;
        if (i == j) continue;
        long long lo = std::min(i, j), hi = std::max(i, j);
        double predicted;
        if (lo == 0 && hi == static_cast<long long>(n - 1)) {
            predicted = 1.0 - 1.0 / beta;
        } else if (lo == 0) {
            predicted = 1.0 - std::pow(beta, static_cast<double>(hi) - static_cast<double>(n));
        } else {
            predicted = v[lo] - v[n - 1];
        }
        bool match = std::abs(std::abs(t) - predicted) < 1e-6;
        if (!match) {
            ++mismatches;
            std::printf("  MISMATCH (i=%lld,j=%lld): |t|=%.6f predicted=%.6f\n", i, j, std::abs(t), predicted);
        }
    }
    std::printf("  checked %zu pairs, %d mismatches\n\n", best_t.size(), mismatches);
}

}  // namespace

int main() {
    check<8>(8, 200000);
    check<9>(9, 200000);
    check<10>(10, 200000);
    return 0;
}
