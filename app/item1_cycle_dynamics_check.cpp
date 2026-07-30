// item1_cycle_dynamics_check.cpp
//
// Develops the "deep interior contacts are more robust" intuition
// into an actual dynamical argument. Since G_B's edges (the ones
// Red() prunes on, and that determine the recurrent SCC structure)
// come from `simple_forward_targets` (type-1 only), and v is the LEFT
// Perron eigenvector of M (v^T M = beta v^T), one forward step
// [i,x,j] -> [i',x',j'] satisfies M x' = x + l(q.prefix)
// - l(p.prefix), so
//
//   beta * t' = t + <l(q.prefix),v> - <l(p.prefix),v>,   t := <x,v>
//
// For n-bonacci, <l(prefix),v> is 0 (if the parent came from decomp(0),
// which has an EMPTY prefix) or v[0]=1 (if from decomp(c>=1), prefix
// exactly [0]). decomp(0) has n parents (i.e. i=0 forces
// <l(p.prefix),v>=0); decomp(c>=1) has a UNIQUE parent c-1 (prefix
// [0], so <l(p.prefix),v>=1). So writing delta = [j>=1]-[i>=1] in
// {-1,0,+1}:
//
//   t' = (t + delta) / beta                                   ... (A)
//
// This is a CONTRACTION (ratio 1/beta < 1) toward one of three fixed
// points: 0 (delta=0, i.e. i,j on the same side of zero), +1/(beta-1)
// (delta=+1, i=0 j>=1), -1/(beta-1) (delta=-1, i>=1 j=0). A node on an
// actual CYCLE of length L must have t equal to the UNIQUE fixed point
// of the length-L composed contraction along that specific cycle's
// delta-sequence -- this file verifies (A) directly, then traces real
// cycles in Hexanacci's dominant vs non-dominant recurrent SCCs to see
// which fixed points they hit, and whether that explains the observed
// "dominant SCC is always doubly-valid" pattern.
//
// STEP 3 prints the worst (min-margin) node for EVERY recurrent SCC,
// not just the dominant one -- this is what exposed the "composite of
// several elementary hops" mechanism for why non-dominant SCCs can
// violate the margin (see docs/RESEARCH_STATUS.md, item-1 investigation,
// and app/item1_conjecture_n7_check.cpp for the n=7 companion run).

#include <cmath>
#include <cstdio>
#include <map>
#include <numeric>
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
constexpr std::size_t D = 6;

std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}
}  // namespace

int main() {
    auto sigma = n_bonacci_rule(D);
    SubstitutionRule rule(sigma);
    double beta = 1.9835828434243288;
    auto subst = make_substitution<D>(rule, beta);

    // STEP 1: verify (A) directly against real simple_forward_targets edges.
    std::printf("=== STEP 1: verify t' = (t+delta)/beta for real forward edges ===\n");
    auto d_cont_cands = search_D_cont<D>(subst, 2);
    std::vector<ANode<D>> d_cont_an;
    for (const auto& c : d_cont_cands) {
        ANode<D> n; n.i = c.i; n.j = c.j; n.x = c.x;
        d_cont_an.push_back(n);
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
        for (const auto& n : corona_nodes) {
            auto fwd = simple_forward_targets<D>(subst, n);
            for (const auto& [dest, pq] : fwd) {
                (void)pq;
                if (corona_nodes.count(dest) > 0) edges.push_back({n, dest, {}, {}});
            }
        }
        auto red_result = red<D>(corona_nodes, edges);
        if (red_result.first == A_prev) { gb = red_result.first; break; }
        A_prev = red_result.first;
        gb = A_prev;
    }
    std::printf("|G_B| = %zu\n", gb.size());

    std::vector<SNode<D>> nodes(gb.begin(), gb.end());
    std::map<SNode<D>, std::size_t> idx;
    for (std::size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;

    // Build the actual edge list (simple_forward_targets restricted to gb),
    // and a "first successor" map (node -> one chosen destination) so we
    // can trace deterministic-ish forward paths per node (there may be
    // more than one edge; we store ALL of them but pick a representative
    // for path-tracing).
    std::vector<std::vector<std::size_t>> succ(nodes.size());
    {
        int checked = 0, mismatches = 0;
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            auto fwd = simple_forward_targets<D>(subst, nodes[i]);
            for (const auto& [dest, pq] : fwd) {
                (void)pq;
                auto it = idx.find(dest);
                if (it == idx.end()) continue;
                succ[i].push_back(it->second);
                // Verify (A): delta = [j>=1] - [i>=1], using the SOURCE
                // node's own i,j (the decomposition being of the
                // SOURCE's i and j, per simple_forward_targets).
                double t = subst.dot_v(nodes[i].x);
                double tp = subst.dot_v(dest.x);
                long long dd = (nodes[i].j != 0 ? 1 : 0) - (nodes[i].i != 0 ? 1 : 0);
                double predicted = (t + static_cast<double>(dd)) / beta;
                ++checked;
                if (std::abs(predicted - tp) > 1e-6) ++mismatches;
            }
        }
        std::printf("  %d forward edges checked, %d violate t'=(t+delta)/beta (expect 0)\n",
                    checked, mismatches);
    }

    // Build dense adjacency (for SCC extraction) and classify nodes.
    std::vector<std::vector<long long>> dense(nodes.size(), std::vector<long long>(nodes.size(), 0));
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        for (std::size_t d : succ[i]) dense[i][d] += 1;
    }
    auto gb_graph = WeightedDigraph::from_dense(dense);
    auto [dom_core, dom_idx] = extract_dominant_recurrent_core(gb_graph, 500);
    (void)dom_core;
    std::set<std::size_t> dom_set(dom_idx.begin(), dom_idx.end());
    auto sccs = tarjan_scc(gb_graph);

    // For each recurrent SCC, trace ONE cycle by repeatedly following
    // the FIRST successor from an arbitrary start node until a repeat
    // is found (guaranteed within |SCC| steps since every node has
    // out-degree >=1 within a recurrent SCC and the state space is
    // finite); record the delta-sequence around that cycle, compute
    // the closed-form fixed point, and compare to the actual observed
    // t value.
    std::printf("\n=== STEP 2: trace one cycle per recurrent SCC, check t == closed-form fixed point ===\n");
    int scc_num = 0;
    for (auto& comp : sccs) {
        if (!is_recurrent_scc(gb_graph, comp)) continue;
        ++scc_num;
        bool is_dominant = !comp.empty() && dom_set.count(comp[0]) > 0;

        // Trace: start at comp[0], follow succ[] (restricted to nodes
        // within this SCC) until we revisit a node.
        std::size_t start = comp[0];
        std::set<std::size_t> comp_set(comp.begin(), comp.end());
        std::vector<std::size_t> path;
        std::vector<long long> deltas;
        std::size_t cur = start;
        std::map<std::size_t, int> visited_at;
        while (!visited_at.count(cur)) {
            visited_at[cur] = static_cast<int>(path.size());
            path.push_back(cur);
            // pick the first successor that stays within this SCC.
            std::size_t next = SIZE_MAX;
            for (std::size_t s : succ[cur]) {
                if (comp_set.count(s)) { next = s; break; }
            }
            if (next == SIZE_MAX) break;  // shouldn't happen for a genuine recurrent SCC
            long long dd = (nodes[cur].j != 0 ? 1 : 0) - (nodes[cur].i != 0 ? 1 : 0);
            deltas.push_back(dd);
            cur = next;
        }
        if (!visited_at.count(cur)) {
            std::printf("  SCC #%d (size %zu, %s): failed to close a cycle (unexpected)\n",
                        scc_num, comp.size(), is_dominant ? "DOMINANT" : "non-dominant");
            continue;
        }
        int cycle_start_idx = visited_at[cur];
        std::vector<long long> cycle_deltas(deltas.begin() + cycle_start_idx, deltas.end());
        int L = static_cast<int>(cycle_deltas.size());

        // Closed-form fixed point: unrolling t_1=(t_0+d_0)/beta,
        // t_2=(t_1+d_1)/beta, ..., gives t_L = t_0/beta^L +
        // sum_{k=0}^{L-1} d_k/beta^(L-k); setting t_L=t_0 (cycle
        // closure) and solving for t_0 gives:
        double beta_L = std::pow(beta, static_cast<double>(L));
        double numer = 0.0, divisor = beta_L;
        for (int k = 0; k < L; ++k) {
            numer += static_cast<double>(cycle_deltas[k]) / divisor;
            divisor /= beta;
        }
        double denom = 1.0 - 1.0 / beta_L;
        double t_star = numer / denom;

        std::size_t cycle_node_idx = path[cycle_start_idx];
        double t_actual = subst.dot_v(nodes[cycle_node_idx].x);

        std::printf("  SCC #%d (size %zu, %s): cycle length %d, delta-sequence sum=%lld, "
                    "predicted t*=%.6f, actual t=%.6f, match=%s\n",
                    scc_num, comp.size(), is_dominant ? "DOMINANT" : "non-dominant", L,
                    std::accumulate(cycle_deltas.begin(), cycle_deltas.end(), 0LL),
                    t_star, t_actual, std::abs(t_star - t_actual) < 1e-6 ? "YES" : "NO");
    }

    // STEP 3: full |t| distribution across EVERY node in each recurrent
    // SCC (not just one traced cycle), and each SCC's OWN min(v_i,v_j)
    // safety margin, to see if the dominant SCC is systematically
    // smaller across the board or just on the one sampled cycle.
    std::printf("\n=== STEP 3: |t| distribution and safety margin, ALL nodes per SCC ===\n");
    scc_num = 0;
    for (auto& comp : sccs) {
        if (!is_recurrent_scc(gb_graph, comp)) continue;
        ++scc_num;
        bool is_dominant = !comp.empty() && dom_set.count(comp[0]) > 0;
        double min_abs_t = 1e18, max_abs_t = -1.0, min_margin = 1e18;
        std::size_t worst_node = 0;
        for (std::size_t u : comp) {
            double t = subst.dot_v(nodes[u].x);
            double vi = subst.v[static_cast<std::size_t>(nodes[u].i)];
            double vj = subst.v[static_cast<std::size_t>(nodes[u].j)];
            double lo = std::min(vi, vj);
            min_abs_t = std::min(min_abs_t, std::abs(t));
            max_abs_t = std::max(max_abs_t, std::abs(t));
            double margin = lo - std::abs(t);
            if (margin < min_margin) { min_margin = margin; worst_node = u; }
        }
        std::printf("  SCC #%d (size %4zu, %-11s): |t| range [%.6f, %.6f], "
                    "worst-case safety margin (min(vi,vj)-|t|) = %.6f%s\n",
                    scc_num, comp.size(), is_dominant ? "DOMINANT" : "non-dominant",
                    min_abs_t, max_abs_t, min_margin,
                    min_margin < 0 ? "  <-- NEGATIVE: in the danger band somewhere" : "");
        {
            // Print the worst node for EVERY recurrent SCC, not just the
            // dominant one: seeing a non-dominant SCC's actual violating
            // x-vector is what exposed the "composite of several
            // elementary hops" mechanism (docs/RESEARCH_STATUS.md) -- printing
            // only the dominant core's worst node (the original form of
            // this block) hid that entirely.
            const auto& wn = nodes[worst_node];
            double t = subst.dot_v(wn.x);
            std::printf("    worst node in this SCC: [i=%lld j=%lld] t=%.6f  v[i]=%.6f v[j]=%.6f  x=(",
                        wn.i, wn.j, t, subst.v[static_cast<std::size_t>(wn.i)],
                        subst.v[static_cast<std::size_t>(wn.j)]);
            for (std::size_t k = 0; k < D; ++k) std::printf("%s%lld", k ? "," : "", wn.x[k]);
            std::printf(")  (%.2f%% of threshold)\n", 100.0 * min_margin / std::min(subst.v[static_cast<std::size_t>(wn.i)],
                                                       subst.v[static_cast<std::size_t>(wn.j)]));
        }
    }

    return 0;
}
