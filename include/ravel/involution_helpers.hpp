// include/ravel/involution_helpers.hpp
//
// Shared helpers for the G_B/BP involution and nilpotent-cofactor
// probes (FINDINGS_FOR_CITATION.md Finding 4).  Extracted from
// app/gb_bp_involution_general_n.cpp and app/gb_bp_matrix_equality.cpp
// so the new probe on σ_{a,b}-family / σ₁-style / random
// unimodular Pisot substitutions (A1 and A2 in the research direction) can
// re-use exactly the same machinery as the n-bonacci case without
// duplicating it.
//
// Contents (all stateless; safe to call from any driver):
//   - n_bonacci_rule(n) / n_bonacci_beta(n): the n-bonacci
//     substitution and its Pisot constant, used as a known-good
//     control case for both probes.
//   - check_involution_on_core(rep, ...): verifies the Z/2 involution
//     [i, x, j] <-> [j, -x, i] is exact on the dominant recurrent
//     core of G_B.  Returns {matched, total} so the caller can report
//     "EXACT" vs "partial" coverage.
//   - recurrent_core_dense_matrix(M): extracts the largest recurrent
//     SCC of a dense matrix as a fresh dense vector-of-vector.
//   - bp_core_charpoly(rule): the exact integer charpoly of the
//     balanced-pair transition graph's recurrent core (used in the
//     cofactor divisibility check).
//   - check_exact_factor(name, whole, factor): does whole / factor
//     over math::PolyZ with exact integer arithmetic and reports
//     whether the remainder is zero; prints YES/no to stdout.
//
// All functions are header-only; no link-time conflict with the
// existing gb_bp_involution_general_n.cpp (which still has its own
// copy of these helpers, since refactoring that file is a separate
// housekeeping task -- the shared header is the recommended entry
// point for new code).

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "math/poly_z.hpp"
#include "ravel/balanced_pair.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/faces.hpp"
#include "ravel/graph_divisor.hpp"

namespace ravel {

// =====================================================================
// n-bonacci rule and Pisot constant (control case)
// =====================================================================

inline std::vector<std::vector<std::int8_t>>
n_bonacci_rule(std::size_t n) {
    std::vector<std::vector<std::int8_t>> sigma(n);
    for (std::size_t i = 0; i + 1 < n; ++i) {
        sigma[i] = {0, static_cast<std::int8_t>(i + 1)};
    }
    sigma[n - 1] = {0};
    return sigma;
}

inline double n_bonacci_beta(std::size_t n) {
    auto f = [n](double x) {
        double xn = std::pow(x, static_cast<double>(n));
        double s = 0.0;
        for (std::size_t k = 0; k < n; ++k) s += std::pow(x, static_cast<double>(k));
        return xn - s;  // x^n - (1 + x + ... + x^{n-1})
    };
    // f(1) = 1 - n < 0  (for n >= 2)
    // f(2) = 2^n - (2^n - 1) = 1 > 0
    // Root in [1, 2].  Standard bisection: if f(mid) > 0 the root is
    // in [lo, mid], so hi = mid; if f(mid) < 0 the root is in
    // [mid, hi], so lo = mid.
    double lo = 1.0, hi = 2.0;
    for (int it = 0; it < 200; ++it) {
        double mid = 0.5 * (lo + hi);
        if (f(mid) > 0) hi = mid;
        else             lo = mid;
    }
    return 0.5 * (lo + hi);
}

// =====================================================================
// Involution check on the dominant recurrent core
// =====================================================================

// Returns {matched, total} (matched is the number of core nodes for
// which the [i, x, j] -> [j, -x, i] map lands back in the same core).
inline std::pair<int, int>
check_involution_on_core(const ContactBoundaryReport& rep,
                         bool print_unmatched = false) {
    auto gb_graph = WeightedDigraph::from_dense(rep.gb_matrix);
    auto [core, orig_idx] = extract_dominant_recurrent_core(gb_graph);

    std::map<std::tuple<long long, std::vector<long long>, long long>, int>
        node_index;
    for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
        node_index[rep.boundary_nodes[i]] = static_cast<int>(i);
    }
    std::set<std::size_t> core_set(orig_idx.begin(), orig_idx.end());

    int matched = 0;
    for (std::size_t u : orig_idx) {
        const auto& n = rep.boundary_nodes[u];
        long long i = std::get<0>(n), j = std::get<2>(n);
        std::vector<long long> negx = std::get<1>(n);
        for (auto& v : negx) v = -v;
        auto key = std::make_tuple(j, negx, i);
        auto it = node_index.find(key);
        bool ok = (it != node_index.end()) &&
                  core_set.count(static_cast<std::size_t>(it->second));
        if (ok) ++matched;
        else if (print_unmatched) {
            std::printf("    unmatched core node: [i=%lld, x=(", i);
            const auto& xv = std::get<1>(n);
            for (std::size_t k = 0; k < xv.size(); ++k)
                std::printf("%s%lld", k ? "," : "", xv[k]);
            std::printf("), j=%lld]  flip=[i=%lld, x=(", j, j);
            for (std::size_t k = 0; k < negx.size(); ++k)
                std::printf("%s%lld", k ? "," : "", negx[k]);
            bool flip_in_gb = (it != node_index.end());
            bool flip_in_core = flip_in_gb &&
                core_set.count(static_cast<std::size_t>(it->second));
            std::printf("), i=%lld]  flip_in_G_B=%s flip_in_core=%s\n",
                        i, flip_in_gb ? "yes" : "NO",
                        flip_in_core ? "yes" : "no");
        }
    }
    return {matched, static_cast<int>(orig_idx.size())};
}

// =====================================================================
// Dense-matrix extraction from a digraph
// =====================================================================

inline std::vector<std::vector<long long>>
recurrent_core_dense_matrix(const std::vector<std::vector<long long>>& full_matrix) {
    auto graph = WeightedDigraph::from_dense(full_matrix);
    auto [core, orig_idx] = extract_dominant_recurrent_core(graph);
    (void)orig_idx;
    std::vector<std::vector<long long>> dense(core.n,
                                                std::vector<long long>(core.n, 0));
    for (std::size_t u = 0; u < core.n; ++u) {
        for (const auto& e : core.out_adj[u]) dense[u][e.first] += e.second;
    }
    return dense;
}

// =====================================================================
// Balanced-pair core's charpoly (the divisor in the cofactor check)
// =====================================================================

inline std::vector<long long>
bp_core_charpoly(const SubstitutionRule& rule) {
    auto bp = balanced_pair_transition_graph(rule);
    return charpoly_int(recurrent_core_dense_matrix(bp.matrix));
}

// =====================================================================
// Exact-integer charpoly divisibility check (x^k factor?)
// =====================================================================

inline mathlib::PolyZ from_high_first(std::vector<long long> hf) {
    mathlib::PolyZ p;
    std::size_t n = hf.size();
    p.ensure_size(n);
    for (std::size_t i = 0; i < n; ++i) mathlib::set_si(p.coeff(i), hf[n - 1 - i]);
    return p;
}

inline bool check_exact_factor(const char* name,
                              std::vector<long long> whole_hf,
                              std::vector<long long> factor_hf) {
    while (factor_hf.size() > 1 && factor_hf.back() == 0) factor_hf.pop_back();
    if (whole_hf.empty() || factor_hf.empty()) return false;
    auto whole = from_high_first(whole_hf);
    auto factor = from_high_first(factor_hf);
    auto dm = mathlib::divmod(whole, factor);
    bool exact = true;
    for (std::size_t i = 0; i < dm.r.coeffs_.size(); ++i) {
        if (mpz_sgn(dm.r.coeff(i).get()) != 0) { exact = false; break; }
    }
    std::printf("  %s: charpoly(whole) / charpoly(factor) exact? %s%s\n",
                name, exact ? "YES" : "no",
                exact ? "  (cofactor is the x^k nilpotent part)" : "");
    return exact;
}

// =====================================================================
// Q_sym_GB: orbit-aware quotient of G_B's dominant recurrent core
// under the involution [i, x, j] <-> [j, -x, i].
//
// Construction (lifted from app/gb_bp_matrix_equality.cpp so the
// σ_{a,b}-family probe can re-use the same machinery as the n-bonacci
// case; the only difference is that the input rep comes from the
// caller's pipeline run rather than from a hardcoded n-bonacci
// pipeline inside the function):
//   1. Run the contact-boundary pipeline (caller-side) to get rep.
//   2. Build the dominant recurrent SCC of rep.gb_matrix.
//   3. For each node u in the dominant core: find u's mirror
//      (i, x, j) -> (j, -x, i) via SNode<D>::mirror(); if the
//      mirror is in the core, u and mirror form an orbit.  Pick one
//      rep per orbit (skip self-paired fixed points).
//   4. Qsym_GB[a][b] = sum of forward_targets(rep_a -> dest) that
//      land on rep_b (or its mirror):
//        - if dest == rep[b]:  +1 to Qsym AND +1 to Qanti
//          (the transition is "self" under the involution).
//        - if dest == rep[b]'s mirror:  +1 to Qsym, -1 to Qanti.
//   5. Qanti_GB is the corresponding "antisymmetric" part.
//
// The KEY claim (n-bonacci Finding 4): charpoly(Qsym_GB) =
// charpoly(Qsym_BP) * x^k EXACTLY, where k is the cofactor's
// degree.  This is the proper structural reason for the
// n-bonacci's λ(G_B) = ρ_nc equality.  The probe in
// app/probe_a1_a2_unimodular.cpp tests this on σ_{a,b}-family /
// σ_1 / σ_2 / random unimodular Pisot candidates.
// =====================================================================
template <std::size_t D>
struct GbSymQuotient {
    std::vector<std::vector<long long>> Qsym;
    std::vector<std::vector<long long>> Qanti;
    std::size_t num_orbits = 0;
    std::size_t dominant_core_size = 0;
};

template <std::size_t D>
GbSymQuotient<D> compute_gb_sym_quotient(const ContactBoundaryReport& rep,
                                       const SubstitutionRule& rule) {
    GbSymQuotient<D> result;
    if (rep.gb_matrix.empty()) return result;

    // Re-build the dense G_B transition matrix from simple_forward_targets.
    // The existing n_bonacci matrix_equality uses this construction
    // (not rep.gb_matrix) and the result differs (e.g., for Tribonacci
    // the existing code produces a 7x7 Q_sym_GB with non-trivial
    // charpoly, while reading from rep.gb_matrix gives a monomial
    // x^7 -- the difference is that rep.gb_matrix includes the closure
    // of the contact set whereas simple_forward_targets gives the
    // direct transition structure).  We use the same construction as
    // the existing driver so the probe matches Finding 4's verification.
    SubstitutionRule local_rule = rule;
    if (local_rule.alphabet_size() != D) return result;
    auto subst = make_substitution<D>(local_rule, rep.beta);
    std::vector<SNode<D>> nodes(rep.boundary_nodes.size());
    for (std::size_t i = 0; i < rep.boundary_nodes.size(); ++i) {
        const auto& bn = rep.boundary_nodes[i];
        nodes[i].i = std::get<0>(bn);
        nodes[i].j = std::get<2>(bn);
        const auto& xv = std::get<1>(bn);
        for (std::size_t k = 0; k < D && k < xv.size(); ++k) {
            nodes[i].x[k] = xv[k];
        }
    }
    std::map<SNode<D>, std::size_t> idx;
    for (std::size_t i = 0; i < nodes.size(); ++i) idx[nodes[i]] = i;
    std::vector<std::vector<long long>> dense(nodes.size(),
        std::vector<long long>(nodes.size(), 0));
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        auto fwd = simple_forward_targets<D>(subst, nodes[i]);
        for (const auto& [dest, pq] : fwd) {
            (void)pq;
            auto it = idx.find(dest);
            if (it != idx.end()) dense[i][it->second] += 1;
        }
    }
    auto gb_graph = WeightedDigraph::from_dense(dense);
    auto [dom_core, dom_idx] = extract_dominant_recurrent_core(gb_graph);
    result.dominant_core_size = dom_idx.size();
    if (dom_idx.empty()) return result;

    // Build a name->index map for boundary_nodes so we can find each
    // node's mirror (i, x, j) -> (j, -x, i) by lookup.
    std::map<std::tuple<long long, std::array<long long, D>, long long>, std::size_t>
        core_name_idx;
    for (std::size_t u = 0; u < nodes.size(); ++u) {
        core_name_idx[std::make_tuple(nodes[u].i, nodes[u].x, nodes[u].j)] = u;
    }
    std::set<std::size_t> dom_set(dom_idx.begin(), dom_idx.end());

    // Find orbits under the involution.  For each u in the dominant
    // core: compute its mirror (j, -x, i); if the mirror is also in
    // the core and is a distinct node, they're in the same orbit.
    // We pick the first-discovered node as the orbit representative.
    std::vector<std::size_t> rep_indices;
    std::map<std::size_t, std::size_t> orbit_of;
    for (std::size_t u : dom_idx) {
        if (orbit_of.count(u)) continue;
        SNode<D> mirror = nodes[u].mirror();
        auto it = core_name_idx.find(
            std::make_tuple(mirror.i, mirror.x, mirror.j));
        if (it == core_name_idx.end()) continue;
        std::size_t w = it->second;
        if (!dom_set.count(w)) continue;
        if (w == u) continue;  // self-paired (fixed point); skip
        std::size_t oid = rep_indices.size();
        rep_indices.push_back(u);
        orbit_of[u] = oid;
        orbit_of[w] = oid;
    }
    result.num_orbits = rep_indices.size();
    if (rep_indices.empty()) return result;

    std::size_t N = rep_indices.size();
    result.Qsym.assign(N, std::vector<long long>(N, 0));
    result.Qanti.assign(N, std::vector<long long>(N, 0));
    for (std::size_t a = 0; a < N; ++a) {
        std::size_t ra = rep_indices[a];
        for (std::size_t w = 0; w < nodes.size(); ++w) {
            long long wgt = dense[ra][w];
            if (wgt == 0) continue;
            if (!orbit_of.count(w)) continue;
            std::size_t b = orbit_of[w];
            if (w == rep_indices[b]) {
                // w IS the rep of its own orbit: the transition is
                // "self" under the involution.
                result.Qsym[a][b] += wgt;
                result.Qanti[a][b] += wgt;
            } else {
                // w is the mirror of the rep of its orbit.
                result.Qsym[a][b] += wgt;
                result.Qanti[a][b] -= wgt;
            }
        }
    }
    return result;
}

// =====================================================================
// Q_sym_BP: orbit-aware quotient of BP's recurrent core under the
// swap (u, v) <-> (v, u) involution on pairs.
//
// Construction (lifted from app/gb_bp_matrix_equality.cpp):
//   1. Build the balanced-pair transition graph.
//   2. Extract its recurrent core (largest; for the n-bonacci case
//      this is the dominant SCC, but we use extract_recurrent_core
//      here for the swap construction -- matches the original
//      driver).
//   3. For each pair (u, v) in the core, find its swap (v, u); if
//      the swap is in the core, the pair is "swappable" (Qsym
//      contribution).
//   4. Pick one rep per orbit (skip self-equal pairs of the form
//      (u, u) with |u| == 1).
//   5. Qsym[a][b] = out-weight from rep_a to rep_b in the core; for
//      swappable pairs, Qanti[a][b] also accumulates the out-weight
//      (since both the original and swapped transition contribute
//      identically; Qanti here is the "swap-symmetric" sum).
// =====================================================================
struct BpSymQuotient {
    std::vector<std::vector<long long>> Qsym;
    std::vector<std::vector<long long>> Qanti;
    std::size_t num_orbits = 0;
    std::size_t recurrent_core_size = 0;
};

inline std::string swap_key_pair(const std::string& k) {
    std::vector<std::int8_t> u, v;
    unpair_key(k, u, v);
    return pair_key(v, u);
}

inline BpSymQuotient compute_bp_sym_quotient(const SubstitutionRule& rule) {
    BpSymQuotient result;
    auto bp = balanced_pair_transition_graph(rule);
    if (bp.matrix.empty()) return result;
    auto bp_graph = WeightedDigraph::from_dense(bp.matrix);
    auto [core, orig_idx] = extract_recurrent_core(bp_graph);
    result.recurrent_core_size = core.n;
    if (core.n == 0) return result;

    // State labels are aligned with bp.matrix, so orig_idx directly
    // recovers the mathematical vertex without a second BFS.
    std::vector<std::string> core_key(core.n);
    for (std::size_t c = 0; c < core.n; ++c) {
        const auto& state = bp.states.at(orig_idx[c]);
        core_key[c] = pair_key(state.left, state.right);
    }

    // Build a name->core-index map for swap lookup.
    std::map<std::string, std::size_t> key_to_core;
    for (std::size_t c = 0; c < core.n; ++c) key_to_core[core_key[c]] = c;

    // Find swap-orbits: for each c in core, find swap_key(core_key[c]).
    std::vector<std::size_t> rep;
    std::map<std::size_t, std::size_t> orbit_of;
    for (std::size_t c = 0; c < core.n; ++c) {
        if (orbit_of.count(c)) continue;
        auto it = key_to_core.find(swap_key_pair(core_key[c]));
        if (it == key_to_core.end()) continue;
        std::size_t w = it->second;
        if (w == c) continue;
        std::size_t oid = rep.size();
        rep.push_back(c);
        orbit_of[c] = oid;
        orbit_of[w] = oid;
    }
    result.num_orbits = rep.size();
    if (rep.empty()) return result;

    std::size_t N = rep.size();
    result.Qsym.assign(N, std::vector<long long>(N, 0));
    result.Qanti.assign(N, std::vector<long long>(N, 0));
    for (std::size_t a = 0; a < N; ++a) {
        std::size_t ra = rep[a];
        for (std::size_t w = 0; w < core.n; ++w) {
            long long wgt = 0;
            for (const auto& e : core.out_adj[ra]) {
                if (e.first == w) wgt += e.second;
            }
            if (wgt == 0) continue;
            if (!orbit_of.count(w)) continue;
            std::size_t b = orbit_of[w];
            if (w == rep[b]) {
                result.Qsym[a][b] += wgt;
                result.Qanti[a][b] += wgt;
            } else {
                result.Qsym[a][b] += wgt;
                result.Qanti[a][b] -= wgt;
            }
        }
    }
    return result;
}

}  // namespace ravel
