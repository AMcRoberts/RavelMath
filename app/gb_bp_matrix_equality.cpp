// gb_bp_matrix_equality.cpp
//
// Item (1), thread A: attacking the matrix-equality claim directly,
// instead of the node-level bijection (which docs/RESEARCH_STATUS.md's "The
// G_B cross-check..." entries show cannot exist in the naive sense --
// BP-core states at different ruler-sequence depths share identical
// boundary letters and provably cannot be told apart by any
// fixed-window combinatorial feature, yet G_B's dominant core doesn't
// have enough distinct nodes at each matching letter-pair to hold them
// separately).
//
// What IS already established (gb_bp_twisted_quotient_check.cpp,
// checked n=3,4,5): Q_sym_BP and Q_sym's genuine factor have IDENTICAL
// characteristic polynomials (same for Q_anti). That is an orbit-level,
// spectral claim -- it does not require any node-level correspondence
// at all. This driver prints the RAW matrices (not just their
// characteristic polynomials) to look for a direct algebraic relation
// that would prove this in general, rather than checking case by case.
//
// Checked n=3, 4, 5 (extended from the original n=3,4 pass): the
// divisibility charpoly(Q_sym_GB) = charpoly(Q_sym_BP) * x^k holds
// with EXACTLY ZERO remainder every time -- N=7,23,54 splitting as
// (4+3), (10+13), (20+34). Also reports p(0) := charpoly(Q_sym_BP)(0),
// which (Q_sym_BP has even dimension in every case checked) equals
// det(Q_sym_BP) directly: p(0) != 0 is exactly what makes the x^k
// split above an automatic primary-decomposition consequence rather
// than a coincidence to re-check per n. p(0) = -1, 1, 1 for n=3,4,5
// (Q_anti_BP: 1, 1, 1). See app/bp_full_core_det.cpp for the
// follow-up attacking WHY p(0) != 0 directly, and
// docs/RESEARCH_STATUS.md ("Attacking det(Q_sym_BP) directly") for the
// full story.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/gb_bp_matrix_equality.cpp math/out/libmath.a -o gb_bp_matrix_equality

#include <cstdio>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "math/charpoly.hpp"
#include "math/poly_z.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
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
    for (std::size_t c = 0; c + 1 < n; ++c) { M[0][c] += 1; M[c + 1][c] += 1; }
    M[0][n - 1] += 1;
    return classify_matrix_spectral(M).beta;
}

void print_matrix(const char* label, const std::vector<std::vector<long long>>& M) {
    std::printf("  %s (%zux%zu):\n", label, M.size(), M.empty() ? 0 : M[0].size());
    for (const auto& row : M) {
        std::printf("    [");
        for (auto v : row) std::printf("%3lld ", v);
        std::printf("]\n");
    }
}

// ---- G_B side: phi-decomposition, exposing the raw matrices + which
// SNode each orbit representative actually is (for inspection). -------

template <std::size_t D>
struct GbQuotient {
    std::vector<std::vector<long long>> Qsym, Qanti;
    std::vector<SNode<D>> rep_nodes;  // the SNode each orbit's representative is
};

template <std::size_t D>
GbQuotient<D> gb_quotient_matrices(std::size_t n, double beta) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);

    auto d_cont_cands = search_D_cont<D>(subst, 2);
    std::vector<ANode<D>> d_cont_an;
    for (const auto& c : d_cont_cands) { ANode<D> nd; nd.i = c.i; nd.j = c.j; nd.x = c.x; d_cont_an.push_back(nd); }
    auto gp_nodes = backward_closure<D>(subst, d_cont_an, kContactBoundaryMaxNodesDefault);
    auto gp_edges = induced_restricted_edges<D>(subst, gp_nodes);
    std::set<ANode<D>> gp_set(gp_nodes.begin(), gp_nodes.end());
    auto red_gp = red_anode<D>(gp_set, gp_edges);
    std::set<SNode<D>> C;
    for (const auto& a : red_gp.first) { SNode<D> s; s.i = a.i; s.j = a.j; s.x = a.x; C.insert(s); }
    auto pmC = build_signed_contact_set<D>(C);
    auto A_prev = pmC;
    std::set<SNode<D>> gb;
    for (int p = 2; p <= 8; ++p) {
        auto corona_nodes = c_corona<D>(subst, A_prev, A_prev);
        if (corona_nodes.size() > 50000) break;
        std::vector<std::tuple<SNode<D>, SNode<D>, std::vector<long long>, std::vector<long long>>> edges;
        for (const auto& nn : corona_nodes) {
            auto fwd = simple_forward_targets<D>(subst, nn);
            for (const auto& [dest, pq] : fwd) { (void)pq; if (corona_nodes.count(dest) > 0) edges.push_back({nn, dest, {}, {}}); }
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
        for (const auto& [dest, pq] : fwd) { (void)pq; auto it = idx.find(dest); if (it != idx.end()) dense[i][it->second] += 1; }
    }
    auto gb_graph = WeightedDigraph::from_dense(dense);
    auto [dom_core, dom_idx] = extract_dominant_recurrent_core(gb_graph, 500);
    (void)dom_core;

    std::set<std::size_t> dom_set(dom_idx.begin(), dom_idx.end());
    std::vector<std::size_t> rep;
    std::map<std::size_t, std::size_t> orbit_of;
    for (auto u : dom_idx) {
        if (orbit_of.count(u)) continue;
        SNode<D> mirror = nodes[u].mirror();
        auto it = idx.find(mirror);
        if (it == idx.end() || !dom_set.count(it->second)) continue;
        std::size_t w = it->second;
        if (w == u) continue;
        std::size_t oid = rep.size();
        rep.push_back(u);
        orbit_of[u] = oid;
        orbit_of[w] = oid;
    }

    std::size_t N = rep.size();
    GbQuotient<D> result;
    result.Qsym.assign(N, std::vector<long long>(N, 0));
    result.Qanti.assign(N, std::vector<long long>(N, 0));
    for (auto u : rep) result.rep_nodes.push_back(nodes[u]);
    for (std::size_t a = 0; a < N; ++a) {
        std::size_t ra = rep[a];
        auto fwd = simple_forward_targets<D>(subst, nodes[ra]);
        for (const auto& [dest, pq] : fwd) {
            (void)pq;
            auto it = idx.find(dest);
            if (it == idx.end() || !dom_set.count(it->second)) continue;
            std::size_t w = it->second;
            if (!orbit_of.count(w)) continue;
            std::size_t b = orbit_of[w];
            if (w == rep[b]) { result.Qsym[a][b] += 1; result.Qanti[a][b] += 1; }
            else { result.Qsym[a][b] += 1; result.Qanti[a][b] -= 1; }
        }
    }
    return result;
}

// ---- BP-core side: swap-decomposition --------------------------------

std::string swap_key(const std::string& k) {
    std::vector<std::int8_t> u, v;
    unpair_key(k, u, v);
    return pair_key(v, u);
}

struct BpQuotient {
    std::vector<std::vector<long long>> Qsym, Qanti;
    std::vector<std::string> rep_keys;
};

BpQuotient bp_quotient_matrices(std::size_t n) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    const std::size_t nl = rule.alphabet_size();

    std::map<std::string, std::size_t> seen;
    std::map<std::string, std::vector<std::size_t>> succ;
    std::deque<std::size_t> queue;
    std::vector<std::string> order;
    {
        auto initial = reduce_pair({0, 1}, {1, 0}, nl);
        for (auto& ch : initial) {
            std::string k = pair_key(ch.first, ch.second);
            if (!seen.count(k)) { seen[k] = order.size(); order.push_back(k); queue.push_back(order.size() - 1); }
        }
    }
    constexpr std::size_t max_pairs = 20000, max_len = 60000;
    while (!queue.empty()) {
        std::size_t idx = queue.front(); queue.pop_front();
        std::string k = order[idx];
        std::vector<std::int8_t> u, v;
        unpair_key(k, u, v);
        auto img = sigma_pair(rule, {u, v});
        if (img.first.size() > max_len || seen.size() > max_pairs) return {};
        auto chunks = reduce_pair(img.first, img.second, nl);
        std::vector<std::size_t> children;
        for (auto& ch : chunks) {
            std::string ck = pair_key(ch.first, ch.second);
            auto it = seen.find(ck);
            std::size_t ci;
            if (it == seen.end()) { ci = order.size(); seen[ck] = ci; order.push_back(ck); queue.push_back(ci); }
            else ci = it->second;
            children.push_back(ci);
        }
        succ[k] = std::move(children);
    }

    std::map<std::size_t, std::size_t> idx_map;
    std::vector<std::size_t> noncoin;
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::vector<std::int8_t> u, v;
        unpair_key(order[i], u, v);
        if (!(u.size() == 1 && u == v)) { idx_map[i] = noncoin.size(); noncoin.push_back(i); }
    }
    std::size_t nn = noncoin.size();
    std::vector<std::vector<long long>> full_dense(nn, std::vector<long long>(nn, 0));
    for (std::size_t i : noncoin) {
        auto it = succ.find(order[i]);
        if (it == succ.end()) continue;
        for (std::size_t cj : it->second) { auto jt = idx_map.find(cj); if (jt != idx_map.end()) full_dense[idx_map[i]][jt->second] += 1; }
    }
    auto full_graph = WeightedDigraph::from_dense(full_dense);
    auto [core, orig_idx] = extract_recurrent_core(full_graph);

    std::vector<std::string> core_key(core.n);
    for (std::size_t c = 0; c < core.n; ++c) core_key[c] = order[noncoin[orig_idx[c]]];
    std::map<std::string, std::size_t> key_to_core;
    for (std::size_t c = 0; c < core.n; ++c) key_to_core[core_key[c]] = c;

    std::vector<std::size_t> rep;
    std::map<std::size_t, std::size_t> orbit_of;
    for (std::size_t c = 0; c < core.n; ++c) {
        if (orbit_of.count(c)) continue;
        auto it = key_to_core.find(swap_key(core_key[c]));
        if (it == key_to_core.end()) continue;
        std::size_t w = it->second;
        if (w == c) continue;
        std::size_t oid = rep.size();
        rep.push_back(c);
        orbit_of[c] = oid;
        orbit_of[w] = oid;
    }

    std::size_t N = rep.size();
    BpQuotient result;
    result.Qsym.assign(N, std::vector<long long>(N, 0));
    result.Qanti.assign(N, std::vector<long long>(N, 0));
    for (auto c : rep) result.rep_keys.push_back(core_key[c]);
    for (std::size_t a = 0; a < N; ++a) {
        std::size_t ra = rep[a];
        for (std::size_t w = 0; w < core.n; ++w) {
            long long wgt = 0;
            for (auto& e : core.out_adj[ra]) if (e.first == w) wgt += e.second;
            if (wgt == 0 || !orbit_of.count(w)) continue;
            std::size_t b = orbit_of[w];
            if (w == rep[b]) { result.Qsym[a][b] += wgt; result.Qanti[a][b] += wgt; }
            else { result.Qsym[a][b] += wgt; result.Qanti[a][b] -= wgt; }
        }
    }
    return result;
}

void print_charpoly(const char* label, const std::vector<std::vector<long long>>& M) {
    auto cp = mathlib::charpoly_faddeev_leverrier(M);
    std::printf("  charpoly(%s) [degree %zu]: ", label, cp.degree());
    for (long long k = static_cast<long long>(cp.degree()); k >= 0; --k) {
        std::printf("%s", mathlib::str(cp.coeff(static_cast<std::size_t>(k))).c_str());
        if (k > 0) std::printf("*x^%lld + ", k);
    }
    std::printf("\n");
}

}  // namespace

int main() {
    for (std::size_t n : {3u, 4u, 5u}) {
        double beta = compute_beta(n);
        std::printf("=== n=%zu ===\n", n);

        if (n == 3) {
            auto gbq = gb_quotient_matrices<3>(n, beta);
            print_matrix("G_B Q_sym", gbq.Qsym);
            print_matrix("G_B Q_anti", gbq.Qanti);
            print_charpoly("G_B Q_sym", gbq.Qsym);
            print_charpoly("G_B Q_anti", gbq.Qanti);
            std::printf("  G_B orbit representative nodes:\n");
            for (std::size_t k = 0; k < gbq.rep_nodes.size(); ++k) {
                auto& nd = gbq.rep_nodes[k];
                std::printf("    orbit %zu: [i=%lld, x=(", k, nd.i);
                for (std::size_t d = 0; d < 3; ++d) std::printf("%lld%s", nd.x[d], d + 1 < 3 ? "," : "");
                std::printf("), j=%lld]\n", nd.j);
            }
        } else if (n == 4) {
            auto gbq = gb_quotient_matrices<4>(n, beta);
            print_matrix("G_B Q_sym", gbq.Qsym);
            print_matrix("G_B Q_anti", gbq.Qanti);
            print_charpoly("G_B Q_sym", gbq.Qsym);
            print_charpoly("G_B Q_anti", gbq.Qanti);
            std::printf("  G_B orbit representative nodes:\n");
            for (std::size_t k = 0; k < gbq.rep_nodes.size(); ++k) {
                auto& nd = gbq.rep_nodes[k];
                std::printf("    orbit %zu: [i=%lld, x=(", k, nd.i);
                for (std::size_t d = 0; d < 4; ++d) std::printf("%lld%s", nd.x[d], d + 1 < 4 ? "," : "");
                std::printf("), j=%lld]\n", nd.j);
            }
        } else {
            auto gbq = gb_quotient_matrices<5>(n, beta);
            print_matrix("G_B Q_sym", gbq.Qsym);
            print_matrix("G_B Q_anti", gbq.Qanti);
            print_charpoly("G_B Q_sym", gbq.Qsym);
            print_charpoly("G_B Q_anti", gbq.Qanti);
            std::printf("  G_B orbit representative nodes:\n");
            for (std::size_t k = 0; k < gbq.rep_nodes.size(); ++k) {
                auto& nd = gbq.rep_nodes[k];
                std::printf("    orbit %zu: [i=%lld, x=(", k, nd.i);
                for (std::size_t d = 0; d < 5; ++d) std::printf("%lld%s", nd.x[d], d + 1 < 5 ? "," : "");
                std::printf("), j=%lld]\n", nd.j);
            }
        }

        auto bpq = bp_quotient_matrices(n);
        print_matrix("BP  Q_sym", bpq.Qsym);
        print_matrix("BP  Q_anti", bpq.Qanti);
        print_charpoly("BP Q_sym", bpq.Qsym);
        print_charpoly("BP Q_anti", bpq.Qanti);

        // Exact divisibility check: does BP's Q_sym charpoly divide
        // G_B's FULL Q_sym charpoly with zero remainder (i.e. is BP's
        // piece a genuine algebraic factor, cofactor = the nilpotent
        // part)? Same for Q_anti. Also reports p(0) := charpoly(0),
        // which (since Q_sym_BP/Q_anti_BP have EVEN dimension for
        // every n checked so far) equals det(Q_sym_BP)/det(Q_anti_BP)
        // directly -- p(0) != 0 is exactly "Q_sym_BP is invertible",
        // the fact that makes the x^k cofactor split above an
        // automatic primary-decomposition consequence rather than a
        // per-n coincidence. See app/bp_full_core_det.cpp for the
        // follow-up attacking WHY p(0) != 0 (short answer so far:
        // p(0) = sign of a genuine permutation hiding inside the
        // un-quotiented recurrent-core matrix, checked exactly n=3..6;
        // see that file's header comment and docs/RESEARCH_STATUS.md).
        {
            auto bp_sym_cp = mathlib::charpoly_faddeev_leverrier(bpq.Qsym);
            auto bp_anti_cp = mathlib::charpoly_faddeev_leverrier(bpq.Qanti);
            std::vector<std::vector<long long>> gb_sym_M, gb_anti_M;
            if (n == 3) { auto g = gb_quotient_matrices<3>(n, beta); gb_sym_M = g.Qsym; gb_anti_M = g.Qanti; }
            else if (n == 4) { auto g = gb_quotient_matrices<4>(n, beta); gb_sym_M = g.Qsym; gb_anti_M = g.Qanti; }
            else { auto g = gb_quotient_matrices<5>(n, beta); gb_sym_M = g.Qsym; gb_anti_M = g.Qanti; }
            auto gb_sym_cp = mathlib::charpoly_faddeev_leverrier(gb_sym_M);
            auto gb_anti_cp = mathlib::charpoly_faddeev_leverrier(gb_anti_M);
            auto dm_sym = mathlib::divmod(gb_sym_cp, bp_sym_cp);
            auto dm_anti = mathlib::divmod(gb_anti_cp, bp_anti_cp);
            std::printf("  p(0) [BP Q_sym charpoly constant term, = det(Q_sym_BP) since dim is even] = %s\n",
                        mathlib::str(bp_sym_cp.coeff(0)).c_str());
            std::printf("  p(0) [BP Q_anti charpoly constant term, = det(Q_anti_BP) since dim is even] = %s\n",
                        mathlib::str(bp_anti_cp.coeff(0)).c_str());
            std::printf("  DIVISIBILITY: G_B Q_sym charpoly / BP Q_sym charpoly -- remainder is %s\n",
                        dm_sym.r.is_zero() ? "EXACTLY ZERO (clean factor)" : "NONZERO");
            if (!dm_sym.r.is_zero()) {
                std::printf("    remainder degree %zu, leading coeffs printed for inspection\n", dm_sym.r.degree());
            } else {
                std::printf("    cofactor (the 'nilpotent part'), degree %zu: ", dm_sym.q.degree());
                for (long long k = static_cast<long long>(dm_sym.q.degree()); k >= 0; --k) {
                    std::printf("%s*x^%lld ", mathlib::str(dm_sym.q.coeff(static_cast<std::size_t>(k))).c_str(), k);
                }
                std::printf("\n");
            }
            std::printf("  DIVISIBILITY: G_B Q_anti charpoly / BP Q_anti charpoly -- remainder is %s\n",
                        dm_anti.r.is_zero() ? "EXACTLY ZERO (clean factor)" : "NONZERO");
            if (dm_anti.r.is_zero()) {
                std::printf("    cofactor (the 'nilpotent part'), degree %zu: ", dm_anti.q.degree());
                for (long long k = static_cast<long long>(dm_anti.q.degree()); k >= 0; --k) {
                    std::printf("%s*x^%lld ", mathlib::str(dm_anti.q.coeff(static_cast<std::size_t>(k))).c_str(), k);
                }
                std::printf("\n");
            }
        }
        std::printf("  BP orbit representative states:\n");
        for (std::size_t k = 0; k < bpq.rep_keys.size(); ++k) {
            std::vector<std::int8_t> u, v;
            unpair_key(bpq.rep_keys[k], u, v);
            std::printf("    orbit %zu: u=[", k);
            for (auto c : u) std::printf("%d", c);
            std::printf("] v=[");
            for (auto c : v) std::printf("%d", c);
            std::printf("]\n");
        }
        std::printf("\n");
    }
    return 0;
}
