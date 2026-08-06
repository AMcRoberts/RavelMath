#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "ravel/proof/condition_f_component_maximality.hpp"
#include "ravel/proof/condition_f_twisted_component_quotient.hpp"
#include "ravel/proof/fibered_twisted_quotient.hpp"
#include "ravel/proof/finite_quotient_core_maximality.hpp"

namespace ravel::proof {

struct ConditionFChannelCountCollapse {
    std::size_t quotient_vertices = 0;
    std::size_t quotient_components = 0;
    std::size_t core_component = 0;
    bool unique_path_lifting = false;
    bool spectral_radius_preserved = false;
    FiniteQuotientCoreMaximality quotient_maximality;
    bool core_spectrally_maximal_in_lift = false;
    std::string obstruction;
};

namespace condition_f_channel_count_detail {

inline ConditionFChannelCountCollapse derive(
    const std::vector<std::vector<std::size_t>>& adjacency,
    bool unique_path_lifting,
    bool spectral_radius_preserved,
    std::size_t core_vertex,
    std::size_t horizon) {
    ConditionFChannelCountCollapse out;
    out.quotient_vertices = adjacency.size();
    out.unique_path_lifting = unique_path_lifting;
    out.spectral_radius_preserved = spectral_radius_preserved;
    if (!unique_path_lifting || !spectral_radius_preserved ||
        adjacency.empty() || core_vertex >= adjacency.size()) {
        out.obstruction = "Condition-F channel-count collapse requires a valid path-lifting quotient and core role";
        return out;
    }

    const auto label = condition_f_component_detail::scc_labels(adjacency);
    out.quotient_components = *std::max_element(label.begin(), label.end()) + 1;
    out.core_component = label[core_vertex];
    auto core = condition_f_component_detail::component_matrix(
        adjacency, label, out.core_component);
    std::vector<std::vector<std::vector<long long>>> competitors;
    for (std::size_t c = 0; c < out.quotient_components; ++c) {
        if (c == out.core_component) continue;
        auto matrix = condition_f_component_detail::component_matrix(adjacency, label, c);
        if (condition_f_component_detail::recurrent_component(
                adjacency, label, c, matrix.size()))
            competitors.push_back(std::move(matrix));
    }
    out.quotient_maximality = derive_finite_quotient_core_maximality(
        core, competitors, horizon);
    out.core_spectrally_maximal_in_lift =
        out.spectral_radius_preserved &&
        out.quotient_maximality.core_spectrally_maximal;
    if (!out.core_spectrally_maximal_in_lift)
        out.obstruction = out.quotient_maximality.obstruction.empty()
            ? "channel-count quotient core is not spectrally maximal"
            : out.quotient_maximality.obstruction;
    return out;
}

} // namespace condition_f_channel_count_detail

/** Forget all sheet permutations and retain only one parallel base edge per
 * transport channel. Unique path lifting gives equality of path counts from
 * every point over a base role, hence equality of spectral radii. Twists can
 * change lifted SCC assembly, but they cannot change exponential growth. */
inline ConditionFChannelCountCollapse derive_condition_f_channel_count_collapse(
    const ConditionFTwistedComponentQuotientProof& quotient,
    std::size_t core_role,
    std::size_t horizon = 160) {
    if (!quotient.proved || !quotient.skew_product.valid) {
        ConditionFChannelCountCollapse out;
        out.obstruction = "Condition-F channel-count collapse requires a closed twisted quotient";
        return out;
    }
    return condition_f_channel_count_detail::derive(
        quotient.skew_product.base_adjacency,
        quotient.skew_product.unique_path_lifting,
        quotient.skew_product.spectral_radius_equal,
        core_role, horizon);
}

inline ConditionFChannelCountCollapse derive_condition_f_channel_count_collapse(
    const FiberedTwistedQuotientCertificate& quotient,
    std::size_t core_class,
    std::size_t horizon = 160) {
    if (!quotient.valid) {
        ConditionFChannelCountCollapse out;
        out.obstruction = "Condition-F channel-count collapse requires a valid fibered quotient";
        return out;
    }
    return condition_f_channel_count_detail::derive(
        quotient.quotient_adjacency,
        quotient.unique_path_lifting,
        quotient.spectral_radius_preserved,
        core_class, horizon);
}

} // namespace ravel::proof
