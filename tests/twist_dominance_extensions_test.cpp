#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/twist_dominance_extensions.hpp"

using namespace ravel::proof;

int main() {
    // Gauge/switching: the same two-sheet lift in two vertex gauges.
    std::vector<PermutationSkewEdge> edges{
        {0, 1, {1, 0}},
        {1, 0, {0, 1}},
        {1, 1, {1, 0}}
    };
    auto gauge = derive_gauge_twist_invariance(
        2, 2, edges, {{1, 0}, {0, 1}}, 16);
    assert(gauge.proved);
    assert(gauge.spectral_radius_invariant);

    // Product twist: orientation x three-phase memory.
    std::vector<ProductTwistEdge> product_edges{
        {0, 1, {1, 0}, {1, 2, 0}},
        {1, 0, {0, 1}, {2, 0, 1}},
        {1, 1, {1, 0}, {0, 1, 2}}
    };
    auto product = derive_product_twist_dominance(
        2, 2, 3, product_edges, 16);
    assert(product.proved);
    assert(product.product.fibre_size == 6);
    assert(product.spectral_radius_equal_to_base);

    // Unitary/contractive fibre actions project the ordinary dominance theorem.
    std::vector<NormWeightedTwistChannel> unitary{
        {0, 0, 0, {1, 1}, 2},
        {0, 1, 1, {1, 1}, 1},
        {1, 0, 1, {1, 2}, 1}
    };
    auto contractive = derive_norm_weighted_twist_dominance(2, unitary);
    assert(contractive.proved);
    assert(contractive.all_fibre_actions_contractive);
    assert(contractive.ordinary_augmentation_dominates);
    assert(!contractive.new_dominance_principle_required);

    // Exotic expanding twist: one loop with fibre norm 2.
    std::vector<NormWeightedTwistChannel> expanding{
        {0, 0, 0, {2, 1}, 1}
    };
    auto exotic = derive_norm_weighted_twist_dominance(1, expanding);
    assert(exotic.proved);
    assert(exotic.expansive_twist_detected);
    assert(!exotic.ordinary_augmentation_dominates);
    assert(exotic.norm_augmentation_dominates);
    assert(exotic.one_vertex_strict_growth_example);
    assert(exotic.new_dominance_principle_required);

    auto malformed = expanding;
    malformed.push_back({1, 0, 0, {1, 1}, 1});
    assert(!derive_norm_weighted_twist_dominance(1, malformed).proved);

    std::cout << "twist dominance extensions PASS\n";
}
