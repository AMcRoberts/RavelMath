#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <numeric>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

struct PermutationSkewEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::vector<std::size_t> permutation;
};

struct PermutationSkewProductCertificate {
    std::size_t base_vertices = 0;
    std::size_t fibre_size = 0;
    std::vector<PermutationSkewEdge> edges;
    std::vector<std::vector<std::size_t>> base_adjacency;
    std::vector<std::vector<std::size_t>> lifted_adjacency;
    std::size_t replay_depth = 0;
    bool permutations_valid = false;
    bool lift_exact = false;
    bool unique_path_lifting = false;
    bool path_counts_aggregate = false;
    bool spectral_radius_equal = false;
    bool valid = false;
    std::string obstruction;
};

inline bool is_permutation(const std::vector<std::size_t>& p) {
    std::vector<std::size_t> sorted = p;
    std::sort(sorted.begin(), sorted.end());
    for (std::size_t i = 0; i < sorted.size(); ++i)
        if (sorted[i] != i) return false;
    return true;
}

/** Certify a finite permutation skew product over a directed multigraph.
 *
 * Every base edge has exactly one lift from each fibre point. Consequently
 * every base path has exactly one lift from each chosen initial fibre point.
 * Row path counts of every power therefore agree after projection. By the
 * Gelfand path-growth characterization for nonnegative matrices, base and
 * lift have equal spectral radius.
 */
inline PermutationSkewProductCertificate derive_permutation_skew_product(
    std::size_t base_vertices,
    std::size_t fibre_size,
    std::vector<PermutationSkewEdge> edges,
    std::size_t replay_depth = 12) {
    PermutationSkewProductCertificate c;
    c.base_vertices = base_vertices;
    c.fibre_size = fibre_size;
    c.edges = std::move(edges);
    c.replay_depth = replay_depth;
    if (base_vertices == 0 || fibre_size == 0) {
        c.obstruction = "skew product requires nonempty base and fibre";
        return c;
    }
    c.base_adjacency.assign(base_vertices, {});
    c.lifted_adjacency.assign(base_vertices * fibre_size, {});
    c.permutations_valid = true;
    for (const auto& e : c.edges) {
        c.permutations_valid &= e.source < base_vertices && e.target < base_vertices;
        c.permutations_valid &= e.permutation.size() == fibre_size &&
                                is_permutation(e.permutation);
        if (!c.permutations_valid) break;
        c.base_adjacency[e.source].push_back(e.target);
        for (std::size_t a = 0; a < fibre_size; ++a) {
            const auto u = e.source * fibre_size + a;
            const auto v = e.target * fibre_size + e.permutation[a];
            c.lifted_adjacency[u].push_back(v);
        }
    }
    if (!c.permutations_valid) {
        c.obstruction = "edge data is not a valid permutation lift";
        return c;
    }
    c.lift_exact = true;
    c.unique_path_lifting = true;

    std::vector<std::uint64_t> base_counts(base_vertices, 1);
    std::vector<std::uint64_t> lift_counts(base_vertices * fibre_size, 1);
    c.path_counts_aggregate = true;
    for (std::size_t depth = 0; depth <= replay_depth; ++depth) {
        for (std::size_t u = 0; u < base_vertices; ++u)
            for (std::size_t a = 0; a < fibre_size; ++a)
                c.path_counts_aggregate &=
                    lift_counts[u * fibre_size + a] == base_counts[u];
        std::vector<std::uint64_t> next_base(base_vertices, 0);
        std::vector<std::uint64_t> next_lift(base_vertices * fibre_size, 0);
        for (std::size_t u = 0; u < base_vertices; ++u)
            for (const auto v : c.base_adjacency[u])
                next_base[u] += base_counts[v];
        for (std::size_t u = 0; u < c.lifted_adjacency.size(); ++u)
            for (const auto v : c.lifted_adjacency[u])
                next_lift[u] += lift_counts[v];
        base_counts = std::move(next_base);
        lift_counts = std::move(next_lift);
    }
    // The finite replay checks the implementation; the theorem follows from
    // unique path lifting at every length, not from the replay cutoff.
    c.spectral_radius_equal = c.unique_path_lifting && c.path_counts_aggregate;
    c.valid = c.permutations_valid && c.lift_exact &&
              c.unique_path_lifting && c.path_counts_aggregate &&
              c.spectral_radius_equal;
    if (!c.valid) c.obstruction = "permutation skew-product replay failed";
    return c;
}

} // namespace ravel::proof
