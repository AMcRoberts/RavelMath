// gb_bp_hop_rule.hpp
//
// The hop-distance rule discovered while cross-checking BP-core's
// candidate bijection against G_B's real dominant recurrent core
// (see app/gb_bp_crosscheck.cpp and docs/RESEARCH_STATUS.md "The G_B
// cross-check and the hop-distance dichotomy"), extracted into a
// reusable, testable form.
//
// CLAIM (checked exhaustively, n=3..6, every confirmed-confirmed edge,
// zero mismatches -- see tests/gb_bp_hop_rule_test.cpp for the
// regression check): at a BP-core transition (parent -> child), locate
// the letters Up[ku], Vp[kv] at the split point (via `locate()`,
// reused from app/gb_bp_bijection_attempt.cpp's word-level formula).
// The hop distance between the child's and parent's real G_B nodes
// (via repeated `simple_forward_targets`) is:
//   - 1, if both located letters are 0
//   - n+1, if both located letters are equal and nonzero
//   - 1, if the letters differ and are not the pair {0, n-2}
//   - for the pair {0, n-2} specifically: 1 if the child's word is
//     longer than the parent's, else n+1
//
// This file provides:
//   - `build_gb_dominant_core<D>`: G_B's real dominant recurrent core
//     as a `set<SNode<D>>`, via the exact pipeline in
//     gb_bp_twisted_quotient_check.cpp, stopping after `dom_idx`.
//   - `build_bp_recurrent_states_with_edges`: BP-core's recurrent
//     states and their transition graph, each edge annotated with the
//     located-letter data needed for the rule above.
//   - `gb_hop_distance<D>`: BFS hop distance via `simple_forward_targets`.
//   - `predicted_hop`: the rule itself, as a pure function.
//   - `verify_hop_rule<D>`: ties everything together -- confirms each
//     BP-core state's real G_B node via iterative neighbor-consistency
//     propagation (using the rule as the edge criterion), then reports
//     exactly how many states were confirmed and how many edges the
//     rule predicted correctly, for a caller (a test) to assert on.

#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/graph_divisor.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

namespace ravel {
namespace gb_bp_hop {

inline std::vector<std::vector<std::int8_t>> n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    sigma[n - 1] = {0};
    return sigma;
}

inline double compute_beta(std::size_t n) {
    std::vector<std::vector<long long>> M(n, std::vector<long long>(n, 0));
    for (std::size_t c = 0; c + 1 < n; ++c) { M[0][c] += 1; M[c + 1][c] += 1; }
    M[0][n - 1] += 1;
    return classify_matrix_spectral(M).beta;
}

// ---- G_B side: the real dominant recurrent core -----------------------
// Same pipeline as gb_bp_twisted_quotient_check.cpp's gb_twisted_quotient,
// stopping right after dom_idx (skipping the phi-orbit/Qsym/Qanti
// machinery, which this file doesn't need).
template <std::size_t D>
std::set<SNode<D>> build_gb_dominant_core(const Substitution<D>& subst) {
    auto d_cont_cands = search_D_cont<D>(subst, 2);
    std::vector<ANode<D>> d_cont_an;
    for (const auto& c : d_cont_cands) { ANode<D> nd; nd.i = c.i; nd.j = c.j; nd.x = c.x; d_cont_an.push_back(nd); }
    auto gp_nodes = backward_closure<D>(
        subst, d_cont_an, kContactBoundaryMaxNodesDefault);
    auto gp_edges = induced_restricted_edges<D>(subst, gp_nodes);
    std::set<ANode<D>> gp_set(gp_nodes.begin(), gp_nodes.end());
    auto red_gp = red_anode<D>(gp_set, gp_edges);
    std::set<SNode<D>> C;
    for (const auto& a : red_gp.first) { SNode<D> s; s.i = a.i; s.j = a.j; s.x = a.x; C.insert(s); }
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

    std::set<SNode<D>> dom_nodes;
    for (auto u : dom_idx) dom_nodes.insert(nodes[u]);
    return dom_nodes;
}

template <std::size_t D>
bool gb_edge_exists(const Substitution<D>& subst, const SNode<D>& from, const SNode<D>& to) {
    auto fwd = simple_forward_targets<D>(subst, from);
    for (const auto& [dest, pq] : fwd) { (void)pq; if (dest == to) return true; }
    return false;
}

// BFS via repeated simple_forward_targets, level by level. Returns the
// hop count at which `to` FIRST appears from `from`, or -1 if absent
// within max_hops.
template <std::size_t D>
int gb_hop_distance(const Substitution<D>& subst, const SNode<D>& from, const SNode<D>& to,
                     int max_hops = 10) {
    if (from == to) return 0;
    std::set<SNode<D>> frontier{from};
    std::set<SNode<D>> visited{from};
    for (int h = 1; h <= max_hops; ++h) {
        std::set<SNode<D>> next;
        for (const auto& node : frontier) {
            auto fwd = simple_forward_targets<D>(subst, node);
            for (const auto& [dest, pq] : fwd) {
                (void)pq;
                if (dest == to) return h;
                if (!visited.count(dest)) { next.insert(dest); visited.insert(dest); }
            }
        }
        if (next.empty()) break;
        frontier = std::move(next);
    }
    return -1;
}

// ---- BP side: recurrent states, transitions, and located-letter data --

struct BpState {
    std::vector<std::int8_t> u, v;
};

// Reduce_pair, but also returning each chunk's flat start position s
// within the parent's expanded image sigma(u)=sigma(v).
inline std::vector<std::tuple<std::vector<std::int8_t>, std::vector<std::int8_t>, std::size_t>>
reduce_pair_with_starts(const std::vector<std::int8_t>& u, const std::vector<std::int8_t>& v,
                         std::size_t nl) {
    std::vector<std::tuple<std::vector<std::int8_t>, std::vector<std::int8_t>, std::size_t>> chunks;
    if (u.size() != v.size()) return chunks;
    std::vector<long long> cu(nl, 0), cv(nl, 0);
    std::size_t start = 0;
    for (std::size_t i = 0; i < u.size(); ++i) {
        cu[static_cast<std::size_t>(u[i])] += 1;
        cv[static_cast<std::size_t>(v[i])] += 1;
        if (cu == cv) {
            chunks.emplace_back(
                std::vector<std::int8_t>(u.begin() + start, u.begin() + i + 1),
                std::vector<std::int8_t>(v.begin() + start, v.begin() + i + 1),
                start);
            start = i + 1;
            std::fill(cu.begin(), cu.end(), 0);
            std::fill(cv.begin(), cv.end(), 0);
        }
    }
    return chunks;
}

// Which LETTER of `word` (by index) the flat position p falls under
// (after each letter is expanded to its own sigma-image), and the
// local offset within that letter's image.
inline std::pair<std::size_t, std::size_t> locate(
        const std::vector<std::int8_t>& word, const SubstitutionRule& rule, std::size_t p) {
    std::size_t pos = 0;
    for (std::size_t k = 0; k < word.size(); ++k) {
        std::size_t len = rule.image(word[k]).size();
        if (p < pos + len) return {k, p - pos};
        pos += len;
    }
    throw std::out_of_range("locate: position beyond word's image");
}

struct EdgeInfo {
    std::size_t parent, child;
    std::size_t s;
    std::size_t ku, kv;
    std::int8_t letter_u, letter_v;
    std::size_t ou, ov;
};

struct BpGraphResult {
    std::vector<BpState> states;
    std::vector<std::vector<std::size_t>> edges;
    std::vector<EdgeInfo> edge_info;
};

inline BpGraphResult build_bp_recurrent_states_with_edges(std::size_t n) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    const std::size_t nl = rule.alphabet_size();

    std::map<std::string, std::size_t> seen;
    std::deque<std::size_t> queue;
    std::vector<std::string> order;
    std::vector<BpState> word_of;
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> raw_edges;
    {
        auto initial = reduce_pair({0, 1}, {1, 0}, nl);
        for (auto& ch : initial) {
            std::string k = pair_key(ch.first, ch.second);
            if (!seen.count(k)) {
                seen[k] = order.size();
                order.push_back(k);
                word_of.push_back({ch.first, ch.second});
                queue.push_back(order.size() - 1);
            }
        }
    }
    constexpr std::size_t max_pairs = 20000, max_len = 60000;
    while (!queue.empty()) {
        std::size_t idx = queue.front();
        queue.pop_front();
        auto [u, v] = word_of[idx];
        auto img = sigma_pair(rule, {u, v});
        if (img.first.size() > max_len || seen.size() > max_pairs) break;
        auto chunks = reduce_pair_with_starts(img.first, img.second, nl);
        for (auto& [uc, vc, s] : chunks) {
            std::string ck = pair_key(uc, vc);
            auto it = seen.find(ck);
            std::size_t ci;
            if (it == seen.end()) {
                ci = order.size();
                seen[ck] = ci;
                order.push_back(ck);
                word_of.push_back({uc, vc});
                queue.push_back(ci);
            } else {
                ci = it->second;
            }
            raw_edges.push_back({idx, ci, s});
        }
    }

    std::vector<std::size_t> noncoin;
    for (std::size_t i = 0; i < order.size(); ++i) {
        if (!(word_of[i].u.size() == 1 && word_of[i].u == word_of[i].v)) noncoin.push_back(i);
    }
    std::map<std::size_t, std::size_t> idx_map;
    for (std::size_t k = 0; k < noncoin.size(); ++k) idx_map[noncoin[k]] = k;
    std::vector<std::vector<long long>> dense(noncoin.size(), std::vector<long long>(noncoin.size(), 0));
    for (auto& [p, c, s] : raw_edges) {
        auto pit = idx_map.find(p), cit = idx_map.find(c);
        if (pit != idx_map.end() && cit != idx_map.end()) dense[pit->second][cit->second] += 1;
        (void)s;
    }
    auto full_graph = WeightedDigraph::from_dense(dense);
    auto [core, orig_idx] = extract_recurrent_core(full_graph);

    std::vector<std::size_t> full_to_core(noncoin.size(), SIZE_MAX);
    for (std::size_t c = 0; c < orig_idx.size(); ++c) full_to_core[orig_idx[c]] = c;

    BpGraphResult result;
    for (auto c : orig_idx) result.states.push_back(word_of[noncoin[c]]);
    result.edges.assign(core.n, {});
    for (auto& [p, c, s] : raw_edges) {
        auto pit = idx_map.find(p), cit = idx_map.find(c);
        if (pit == idx_map.end() || cit == idx_map.end()) continue;
        std::size_t cp = full_to_core[pit->second], cc = full_to_core[cit->second];
        if (cp == SIZE_MAX || cc == SIZE_MAX) continue;
        result.edges[cp].push_back(cc);
        auto [u, v] = word_of[p];
        auto [ku, ou] = locate(u, rule, s);
        auto [kv, ov] = locate(v, rule, s);
        result.edge_info.push_back({cp, cc, s, ku, kv, u[ku], v[kv], ou, ov});
    }
    return result;
}

// ---- The rule itself, as a pure function -------------------------------

// The complete hop-distance rule (see file header). `n` is the
// n-bonacci parameter; `letter_u`/`letter_v` are the located letters at
// the split; `parent_len`/`child_len` are only consulted for the
// {0,n-2} tiebreak.
inline int predicted_hop(std::size_t n, std::int8_t letter_u, std::int8_t letter_v,
                          std::size_t parent_len, std::size_t child_len) {
    const std::int8_t mixed_letter = static_cast<std::int8_t>(n) - 2;
    bool is_mixed_pair = (letter_u == 0 && letter_v == mixed_letter) ||
                         (letter_u == mixed_letter && letter_v == 0);
    if (is_mixed_pair) return (child_len > parent_len) ? 1 : static_cast<int>(n) + 1;
    if (letter_u == letter_v && letter_u != 0) return static_cast<int>(n) + 1;
    return 1;
}

// ---- Tying it together: confirm states, then verify the rule ----------

struct HopRuleReport {
    std::size_t total_states = 0;
    std::size_t confirmed_states = 0;
    std::size_t edges_checked = 0;
    std::size_t edges_matched = 0;
    bool fully_confirmed() const { return confirmed_states == total_states; }
    bool rule_fully_matches() const { return edges_checked > 0 && edges_checked == edges_matched; }
};

template <std::size_t D>
HopRuleReport verify_hop_rule(std::size_t n) {
    HopRuleReport report;
    double beta = compute_beta(n);
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);
    auto dom = build_gb_dominant_core<D>(subst);
    auto bp = build_bp_recurrent_states_with_edges(n);
    const auto& bp_states = bp.states;
    report.total_states = bp_states.size();

    std::vector<bool> ambiguous(bp_states.size(), false);
    std::vector<SNode<D>> x_true(bp_states.size());
    std::vector<std::vector<SNode<D>>> candidates(bp_states.size());
    for (std::size_t si = 0; si < bp_states.size(); ++si) {
        long long ci_ = bp_states[si].u[0], cj_ = bp_states[si].v[0];
        for (const auto& node : dom) {
            if (node.i == ci_ && node.j == cj_) candidates[si].push_back(node);
        }
        ambiguous[si] = candidates[si].size() > 1;
        if (candidates[si].size() == 1) x_true[si] = candidates[si][0];
    }

    std::vector<bool> confirmed_flag(bp_states.size());
    for (std::size_t s = 0; s < bp_states.size(); ++s) confirmed_flag[s] = !ambiguous[s];

    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t si = 0; si < bp_states.size(); ++si) {
            if (confirmed_flag[si]) continue;
            std::vector<bool> survives(candidates[si].size(), true);
            bool any_constraint = false;
            for (std::size_t cidx = 0; cidx < candidates[si].size(); ++cidx) {
                const auto& cand = candidates[si][cidx];
                for (const auto& ei : bp.edge_info) {
                    if (ei.parent == si && confirmed_flag[ei.child]) {
                        any_constraint = true;
                        std::size_t clen = bp_states[ei.child].u.size(), plen = bp_states[si].u.size();
                        int h = predicted_hop(n, ei.letter_u, ei.letter_v, plen, clen);
                        if (gb_hop_distance<D>(subst, x_true[ei.child], cand, h) != h) survives[cidx] = false;
                    }
                    if (ei.child == si && confirmed_flag[ei.parent]) {
                        any_constraint = true;
                        std::size_t plen = bp_states[ei.parent].u.size(), clen = bp_states[si].u.size();
                        int h = predicted_hop(n, ei.letter_u, ei.letter_v, plen, clen);
                        if (gb_hop_distance<D>(subst, cand, x_true[ei.parent], h) != h) survives[cidx] = false;
                    }
                }
            }
            if (!any_constraint) continue;
            std::size_t n_survive = 0, survivor = 0;
            for (std::size_t cidx = 0; cidx < candidates[si].size(); ++cidx)
                if (survives[cidx]) { ++n_survive; survivor = cidx; }
            if (n_survive == 1) {
                x_true[si] = candidates[si][survivor];
                confirmed_flag[si] = true;
                changed = true;
            }
        }
    }
    report.confirmed_states = static_cast<std::size_t>(
        std::count(confirmed_flag.begin(), confirmed_flag.end(), true));

    for (const auto& ei : bp.edge_info) {
        if (!confirmed_flag[ei.parent] || !confirmed_flag[ei.child]) continue;
        ++report.edges_checked;
        std::size_t plen = bp_states[ei.parent].u.size(), clen = bp_states[ei.child].u.size();
        int predicted = predicted_hop(n, ei.letter_u, ei.letter_v, plen, clen);
        int actual = gb_hop_distance<D>(subst, x_true[ei.child], x_true[ei.parent], predicted + 2);
        if (actual == predicted) ++report.edges_matched;
    }
    return report;
}

}  // namespace gb_bp_hop
}  // namespace ravel
