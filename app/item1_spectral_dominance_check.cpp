// item1_spectral_dominance_check.cpp
//
// Investigates the SPECTRAL/GEOMETRIC mechanism behind why the
// DOMINANT recurrent SCC specifically (not just any recurrent SCC) is
// free of the same_letter_H(y,j) vs same_letter_H(-y,i) asymmetry
// (see docs/DIRECTION_AND_OPEN_THREADS.md thread A item 1b, and
// docs/RESEARCH_STATUS.md for how this was found).
//
// STEP 1 -- exact reformulation (algebra, checked computationally
// below, not just asserted): since in_H_sigma(x,j) = "0 <= <x,v> <
// v[j]", writing t = <x,v>:
//
//   same_letter_H(x,j) = in_H_sigma(x,j) OR in_H_sigma(-x,j)
//                      = (0<=t<v[j]) OR (-v[j]<t<=0)
//                      = |t| < v[j]                      ... (*)
//
// For a VALID node [i,x,j] (is_valid_simple_node true, i.e. t in the
// validity window (-v[i], v[j])), a case split on sign(t) shows the
// asymmetry same_letter_H(x,j) != same_letter_H(-x,i) [i.e. |t|<v[j]
// XOR |t|<v[i]] occurs EXACTLY when
//
//   |t|  in  [min(v[i],v[j]), max(v[i],v[j]))            ... (**)
//
// -- a "band" between the two eigenvector components at the node's
// own two face-indices. This file checks (*) and (**) directly
// against actual Substitution/G_B data (not just algebra), then asks:
// do DOMINANT-SCC nodes structurally avoid this band, and if so, why?

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

    std::printf("=== Hexanacci's eigenvector v (used throughout in_H_sigma) ===\n");
    for (std::size_t k = 0; k < D; ++k) std::printf("  v[%zu] = %.10f\n", k, subst.v[k]);
    bool monotone_decreasing = true;
    for (std::size_t k = 0; k + 1 < D; ++k) if (!(subst.v[k] > subst.v[k + 1])) monotone_decreasing = false;
    std::printf("  monotone decreasing (v[0] > v[1] > ... > v[n-1])? %s\n",
                monotone_decreasing ? "YES" : "no");

    // STEP 1: verify (*) same_letter_H(x,j) == (|<x,v>| < v[j]) directly.
    std::printf("\n=== STEP 1: verify same_letter_H(x,j) == (|<x,v>| < v[j]) ===\n");
    {
        int checked = 0, mismatches = 0;
        for (long long j = 0; j < static_cast<long long>(D); ++j) {
            for (long long x0 = -3; x0 <= 3; ++x0) {
                for (long long x1 = -3; x1 <= 3; ++x1) {
                    IVec<D> x{}; x[0] = x0; x[1] = x1;
                    bool lhs = same_letter_H<D>(subst, x, static_cast<std::size_t>(j));
                    double t = subst.dot_v(x);
                    bool rhs = std::abs(t) < subst.v[static_cast<std::size_t>(j)];
                    ++checked;
                    if (lhs != rhs) ++mismatches;
                }
            }
        }
        std::printf("  %d probe (x,j) pairs checked, %d mismatches (expect 0)\n", checked, mismatches);
    }

    // STEP 2: build the full G_B, get the dominant core, and check --
    // for EVERY node -- whether |<x,v>| falls in the "danger band"
    // [min(v_i,v_j), max(v_i,v_j)), and whether that predicts the
    // same_letter_H asymmetry exactly (confirming (**)), separately
    // for dominant-core, non-dominant-recurrent, and transient nodes.
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
    std::printf("\n|G_B| = %zu\n", gb.size());

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
    auto sccs = tarjan_scc(gb_graph);
    std::set<std::size_t> any_recurrent_set;
    for (auto& c : sccs) {
        if (!is_recurrent_scc(gb_graph, c)) continue;
        for (auto u : c) any_recurrent_set.insert(u);
    }

    // For each category, tabulate: |i-j| distribution, and whether
    // (**) (band membership) exactly predicts the observed asymmetry.
    auto analyze = [&](const char* label, const std::vector<std::size_t>& group) {
        std::map<long long, int> abs_ij_hist;
        int band_predicts_asym = 0, band_check_total = 0;
        int band_hits = 0;
        for (std::size_t gi : group) {
            const auto& n = nodes[gi];
            long long absij = std::llabs(n.i - n.j);
            abs_ij_hist[absij]++;
            double t = subst.dot_v(n.x);
            double vi = subst.v[static_cast<std::size_t>(n.i)];
            double vj = subst.v[static_cast<std::size_t>(n.j)];
            double lo = std::min(vi, vj), hi = std::max(vi, vj);
            bool in_band = (std::abs(t) >= lo && std::abs(t) < hi);
            bool self_ok = same_letter_H<D>(subst, n.x, static_cast<std::size_t>(n.j));
            std::array<long long, D> negx{};
            for (std::size_t k = 0; k < D; ++k) negx[k] = -n.x[k];
            bool mirror_source_ok = same_letter_H<D>(subst, negx, static_cast<std::size_t>(n.i));
            bool asym = (self_ok != mirror_source_ok);
            ++band_check_total;
            if (in_band == asym) ++band_predicts_asym;
            if (in_band) ++band_hits;
        }
        std::printf("  %s: %zu nodes, |i-j| histogram:", label, group.size());
        for (auto& [k, c] : abs_ij_hist) std::printf(" |i-j|=%lld:%d", k, c);
        std::printf("\n    band-membership predicts asymmetry exactly: %d/%d  "
                    "(nodes actually in the danger band: %d)\n",
                    band_predicts_asym, band_check_total, band_hits);
    };

    std::vector<std::size_t> dom_group(dom_idx.begin(), dom_idx.end());
    std::vector<std::size_t> nondom_recurrent_group, transient_group;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (dom_set.count(i)) continue;
        if (any_recurrent_set.count(i)) nondom_recurrent_group.push_back(i);
        else transient_group.push_back(i);
    }

    std::printf("\n=== STEP 2: |i-j| structure and band-membership, by category ===\n");
    analyze("Dominant core", dom_group);
    analyze("Non-dominant recurrent SCCs", nondom_recurrent_group);
    analyze("Transient", transient_group);

    // STEP 3: for a VALID node, validity (t in (-v_i,v_j)) already
    // forbids |t|>=max(v_i,v_j) -- so avoiding the band should reduce
    // EXACTLY to the single condition |t| < min(v_i,v_j) ("doubly
    // valid": deep inside BOTH thresholds, not just the OR-rescued
    // boundary case). Verify this reduction directly, and check the
    // mirror-pair sign-splitting story: t and <mirror.x,v>=-t have
    // opposite signs, so within a "doubly valid" (band-free) mirror
    // pair, EXACTLY one member has t>=0 (passes the PLAIN in_H_sigma
    // test) and the other has t<0 (relies on the OR's 2nd disjunct)
    // -- tying together the earlier 105/105 split finding and this
    // file's band characterization into one coherent picture.
    std::printf("\n=== STEP 3: band-avoidance == |t| < min(vi,vj); mirror-pair sign split ===\n");
    {
        int checked = 0, reduction_mismatches = 0;
        int dom_plain_true = 0, dom_plain_false = 0;
        for (std::size_t gi : dom_group) {
            const auto& n = nodes[gi];
            double t = subst.dot_v(n.x);
            double vi = subst.v[static_cast<std::size_t>(n.i)];
            double vj = subst.v[static_cast<std::size_t>(n.j)];
            double lo = std::min(vi, vj), hi = std::max(vi, vj);
            bool in_band = (std::abs(t) >= lo && std::abs(t) < hi);
            bool doubly_valid = std::abs(t) < lo;
            ++checked;
            if (in_band == doubly_valid) ++reduction_mismatches;  // should NEVER both be true
            bool plain = subst.in_H_sigma(n.x, static_cast<std::size_t>(n.j));
            if (plain) ++dom_plain_true; else ++dom_plain_false;
        }
        std::printf("  reduction check (in_band and doubly_valid should be MUTUALLY EXCLUSIVE "
                    "and jointly exhaustive for valid nodes): %d violations out of %zu\n",
                    reduction_mismatches, dom_group.size());
        std::printf("  dominant core: plain in_H_sigma(x,j) true for %d, false (needs OR) for %d "
                    "(t>=0 vs t<0 split within band-free/doubly-valid nodes)\n",
                    dom_plain_true, dom_plain_false);
    }

    return 0;
}
