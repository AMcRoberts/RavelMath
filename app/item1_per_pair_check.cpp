// item1_per_pair_check.cpp
//
// Tests the REDUCTION step of item 1's global-worst-margin claim
// (docs/DIRECTION_AND_OPEN_THREADS.md thread A item (1); see
// docs/RESEARCH_STATUS.md for how this was found). Groups every node of
// G_B's dominant recurrent core by its (i,j) face pair and finds each
// pair's actual max |t| (t = <x,v>), instead of just the single global
// worst node that item1_margin_trend_check.cpp reports.
//
// Two closed forms, found by inspecting the argmax x-vector per row:
//   - row i=0, column j in [1,n-2]: argmax x = -e_0 + e_j - e_{j+1},
//     giving margin(0,j) = (2-beta)*(1-beta^(j-n))/(beta-1), which is
//     an ALGEBRAIC IDENTITY (checked below): strictly decreasing in j.
//   - row i>=1 (any j>i): the SAME vector x = -e_i + e_{n-1} recurs
//     across the WHOLE row (not just at j=n-1), because
//     is_valid_simple_node's OR clause "[-x,i] in H_sigma" doesn't
//     depend on j at all -- once that disjunct holds for one j it
//     holds for all of them. margin(i,j) = v_j - (v_i - v_{n-1}),
//     also strictly decreasing in j.
// Chaining both: each row's own worst is at j=n-1, giving
// margin(i,n-1) = 2*v_{n-1} - v_i (row i>=1) or 2/beta-1 (row 0,
// i.e. the i=0 case of the same formula since v_0=1) -- and since
// v_i <= v_0 = 1 for every i, this is minimized at i=0. So IF these
// two closed forms really are each row's maximum (verified here,
// exactly, for n=3..6; not derived from the corona construction for
// general n), the global worst node is provably (i,j)=(0,n-1) /
// (n-1,0), matching item1_margin_trend_check.cpp's empirical finding.
//
// This driver is the verification: it prints every (i,j) pair's
// actual max |t| next to the closed-form prediction so a mismatch
// (i.e. a case where the reduction's premise fails) would be visible
// immediately.

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
void check(std::size_t n, double beta) {
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

    std::map<std::pair<long long, long long>, std::vector<std::size_t>> groups;
    for (std::size_t u : dom_idx) groups[{nodes[u].i, nodes[u].j}].push_back(u);

    std::printf("=== n=%zu: %zu (i,j) pairs in dominant core (size %zu) ===\n",
                n, groups.size(), dom_idx.size());
    double worst_margin = 1e18;
    int mismatches = 0;
    for (auto& [ij, us] : groups) {
        auto [i, j] = ij;
        if (i == j) continue;  // diagonal identity-type nodes: not part of the claim
        double vi = subst.v[static_cast<std::size_t>(i)];
        double vj = subst.v[static_cast<std::size_t>(j)];
        double m = std::min(vi, vj);
        double max_abs_t = -1.0;
        for (std::size_t u : us) max_abs_t = std::max(max_abs_t, std::abs(subst.dot_v(nodes[u].x)));

        // Closed-form prediction, using i<j WLOG (mirror pairs share margin).
        long long lo = std::min(i, j), hi = std::max(i, j);
        double predicted;
        if (lo == 0 && hi == static_cast<long long>(n) - 1) {
            predicted = 1.0 - 1.0 / beta;  // shared boundary of both formulas
        } else if (lo == 0) {
            predicted = 1.0 - std::pow(beta, static_cast<double>(hi) - static_cast<double>(n));
        } else {
            double v_lo = subst.v[static_cast<std::size_t>(lo)];
            double v_last = subst.v[D - 1];
            predicted = v_lo - v_last;
        }
        bool match = std::fabs(max_abs_t - predicted) < 1e-6;
        if (!match) ++mismatches;
        double margin = m - max_abs_t;
        worst_margin = std::min(worst_margin, margin);
        std::printf("  (i=%2lld,j=%2lld): max|t|=%.6f  predicted=%.6f  %-3s  margin=%.6f\n",
                    i, j, max_abs_t, predicted, match ? "ok" : "MISMATCH", margin);
    }
    double closed_form_worst = 2.0 / beta - 1.0;
    std::printf("  worst margin overall: %.6f  (closed form 2/beta-1 = %.6f)  %d row-formula mismatches\n\n",
                worst_margin, closed_form_worst, mismatches);
}

}  // namespace

int main() {
    check<3>(3, 1.8392867552141612);
    check<4>(4, 1.9275619754829254);
    check<5>(5, 1.9659482366454853);
    check<6>(6, 1.9835828434243288);
    check<7>(7, 1.9919641966050352);
    return 0;
}
