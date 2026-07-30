// balanced_pair.hpp
//
// Hollander-Solomyak / Barge-Diamond style balanced-pair algorithm
// for Pisot substitution certification.  Mirrors the reference
// `python/balanced_pair.py`:
//   seed = ((0,1), (1,0))
//   reduce_pair(u, v): split at every prefix-balanced point into
//     irreducible chunks (u', v').  Each chunk has equal Parikh
//     vectors and is non-empty on both sides, with no internal
//     balanced prefix.
//   The BFS explores irreducible pairs under sigma + reduction,
//     bounded by (max_pairs, max_len).  Terminates with `certified`
//     iff every reachable irreducible pair reaches at least one
//     coincidence chunk; pure-point iff certified (ABBLS Thm 5.3).
//
// rho_nc(subst, ...): the largest-magnitude eigenvalue of the
//   irreducible-noncoincidence transition matrix.  Mirrors
//   `python/closure_test.py::rho_nc`.  Returns +inf if the BFS
//   does not terminate within caps.
//
// Implementation note: we use string-serialised keys rather than
// vector< int8_t > keys for the index maps, which makes hash,
// equality, and memory layout robust against the heap churn from
// repeated vector copy in the tight BFS.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ravel/substitution.hpp"

namespace ravel {

// Split (u, v) into irreducible balanced chunks.  A chunk is cut
// at the EARLIEST position after which both Parikh vectors equal.
inline std::vector<std::pair<std::vector<std::int8_t>,
                              std::vector<std::int8_t>>>
reduce_pair(const std::vector<std::int8_t>& u,
            const std::vector<std::int8_t>& v,
            std::size_t nl) {
    std::vector<std::pair<std::vector<std::int8_t>,
                          std::vector<std::int8_t>>> chunks;
    if (u.size() != v.size()) {
        return chunks;
    }
    std::vector<long long> cu(nl, 0), cv(nl, 0);
    std::size_t start = 0;
    for (std::size_t i = 0; i < u.size(); ++i) {
        cu[static_cast<std::size_t>(u[i])] += 1;
        cv[static_cast<std::size_t>(v[i])] += 1;
        if (cu == cv) {
            chunks.emplace_back(
                std::vector<std::int8_t>(u.begin() + start, u.begin() + i + 1),
                std::vector<std::int8_t>(v.begin() + start, v.begin() + i + 1));
            start = i + 1;
            std::fill(cu.begin(), cu.end(), 0);
            std::fill(cv.begin(), cv.end(), 0);
        }
    }
    return chunks;
}

inline std::pair<std::vector<std::int8_t>, std::vector<std::int8_t>>
sigma_pair(const SubstitutionRule& rule,
           const std::pair<std::vector<std::int8_t>,
                           std::vector<std::int8_t>>& p) {
    return { rule.apply_once(p.first), rule.apply_once(p.second) };
}

// String-serialised key for a (u, v) pair.
inline std::string pair_key(const std::vector<std::int8_t>& u,
                            const std::vector<std::int8_t>& v) {
    std::string s;
    s.reserve(u.size() + v.size() + 2);
    s.push_back('#');
    for (auto c : u) { s.push_back(static_cast<char>('a' + c)); s.push_back(','); }
    s.push_back('#');
    for (auto c : v) { s.push_back(static_cast<char>('a' + c)); s.push_back(','); }
    s.push_back('#');
    return s;
}

inline void unpair_key(const std::string& s,
                       std::vector<std::int8_t>& u,
                       std::vector<std::int8_t>& v) {
    u.clear(); v.clear();
    std::size_t i = 0;
    ++i;  // skip leading '#'
    while (i < s.size() && s[i] != '#') {
        if (s[i] >= 'a' && s[i] <= 'z') {
            u.push_back(static_cast<std::int8_t>(s[i] - 'a'));
        }
        ++i;
    }
    ++i;  // skip '#'
    while (i < s.size() && s[i] != '#') {
        if (s[i] >= 'a' && s[i] <= 'z') {
            v.push_back(static_cast<std::int8_t>(s[i] - 'a'));
        }
        ++i;
    }
}

struct BalancedPairCertificate {
    bool certified;          // true iff every irreducible pair reaches a coincidence
    bool terminated;         // true iff the BFS finished within (max_pairs, max_len)
    std::size_t n_irreducible;
    std::size_t n_coincidences;
    std::size_t max_image_length;
};

inline BalancedPairCertificate balanced_pair_certify(
    const SubstitutionRule& rule,
    std::size_t max_pairs = 20000,
    std::size_t max_len = 4000) {
    const std::size_t nl = rule.alphabet_size();
    BalancedPairCertificate out{false, true, 0, 0, 0};

    auto initial = reduce_pair({0, 1}, {1, 0}, nl);

    using Map = std::map<std::string, std::size_t>;
    Map seen;
    std::map<std::string, std::vector<std::size_t>> succ;
    std::deque<std::size_t> queue;
    std::vector<std::string> order;

    for (auto& ch : initial) {
        std::string k = pair_key(ch.first, ch.second);
        if (seen.find(k) == seen.end()) {
            seen[k] = order.size();
            order.push_back(k);
            queue.push_back(order.size() - 1);
        }
    }

    while (!queue.empty()) {
        std::size_t idx = queue.front();
        queue.pop_front();
        std::string k = order[idx];
        std::vector<std::int8_t> u, v;
        unpair_key(k, u, v);
        auto img = sigma_pair(rule, { u, v });
        out.max_image_length = std::max(out.max_image_length, img.first.size());
        if (img.first.size() > max_len || seen.size() > max_pairs) {
            out.terminated = false;
            break;
        }
        auto chunks = reduce_pair(img.first, img.second, nl);
        std::vector<std::size_t> children;
        for (auto& ch : chunks) {
            std::string ck = pair_key(ch.first, ch.second);
            auto it = seen.find(ck);
            std::size_t child_idx;
            if (it == seen.end()) {
                child_idx = order.size();
                seen[ck] = child_idx;
                order.push_back(std::move(ck));
                queue.push_back(child_idx);
            } else {
                child_idx = it->second;
            }
            children.push_back(child_idx);
        }
        succ[k] = std::move(children);
        out.n_irreducible = order.size();
    }

    if (out.terminated) {
        std::unordered_set<std::size_t> coin;
        for (std::size_t i = 0; i < order.size(); ++i) {
            std::vector<std::int8_t> u, v;
            unpair_key(order[i], u, v);
            if (u.size() == 1 && u == v) {
                coin.insert(i);
                ++out.n_coincidences;
            }
        }
        std::unordered_set<std::size_t> reach = coin;
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& kv : succ) {
                auto pi_it = seen.find(kv.first);
                if (pi_it == seen.end()) continue;
                std::size_t pi = pi_it->second;
                if (reach.count(pi)) continue;
                for (std::size_t cj : kv.second) {
                    if (reach.count(cj)) {
                        reach.insert(pi);
                        changed = true;
                        break;
                    }
                }
            }
        }
        out.certified = (reach.size() == order.size());
    }
    return out;
}

// Largest-magnitude eigenvalue of a non-negative matrix via the
// power method with Rayleigh-quotient refinement.  Exact for
// irreducible non-negative matrices with a single dominant
// positive eigenvalue.
inline double largest_abs_eigenvalue(
    const std::vector<std::vector<double>>& A,
    int iters = 8000,
    double tol = 1e-12) {
    const std::size_t n = A.size();
    if (n == 0) return 0.0;
    std::vector<double> v(n, 1.0 / static_cast<double>(n));
    std::vector<double> w(n, 0.0);
    double lambda = 0.0;
    for (int it = 0; it < iters; ++it) {
        double norm = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            double s = 0.0;
            for (std::size_t j = 0; j < n; ++j) s += A[i][j] * v[j];
            w[i] = s;
            norm = std::max(norm, std::abs(s));
        }
        if (norm == 0.0) return 0.0;
        for (auto& x : w) x /= norm;
        double num = 0.0, den = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            double s = 0.0;
            for (std::size_t j = 0; j < n; ++j) s += A[i][j] * w[j];
            num += w[i] * s;
            den += w[i] * w[i];
        }
        double lam_new = (den > 0.0) ? num / den : lambda;
        if (it > 0 && std::abs(lam_new - lambda) < tol * std::abs(lam_new) + tol) {
            return lam_new;
        }
        lambda = lam_new;
        v = w;
    }
    return lambda;
}

// rho_nc(subst): largest-magnitude eigenvalue of the
// (non-coincidence, irreducible) transition matrix.  Mirrors
// `python/closure_test.py::rho_nc` exactly.  Returns +inf when
// the BFS does not terminate within the given caps.
inline double rho_nc(const SubstitutionRule& rule,
                     std::size_t max_pairs = 20000,
                     std::size_t max_len   = 60000) {
    const std::size_t nl = rule.alphabet_size();
    using Map = std::map<std::string, std::size_t>;
    Map seen;
    std::map<std::string, std::vector<std::size_t>> succ;
    std::deque<std::size_t> queue;
    std::vector<std::string> order;

    {
        auto initial = reduce_pair({0, 1}, {1, 0}, nl);
        for (auto& ch : initial) {
            std::string k = pair_key(ch.first, ch.second);
            if (seen.find(k) == seen.end()) {
                seen[k] = order.size();
                order.push_back(std::move(k));
                queue.push_back(order.size() - 1);
            }
        }
    }

    while (!queue.empty()) {
        std::size_t idx = queue.front();
        queue.pop_front();
        std::string k = order[idx];
        std::vector<std::int8_t> u, v;
        unpair_key(k, u, v);
        auto img = sigma_pair(rule, { u, v });
        if (img.first.size() > max_len || seen.size() > max_pairs) {
            return std::numeric_limits<double>::infinity();
        }
        auto chunks = reduce_pair(img.first, img.second, nl);
        std::vector<std::size_t> children;
        for (auto& ch : chunks) {
            std::string ck = pair_key(ch.first, ch.second);
            auto it = seen.find(ck);
            std::size_t child_idx;
            if (it == seen.end()) {
                child_idx = order.size();
                seen[ck] = child_idx;
                order.push_back(std::move(ck));
                queue.push_back(child_idx);
            } else {
                child_idx = it->second;
            }
            children.push_back(child_idx);
        }
        succ[k] = std::move(children);
    }

    std::unordered_map<std::size_t, std::size_t> idx_map;
    std::vector<std::size_t> noncoin;
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::vector<std::int8_t> u, v;
        unpair_key(order[i], u, v);
        if (!(u.size() == 1 && u == v)) {
            idx_map[i] = noncoin.size();
            noncoin.push_back(i);
        }
    }
    const std::size_t n = noncoin.size();
    if (n == 0) return 0.0;

    std::vector<std::vector<double>> T(n, std::vector<double>(n, 0.0));
    for (std::size_t i : noncoin) {
        std::string ok = order[i];
        auto it = succ.find(ok);
        if (it == succ.end()) continue;
        for (std::size_t cj : it->second) {
            auto jt = idx_map.find(cj);
            if (jt != idx_map.end()) {
                T[jt->second][idx_map[i]] += 1.0;
            }
        }
    }
    return largest_abs_eigenvalue(T);
}

// BalancedPairTransitionGraph: exposes the noncoincidence transition
// graph that rho_nc() builds internally, in the SAME dense
// adjacency/multiplicity format as ContactBoundaryReport::gb_matrix
// (matrix[i][j] = edge multiplicity i -> j, natural "source row,
// target column" direction -- note this is the TRANSPOSE of the `T`
// matrix rho_nc() feeds to largest_abs_eigenvalue, which uses the
// opposite row/column convention; transposing doesn't change the
// characteristic polynomial or the Perron root, so both give the
// same rho_nc, but callers building a ravel::WeightedDigraph
// (graph_divisor.hpp) from this matrix get genuine directed edges in
// the automaton's own forward direction, not the reversed one).
//
// Built for `docs/DIRECTION_AND_OPEN_THREADS.md`'s central-conjecture
// work: this is one of the two graphs app/test_bp_gb_divisor.cpp
// feeds to ravel::compare_divisors alongside G_B's own gb_matrix,
// to test whether the two graphs share a common coarsest equitable
// partition (which would force bp_rho_nc == lambda(G_B) as a
// structural consequence of Perron-Frobenius rather than an
// empirical per-candidate coincidence).
//
// Duplicates rho_nc()'s BFS rather than factoring it out, matching
// this file's existing style (balanced_pair_certify and rho_nc are
// likewise two independent BFS copies) -- see the top-of-file note.
struct BalancedPairState {
    std::vector<std::int8_t> left;
    std::vector<std::int8_t> right;
};

struct BalancedPairTransitionGraph {
    bool terminated = true;    // false iff the BFS hit (max_pairs, max_len)
    std::vector<std::vector<long long>> matrix;  // n x n, noncoincidence pairs only
    // Aligned with matrix rows/columns. Exposing the irreducible words
    // prevents research drivers from duplicating the BFS merely to
    // recover the mathematical meaning of a vertex.
    std::vector<BalancedPairState> states;
};

inline BalancedPairTransitionGraph balanced_pair_transition_graph(
    const SubstitutionRule& rule,
    std::size_t max_pairs = 20000,
    std::size_t max_len   = 60000) {
    const std::size_t nl = rule.alphabet_size();
    BalancedPairTransitionGraph out;

    using Map = std::map<std::string, std::size_t>;
    Map seen;
    std::map<std::string, std::vector<std::size_t>> succ;
    std::deque<std::size_t> queue;
    std::vector<std::string> order;

    {
        auto initial = reduce_pair({0, 1}, {1, 0}, nl);
        for (auto& ch : initial) {
            std::string k = pair_key(ch.first, ch.second);
            if (seen.find(k) == seen.end()) {
                seen[k] = order.size();
                order.push_back(std::move(k));
                queue.push_back(order.size() - 1);
            }
        }
    }

    while (!queue.empty()) {
        std::size_t idx = queue.front();
        queue.pop_front();
        std::string k = order[idx];
        std::vector<std::int8_t> u, v;
        unpair_key(k, u, v);
        auto img = sigma_pair(rule, { u, v });
        if (img.first.size() > max_len || seen.size() > max_pairs) {
            out.terminated = false;
            return out;
        }
        auto chunks = reduce_pair(img.first, img.second, nl);
        std::vector<std::size_t> children;
        for (auto& ch : chunks) {
            std::string ck = pair_key(ch.first, ch.second);
            auto it = seen.find(ck);
            std::size_t child_idx;
            if (it == seen.end()) {
                child_idx = order.size();
                seen[ck] = child_idx;
                order.push_back(std::move(ck));
                queue.push_back(child_idx);
            } else {
                child_idx = it->second;
            }
            children.push_back(child_idx);
        }
        succ[k] = std::move(children);
    }

    std::unordered_map<std::size_t, std::size_t> idx_map;
    std::vector<std::size_t> noncoin;
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::vector<std::int8_t> u, v;
        unpair_key(order[i], u, v);
        if (!(u.size() == 1 && u == v)) {
            idx_map[i] = noncoin.size();
            noncoin.push_back(i);
        }
    }
    const std::size_t n = noncoin.size();
    out.matrix.assign(n, std::vector<long long>(n, 0));
    out.states.reserve(n);
    for (std::size_t i : noncoin) {
        BalancedPairState state;
        unpair_key(order[i], state.left, state.right);
        out.states.push_back(std::move(state));
    }
    for (std::size_t i : noncoin) {
        std::string ok = order[i];
        auto it = succ.find(ok);
        if (it == succ.end()) continue;
        for (std::size_t cj : it->second) {
            auto jt = idx_map.find(cj);
            if (jt != idx_map.end()) {
                out.matrix[idx_map[i]][jt->second] += 1;
            }
        }
    }
    return out;
}

}  // namespace ravel
