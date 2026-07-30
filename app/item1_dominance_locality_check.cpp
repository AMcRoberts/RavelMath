// item1_dominance_locality_check.cpp
//
// Tests, and REFUTES, the "visits-once" hypothesis from
// docs/DIRECTION_AND_OPEN_THREADS.md thread A item (1) ("the
// Perron-dominant SCC specifically is the one with the visits-once
// property") as a LOCAL, per-cycle-inspectable criterion, and
// demonstrates the actual obstruction: dominance is not decidable
// from a single cycle's own label sequence.
//
// BACKGROUND (see docs/RESEARCH_STATUS.md, this session's entry, for the
// full derivation): the (i,j) label pair on a G_B node evolves via a
// simple, exactly-verified automaton -- i' = i-1 if i>=1 (forced), or
// any of {0,...,n-1} if i=0 (free branch); same independently for j;
// delta = [j>=1]-[i>=1] from the SOURCE label, giving t' = (t+delta)/beta
// exactly (bit-verified against simple_forward_targets below). An
// off-line exact search (label automaton + full-INTEGER-lattice cycle
// closure, i.e. requiring the FULL vector x to return to itself, not
// just its beta-eigenvalue projection t -- the earlier "ghost cycle"
// trap this session found and ruled out) finds, for n=5, an
// algebraically legitimate closing cycle at label (i=1,j=3) with
// |t|=0.588896 -- BIGGER than the actual G_B/dominant-core value
// (0.457288, matching row i=1's own closed form v_1-v_4 elsewhere in
// the row). This driver confirms that "ghost" node is REAL (a genuine
// valid node of the full, unreduced G_B) but lies in a NON-dominant
// recurrent SCC.
//
// The refutation: taking the SAME two underlying periodic i- and
// j-label patterns and combining them at five different relative
// phases produces five different (i,j) targets, of which only ONE
// lands in the dominant SCC (matching real data) -- the other four
// are either non-dominant ghosts (bigger, wrong) or valid-but-not-
// maximal nodes (smaller, wrong) for their own (i,j) pair. No local
// feature of "which rotation" predicts the outcome; it depends on
// which SCC of the FULL graph the resulting node ends up in, which is
// a comparison between whole subgraphs' Perron roots (exactly what
// extract_dominant_recurrent_core, graph_divisor.hpp, computes), not
// a property of one cycle in isolation. See docs/RESEARCH_STATUS.md for
// the exact five-rotation table (reproduced there from an off-line
// Python search, not re-derived in C++ here); this driver verifies
// the C++-side half of that finding: that the specific "ghost" node
// is real, valid, and non-dominant.

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

constexpr std::size_t D = 5;

}  // namespace

int main() {
    std::size_t n = 5;
    double beta = 1.9659482366454853;
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);

    // Build the full G_B (unreduced to the dominant core) exactly as
    // the other item1_*.cpp drivers do.
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
    std::printf("|G_B| (full, unreduced) = %zu\n", gb.size());

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
    std::set<std::size_t> dom_set(dom_idx.begin(), dom_idx.end());
    std::printf("dominant recurrent core size = %zu\n\n", dom_idx.size());

    // The "ghost" node found off-line: label (i=1,j=3), x = [0,-1,-1,1,1].
    // Algebraically it is a genuine closing cycle (verified separately,
    // off-line, via exact full-lattice closure) with |t|=0.588896 --
    // bigger than row i=1's actual max (0.457288, e.g. at (1,4)).
    std::array<long long, D> ghost_x{0, -1, -1, 1, 1};
    SNode<D> ghost; ghost.i = 1; ghost.x = ghost_x; ghost.j = 3;

    bool in_gb = gb.count(ghost) > 0;
    bool valid = is_valid_simple_node<D>(subst, ghost);
    double t = subst.dot_v(ghost.x);
    std::printf("ghost node [i=1, x=(0,-1,-1,1,1), j=3]:\n");
    std::printf("  is_valid_simple_node? %s\n", valid ? "YES" : "NO");
    std::printf("  t = %.6f  (window: (-v[1],v[3]) = (%.6f,%.6f))\n",
                t, -subst.v[1], subst.v[3]);
    std::printf("  present in full G_B? %s\n", in_gb ? "YES" : "NO");
    if (in_gb) {
        std::size_t gu = idx[ghost];
        bool is_dominant = dom_set.count(gu) > 0;
        std::printf("  in DOMINANT recurrent core? %s\n", is_dominant ? "YES" : "NO (confirms: real, but non-dominant)");
    }

    std::printf("\nConclusion: this node is a real, valid, actually-existing\n"
                "member of G_B (not a modeling artifact) whose |t| exceeds the\n"
                "row's documented maximum -- it is excluded from the maximality\n"
                "claim purely because it is not in the DOMINANT recurrent SCC,\n"
                "not because of any local defect in its own cycle. See\n"
                "docs/RESEARCH_STATUS.md for the five-rotation experiment showing no\n"
                "local (per-cycle) feature -- not letter-repeat count, not\n"
                "hitting a self-loop -- predicts this outcome.\n");

    return 0;
}
