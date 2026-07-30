// item1_conjecture_n6_check.cpp
//
// Tests the conjecture stated in thread A item (1b) of
// docs/DIRECTION_AND_OPEN_THREADS.md -- that Red()'s pruning confines
// the same_letter_H(y,j) vs same_letter_H(-y,i) asymmetry to TRANSIENT
// nodes, never the recurrent core, and that this holds not just for
// Tribonacci's single clean 14-node SCC but for Hexanacci's much
// harder structure (7 recurrent SCCs, the dominant one being the
// 210-node component -- NOT the largest by size; see
// docs/RESEARCH_STATUS.md for how that was found). If even one node of the
// 210-node DOMINANT core shows the asymmetry, the "confined to
// transients" framing of the conjecture is false and needs revision;
// if none do, that's real evidence for it surviving a much harder
// case than the one it was first observed on.

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
}  // namespace

int main() {
    std::vector<std::vector<std::int8_t>> sigma = {
        {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0}};
    SubstitutionRule rule(sigma);
    double beta = 1.9835828434243288;
    auto subst = make_substitution<D>(rule, beta);

    // Full pipeline (same as before) to get the final G_B.
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
    std::printf("Hexanacci: |G_B|=%zu\n", gb.size());

    // Build G_B's WeightedDigraph and get the CORRECTED dominant
    // recurrent core (extract_dominant_recurrent_core, not the
    // largest-by-size extract_recurrent_core).
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
    std::printf("Corrected dominant recurrent core: %zu nodes\n", dom_idx.size());

    // Check same_letter_H(y,j) vs same_letter_H(-y,i) for EVERY node
    // in the dominant core, and separately for every OTHER (transient
    // or non-dominant-SCC) node, for comparison.
    std::set<std::size_t> dom_set(dom_idx.begin(), dom_idx.end());
    int core_checked = 0, core_asym = 0;
    int other_checked = 0, other_asym = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const auto& n = nodes[i];
        bool self_ok = same_letter_H<D>(subst, n.x, static_cast<std::size_t>(n.j));
        std::array<long long, D> negx{};
        for (std::size_t k = 0; k < D; ++k) negx[k] = -n.x[k];
        bool mirror_source_ok = same_letter_H<D>(subst, negx, static_cast<std::size_t>(n.i));
        bool asym = (self_ok != mirror_source_ok);
        if (dom_set.count(i)) {
            ++core_checked;
            if (asym) ++core_asym;
        } else {
            ++other_checked;
            if (asym) ++other_asym;
        }
    }
    std::printf("\nDominant core (%d nodes): %d have the same_letter_H asymmetry"
                "%s\n", core_checked, core_asym,
                core_asym == 0 ? "  <-- CONJECTURE HOLDS (confined to transients)"
                               : "  <-- CONJECTURE FALSE for this case!");

    // Refine "everything else": separate TRANSIENT nodes from
    // NON-DOMINANT RECURRENT SCC nodes, to see whether the
    // asymmetry-free property is specific to the dominant SCC or
    // holds for "any recurrent SCC" more generally.
    auto sccs = tarjan_scc(gb_graph);
    std::set<std::size_t> any_recurrent_set;
    for (auto& c : sccs) {
        if (!is_recurrent_scc(gb_graph, c)) continue;
        for (auto u : c) any_recurrent_set.insert(u);
    }
    int transient_checked = 0, transient_asym = 0;
    int nondom_recurrent_checked = 0, nondom_recurrent_asym = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (dom_set.count(i)) continue;
        const auto& n = nodes[i];
        bool self_ok = same_letter_H<D>(subst, n.x, static_cast<std::size_t>(n.j));
        std::array<long long, D> negx{};
        for (std::size_t k = 0; k < D; ++k) negx[k] = -n.x[k];
        bool mirror_source_ok = same_letter_H<D>(subst, negx, static_cast<std::size_t>(n.i));
        bool asym = (self_ok != mirror_source_ok);
        if (any_recurrent_set.count(i)) {
            ++nondom_recurrent_checked;
            if (asym) ++nondom_recurrent_asym;
        } else {
            ++transient_checked;
            if (asym) ++transient_asym;
        }
    }
    std::printf("  Non-dominant recurrent SCCs (%d nodes): %d have the asymmetry%s\n",
                nondom_recurrent_checked, nondom_recurrent_asym,
                nondom_recurrent_asym == 0 ? "  <-- also asymmetry-free!" : "  <-- NOT asymmetry-free");
    std::printf("  Genuinely transient nodes (%d nodes): %d have the asymmetry\n",
                transient_checked, transient_asym);

    // Sharper hypothesis: does every dominant-core node satisfy the
    // PLAIN in_H_sigma(x,j) test directly (the first disjunct of
    // same_letter_H/is_valid_simple_node's OR), rather than needing
    // the OR's second disjunct as a fallback? If so, same_letter_H(x,j)
    // is trivially TRUE for every core node, and if the core is also
    // phi-symmetric (already established), mirror(node)'s own pair
    // is trivially TRUE too -- which would fully explain the zero
    // asymmetry above without needing to track same_letter_H(-y,i) as
    // a separate, unrelated computation at all.
    int plain_true = 0, plain_false_needs_or = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (!dom_set.count(i)) continue;
        const auto& n = nodes[i];
        bool plain = subst.in_H_sigma(n.x, static_cast<std::size_t>(n.j));
        if (plain) ++plain_true; else ++plain_false_needs_or;
    }
    std::printf("\nSharper check -- of the %d dominant-core nodes, how many satisfy "
                "PLAIN in_H_sigma(x,j) directly (vs needing the OR fallback)?\n", core_checked);
    std::printf("  plain in_H_sigma(x,j) TRUE:  %d\n", plain_true);
    std::printf("  needs the OR's 2nd disjunct: %d%s\n", plain_false_needs_or,
                plain_false_needs_or == 0 ? "  <-- SHARPER HYPOTHESIS HOLDS" : "  <-- hypothesis false, OR fallback genuinely used");

    return 0;
}
