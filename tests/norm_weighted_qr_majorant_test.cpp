#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/norm_weighted_qr_majorant.hpp"

using namespace ravel::proof;

int main() {
    std::vector<NormWeightedQRChannel> contractive{
        {0, 0, 0, 2, {1, 1}},
        {0, 1, 1, 1, {1, 2}},
        {1, 0, 0, 1, {3, 4}},
        {1, 1, 1, 1, {1, 1}}
    };
    const std::vector<QRGeneratorWord> boundary{{0}, {1,0}, {0,1,1}};
    const auto c = derive_norm_weighted_qr_majorant(2, contractive, boundary);
    assert(c.proved);
    assert(c.exact_qr_augmentation_derived);
    assert(c.universal_qr_word_majorant_derived);
    assert(c.nonnegative_boundary_polynomial_majorant_derived);
    assert(c.all_channels_contractive);
    assert(c.ordinary_qr_count_majorizes_norm_qr);
    assert(c.ordinary_word_and_boundary_projection_derived);
    assert(!c.expansive_channel_detected);
    assert(c.replayed_words.size() == boundary.size());

    std::vector<NormWeightedQRChannel> expansive{{0, 0, 0, 1, {2, 1}}};
    const auto e = derive_norm_weighted_qr_majorant(1, expansive, {{0}, {0,0}});
    assert(e.proved);
    assert(e.expansive_channel_detected);
    assert(!e.ordinary_qr_count_majorizes_norm_qr);
    assert(!e.ordinary_word_and_boundary_projection_derived);
    assert(e.unweighted_dominance_counterexample_derived);
    assert(e.norm_weighted_theorem_covers_expansive_case);
    assert(e.q_count[0][0].numerator == 1);
    assert(e.q_norm[0][0].numerator == 2);
    assert(e.replayed_words[1].norm_word[0][0].numerator == 4);

    auto malformed = expansive;
    malformed[0].certified_operator_norm_bound.denominator = 0;
    assert(!derive_norm_weighted_qr_majorant(1, malformed).proved);

    std::cout << "norm-weighted Q/R majorant PASS\n";
}
