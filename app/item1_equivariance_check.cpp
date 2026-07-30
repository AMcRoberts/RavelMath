// item1_equivariance_check.cpp
//
// Computational verification of two claims made in the item-1 writeup
// (docs/DIRECTION_AND_OPEN_THREADS.md thread A); see docs/RESEARCH_STATUS.md
// for how these were found.
//
// CLAIM A (proven algebraically, n-bonacci-specific, all n): the
// TYPE-1-ONLY `simple_forward_targets` relation (corona.hpp) is
// EXACTLY phi-equivariant -- phi(simple_forward_targets(node)) ==
// simple_forward_targets(phi(node)), as validity-filtered candidate
// SETS -- because of n-bonacci's special decomposition structure:
// letter 0 has n parents, each with an EMPTY prefix; every letter
// c>=1 has a UNIQUE parent c-1, with prefix EXACTLY [0]. This is
// checked directly here by brute-force comparison, not re-derived
// from the formula.
//
// CLAIM B (identifies the likely mechanism behind the n=6 asymmetry):
// `c_corona`'s hop-composition rule applies `same_letter_H(y,j)` as a
// filter on which A_prev elements may act as a SOURCE (the (i1,y,j)
// role), but never checks it (or any mirror-analog) on elements used
// purely as a HOP (the (j,delta,i2) role). same_letter_H(y,j) and
// same_letter_H(-y,i) [what mirror(node)'s own source-role would need]
// are unrelated predicates in general -- so a node can freely act as
// a hop while its mirror fails to qualify as a source, breaking
// A_prev's mirror-closure inductively as rounds progress. Checked
// here by looking for actual (y,j) pairs, among nodes that occur in
// a real corona round, where same_letter_H(y,j) holds but
// same_letter_H(-y, <that node's i>) does not (or vice versa).

#include <cstdio>
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

// CLAIM A check, for a given alphabet size D and a batch of random-ish
// probe nodes: verify phi(simple_forward_targets(node)) ==
// simple_forward_targets(phi(node)) as SETS of (i,x,j) destination
// triples (dropping the parent-decomposition metadata, which isn't
// part of the node identity).
template <std::size_t D>
void check_claim_a(double beta, int& checked, int& mismatches) {
    auto sigma = n_bonacci_rule(D);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);

    for (long long i = 0; i < static_cast<long long>(D); ++i) {
        for (long long j = 0; j < static_cast<long long>(D); ++j) {
            for (long long x0 = -2; x0 <= 2; ++x0) {
                for (long long x1 = -2; x1 <= 2; ++x1) {
                    SNode<D> node{};
                    node.i = i; node.j = j;
                    node.x[0] = x0;
                    if (D > 1) node.x[1] = x1;
                    SNode<D> mir = node.mirror();

                    auto fwd_node = simple_forward_targets<D>(subst, node);
                    auto fwd_mir = simple_forward_targets<D>(subst, mir);

                    std::set<std::tuple<long long, std::array<long long, D>, long long>> phi_of_fwd_node;
                    for (const auto& [cand, pq] : fwd_node) {
                        (void)pq;
                        SNode<D> pc = cand.mirror();
                        phi_of_fwd_node.emplace(pc.i, pc.x, pc.j);
                    }
                    std::set<std::tuple<long long, std::array<long long, D>, long long>> fwd_mir_set;
                    for (const auto& [cand, pq] : fwd_mir) {
                        (void)pq;
                        fwd_mir_set.emplace(cand.i, cand.x, cand.j);
                    }
                    ++checked;
                    if (phi_of_fwd_node != fwd_mir_set) {
                        ++mismatches;
                        if (mismatches <= 5) {
                            std::printf("    MISMATCH (D=%zu) at node [i=%lld x=(%lld,%lld,...) j=%lld]: "
                                        "phi(fwd(node)) has %zu elts, fwd(phi(node)) has %zu elts\n",
                                        D, i, x0, (D > 1 ? x1 : 0), j,
                                        phi_of_fwd_node.size(), fwd_mir_set.size());
                        }
                    }
                }
            }
        }
    }
}

}  // namespace

int main() {
    std::printf("=== CLAIM A: simple_forward_targets is phi-equivariant for n-bonacci ===\n");
    {
        int checked = 0, mismatches = 0;
        check_claim_a<3>(1.8392867552141612, checked, mismatches);
        std::printf("  n=3 (Tribonacci): %d probe nodes checked, %d mismatches\n", checked, mismatches);
    }
    {
        int checked = 0, mismatches = 0;
        check_claim_a<4>(1.9275619754829254, checked, mismatches);
        std::printf("  n=4 (Tetrabonacci): %d probe nodes checked, %d mismatches\n", checked, mismatches);
    }
    {
        int checked = 0, mismatches = 0;
        check_claim_a<5>(1.9659482366454853, checked, mismatches);
        std::printf("  n=5 (Pentanacci): %d probe nodes checked, %d mismatches\n", checked, mismatches);
    }

    std::printf("\n=== CLAIM B: same_letter_H(y,j) vs same_letter_H(-y,i) for actual corona nodes ===\n");
    {
        // Build a real corona round for Tribonacci (small, fast) and
        // check, for every node [i,y,j] in it, whether
        // same_letter_H(y,j) [what c_corona needs to use it as a
        // SOURCE] agrees with same_letter_H(-y,i) [what its mirror
        // [j,-y,i] would need to use ITSELF as a source].
        constexpr std::size_t D = 3;
        auto sigma = n_bonacci_rule(D);
        SubstitutionRule rule(sigma);
        double beta = 1.8392867552141612;
        auto subst = make_substitution<D>(rule, beta);

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

        int checked = 0, source_asym = 0;
        int asym_survives_to_final = 0;
        std::set<SNode<D>> final_gb;
        auto check_round = [&](const std::set<SNode<D>>& A, const char* label) {
            int round_checked = 0, round_asym = 0;
            for (const auto& n : A) {
                bool self_ok = same_letter_H<D>(subst, n.x, static_cast<std::size_t>(n.j));
                std::array<long long, D> negx{};
                for (std::size_t k = 0; k < D; ++k) negx[k] = -n.x[k];
                bool mirror_source_ok = same_letter_H<D>(subst, negx, static_cast<std::size_t>(n.i));
                ++checked; ++round_checked;
                if (self_ok != mirror_source_ok) {
                    ++source_asym; ++round_asym;
                }
            }
            std::printf("    %s: %d nodes, %d with same_letter_H(y,j) != same_letter_H(-y,i)\n",
                        label, round_checked, round_asym);
        };
        check_round(pmC, "pmC (seed)");
        auto A_prev = pmC;
        int round_num = 2;
        for (int p = 2; p <= 6; ++p, ++round_num) {
            auto corona_nodes = c_corona<D>(subst, A_prev, A_prev);
            char label[32];
            std::snprintf(label, sizeof(label), "corona round %d (pre-Red)", round_num);
            check_round(corona_nodes, label);
            std::vector<std::tuple<SNode<D>, SNode<D>, std::vector<long long>, std::vector<long long>>> edges;
            for (const auto& n : corona_nodes) {
                auto fwd = simple_forward_targets<D>(subst, n);
                for (const auto& [dest, pq] : fwd) {
                    (void)pq;
                    if (corona_nodes.count(dest) > 0) edges.push_back({n, dest, {}, {}});
                }
            }
            auto red_result = red<D>(corona_nodes, edges);
            if (red_result.first == A_prev) { final_gb = red_result.first; break; }
            A_prev = red_result.first;
            final_gb = A_prev;
        }
        // Re-check: of the asymmetric (y,j) pairs found across all rounds,
        // how many correspond to nodes that survive into the FINAL fixed
        // point G_B?
        int final_asym = 0, final_checked = 0;
        std::vector<SNode<D>> asym_nodes;
        for (const auto& n : final_gb) {
            bool self_ok = same_letter_H<D>(subst, n.x, static_cast<std::size_t>(n.j));
            std::array<long long, D> negx{};
            for (std::size_t k = 0; k < D; ++k) negx[k] = -n.x[k];
            bool mirror_source_ok = same_letter_H<D>(subst, negx, static_cast<std::size_t>(n.i));
            ++final_checked;
            if (self_ok != mirror_source_ok) { ++final_asym; asym_nodes.push_back(n); }
        }
        (void)asym_survives_to_final;
        std::printf("  TOTAL across all rounds: %d node-instances checked, %d asymmetric\n",
                    checked, source_asym);
        std::printf("  FINAL fixed-point G_B (%d nodes): %d have the same_letter_H asymmetry\n",
                    final_checked, final_asym);

        // Is the residual asymmetric node TRANSIENT (not in the
        // recurrent core) -- which would fully explain why the
        // involution still comes out exact on Tribonacci's 14-node
        // recurrent core despite this real underlying asymmetry?
        if (!asym_nodes.empty()) {
            std::vector<std::vector<long long>> dense(final_gb.size(),
                                                        std::vector<long long>(final_gb.size(), 0));
            std::vector<SNode<D>> nodes(final_gb.begin(), final_gb.end());
            std::map<SNode<D>, std::size_t> idx;
            for (std::size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;
            for (std::size_t i = 0; i < nodes.size(); ++i) {
                auto fwd = simple_forward_targets<D>(subst, nodes[i]);
                for (const auto& [dest, pq] : fwd) {
                    (void)pq;
                    auto it = idx.find(dest);
                    if (it != idx.end()) dense[i][it->second] += 1;
                }
            }
            auto gb_graph = WeightedDigraph::from_dense(dense);
            auto [core, orig_idx] = extract_recurrent_core(gb_graph);
            (void)core;
            std::set<std::size_t> core_set(orig_idx.begin(), orig_idx.end());
            for (const auto& an : asym_nodes) {
                std::size_t node_idx = idx[an];
                bool in_core = core_set.count(node_idx) > 0;
                std::printf("    residual asymmetric node [i=%lld j=%lld] is %s\n",
                            an.i, an.j, in_core ? "IN the recurrent core (!)" : "TRANSIENT (not in recurrent core)");
            }
        }
    }

    return 0;
}
