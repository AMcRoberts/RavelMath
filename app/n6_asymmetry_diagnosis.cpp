// n6_asymmetry_diagnosis.cpp
//
// Investigates whether `corona.hpp::simple_forward_targets` (which is
// explicitly TYPE-1 ONLY, per its own comment, whereas the full
// ambient graph's `forward_edges` in ambient_graph.hpp combines type-1
// AND type-2, and type-2 edges out of a node correspond exactly to
// type-1 edges out of phi(node) -- see docs/RESEARCH_STATUS.md) can produce
// a genuine out-degree asymmetry under phi within a corona round: if
// Algorithm 2's Red() step (which prunes a corona candidate whenever
// it has ZERO outgoing edges under simple_forward_targets
// specifically) is asymmetric under phi as a result (out-degree(node)
// != out-degree(phi(node)) in the type-1-only graph), that's a real
// mechanism by which phi could fail to be a graph automorphism of the
// CONSTRUCTED G_B, even though `is_valid_simple_node` itself (the
// node-level validity test) is provably phi-symmetric (it's an OR
// over {node, mirror(node)}, so both are valid or neither is --
// checked below too, as a sanity control). Confirmed: this asymmetry
// is real (see docs/RESEARCH_STATUS.md for the numbers) -- though the
// specific "15 unmatched nodes at n=6" puzzle that motivated this
// file turned out to have a different, simpler cause (a wrong-SCC bug
// in `extract_recurrent_core`, fixed via `extract_dominant_recurrent_core`
// in graph_divisor.hpp); this file's own finding (the type-1-only
// relation's genuine phi-asymmetry) remained a real, separately-used
// piece of the broader item-1 investigation.
//
// This file replicates compute_contact_boundary<d>'s pipeline by hand
// (same public functions: search_D_cont, backward_closure, red_anode,
// build_signed_contact_set, c_corona, simple_forward_targets, red)
// so it can inspect the PRE-Red() corona_nodes set and the edge list
// at each round -- neither is exposed by ContactBoundaryReport.

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

constexpr std::size_t D = 6;

SNode<D> mirror_of(const SNode<D>& n) { return n.mirror(); }

}  // namespace

int main() {
    // Hexanacci: sigma(i) = [0, i+1] for i<5, sigma(5) = [0].
    std::vector<std::vector<std::int8_t>> sigma = {
        {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0}};
    SubstitutionRule rule(sigma);
    double beta = 1.9835828434243288;  // n=6 n-bonacci constant
    auto subst = make_substitution<D>(rule, beta);

    // ---- Sanity control: is_valid_simple_node IS provably phi- ----
    // ---- symmetric (both-or-neither), for ANY candidate node.   ----
    {
        int checked = 0, asym = 0;
        for (long long i = 0; i < static_cast<long long>(D); ++i) {
            for (long long j = 0; j < static_cast<long long>(D); ++j) {
                for (long long x0 = -2; x0 <= 2; ++x0) {
                    SNode<D> n{};
                    n.i = i; n.j = j;
                    n.x = {x0, 0, 0, 0, 0, 0};
                    bool v1 = is_valid_simple_node<D>(subst, n);
                    bool v2 = is_valid_simple_node<D>(subst, mirror_of(n));
                    ++checked;
                    if (v1 != v2) ++asym;
                }
            }
        }
        std::printf("sanity control: is_valid_simple_node symmetry over %d random-ish "
                    "(i,x,j) triples: %d asymmetric (expect 0)\n\n", checked, asym);
    }

    // ---- Replicate the pipeline by hand, keeping PRE-Red() state. ----
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
    auto& c_nodes_an = red_gp.first;

    std::set<SNode<D>> C;
    for (const auto& a : c_nodes_an) {
        SNode<D> s; s.i = a.i; s.j = a.j; s.x = a.x;
        C.insert(s);
    }
    auto pmC = build_signed_contact_set<D>(C);

    std::printf("|D_cont|=%zu |G_P|=%zu |C|=%zu |pmC|=%zu\n",
                d_cont_an.size(), gp_nodes.size(), C.size(), pmC.size());

    auto A_prev = pmC;
    std::set<SNode<D>> gb;
    std::set<SNode<D>> last_corona_nodes;  // PRE-Red() state at the final round
    std::vector<std::pair<SNode<D>, SNode<D>>> last_edges_simple;
    constexpr int MAX_ROUNDS = 8;
    constexpr std::size_t MAX_A_SIZE = 50000;
    bool converged = false;
    for (int p = 2; p <= MAX_ROUNDS; ++p) {
        auto corona_nodes = c_corona<D>(subst, A_prev, A_prev);
        if (corona_nodes.size() > MAX_A_SIZE) break;
        std::vector<std::pair<SNode<D>, SNode<D>>> edges_simple;
        for (const auto& n : corona_nodes) {
            auto fwd = simple_forward_targets<D>(subst, n);
            for (const auto& [dest, pq] : fwd) {
                (void)pq;
                if (corona_nodes.count(dest) > 0) edges_simple.push_back({n, dest});
            }
        }
        last_corona_nodes = corona_nodes;
        last_edges_simple = edges_simple;
        std::vector<std::tuple<SNode<D>, SNode<D>, std::vector<long long>,
                               std::vector<long long>>> edges;
        for (const auto& [n, dest] : edges_simple) edges.push_back({n, dest, {}, {}});
        auto red_result = red<D>(corona_nodes, edges);
        auto& A_cur = red_result.first;
        if (A_cur == A_prev) { gb = A_cur; converged = true; break; }
        A_prev = A_cur;
    }
    std::printf("converged=%s |G_B|=%zu |last pre-Red corona_nodes|=%zu\n\n",
                converged ? "yes" : "NO", gb.size(), last_corona_nodes.size());

    // ---- Out-degree symmetry check on the FINAL pre-Red() corona ----
    // ---- round: for every node whose mirror is ALSO in that same  ----
    // ---- candidate set, compare out-degree(node) vs out-degree    ----
    // ---- (mirror(node)) computed from simple_forward_targets      ----
    // ---- restricted to that candidate set (exactly Red()'s own    ----
    // ---- pruning criterion).                                      ----
    std::map<SNode<D>, int> outdeg;
    for (const auto& [n, dest] : last_edges_simple) { (void)dest; outdeg[n]++; }

    int both_present = 0, symmetric = 0, asymmetric = 0, only_one_present = 0;
    int shown = 0;
    for (const auto& n : last_corona_nodes) {
        SNode<D> m = mirror_of(n);
        bool m_present = last_corona_nodes.count(m) > 0;
        if (!m_present) { ++only_one_present; continue; }
        bool self_mirror = !(n < m) && !(m < n);  // n == m
        if (!self_mirror && m < n) continue;      // only process the n < m half of each pair
        ++both_present;
        int deg_n = outdeg.count(n) ? outdeg[n] : 0;
        int deg_m = outdeg.count(m) ? outdeg[m] : 0;
        if (deg_n == deg_m) {
            ++symmetric;
        } else {
            ++asymmetric;
            if (shown < 20) {
                std::printf("  ASYMMETRIC: [i=%lld x=(", n.i);
                for (std::size_t k = 0; k < D; ++k) std::printf("%s%lld", k ? "," : "", n.x[k]);
                std::printf(") j=%lld] out-degree=%d  vs mirror out-degree=%d\n",
                            n.j, deg_n, deg_m);
                ++shown;
            }
        }
    }
    std::printf("\npre-Red() out-degree symmetry (simple_forward_targets, TYPE-1 ONLY),\n"
                "over %zu candidate nodes in the final round before Red() pruning:\n",
                last_corona_nodes.size());
    std::printf("  pairs with mirror also present in this round: %d (checked once per unordered pair)\n",
                both_present);
    std::printf("  symmetric out-degree: %d\n", symmetric);
    std::printf("  ASYMMETRIC out-degree: %d\n", asymmetric);
    std::printf("  nodes whose mirror is NOT even present in this round's candidate set: %d\n",
                only_one_present);

    return 0;
}
