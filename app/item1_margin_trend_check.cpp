// item1_margin_trend_check.cpp
//
// Computes the dominant recurrent SCC's worst-case spectral safety
// margin (see docs/DIRECTION_AND_OPEN_THREADS.md thread A item 1) as
// a function of n. The worst node is always [i=n-1, j=0], and the
// margin has an exact closed form, `margin(n) = 2/beta_n - 1`,
// provably positive for every finite n (beta_n < 2 always, from the
// n-bonacci constant's own defining identity). This driver prints the
// margin (and, with the x-vector printing enabled, the extremal
// node's x-vector) for n=3..7 so the closed form can be checked
// directly against the actual computed data. See docs/RESEARCH_STATUS.md
// for how the closed form was found.

#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"

using namespace ravel;

namespace {

std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

template <std::size_t D>
void check_margin(std::size_t n, double beta) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);

    auto d_cont_cands = search_D_cont<D>(subst, 2);
    std::vector<ANode<D>> d_cont_an;
    for (const auto& c : d_cont_cands) {
        ANode<D> nd; nd.i = c.i; nd.j = c.j; nd.x = c.x;
        d_cont_an.push_back(nd);
    }
    auto gp_nodes = backward_closure<D>(subst, d_cont_an, kContactBoundaryMaxNodes);
    auto gp_edges = induced_restricted_edges<D>(subst, gp_nodes);
    std::set<ANode<D>> gp_set(gp_nodes.begin(), gp_nodes.end());
    auto red_gp = red_anode<D>(gp_set, gp_edges);
    std::set<SNode<D>> C;
    for (const auto& a : red_gp.first) {
        SNode<D> s; s.i = a.i; s.j = a.j; s.x = a.x;
        C.insert(s);
    }
    auto pmC = build_signed_contact_set<D>(C);
    auto A_prev = pmC;
    std::set<SNode<D>> gb;
    constexpr int MAX_ROUNDS = 8;
    constexpr std::size_t MAX_A_SIZE = 50000;
    for (int p = 2; p <= MAX_ROUNDS; ++p) {
        auto corona_nodes = c_corona<D>(subst, A_prev, A_prev);
        if (corona_nodes.size() > MAX_A_SIZE) break;
        std::vector<std::tuple<SNode<D>, SNode<D>, std::vector<long long>, std::vector<long long>>> edges;
        for (const auto& nn : corona_nodes) {
            auto fwd = simple_forward_targets<D>(subst, nn);
            for (const auto& [dest, pq] : fwd) {
                (void)pq;
                if (corona_nodes.count(dest) > 0) edges.push_back({nn, dest, {}, {}});
            }
        }
        auto red_result = red<D>(corona_nodes, edges);
        if (red_result.first == A_prev) { gb = red_result.first; break; }
        A_prev = red_result.first;
        gb = A_prev;
    }

    std::vector<SNode<D>> nodes(gb.begin(), gb.end());
    std::map<SNode<D>, std::size_t> idx;
    for (std::size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;
    std::vector<std::vector<long long>> dense(nodes.size(), std::vector<long long>(nodes.size(), 0));
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        auto fwd = simple_forward_targets<D>(subst, nodes[i]);
        for (const auto& [dest, pq] : fwd) {
            (void)pq;
            auto it = idx.find(dest);
            if (it != idx.end()) dense[i][it->second] += 1;
        }
    }
    auto gb_graph = WeightedDigraph::from_dense(dense);
    auto [dom_core, dom_idx] = extract_dominant_recurrent_core(gb_graph, 500);
    (void)dom_core;

    double min_margin = 1e18;
    SNode<D> worst{};
    for (std::size_t u : dom_idx) {
        const auto& nd = nodes[u];
        double t = subst.dot_v(nd.x);
        double vi = subst.v[static_cast<std::size_t>(nd.i)];
        double vj = subst.v[static_cast<std::size_t>(nd.j)];
        double lo = std::min(vi, vj);
        double margin = lo - std::abs(t);
        if (margin < min_margin) { min_margin = margin; worst = nd; }
    }
    double worst_lo = std::min(subst.v[static_cast<std::size_t>(worst.i)],
                                subst.v[static_cast<std::size_t>(worst.j)]);
    std::printf("n=%zu: |G_B|=%zu dominant-core size=%zu worst margin=%.6f (%.3f%% of threshold, "
                "at [i=%lld j=%lld])\n",
                n, gb.size(), dom_idx.size(), min_margin,
                100.0 * min_margin / worst_lo, worst.i, worst.j);
    std::printf("    x = (");
    for (std::size_t k = 0; k < D; ++k) std::printf("%s%lld", k ? "," : "", worst.x[k]);
    std::printf(")  t=<x,v>=%.10f  v[n-1]=%.10f  beta=%.10f\n",
                subst.dot_v(worst.x), subst.v[D - 1], beta);
}

}  // namespace

int main() {
    check_margin<3>(3, 1.8392867552141612);
    check_margin<4>(4, 1.9275619754829254);
    check_margin<5>(5, 1.9659482366454853);
    check_margin<6>(6, 1.9835828434243288);
    check_margin<7>(7, 1.9919641966050352);
    return 0;
}
