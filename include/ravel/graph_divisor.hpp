#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unordered_map>
#include <utility>
#include <unistd.h>
#include <vector>
//
// Coarsest-equitable-partition ("graph divisor") machinery for
// weighted directed graphs, built to test a specific hypothesis
// about `docs/DIRECTION_AND_OPEN_THREADS.md`'s central conjecture
// (BP-rho_nc == lambda(G_B)): if the balanced-pair automaton's
// transition graph and the boundary graph G_B admit a COMMON
// equitable partition (i.e. one is a "divisor"/quotient of a graph
// that covers both, or the two graphs are themselves divisor-
// compatible), then their Perron (dominant) eigenvalues are
// EQUAL as a consequence of elementary Perron-Frobenius theory --
// not as an empirical coincidence that needs re-checking survey
// candidate by survey candidate.
//
// Background (why this works): for a partition P of a nonnegative
// weighted digraph's vertex set into classes, P is EQUITABLE if
// every vertex in a class C has the same total out-weight to each
// other class C' (and, for our purposes, the same total in-weight
// from each other class).  When P is equitable, the |classes| x
// |classes| "quotient matrix" Q (row a, col b = the common out-
// weight from class a to class b) has the property that every
// eigenvalue of Q is also an eigenvalue of the full adjacency
// matrix (a classical fact -- the quotient's eigenvectors lift to
// constant-on-classes eigenvectors of the full matrix).  For an
// irreducible nonnegative matrix, the Perron root is simple and is
// therefore preserved exactly under this quotient operation: the
// quotient of an irreducible nonnegative matrix under any equitable
// partition has the SAME Perron eigenvalue as the original matrix.
//
// This module:
//   1. Computes the (unique) COARSEST equitable partition of a
//      weighted digraph via iterated color refinement (the standard
//      1-dimensional Weisfeiler-Leman algorithm, generalized to
//      weighted edges and both in- and out-neighborhoods).  This is
//      polynomial time and, on sparse graphs (bounded out-degree),
//      near-linear per round.
//   2. Builds the quotient matrix under that partition.
//   3. Compares two graphs' quotients (typically much smaller than
//      the original graphs) via their EXACT integer characteristic
//      polynomials (reusing `charpoly_int` from `barge.hpp`), which
//      is the concrete, checkable necessary condition for the
//      "common divisor" hypothesis: if two graphs' coarsest
//      quotients have DIFFERENT characteristic polynomials, no
//      common equitable-partition-compatible cover can give them
//      equal Perron roots via this mechanism (though a different,
//      non-equitable relationship could still coincidentally
//      produce equal roots -- this test finds evidence FOR a
//      structural explanation, it does not itself constitute one
//      unless it succeeds).
//
// This does NOT replace a full graph-isomorphism-search for a
// specific witnessing map between two given graphs of different
// sizes (e.g. "is graph A literally the quotient of graph B") --
// that is a harder search problem in general.  What it gives for
// free, cheaply, on any single graph, is its OWN canonical coarsest
// quotient, which is exactly the object whose eigenvalue must equal
// the original's Perron root.  Comparing two graphs' canonical
// quotients is the practical first-cut test this project needs.

#include "ravel/barge.hpp"  // charpoly_int

namespace ravel {

// A weighted directed graph on {0, ..., n-1}, stored as adjacency
// lists (both directions, since equitable-partition refinement needs
// in- and out-neighborhoods).  Multi-edges are pre-summed into a
// single (target, weight) pair by add_edge when possible, but callers
// may also push multiple parallel entries; refinement sums weights
// per color class either way, so it makes no difference to the
// signatures used for refinement.
struct WeightedDigraph {
    std::size_t n = 0;
    std::vector<std::vector<std::pair<std::size_t, long long>>> out_adj;
    std::vector<std::vector<std::pair<std::size_t, long long>>> in_adj;

    explicit WeightedDigraph(std::size_t n_ = 0) : n(n_), out_adj(n_), in_adj(n_) {}

    void add_edge(std::size_t u, std::size_t v, long long w = 1) {
        if (u >= n || v >= n) {
            throw std::out_of_range(
                "WeightedDigraph::add_edge: vertex index out of range");
        }
        out_adj[u].emplace_back(v, w);
        in_adj[v].emplace_back(u, w);
    }

    // Build from a dense n x n adjacency/multiplicity matrix (the
    // format `ContactBoundaryReport::gb_matrix` and the balanced-pair
    // transition matrix are both stored in).
    static WeightedDigraph from_dense(
        const std::vector<std::vector<long long>>& M) {
        std::size_t nn = M.size();
        WeightedDigraph g(nn);
        for (std::size_t i = 0; i < nn; ++i) {
            if (M[i].size() != nn) {
                throw std::invalid_argument(
                    "WeightedDigraph::from_dense: matrix must be square");
            }
            for (std::size_t j = 0; j < nn; ++j) {
                if (M[i][j] != 0) g.add_edge(i, j, M[i][j]);
            }
        }
        return g;
    }

    // Stream-load a dense n x n matrix from a "GBMT0002"-format file
    // on disk. Returns nullopt on any read failure, magic mismatch,
    // N²-vs-file-size sanity mismatch, checksum mismatch, or N=0.
    //
    // Memory profile: a single row buffer of 8*N bytes (e.g., 60 KB at
    // N=7500, 240 KB at N=30K) plus the digraph's own adjacency
    // storage. Crucially, the full N*N*8-byte dense matrix is NEVER
    // materialized in heap, so we can construct gigabyte-class G_B
    // digraphs on memory-tight sessions (per session MEMORY_POLICY's
    // 70% ceiling).
    //
    // File format (streamable):
    //   8 bytes magic               "GBMT0002"
    //   8 bytes size_t N
    //   8 bytes uint64_t N²
    //   N²*8 bytes row-major longs
    //   8 bytes uint64_t FNV-1a-64 checksum of magic..end-of-data
    //
    // Usage: `g = WeightedDigraph::from_dense_file(path).value();`
    static std::optional<WeightedDigraph> from_dense_file(
            const std::string& path) {
        int fd = ::open(path.c_str(), O_RDONLY);
        if (fd < 0) return std::nullopt;

        // FNV-1a 64-bit (matches the canonical save side in
        // app/reaudit_gb_dominant_scc.cpp).
        auto fnv1a = [](const void* data, std::size_t n,
                       uint64_t h = 0xcbf29ce484222325ULL) {
            const uint8_t* p = static_cast<const uint8_t*>(data);
            for (std::size_t i = 0; i < n; ++i) {
                h ^= p[i];
                h *= 0x100000001b3ULL;
            }
            return h;
        };
        auto read_all = [&](void* p, std::size_t n) -> bool {
            uint8_t* q = static_cast<uint8_t*>(p);
            while (n > 0) {
                ssize_t r = ::read(fd, q, n);
                if (r <= 0) return false;
                q += r; n -= static_cast<std::size_t>(r);
            }
            return true;
        };

        char magic[8];
        if (!read_all(magic, 8)) { ::close(fd); return std::nullopt; }
        if (std::memcmp(magic, "GBMT0002", 8) != 0) {
            ::close(fd); return std::nullopt;
        }
        uint64_t h = fnv1a(magic, 8);

        std::size_t N;
        if (!read_all(&N, sizeof(N))) { ::close(fd); return std::nullopt; }
        h = fnv1a(&N, sizeof(N), h);
        if (N == 0 || N > 1000000ULL) { ::close(fd); return std::nullopt; }

        uint64_t nn;
        if (!read_all(&nn, sizeof(nn))) { ::close(fd); return std::nullopt; }
        h = fnv1a(&nn, sizeof(nn), h);
        if (nn != static_cast<uint64_t>(N) *
                   static_cast<uint64_t>(N)) {
            ::close(fd); return std::nullopt;
        }

        WeightedDigraph g(N);
        std::vector<long long> rowbuf(N);
        for (std::size_t i = 0; i < N; ++i) {
            if (!read_all(rowbuf.data(), sizeof(long long) * N)) {
                ::close(fd); return std::nullopt;
            }
            h = fnv1a(rowbuf.data(), sizeof(long long) * N, h);
            for (std::size_t j = 0; j < N; ++j) {
                if (rowbuf[j] != 0) g.add_edge(i, j, rowbuf[j]);
            }
        }

        uint64_t want;
        if (!read_all(&want, sizeof(want))) { ::close(fd); return std::nullopt; }
        ::close(fd);
        if (want != h) return std::nullopt;
        return g;
    }
};

struct EquitablePartition {
    std::vector<int> color;   // color[v] in [0, num_classes)
    int num_classes = 0;
};

// Iterated color refinement (weighted, both-directions 1-WL).
// Starts from `initial_color` (default: everyone in one class) and
// repeatedly splits classes by (own color, weighted out-histogram of
// neighbor colors, weighted in-histogram of neighbor colors) until
// the number of classes stops growing -- which, because each round
// only ever REFINES the previous partition (every new class is a
// subset of an old class), means the partition itself has stopped
// changing, not just its class count.  This is the standard
// termination argument for color refinement / 1-WL and needs no
// separate fixed-point check.
inline EquitablePartition coarsest_equitable_partition(
    const WeightedDigraph& g,
    std::vector<int> initial_color = {}) {
    const std::size_t n = g.n;
    if (initial_color.empty()) initial_color.assign(n, 0);
    if (initial_color.size() != n) {
        throw std::invalid_argument(
            "coarsest_equitable_partition: initial color size mismatch");
    }

    // Normalize the initial coloring to a dense 0..k-1 range.
    std::vector<int> color(n);
    {
        std::map<int, int> remap;
        for (std::size_t v = 0; v < n; ++v) {
            auto it = remap.find(initial_color[v]);
            if (it == remap.end()) {
                int id = static_cast<int>(remap.size());
                remap[initial_color[v]] = id;
                color[v] = id;
            } else {
                color[v] = it->second;
            }
        }
    }
    int num_classes = 0;
    {
        std::map<int, int> remap;
        for (int c : color) remap.emplace(c, 0);
        num_classes = static_cast<int>(remap.size());
    }

    // At most n rounds can ever be needed (each round that changes
    // anything strictly increases the class count, capped at n).
    for (std::size_t round = 0; round <= n + 1; ++round) {
        std::vector<std::string> sig(n);
        for (std::size_t v = 0; v < n; ++v) {
            std::map<int, long long> out_hist, in_hist;
            for (const auto& e : g.out_adj[v]) out_hist[color[e.first]] += e.second;
            for (const auto& e : g.in_adj[v]) in_hist[color[e.first]] += e.second;
            std::string s = std::to_string(color[v]);
            s += "|o";
            for (const auto& kv : out_hist) {
                s += std::to_string(kv.first) + ":" + std::to_string(kv.second) + ",";
            }
            s += "|i";
            for (const auto& kv : in_hist) {
                s += std::to_string(kv.first) + ":" + std::to_string(kv.second) + ",";
            }
            sig[v] = std::move(s);
        }

        std::vector<std::string> uniq = sig;
        std::sort(uniq.begin(), uniq.end());
        uniq.erase(std::unique(uniq.begin(), uniq.end()), uniq.end());
        std::map<std::string, int> remap;
        for (std::size_t i = 0; i < uniq.size(); ++i) remap[uniq[i]] = static_cast<int>(i);

        std::vector<int> new_color(n);
        for (std::size_t v = 0; v < n; ++v) new_color[v] = remap[sig[v]];
        int new_num_classes = static_cast<int>(uniq.size());

        if (new_num_classes == num_classes) {
            // No split occurred this round (class count can only ever
            // stay flat or grow under this refinement); fixed point.
            color = std::move(new_color);
            num_classes = new_num_classes;
            break;
        }
        color = std::move(new_color);
        num_classes = new_num_classes;
    }

    EquitablePartition ep;
    ep.color = std::move(color);
    ep.num_classes = num_classes;
    return ep;
}

// Verifies (by brute-force re-check, not assumption) that a given
// partition really is equitable for a given graph -- every vertex in
// a class has an identical weighted out-histogram AND in-histogram
// over classes.  Used by the test suite to confirm
// coarsest_equitable_partition's output rather than trusting the
// refinement loop's termination logic blindly.
inline bool is_equitable(const WeightedDigraph& g, const EquitablePartition& p) {
    const int k = p.num_classes;
    if (k < 0 || p.color.size() != g.n) return false;
    for (int c : p.color) {
        if (c < 0 || c >= k) return false;
    }
    std::vector<std::vector<long long>> out_ref(k, std::vector<long long>(k, -1));
    std::vector<std::vector<long long>> in_ref(k, std::vector<long long>(k, -1));
    for (std::size_t v = 0; v < g.n; ++v) {
        int c = p.color[v];
        std::vector<long long> out_row(k, 0), in_row(k, 0);
        for (const auto& e : g.out_adj[v]) out_row[p.color[e.first]] += e.second;
        for (const auto& e : g.in_adj[v]) in_row[p.color[e.first]] += e.second;
        for (int b = 0; b < k; ++b) {
            if (out_ref[c][b] == -1) {
                out_ref[c][b] = out_row[b];
            } else if (out_ref[c][b] != out_row[b]) {
                return false;
            }
            if (in_ref[c][b] == -1) {
                in_ref[c][b] = in_row[b];
            } else if (in_ref[c][b] != in_row[b]) {
                return false;
            }
        }
    }
    return true;
}

// The quotient matrix under an equitable partition: Q[a][b] is the
// common out-weight from class a to class b. Public callers receive an
// explicit error for malformed or non-equitable partitions.
inline std::vector<std::vector<long long>> quotient_matrix(
    const WeightedDigraph& g, const EquitablePartition& p) {
    if (!is_equitable(g, p)) {
        throw std::invalid_argument(
            "quotient_matrix: partition is malformed or not equitable");
    }
    const int k = p.num_classes;
    std::vector<std::vector<long long>> Q(k, std::vector<long long>(k, 0));
    std::vector<long long> rep(k, -1);
    for (std::size_t v = 0; v < g.n; ++v) {
        if (rep[p.color[v]] == -1) rep[p.color[v]] = static_cast<long long>(v);
    }
    for (int a = 0; a < k; ++a) {
        if (rep[a] == -1) continue;  // empty class, shouldn't happen
        std::size_t v = static_cast<std::size_t>(rep[a]);
        for (const auto& e : g.out_adj[v]) {
            Q[a][p.color[e.first]] += e.second;
        }
    }
    return Q;
}

// Power-iteration dominant (Perron) eigenvalue estimate for a
// nonnegative integer matrix, used as a fallback when a quotient is
// too large for charpoly_int's O(k^4) Faddeev-LeVerrier cost to be
// practical (k in the low hundreds is already slow; G_B-scale
// quotients can be in the thousands).  Mirrors
// balanced_pair.hpp::largest_abs_eigenvalue but kept local to avoid
// a cross-header dependency, since graph_divisor.hpp is meant to be
// usable on any weighted digraph, not just balanced-pair automata.
inline double dominant_eigenvalue_estimate(
    const std::vector<std::vector<long long>>& Q,
    int iters = 4000, double tol = 1e-12) {
    const std::size_t k = Q.size();
    if (k == 0) return 0.0;
    std::vector<double> v(k, 1.0 / static_cast<double>(k));
    std::vector<double> w(k, 0.0);
    double lambda = 0.0;
    for (int it = 0; it < iters; ++it) {
        double norm = 0.0;
        for (std::size_t i = 0; i < k; ++i) {
            double s = 0.0;
            for (std::size_t j = 0; j < k; ++j) {
                if (Q[i][j] != 0) s += static_cast<double>(Q[i][j]) * v[j];
            }
            w[i] = s;
            norm = std::max(norm, std::abs(s));
        }
        if (norm == 0.0) return 0.0;
        for (auto& x : w) x /= norm;
        double num = 0.0, den = 0.0;
        for (std::size_t i = 0; i < k; ++i) {
            double s = 0.0;
            for (std::size_t j = 0; j < k; ++j) {
                if (Q[i][j] != 0) s += static_cast<double>(Q[i][j]) * w[j];
            }
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

// Sparse-graph version of dominant_eigenvalue_estimate: power
// iteration directly on a WeightedDigraph's adjacency lists, O(edges)
// per iteration rather than O(n^2).  Needed for graphs with tens of
// thousands of nodes but small average degree (e.g. the adelic
// zero-expansion graph from coincidence_and_property_f.hpp), where
// even allocating a dense n x n matrix is infeasible.
inline double dominant_eigenvalue_estimate_sparse(
    const WeightedDigraph& g, int iters = 8000, double tol = 1e-12) {
    const std::size_t n = g.n;
    if (n == 0) return 0.0;
    std::vector<double> v(n, 1.0 / static_cast<double>(n));
    std::vector<double> w(n, 0.0);
    double lambda = 0.0;
    for (int it = 0; it < iters; ++it) {
        std::fill(w.begin(), w.end(), 0.0);
        double norm = 0.0;
        for (std::size_t u = 0; u < n; ++u) {
            if (v[u] == 0.0) continue;
            for (const auto& e : g.out_adj[u]) {
                w[e.first] += static_cast<double>(e.second) * v[u];
            }
        }
        for (std::size_t i = 0; i < n; ++i) norm = std::max(norm, std::abs(w[i]));
        if (norm == 0.0) return 0.0;
        for (auto& x : w) x /= norm;
        double num = 0.0, den = 0.0;
        std::vector<double> Aw(n, 0.0);
        for (std::size_t u = 0; u < n; ++u) {
            if (w[u] == 0.0) continue;
            for (const auto& e : g.out_adj[u]) {
                Aw[e.first] += static_cast<double>(e.second) * w[u];
            }
        }
        for (std::size_t i = 0; i < n; ++i) {
            num += w[i] * Aw[i];
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

// Result of comparing two graphs' OWN canonical (coarsest) quotients.
// This is the practical, cheap-to-compute test: it does not search
// for a witnessing map between the two original graphs (a harder
// problem in general), but it does answer the concrete question "do
// these two graphs reduce, under the standard equitable-partition
// mechanism, to structurally identical quotients" -- which is exactly
// the mechanism that would force equal Perron roots for free.
struct DivisorComparisonResult {
    int classes_a = 0;
    int classes_b = 0;
    bool used_exact_charpoly = false;  // false => quotients were too large
                                        // for charpoly_int's O(k^4) cost;
                                        // dominant_eigenvalue_a/b (power
                                        // iteration) were used instead
    // charpoly_a / charpoly_b are stored as mathlib::PolyZ so that
    // genuinely large quotient characteristic polynomials are
    // compared at their true coefficients rather than silently
    // weakened via a long long overflow check (see
    // docs/RECOVERY_AUDIT_2026-07-29.md queue item Q4).  Use
    // polyZ_to_long_long_vec (or polyZ_to_string) when a legacy
    // vector<long long> surface is required.
    mathlib::PolyZ charpoly_a;           // only populated if used_exact_charpoly
    mathlib::PolyZ charpoly_b;           // only populated if used_exact_charpoly
    bool charpoly_match = false;         // exact integer charpoly equality
                                          // (only meaningful if used_exact_charpoly)
    double dominant_eigenvalue_a = 0.0;  // power-iteration estimate, always computed
    double dominant_eigenvalue_b = 0.0;
    bool dominant_eigenvalue_match = false;  // |a - b| within numeric tolerance
    bool quotient_permutation_match = false;  // exact matrix match up to
                                               // relabeling (only checked
                                               // when both quotients are
                                               // small enough to brute-
                                               // force; see checked_perm)
    bool checked_perm = false;        // true iff quotient_permutation_match
                                       // was actually evaluated (both
                                       // quotients had <= perm_limit classes)
};

namespace graph_divisor_detail {
inline bool matrices_equal(const std::vector<std::vector<long long>>& A,
                            const std::vector<std::vector<long long>>& B) {
    if (A.size() != B.size()) return false;
    for (std::size_t i = 0; i < A.size(); ++i) {
        if (A[i].size() != B[i].size()) return false;
        for (std::size_t j = 0; j < A[i].size(); ++j) {
            if (A[i][j] != B[i][j]) return false;
        }
    }
    return true;
}

// Brute-force search over permutations of {0..k-1} for one that maps
// quotient A onto quotient B.  Only sane for small k (<= perm_limit).
inline bool exists_permutation_matching(
    const std::vector<std::vector<long long>>& A,
    const std::vector<std::vector<long long>>& B) {
    const std::size_t k = A.size();
    std::vector<int> perm(k);
    for (std::size_t i = 0; i < k; ++i) perm[i] = static_cast<int>(i);
    do {
        std::vector<std::vector<long long>> permuted(k, std::vector<long long>(k, 0));
        for (std::size_t i = 0; i < k; ++i) {
            for (std::size_t j = 0; j < k; ++j) {
                permuted[static_cast<std::size_t>(perm[i])]
                        [static_cast<std::size_t>(perm[j])] = A[i][j];
            }
        }
        if (matrices_equal(permuted, B)) return true;
    } while (std::next_permutation(perm.begin(), perm.end()));
    return false;
}
}  // namespace graph_divisor_detail

inline DivisorComparisonResult compare_divisors(
    const WeightedDigraph& a, const WeightedDigraph& b,
    int perm_limit = 8,
    int exact_charpoly_limit = 120) {
    DivisorComparisonResult r;
    auto pa = coarsest_equitable_partition(a);
    auto pb = coarsest_equitable_partition(b);
    r.classes_a = pa.num_classes;
    r.classes_b = pb.num_classes;
    auto Qa = quotient_matrix(a, pa);
    auto Qb = quotient_matrix(b, pb);

    // charpoly_int is Faddeev-LeVerrier: O(k^4) for a k x k matrix
    // (k matrix-matrix multiplications, each O(k^3)).  That's fine
    // for the handful-of-classes cases this was first written for,
    // but a coarsest partition can legitimately fail to collapse
    // much on a large, low-symmetry graph (e.g. G_B for a several-
    // thousand-node substitution survey candidate), in which case
    // k stays in the thousands and the exact path becomes
    // computationally infeasible.  Fall back to power iteration
    // (dominant_eigenvalue_estimate, O(iterations * k^2)) above the
    // threshold; it answers the practically relevant question
    // (do the two Perron roots match) even when it can't certify
    // exact quotient-matrix equality.
    if (r.classes_a <= exact_charpoly_limit && r.classes_b <= exact_charpoly_limit) {
        r.used_exact_charpoly = true;
        r.charpoly_a = charpoly_PolyZ(Qa);
        r.charpoly_b = charpoly_PolyZ(Qb);
        r.charpoly_match = (r.charpoly_a == r.charpoly_b);
    }
    r.dominant_eigenvalue_a = dominant_eigenvalue_estimate(Qa);
    r.dominant_eigenvalue_b = dominant_eigenvalue_estimate(Qb);
    double m = std::max(r.dominant_eigenvalue_a, r.dominant_eigenvalue_b);
    r.dominant_eigenvalue_match =
        std::abs(r.dominant_eigenvalue_a - r.dominant_eigenvalue_b) < 1e-6 * (m + 1.0);

    if (r.classes_a == r.classes_b && r.classes_a <= perm_limit) {
        r.checked_perm = true;
        r.quotient_permutation_match =
            graph_divisor_detail::exists_permutation_matching(Qa, Qb);
    }
    return r;
}

// ===================================================================
// Strongly connected components and the "recurrent core."
// ===================================================================
//
// A graph's dominant (Perron) eigenvalue is determined ENTIRELY by
// its recurrent structure -- the union of all nontrivial strongly
// connected components (SCCs of size > 1, or a single node with a
// self-loop). Nodes outside any such SCC are transient: they can be
// reached and can reach the recurrent part, but once left behind they
// are never revisited, and they contribute nothing to the spectral
// radius. This matters concretely for `G_B` (contact_boundary.hpp):
// the D_cont/closure/corona construction produces many transient
// "feeder" nodes on the way into the actual recurrent structure that
// determines `lambda(G_B)`, and comparing G_B's FULL graph against
// another graph (e.g. the balanced-pair automaton) via
// `compare_divisors` dilutes any real structural relationship with
// these transient nodes, which is exactly why the divisor experiment
// in `app/test_bp_gb_divisor.cpp` found nothing: it ran
// on the full graphs, not their recurrent cores. Restricting to the
// core first is what actually found the exact free
// involution `[i,x,j] <-> [j,-x,i]` underlying the n-bonacci special
// case of the central conjecture -- see
// docs/FINDINGS_FOR_CITATION.md Finding 1 and
// app/gb_bp_involution_check.cpp for the full experiment, and
// docs/RESEARCH_STATUS.md for how this was found.

// Tarjan's algorithm. Returns the list of SCCs (each a list of node
// indices); a node's own SCC can be found by membership. Iterative-
// recursion via std::function keeps this simple; fine for the graph
// sizes this project deals with (up to a few thousand nodes).
inline std::vector<std::vector<std::size_t>> tarjan_scc(const WeightedDigraph& g) {
    const std::size_t n = g.n;
    std::vector<int> disc(n, -1), low(n, -1);
    std::vector<bool> on_stack(n, false);
    std::vector<std::size_t> stk;
    int timer = 0;
    std::vector<std::vector<std::size_t>> sccs;

    std::function<void(std::size_t)> dfs = [&](std::size_t u) {
        disc[u] = low[u] = timer++;
        stk.push_back(u);
        on_stack[u] = true;
        for (const auto& e : g.out_adj[u]) {
            std::size_t v = e.first;
            if (disc[v] == -1) {
                dfs(v);
                low[u] = std::min(low[u], low[v]);
            } else if (on_stack[v]) {
                low[u] = std::min(low[u], static_cast<int>(disc[v]));
            }
        }
        if (low[u] == disc[u]) {
            std::vector<std::size_t> comp;
            while (true) {
                std::size_t v = stk.back();
                stk.pop_back();
                on_stack[v] = false;
                comp.push_back(v);
                if (v == u) break;
            }
            sccs.push_back(std::move(comp));
        }
    };
    for (std::size_t i = 0; i < n; ++i) {
        if (disc[i] == -1) dfs(i);
    }
    return sccs;
}

// True iff the given SCC (a list of node indices, as returned by
// tarjan_scc) is "recurrent": size > 1, or a single node with a
// self-loop (both cases contribute a nontrivial cycle).
inline bool is_recurrent_scc(const WeightedDigraph& g, const std::vector<std::size_t>& scc) {
    if (scc.size() > 1) return true;
    if (scc.empty()) return false;
    std::size_t u = scc[0];
    for (const auto& e : g.out_adj[u]) {
        if (e.first == u) return true;
    }
    return false;
}

// Extracts the LARGEST recurrent SCC as its own WeightedDigraph (nodes
// renumbered 0..k-1), together with a map back to the original node
// indices (result.second[i] = original index of core-node i). This is
// "the recurrent core" -- the subgraph that actually determines the
// original graph's dominant eigenvalue, with transient feeder nodes
// stripped out.
//
// CONFIRMED COUNTEREXAMPLE (Hexanacci / n=6-bonacci's
// G_B): the assumption that the LARGEST-BY-NODE-COUNT recurrent SCC
// is also the DOMINANT one (the one whose own Perron root equals the
// full graph's) is FALSE in general -- Hexanacci's G_B has 7 recurrent
// SCCs of sizes 221, 210, 80, 6, 6, 6, 2, and the dominant one (Perron
// root ~1.836, matching the reported lambda(G_B)) is the 210-node SCC,
// NOT the 221-node one this function would return. Checking the
// free-involution property on the wrong (221-node) SCC
// produced a spurious "15 nodes unmatched, partial" result; re-checked
// on the actually-dominant 210-node SCC, the involution is exact
// (210/210) and the exact charpoly-divisibility mechanism holds too --
// see app/gb_bp_involution_general_n.cpp and docs/RESEARCH_STATUS.md for
// how this was found. Use
// `extract_dominant_recurrent_core` below instead whenever the
// graph might have more than one nontrivial recurrent SCC (which,
// per this counterexample, should now be assumed possible rather than
// dismissed as "unusual", for any G_B/BP-core comparison).
inline std::pair<WeightedDigraph, std::vector<std::size_t>> extract_recurrent_core(
    const WeightedDigraph& g) {
    auto sccs = tarjan_scc(g);
    std::vector<std::size_t> best;
    for (auto& c : sccs) {
        if (is_recurrent_scc(g, c) && c.size() > best.size()) best = c;
    }
    std::unordered_map<std::size_t, std::size_t> idx;
    for (std::size_t i = 0; i < best.size(); ++i) idx[best[i]] = i;
    WeightedDigraph core(best.size());
    for (std::size_t i = 0; i < best.size(); ++i) {
        std::size_t u = best[i];
        for (const auto& e : g.out_adj[u]) {
            auto it = idx.find(e.first);
            if (it != idx.end()) core.add_edge(i, it->second, e.second);
        }
    }
    return {std::move(core), std::move(best)};
}

// Extracts the DOMINANT recurrent SCC -- the one whose own Perron
// root equals the FULL graph's dominant eigenvalue (max over all
// nonnegative-matrix irreducible blocks, standard Perron-Frobenius
// fact for reducible matrices) -- rather than extract_recurrent_core's
// largest-by-NODE-COUNT choice, which the Hexanacci counterexample
// above shows can pick the wrong component.
//
// Cost: one power-iteration eigenvalue estimate
// (dominant_eigenvalue_estimate_sparse) per recurrent SCC, so O(SCC
// count * iters * average edges per SCC) -- cheap relative to
// building the SCCs themselves via tarjan_scc, and only run once per
// call, not in a hot loop, for any graph size this project deals
// with (a few thousand nodes).
//
// Ties (two SCCs with equal, or numerically indistinguishable,
// Perron roots) are broken by returning the FIRST one encountered
// with the maximal estimate; a caller that needs to distinguish a
// genuine tie from numerical noise should re-check the tied
// candidates with `mathlib::compare_perron_roots_exact` or
// `mathlib::certify_perron_bracket_bigfloat` (math/ball.hpp,
// math/bigfloat.hpp) directly on their own dense matrices.
inline std::pair<WeightedDigraph, std::vector<std::size_t>> extract_dominant_recurrent_core(
    const WeightedDigraph& g, int power_iters = 2000, double tol = 1e-12) {
    auto sccs = tarjan_scc(g);
    std::vector<std::size_t> best;
    double best_eigenvalue = -1.0;
    for (auto& c : sccs) {
        if (!is_recurrent_scc(g, c)) continue;
        std::unordered_map<std::size_t, std::size_t> idx;
        for (std::size_t i = 0; i < c.size(); ++i) idx[c[i]] = i;
        WeightedDigraph comp(c.size());
        for (std::size_t i = 0; i < c.size(); ++i) {
            std::size_t u = c[i];
            for (const auto& e : g.out_adj[u]) {
                auto it = idx.find(e.first);
                if (it != idx.end()) comp.add_edge(i, it->second, e.second);
            }
        }
        double ev = dominant_eigenvalue_estimate_sparse(comp, power_iters, tol);
        if (ev > best_eigenvalue) {
            best_eigenvalue = ev;
            best = c;
        }
    }
    std::unordered_map<std::size_t, std::size_t> idx;
    for (std::size_t i = 0; i < best.size(); ++i) idx[best[i]] = i;
    WeightedDigraph core(best.size());
    for (std::size_t i = 0; i < best.size(); ++i) {
        std::size_t u = best[i];
        for (const auto& e : g.out_adj[u]) {
            auto it = idx.find(e.first);
            if (it != idx.end()) core.add_edge(i, it->second, e.second);
        }
    }
    return {std::move(core), std::move(best)};
}

}  // namespace ravel
