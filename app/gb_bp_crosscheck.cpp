// gb_bp_crosscheck.cpp
//
// Item (1), thread A: the concrete next step flagged (but never done)
// at the end of app/gb_bp_bijection_attempt.cpp's header comment.
//
// That file's periodic fixed-point solve found X = 0 identically for
// every BP-core recurrent state, and left two un-checked
// interpretations of what this means for the sought bijection between
// BP-core states and G_B's dominant-core nodes:
//   (a) the true map is [i=u[0], x=0, j=v[0]] plus a FIXED per-(i,j)
//       offset this construction doesn't track, or
//   (b) this whole tracking scheme is anchored wrong.
//
// THIS DRIVER: for each BP-core recurrent state (u,v), builds the
// literal candidate SNode [i=u[0], x=(0,...,0), j=v[0]] and checks
// membership in G_B's ACTUAL dominant recurrent core (built via the
// exact same pipeline as gb_bp_twisted_quotient_check.cpp -- search_D_cont,
// backward_closure, the C-corona iteration, extract_dominant_recurrent_core).
//
// If (a) were simply true with a ZERO offset (i.e. X=0 really is
// already the right x-coordinate), every candidate should be a member.
// If none are, or only some are, that's direct evidence toward (b), or
// toward a nontrivial per-(i,j) offset under (a) -- either way, this
// is the first actual data on the question, not a guess.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/gb_bp_crosscheck.cpp math/out/libmath.a -o gb_bp_crosscheck

#include <algorithm>
#include <cstdio>
#include <deque>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

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

// ---- G_B side: build the dominant recurrent core's ACTUAL SNode set,
// exactly the same pipeline as gb_bp_twisted_quotient_check.cpp's
// gb_twisted_quotient, stopping right after dom_idx is extracted
// (skipping the phi-orbit / Qsym / Qanti machinery this driver doesn't
// need). ---------------------------------------------------------------

template <std::size_t D>
std::set<SNode<D>> build_gb_dominant_core(const Substitution<D>& subst) {
    auto d_cont_cands = search_D_cont<D>(subst, 2);
    std::vector<ANode<D>> d_cont_an;
    for (const auto& c : d_cont_cands) { ANode<D> nd; nd.i = c.i; nd.j = c.j; nd.x = c.x; d_cont_an.push_back(nd); }
    auto gp_nodes = backward_closure<D>(subst, d_cont_an, kContactBoundaryMaxNodes);
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
    bool converged = false;
    int rounds_run = 0;
    for (int p = 2; p <= MAX_ROUNDS; ++p) {
        rounds_run = p;
        auto corona_nodes = c_corona<D>(subst, A_prev, A_prev);
        if (corona_nodes.size() > MAX_A_SIZE) { std::printf("  [G_B build] hit MAX_A_SIZE at round %d\n", p); break; }
        std::vector<std::tuple<SNode<D>, SNode<D>, std::vector<long long>, std::vector<long long>>> edges;
        for (const auto& nn : corona_nodes) {
            auto fwd = simple_forward_targets<D>(subst, nn);
            for (const auto& [dest, pq] : fwd) { (void)pq; if (corona_nodes.count(dest) > 0) edges.push_back({nn, dest, {}, {}}); }
        }
        auto red_result = red<D>(corona_nodes, edges);
        if (red_result.first == A_prev) { gb = red_result.first; converged = true; break; }
        A_prev = red_result.first;
        gb = A_prev;
    }
    std::printf("  [G_B build] %s after %d round(s), final size %zu\n",
                converged ? "CONVERGED (fixed point)" : "DID NOT CONVERGE -- hit round/size limit",
                rounds_run, gb.size());

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

// ---- BP side: recover the recurrent-core states' (u,v) word pairs. --

struct BpState {
    std::vector<std::int8_t> u, v;
};

// Reduce_pair, but also returning each chunk's flat start position s
// within the parent's expanded image sigma(u)=sigma(v) -- needed to
// compute ku/kv (which parent LETTER the split falls under) via
// locate() below, reused verbatim from gb_bp_bijection_attempt.cpp.
std::vector<std::tuple<std::vector<std::int8_t>, std::vector<std::int8_t>, std::size_t>>
reduce_pair_with_starts(const std::vector<std::int8_t>& u,
                         const std::vector<std::int8_t>& v,
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
// local offset within that letter's image. Reused verbatim from
// gb_bp_bijection_attempt.cpp.
std::pair<std::size_t, std::size_t> locate(
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
    std::size_t s;    // chunk start position within sigma(parent.u)
    std::size_t ku;   // which letter POSITION of parent.u the split falls under
    std::size_t kv;   // which letter POSITION of parent.v the split falls under
    std::int8_t letter_u;  // the actual letter VALUE Up[ku]
    std::int8_t letter_v;  // the actual letter VALUE Vp[kv]
    std::size_t ou;   // local offset within Up[ku]'s own image (0 or 1, since images have length <=2)
    std::size_t ov;   // local offset within Vp[kv]'s own image
};

struct BpGraphResult {
    std::vector<BpState> states;
    std::vector<std::vector<std::size_t>> edges;  // plain adjacency, for cycle/BFS use
    std::vector<EdgeInfo> edge_info;              // same edges, with s/ku/kv attached
};

BpGraphResult build_bp_recurrent_states_with_edges(std::size_t n) {
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    const std::size_t nl = rule.alphabet_size();

    std::map<std::string, std::size_t> seen;
    std::deque<std::size_t> queue;
    std::vector<std::string> order;
    std::vector<BpState> word_of;
    std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> raw_edges;  // (parent, child, s), full index space
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

std::vector<BpState> build_bp_recurrent_states(std::size_t n) {
    return build_bp_recurrent_states_with_edges(n).states;
}

// Enumerates all simple directed cycles in a small graph via DFS
// (feasible here: BP-core has at most a couple of out-degree-2 nodes,
// so very few distinct simple cycles exist despite the graph not
// being a simple functional graph).
std::vector<std::vector<std::size_t>> enumerate_simple_cycles(
        const std::vector<std::vector<std::size_t>>& edges) {
    std::size_t n = edges.size();
    std::vector<std::vector<std::size_t>> cycles;
    std::vector<std::size_t> path;
    std::vector<bool> on_path(n, false);
    std::function<void(std::size_t, std::size_t)> dfs = [&](std::size_t start, std::size_t cur) {
        if (cycles.size() > 200) return;  // safety cap
        for (auto nxt : edges[cur]) {
            if (nxt == start) {
                cycles.push_back(path);
                continue;
            }
            if (on_path[nxt]) continue;
            path.push_back(nxt);
            on_path[nxt] = true;
            dfs(start, nxt);
            on_path[nxt] = false;
            path.pop_back();
        }
    };
    for (std::size_t s = 0; s < n; ++s) {
        path = {s};
        on_path[s] = true;
        dfs(s, s);
        on_path[s] = false;
    }
    // Dedup cycles that are rotations of each other.
    auto canon = [](std::vector<std::size_t> c) {
        std::size_t m = *std::min_element(c.begin(), c.end());
        std::size_t mi = 0;
        for (std::size_t k = 0; k < c.size(); ++k) if (c[k] == m) { mi = k; break; }
        std::vector<std::size_t> r;
        for (std::size_t k = 0; k < c.size(); ++k) r.push_back(c[(mi + k) % c.size()]);
        return r;
    };
    std::set<std::vector<std::size_t>> uniq;
    for (auto& c : cycles) uniq.insert(canon(c));
    return std::vector<std::vector<std::size_t>>(uniq.begin(), uniq.end());
}
template <std::size_t D>
bool gb_edge_exists(const Substitution<D>& subst, const SNode<D>& from, const SNode<D>& to) {
    auto fwd = simple_forward_targets<D>(subst, from);
    for (const auto& [dest, pq] : fwd) { (void)pq; if (dest == to) return true; }
    return false;
}

// BFS via repeated simple_forward_targets, level by level, starting at
// `from`. Returns the hop count at which `to` FIRST appears, or -1 if
// not found within max_hops.
template <std::size_t D>
int gb_hop_distance(const Substitution<D>& subst, const SNode<D>& from, const SNode<D>& to,
                     int max_hops = 6) {
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

// Same BFS as gb_hop_distance, but reconstructs and returns the actual
// intermediate node sequence from `from` to `to` (inclusive), so the
// G_B-side mechanism can be inspected directly rather than inferred
// from a bare hop count.
template <std::size_t D>
std::vector<SNode<D>> gb_hop_path(const Substitution<D>& subst, const SNode<D>& from,
                                   const SNode<D>& to, int max_hops = 10) {
    if (from == to) return {from};
    std::map<SNode<D>, SNode<D>> parent_of;
    std::set<SNode<D>> frontier{from};
    std::set<SNode<D>> visited{from};
    for (int h = 1; h <= max_hops; ++h) {
        std::set<SNode<D>> next;
        for (const auto& node : frontier) {
            auto fwd = simple_forward_targets<D>(subst, node);
            for (const auto& [dest, pq] : fwd) {
                (void)pq;
                if (visited.count(dest)) continue;
                parent_of[dest] = node;
                if (dest == to) {
                    std::vector<SNode<D>> path{dest};
                    SNode<D> cur = dest;
                    while (!(cur == from)) { cur = parent_of[cur]; path.push_back(cur); }
                    std::reverse(path.begin(), path.end());
                    return path;
                }
                next.insert(dest);
                visited.insert(dest);
            }
        }
        if (next.empty()) break;
        frontier = std::move(next);
    }
    return {};  // not found
}


template <std::size_t D>
void run_crosscheck(std::size_t n) {
    double beta = compute_beta(n);
    auto sigma = n_bonacci_rule(n);
    SubstitutionRule rule(sigma);
    auto subst = make_substitution<D>(rule, beta);
    auto dom = build_gb_dominant_core<D>(subst);
    auto bp = build_bp_recurrent_states_with_edges(n);
    const auto& bp_states = bp.states;
    const auto& edges = bp.edges;

    std::printf("=== n=%zu ===\n", n);
    std::printf("  G_B dominant core: %zu nodes\n", dom.size());
    std::printf("  BP-core recurrent states: %zu\n", bp_states.size());

    std::size_t hits = 0;
    std::set<std::pair<long long, long long>> distinct_ij;
    std::vector<bool> ambiguous(bp_states.size(), false);
    std::vector<SNode<D>> x_true(bp_states.size());
    std::vector<std::vector<SNode<D>>> candidates(bp_states.size());
    for (std::size_t si = 0; si < bp_states.size(); ++si) {
        const auto& st = bp_states[si];
        long long ci_ = st.u[0], cj_ = st.v[0];
        distinct_ij.insert({ci_, cj_});
        for (const auto& node : dom) {
            if (node.i == ci_ && node.j == cj_) candidates[si].push_back(node);
        }
        ambiguous[si] = candidates[si].size() > 1;
        if (candidates[si].size() == 1) x_true[si] = candidates[si][0];
        SNode<D> zero_cand; zero_cand.i = ci_; zero_cand.j = cj_;
        for (std::size_t k = 0; k < D; ++k) zero_cand.x[k] = 0;
        if (dom.count(zero_cand) > 0) ++hits;
    }
    std::printf("  candidates found in G_B dominant core (exact x=0): %zu / %zu\n", hits, bp_states.size());
    std::printf("  distinct (i,j) pairs among candidates: %zu / %zu states, %zu ambiguous\n",
                distinct_ij.size(), bp_states.size(),
                static_cast<std::size_t>(std::count(ambiguous.begin(), ambiguous.end(), true)));

    // The correlation search: for every both-unique-match edge, look up
    // its (ku, kv, s) from edge_info and compute the REAL hop distance
    // in G_B between x_true[child] and x_true[parent]. Look for what
    // determines that hop count.
    std::printf("  both-unique-match edges: parent_len child_len  ku  kv  s   hop_distance\n");
    std::map<int, int> hop_histogram;
    for (const auto& ei : bp.edge_info) {
        if (ambiguous[ei.parent] || ambiguous[ei.child]) continue;
        int hop = gb_hop_distance<D>(subst, x_true[ei.child], x_true[ei.parent], 10);
        std::printf("    #%zu->#%zu:  %5zu  %5zu  %3zu %3zu %3zu   hop=%d\n",
                    ei.parent, ei.child, bp_states[ei.parent].u.size(), bp_states[ei.child].u.size(),
                    ei.ku, ei.kv, ei.s, hop);
        hop_histogram[hop]++;
    }
    std::printf("  hop-distance histogram: ");
    for (auto [h, cnt] : hop_histogram) std::printf("[hop=%d: %d edges] ", h, cnt);
    std::printf("\n");

    // ITERATIVE PROPAGATION, now using the EXACT combined rule (letter
    // identity, with {0,n-2} tie-broken by length-growth) confirmed
    // exhaustive above (100% match, every edge, n=3..6) -- not the
    // earlier permissive "reachable within 1..n+1 hops" placeholder,
    // which under-constrained ambiguous states and left several
    // unresolved.
    const std::int8_t mixed_letter_early = static_cast<std::int8_t>(n) - 2;
    auto expected_hop_for = [&](std::int8_t lu, std::int8_t lv, std::size_t plen, std::size_t clen) -> int {
        bool is_mixed = (lu == 0 && lv == mixed_letter_early) || (lu == mixed_letter_early && lv == 0);
        if (is_mixed) return (clen > plen) ? 1 : static_cast<int>(n) + 1;
        if (lu == lv && lu != 0) return static_cast<int>(n) + 1;
        return 1;
    };
    std::vector<bool> confirmed_flag = ambiguous;
    for (std::size_t s = 0; s < bp_states.size(); ++s) confirmed_flag[s] = !ambiguous[s];

    bool changed = true;
    int rounds = 0;
    while (changed) {
        changed = false;
        ++rounds;
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
                        int h = expected_hop_for(ei.letter_u, ei.letter_v, plen, clen);
                        if (gb_hop_distance<D>(subst, x_true[ei.child], cand, h) != h) survives[cidx] = false;
                    }
                    if (ei.child == si && confirmed_flag[ei.parent]) {
                        any_constraint = true;
                        std::size_t plen = bp_states[ei.parent].u.size(), clen = bp_states[si].u.size();
                        int h = expected_hop_for(ei.letter_u, ei.letter_v, plen, clen);
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
    std::size_t n_confirmed = static_cast<std::size_t>(
        std::count(confirmed_flag.begin(), confirmed_flag.end(), true));
    std::printf("  iterative propagation converged after %d round(s): %zu / %zu states confirmed "
                "(%zu still unresolved)\n",
                rounds, n_confirmed, bp_states.size(), bp_states.size() - n_confirmed);

    for (std::size_t si = 0; si < bp_states.size(); ++si) {
        if (!ambiguous[si]) continue;
        long long pi = bp_states[si].u[0], pj = bp_states[si].v[0];
        if (confirmed_flag[si]) {
            const auto& x = x_true[si].x;
            std::printf("    #%zu (i=%lld,j=%lld): CONFIRMED (", si, pi, pj);
            for (std::size_t k = 0; k < D; ++k) std::printf("%lld%s", x[k], k + 1 < D ? "," : "");
            std::printf(")\n");
        } else {
            std::printf("    #%zu (i=%lld,j=%lld): unresolved, %zu candidates remain\n",
                        si, pi, pj, candidates[si].size());
        }
    }

    // Now recompute the hop histogram over the FULL confirmed set,
    // including edges that touch a formerly-ambiguous (short) state --
    // to see whether a genuinely new category shows up there.
    std::map<int, int> full_hop_histogram;
    std::map<std::pair<int, int>, std::pair<int, int>> letter_to_hop_count;  // (letter_u,letter_v)->(hop1,hopnp1)
    for (const auto& ei : bp.edge_info) {
        if (!confirmed_flag[ei.parent] || !confirmed_flag[ei.child]) continue;
        int hop = gb_hop_distance<D>(subst, x_true[ei.child], x_true[ei.parent], static_cast<int>(n) + 3);
        full_hop_histogram[hop]++;
        auto key = std::make_pair(static_cast<int>(ei.letter_u), static_cast<int>(ei.letter_v));
        if (hop == 1) letter_to_hop_count[key].first++;
        else letter_to_hop_count[key].second++;
    }
    std::printf("  FULL hop-distance histogram (all confirmed-confirmed edges): ");
    for (auto [h, cnt] : full_hop_histogram) std::printf("[hop=%d: %d edges] ", h, cnt);
    std::printf("\n");
    std::printf("  (letter_u=Up[ku], letter_v=Vp[kv]) -> (count at hop=1, count at hop=n+1):\n");
    for (auto& [lulv, counts] : letter_to_hop_count) {
        std::printf("    (letter_u=%d,letter_v=%d): hop1=%d, hop(n+1)=%d\n",
                    lulv.first, lulv.second, counts.first, counts.second);
    }

    // Zoom in on the {0, n-2} mixed pair specifically (re-checked the
    // actual table by hand: it's n-2, not n-1 as first guessed -- e.g.
    // at n=4 the mixed entries are (letter_u=0,letter_v=2) and (2,0),
    // not anything involving 3). Print every such edge's ou/ov (local
    // offset within the located letter's own image -- discarded until
    // now) alongside the hop, to find what actually discriminates.
    const std::int8_t mixed_letter = static_cast<std::int8_t>(n) - 2;
    std::printf("  {0,%d} mixed-pair edges, full detail (ou/ov = local offset within the\n"
                "  located letter's own image, discarded from earlier analysis):\n", mixed_letter);
    std::map<std::tuple<std::size_t, std::size_t, int>, int> mixed_length_pairs;
    for (const auto& ei : bp.edge_info) {
        if (!confirmed_flag[ei.parent] || !confirmed_flag[ei.child]) continue;
        bool is_mixed_pair = (ei.letter_u == 0 && ei.letter_v == mixed_letter) ||
                              (ei.letter_u == mixed_letter && ei.letter_v == 0);
        if (!is_mixed_pair) continue;
        int hop = gb_hop_distance<D>(subst, x_true[ei.child], x_true[ei.parent], static_cast<int>(n) + 3);
        std::size_t plen = bp_states[ei.parent].u.size(), clen = bp_states[ei.child].u.size();
        mixed_length_pairs[{plen, clen, hop}]++;
    }
    std::printf("  {0,%d} mixed-pair (parent_len, child_len, hop) -> count:\n", mixed_letter);
    for (auto& [key, cnt] : mixed_length_pairs) {
        auto [plen, clen, hop] = key;
        std::printf("    (%zu, %zu, hop=%d): %d edge(s)\n", plen, clen, hop, cnt);
    }
    // Print the ACTUAL word content for each distinct (parent,child)
    // state pair touching a mixed-pair edge, to ground a real proof
    // attempt in the specific combinatorics rather than length numbers
    // alone.
    std::set<std::size_t> printed;
    std::printf("  {0,%d} mixed-pair actual word content:\n", mixed_letter);
    for (const auto& ei : bp.edge_info) {
        if (!confirmed_flag[ei.parent] || !confirmed_flag[ei.child]) continue;
        bool is_mixed_pair = (ei.letter_u == 0 && ei.letter_v == mixed_letter) ||
                              (ei.letter_u == mixed_letter && ei.letter_v == 0);
        if (!is_mixed_pair) continue;
        for (std::size_t s : {ei.parent, ei.child}) {
            if (printed.count(s)) continue;
            printed.insert(s);
            std::printf("    #%zu: u=[", s);
            for (auto c : bp_states[s].u) std::printf("%d", c);
            std::printf("] v=[");
            for (auto c : bp_states[s].v) std::printf("%d", c);
            std::printf("]\n");
        }
    }
    // Length-growth alone is NOT a universal rule (~50% match overall --
    // essentially uncorrelated outside the mixed pair). It's specifically
    // the tiebreaker for {0,n-2}, where letter identity alone is
    // ambiguous. Verify the COMBINED two-tier rule explains everything:
    // letter identity decides in general; length-growth decides only
    // for the {0,n-2} pair specifically.
    std::size_t combined_checked = 0, combined_matches = 0;
    for (const auto& ei : bp.edge_info) {
        if (!confirmed_flag[ei.parent] || !confirmed_flag[ei.child]) continue;
        ++combined_checked;
        int hop = gb_hop_distance<D>(subst, x_true[ei.child], x_true[ei.parent], static_cast<int>(n) + 3);
        std::size_t plen = bp_states[ei.parent].u.size(), clen = bp_states[ei.child].u.size();
        bool is_mixed_pair = (ei.letter_u == 0 && ei.letter_v == mixed_letter) ||
                              (ei.letter_u == mixed_letter && ei.letter_v == 0);
        int predicted;
        if (is_mixed_pair) {
            predicted = (clen > plen) ? 1 : static_cast<int>(n) + 1;
        } else if (ei.letter_u == ei.letter_v && ei.letter_u != 0) {
            predicted = static_cast<int>(n) + 1;
        } else {
            predicted = 1;
        }
        if (predicted == hop) ++combined_matches;
    }
    std::printf("  COMBINED RULE (letter identity, {0,n-2} tie-broken by length-growth): "
                "%zu / %zu edges match\n", combined_matches, combined_checked);

    // COLLAPSE/QUOTIENT ANALYSIS: the docs' long-standing claim is that
    // BP-core corresponds to G_B's dominant core "modulo a nilpotent
    // part" (BP-core's own Qsym/Qanti carry no leftover nilpotent
    // factor, while G_B's raw dominant core does -- see
    // docs/RESEARCH_STATUS.md). If the x_true map found above is only
    // capturing the GENUINE (non-nilpotent) part of G_B correctly, we
    // should see collisions -- multiple BP-core states landing on the
    // identical G_B node -- whenever BP-core has MORE states than the
    // genuine part has room for, i.e. the map as computed (using only
    // first-letter identity, ignoring word length/depth) is not yet
    // separating states that a correct bijection would keep distinct.
    std::map<SNode<D>, std::vector<std::size_t>> hits_by_node;
    for (std::size_t si = 0; si < bp_states.size(); ++si) {
        if (!confirmed_flag[si]) continue;
        hits_by_node[x_true[si]].push_back(si);
    }
    std::size_t distinct_hit = hits_by_node.size();
    std::size_t collided_states = 0;
    for (auto& [node, states] : hits_by_node) if (states.size() > 1) collided_states += states.size();
    std::size_t n_confirmed_states = static_cast<std::size_t>(
        std::count(confirmed_flag.begin(), confirmed_flag.end(), true));
    std::printf("  COLLAPSE CHECK: %zu confirmed BP-core states hit %zu DISTINCT G_B nodes "
                "(G_B dominant core has %zu nodes total)\n",
                n_confirmed_states, distinct_hit, dom.size());
    std::printf("  %zu states are involved in a collision (>1 state -> same G_B node)\n", collided_states);
    for (auto& [node, states] : hits_by_node) {
        if (states.size() <= 1) continue;
        std::printf("    G_B node [i=%lld,j=%lld] hit by %zu states: ", node.i, node.j, states.size());
        for (auto s : states) {
            std::printf("#%zu(len=%zu,last=%d/%d) ", s, bp_states[s].u.size(),
                        bp_states[s].u.back(), bp_states[s].v.back());
        }
        std::printf("\n");
    }

    // PUSH ON THE DERIVATION: for every SHRINK-side ({0,n-2}, hop=n+1)
    // edge, (a) show the PARENT's FULL decomposition -- every chunk
    // produced by one sigma+reduce_pair step, not just the one already
    // sampled as a BP-core edge -- to see the branching structure that
    // produces the shrink target alongside its grow-continuing
    // siblings; (b) show the ACTUAL G_B-side path (the specific
    // intermediate SNodes), not just the hop count, to look for a
    // matching recursive structure on the corona side.
    auto sigma_local = n_bonacci_rule(n);
    SubstitutionRule rule_local(sigma_local);
    const std::size_t nl_local = rule_local.alphabet_size();
    std::set<std::size_t> shrink_parents_done;
    for (const auto& ei : bp.edge_info) {
        if (!confirmed_flag[ei.parent] || !confirmed_flag[ei.child]) continue;
        bool is_mixed_pair = (ei.letter_u == 0 && ei.letter_v == mixed_letter) ||
                              (ei.letter_u == mixed_letter && ei.letter_v == 0);
        if (!is_mixed_pair) continue;
        int hop = gb_hop_distance<D>(subst, x_true[ei.child], x_true[ei.parent], static_cast<int>(n) + 3);
        if (hop == 1) continue;  // only the shrink side
        if (shrink_parents_done.count(ei.parent)) continue;
        shrink_parents_done.insert(ei.parent);

        const auto& pu = bp_states[ei.parent].u;
        const auto& pv = bp_states[ei.parent].v;
        auto img = sigma_pair(rule_local, {pu, pv});
        auto chunks = reduce_pair_with_starts(img.first, img.second, nl_local);
        std::printf("  FULL decomposition of shrink-parent #%zu (u=[", ei.parent);
        for (auto c : pu) std::printf("%d", c);
        std::printf("] v=[");
        for (auto c : pv) std::printf("%d", c);
        std::printf("], len=%zu), all children from ONE sigma+reduce_pair step:\n", pu.size());
        for (auto& [uc, vc, s] : chunks) {
            bool is_coincidence = (uc.size() == 1 && uc == vc);
            std::printf("    s=%zu len=%zu%s  u=[", s, uc.size(), is_coincidence ? " (coincidence)" : "");
            for (auto c : uc) std::printf("%d", c);
            std::printf("] v=[");
            for (auto c : vc) std::printf("%d", c);
            std::printf("]\n");
        }

        auto path = gb_hop_path<D>(subst, x_true[ei.child], x_true[ei.parent], static_cast<int>(n) + 3);
        std::printf("    edge: parent#%zu (own i,j = %d,%d, len=%zu) -> child#%zu (own i,j = %d,%d, len=%zu)\n",
                    ei.parent, bp_states[ei.parent].u[0], bp_states[ei.parent].v[0], bp_states[ei.parent].u.size(),
                    ei.child, bp_states[ei.child].u[0], bp_states[ei.child].v[0], bp_states[ei.child].u.size());
        std::printf("    G_B-side path (child -> ... -> parent, %zu hops), with #forward-targets\n"
                    "    available AT each node (1 = forced/unique step, >1 = genuine branch point):\n",
                    path.empty() ? 0 : path.size() - 1);
        for (const auto& node : path) {
            auto fwd = simple_forward_targets<D>(subst, node);
            std::printf("      [i=%lld, x=(", node.i);
            for (std::size_t k = 0; k < D; ++k) std::printf("%lld%s", node.x[k], k + 1 < D ? "," : "");
            std::printf("), j=%lld]  (%zu forward targets available here)\n", node.j, fwd.size());
        }
    }
    std::printf("\n");
}

}  // namespace

int main() {
    run_crosscheck<3>(3);
    run_crosscheck<4>(4);
    run_crosscheck<5>(5);
    run_crosscheck<6>(6);
    return 0;
}
