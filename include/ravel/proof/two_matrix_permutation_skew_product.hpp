#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/permutation_skew_product.hpp"

namespace ravel::proof {

struct TwoMatrixSkewEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t generator = 0; // 0 or 1
    std::vector<std::size_t> permutation;
};

struct TwoMatrixPermutationSkewProduct {
    std::size_t base_vertices = 0;
    std::size_t fibre_size = 0;
    std::vector<TwoMatrixSkewEdge> edges;
    std::vector<std::vector<std::vector<std::size_t>>> base_adjacency;
    std::vector<std::vector<std::vector<std::size_t>>> lifted_adjacency;
    std::size_t checked_word_depth = 0;
    std::size_t checked_words = 0;
    bool generators_valid = false;
    bool colored_unique_path_lifting = false;
    bool every_word_path_count_preserved = false;
    bool summed_adjacency_corollary = false;
    bool valid = false;
    std::string obstruction;
};

namespace two_matrix_skew_detail {

inline std::vector<std::uint64_t> apply_adjacency(
    const std::vector<std::vector<std::size_t>>& adjacency,
    const std::vector<std::uint64_t>& counts) {
    std::vector<std::uint64_t> out(adjacency.size(), 0);
    for (std::size_t u = 0; u < adjacency.size(); ++u)
        for (const auto v : adjacency[u]) out[u] += counts[v];
    return out;
}

inline void replay_words(
    const TwoMatrixPermutationSkewProduct& c,
    std::size_t depth,
    std::vector<std::uint64_t> base,
    std::vector<std::uint64_t> lift,
    bool& preserved,
    std::size_t& words) {
    for (std::size_t u = 0; u < c.base_vertices; ++u)
        for (std::size_t a = 0; a < c.fibre_size; ++a)
            preserved &= lift[u * c.fibre_size + a] == base[u];
    ++words;
    if (depth == c.checked_word_depth) return;
    for (std::size_t generator = 0; generator < 2; ++generator)
        replay_words(c, depth + 1,
            apply_adjacency(c.base_adjacency[generator], base),
            apply_adjacency(c.lifted_adjacency[generator], lift),
            preserved, words);
}

} // namespace two_matrix_skew_detail

/** Certify a permutation skew product for a pair of nonnegative matrices.
 *
 * Each colored base edge has one permutation lift from every fibre point.
 * Consequently, for every word w in {0,1}*, the matrix product A_w on the
 * lift has exactly the same row path counts as B_w on the base.  This is the
 * two-generator statement; equality for A_0+A_1 and B_0+B_1 is only its
 * uncolored corollary.
 */
inline TwoMatrixPermutationSkewProduct derive_two_matrix_permutation_skew_product(
    std::size_t base_vertices,
    std::size_t fibre_size,
    std::vector<TwoMatrixSkewEdge> edges,
    std::size_t replay_word_depth = 10) {
    TwoMatrixPermutationSkewProduct c;
    c.base_vertices = base_vertices;
    c.fibre_size = fibre_size;
    c.edges = std::move(edges);
    c.checked_word_depth = replay_word_depth;
    if (base_vertices == 0 || fibre_size == 0) {
        c.obstruction = "two-matrix skew product requires nonempty base and fibre";
        return c;
    }
    c.base_adjacency.assign(2,
        std::vector<std::vector<std::size_t>>(base_vertices));
    c.lifted_adjacency.assign(2,
        std::vector<std::vector<std::size_t>>(base_vertices * fibre_size));
    c.generators_valid = true;
    for (const auto& e : c.edges) {
        c.generators_valid &= e.generator < 2;
        c.generators_valid &= e.source < base_vertices && e.target < base_vertices;
        c.generators_valid &= e.permutation.size() == fibre_size &&
                              is_permutation(e.permutation);
        if (!c.generators_valid) break;
        c.base_adjacency[e.generator][e.source].push_back(e.target);
        for (std::size_t a = 0; a < fibre_size; ++a)
            c.lifted_adjacency[e.generator][e.source * fibre_size + a]
                .push_back(e.target * fibre_size + e.permutation[a]);
    }
    if (!c.generators_valid) {
        c.obstruction = "two-matrix edge data is not a valid colored permutation lift";
        return c;
    }
    c.colored_unique_path_lifting = true;
    c.every_word_path_count_preserved = true;
    std::vector<std::uint64_t> base(base_vertices, 1);
    std::vector<std::uint64_t> lift(base_vertices * fibre_size, 1);
    two_matrix_skew_detail::replay_words(
        c, 0, std::move(base), std::move(lift),
        c.every_word_path_count_preserved, c.checked_words);

    // The replay checks the implementation.  The all-word theorem follows
    // inductively because each colored edge has one lift from every sheet.
    c.summed_adjacency_corollary =
        c.colored_unique_path_lifting && c.every_word_path_count_preserved;
    c.valid = c.generators_valid && c.colored_unique_path_lifting &&
              c.every_word_path_count_preserved && c.summed_adjacency_corollary;
    if (!c.valid) c.obstruction = "two-matrix word-by-word path replay failed";
    return c;
}

} // namespace ravel::proof
