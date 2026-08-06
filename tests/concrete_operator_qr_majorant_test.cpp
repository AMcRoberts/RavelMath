#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/concrete_operator_qr_majorant.hpp"

using namespace ravel::proof;

static ExactSignedMatrix matrix(std::initializer_list<std::initializer_list<std::int64_t>> rows) {
    ExactSignedMatrix out;
    for (const auto& row : rows) {
        std::vector<ExactSignedRational> r;
        for (auto x : row) r.push_back({x, 1});
        out.push_back(std::move(r));
    }
    return out;
}

int main() {
    // A genuinely twisted two-vertex, two-dimensional example.
    std::vector<ConcreteOperatorQRChannel> channels{
        {0, 0, 0, 1, matrix({{0, 1}, {1, 0}})},     // permutation, norm 1
        {0, 1, 1, 1, matrix({{1, 0}, {0, -1}})},    // sign twist, norm 1
        {1, 0, 0, 1, matrix({{1, 1}, {0, 1}})},     // shear, norm 2
        {1, 1, 1, 1, matrix({{1, 0}, {0, 1}})}      // identity, norm 1
    };
    const auto c = derive_concrete_operator_qr_majorant(
        2, 2, channels, {{0}, {1, 0}, {0, 1, 1}}, 5);
    assert(c.proved);
    assert(c.edge_norm_bounds_computed_exactly);
    assert(c.derived_edge_norm_bounds[2].numerator == 2);
    assert(c.qr_majorant.expansive_channel_detected);
    assert(c.universal_block_power_majorization_derived);
    assert(c.finite_power_replay_verified);
    assert(c.gelfand_spectral_radius_closure_derived);
    assert(c.spectral_radius_twist_at_most_majorant);
    assert(c.power_witnesses.size() == 6);

    // The scalar [2] loop is computed, not supplied as a norm label.
    std::vector<ConcreteOperatorQRChannel> scalar{
        {0, 0, 0, 1, matrix({{2}})}
    };
    const auto e = derive_concrete_operator_qr_majorant(1, 1, scalar, {{0}, {0, 0}}, 6);
    assert(e.proved);
    assert(e.derived_edge_norm_bounds[0].numerator == 2);
    assert(e.qr_majorant.unweighted_dominance_counterexample_derived);
    assert(e.power_witnesses[6].twisted_power[0][0].numerator == 64);
    assert(e.power_witnesses[6].scalar_majorant_power[0][0].numerator == 64);

    auto malformed = scalar;
    malformed[0].fibre_operator = matrix({{1, 0}});
    assert(!derive_concrete_operator_qr_majorant(1, 1, malformed).proved);

    std::cout << "concrete operator Q/R majorant PASS\n";
}
