#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ravel/proof/condition_f_twisted_component_quotient.hpp"
#include "ravel/proof/two_matrix_permutation_skew_product.hpp"

namespace ravel::proof {

struct ConditionFTwoMatrixCollapse {
    std::size_t channels = 0;
    std::size_t generator_zero_channels = 0;
    std::size_t generator_one_channels = 0;
    TwoMatrixPermutationSkewProduct cocycle;
    bool word_by_word_growth_preserved = false;
    bool summed_channel_count_preserved = false;
    bool proved = false;
    std::string obstruction;
};

/** Preserve the actual two-generator structure of a Condition-F quotient.
 * `channel_generator[k]` labels quotient channel k as generator 0 or 1.
 * Holonomy permutations are forgotten only after proving unique lifting for
 * every colored word in the pair, never before summing the matrices.
 */
inline ConditionFTwoMatrixCollapse derive_condition_f_two_matrix_collapse(
    const ConditionFTwistedComponentQuotientProof& quotient,
    const std::vector<std::size_t>& channel_generator,
    std::size_t replay_word_depth = 10) {
    ConditionFTwoMatrixCollapse out;
    out.channels = quotient.channels.size();
    if (!quotient.proved || !quotient.skew_product.valid) {
        out.obstruction = "Condition-F two-matrix collapse requires a closed twisted quotient";
        return out;
    }
    if (channel_generator.size() != quotient.channels.size()) {
        out.obstruction = "Condition-F two-matrix collapse requires one generator label per channel";
        return out;
    }
    std::vector<TwoMatrixSkewEdge> edges;
    edges.reserve(quotient.channels.size());
    for (std::size_t k = 0; k < quotient.channels.size(); ++k) {
        if (channel_generator[k] > 1) {
            out.obstruction = "Condition-F channel generator is not binary";
            return out;
        }
        const auto& ch = quotient.channels[k];
        edges.push_back({ch.source_role, ch.target_role,
                         channel_generator[k], ch.permutation});
        if (channel_generator[k] == 0) ++out.generator_zero_channels;
        else ++out.generator_one_channels;
    }
    out.cocycle = derive_two_matrix_permutation_skew_product(
        quotient.base_roles, quotient.fibre_size,
        std::move(edges), replay_word_depth);
    out.word_by_word_growth_preserved =
        out.cocycle.every_word_path_count_preserved;
    out.summed_channel_count_preserved =
        out.cocycle.summed_adjacency_corollary;
    out.proved = out.cocycle.valid && out.word_by_word_growth_preserved &&
                 out.summed_channel_count_preserved;
    if (!out.proved) out.obstruction = out.cocycle.obstruction;
    return out;
}

} // namespace ravel::proof
