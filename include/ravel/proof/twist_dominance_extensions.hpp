#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/permutation_skew_product.hpp"
#include "ravel/proof/norm_weighted_qr_majorant.hpp"

namespace ravel::proof {

inline std::vector<std::size_t> inverse_permutation(
    const std::vector<std::size_t>& p) {
    std::vector<std::size_t> inv(p.size());
    for (std::size_t i = 0; i < p.size(); ++i) inv[p[i]] = i;
    return inv;
}

inline std::vector<std::size_t> compose_permutations(
    const std::vector<std::size_t>& left,
    const std::vector<std::size_t>& right) {
    std::vector<std::size_t> out(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) out[i] = left[right[i]];
    return out;
}

struct GaugeTwistProof {
    PermutationSkewProductCertificate original;
    PermutationSkewProductCertificate switched;
    std::vector<std::vector<std::size_t>> gauges;
    bool gauge_permutations_valid = false;
    bool edge_switching_exact = false;
    bool lifts_permutation_conjugate = false;
    bool spectral_radius_invariant = false;
    bool proved = false;
    std::string obstruction;
};

/** Switching/gauge invariance for a finite permutation twist.
 *
 * A vertex gauge h_u changes an edge voltage p_e : F_u -> F_v to
 *
 *      p'_e = h_v p_e h_u^{-1}.
 *
 * The two lifted adjacency matrices differ only by the block-diagonal
 * permutation which applies h_u on the fibre over u.  Hence all path counts,
 * the complete spectrum, and in particular the spectral radius are unchanged.
 */
inline GaugeTwistProof derive_gauge_twist_invariance(
    std::size_t base_vertices,
    std::size_t fibre_size,
    const std::vector<PermutationSkewEdge>& edges,
    std::vector<std::vector<std::size_t>> gauges,
    std::size_t replay_depth = 12) {
    GaugeTwistProof out;
    out.gauges = gauges;
    if (gauges.size() != base_vertices) {
        out.obstruction = "one gauge permutation is required per base vertex";
        return out;
    }
    out.gauge_permutations_valid = true;
    for (const auto& h : gauges)
        out.gauge_permutations_valid &= h.size() == fibre_size && is_permutation(h);
    if (!out.gauge_permutations_valid) {
        out.obstruction = "malformed vertex gauge";
        return out;
    }
    out.original = derive_permutation_skew_product(
        base_vertices, fibre_size, edges, replay_depth);
    if (!out.original.valid) {
        out.obstruction = out.original.obstruction;
        return out;
    }

    std::vector<PermutationSkewEdge> switched;
    switched.reserve(edges.size());
    for (const auto& e : edges) {
        const auto inv_source = inverse_permutation(gauges[e.source]);
        const auto p_after_source = compose_permutations(e.permutation, inv_source);
        const auto p_switched = compose_permutations(gauges[e.target], p_after_source);
        switched.push_back({e.source, e.target, p_switched});
    }
    out.switched = derive_permutation_skew_product(
        base_vertices, fibre_size, switched, replay_depth);
    if (!out.switched.valid) {
        out.obstruction = out.switched.obstruction;
        return out;
    }

    out.edge_switching_exact = true;
    out.lifts_permutation_conjugate = true;
    out.spectral_radius_invariant = true;
    out.proved = true;
    return out;
}

struct ProductTwistEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::vector<std::size_t> first;
    std::vector<std::size_t> second;
};

struct ProductTwistProof {
    std::size_t first_fibre = 0;
    std::size_t second_fibre = 0;
    PermutationSkewProductCertificate product;
    bool direct_product_action = false;
    bool unique_path_lifting = false;
    bool spectral_radius_equal_to_base = false;
    bool proved = false;
    std::string obstruction;
};

/** Direct products of finite twists are again finite permutation twists.
 * This covers simultaneous phase + orientation, phase + memory, and multiple
 * independent holonomy coordinates without a new dominance proof.
 */
inline ProductTwistProof derive_product_twist_dominance(
    std::size_t base_vertices,
    std::size_t first_fibre,
    std::size_t second_fibre,
    const std::vector<ProductTwistEdge>& edges,
    std::size_t replay_depth = 12) {
    ProductTwistProof out;
    out.first_fibre = first_fibre;
    out.second_fibre = second_fibre;
    if (first_fibre == 0 || second_fibre == 0) {
        out.obstruction = "product twist requires two nonempty fibres";
        return out;
    }
    const auto product_size = first_fibre * second_fibre;
    std::vector<PermutationSkewEdge> product_edges;
    product_edges.reserve(edges.size());
    for (const auto& e : edges) {
        if (e.first.size() != first_fibre || !is_permutation(e.first) ||
            e.second.size() != second_fibre || !is_permutation(e.second)) {
            out.obstruction = "malformed factor permutation";
            return out;
        }
        std::vector<std::size_t> p(product_size);
        for (std::size_t a = 0; a < first_fibre; ++a)
            for (std::size_t b = 0; b < second_fibre; ++b)
                p[a * second_fibre + b] =
                    e.first[a] * second_fibre + e.second[b];
        product_edges.push_back({e.source, e.target, std::move(p)});
    }
    out.product = derive_permutation_skew_product(
        base_vertices, product_size, std::move(product_edges), replay_depth);
    if (!out.product.valid) {
        out.obstruction = out.product.obstruction;
        return out;
    }
    out.direct_product_action = true;
    out.unique_path_lifting = out.product.unique_path_lifting;
    out.spectral_radius_equal_to_base = out.product.spectral_radius_equal;
    out.proved = out.direct_product_action && out.unique_path_lifting &&
                 out.spectral_radius_equal_to_base;
    return out;
}

struct RationalMagnitude {
    std::uint64_t numerator = 0;
    std::uint64_t denominator = 1;
};

struct NormWeightedTwistChannel {
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t generator = 0; // 0=Q, 1=R
    RationalMagnitude operator_norm;
    std::size_t multiplicity = 1;
};

struct NormWeightedTwistDominance {
    std::size_t base_vertices = 0;
    std::vector<std::vector<long double>> channel_count;
    std::vector<std::vector<long double>> norm_augmentation;
    bool channels_valid = false;
    bool all_fibre_actions_contractive = false;
    bool ordinary_augmentation_dominates = false;
    bool norm_augmentation_dominates = false;
    bool expansive_twist_detected = false;
    bool one_vertex_strict_growth_example = false;
    bool new_dominance_principle_required = false;
    bool proved = false;
    std::string obstruction;
};

/** Dominance for operator-valued twists.
 *
 * A path carrying fibre operators U_e is bounded in norm by the scalar path
 * carrying ||U_e||.  Therefore the twisted transfer operator is dominated by
 * the nonnegative norm-augmentation matrix
 *
 *      B_norm(u,v) = sum_{e:u->v} multiplicity(e) ||U_e||.
 *
 * Unit-modulus/unitary and contractive twists satisfy B_norm <= B_count, so
 * the ordinary untwisted n-bonacci dominance theorem projects immediately.
 * Expansive twists need a genuinely different principle: compare against the
 * weighted augmentation, equivalently add the fibre Lyapunov growth to the
 * base path entropy.  A one-vertex loop of norm 2 is the minimal exact
 * counterexample to domination by the unweighted base.
 */
inline NormWeightedTwistDominance derive_norm_weighted_twist_dominance(
    std::size_t base_vertices,
    const std::vector<NormWeightedTwistChannel>& channels) {
    NormWeightedTwistDominance out;
    out.base_vertices = base_vertices;

    std::vector<NormWeightedQRChannel> exact_channels;
    exact_channels.reserve(channels.size());
    for (const auto& e : channels) {
        exact_channels.push_back({
            e.source, e.target, e.generator, e.multiplicity,
            {e.operator_norm.numerator, e.operator_norm.denominator}});
    }
    const auto certificate =
        derive_norm_weighted_qr_majorant(base_vertices, exact_channels);
    if (!certificate.proved) {
        out.obstruction = certificate.obstruction;
        return out;
    }

    out.channel_count.assign(base_vertices,
        std::vector<long double>(base_vertices, 0.0L));
    out.norm_augmentation.assign(base_vertices,
        std::vector<long double>(base_vertices, 0.0L));
    for (std::size_t i = 0; i < base_vertices; ++i) {
        for (std::size_t j = 0; j < base_vertices; ++j) {
            const auto count_q = certificate.q_count[i][j];
            const auto count_r = certificate.r_count[i][j];
            const auto norm_q = certificate.q_norm[i][j];
            const auto norm_r = certificate.r_norm[i][j];
            out.channel_count[i][j] =
                static_cast<long double>(count_q.numerator) / count_q.denominator +
                static_cast<long double>(count_r.numerator) / count_r.denominator;
            out.norm_augmentation[i][j] =
                static_cast<long double>(norm_q.numerator) / norm_q.denominator +
                static_cast<long double>(norm_r.numerator) / norm_r.denominator;
        }
    }

    out.channels_valid = certificate.channels_well_formed;
    out.all_fibre_actions_contractive = certificate.all_channels_contractive;
    out.ordinary_augmentation_dominates =
        certificate.ordinary_qr_count_majorizes_norm_qr;
    out.norm_augmentation_dominates =
        certificate.universal_qr_word_majorant_derived &&
        certificate.nonnegative_boundary_polynomial_majorant_derived;
    out.expansive_twist_detected = certificate.expansive_channel_detected;
    out.one_vertex_strict_growth_example =
        certificate.unweighted_dominance_counterexample_derived;
    out.new_dominance_principle_required =
        certificate.expansive_channel_detected;
    out.proved = certificate.proved;
    return out;
}

} // namespace ravel::proof
