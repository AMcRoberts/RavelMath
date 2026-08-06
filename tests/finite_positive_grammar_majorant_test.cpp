#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/finite_positive_grammar_majorant.hpp"

using namespace ravel::proof;

int main() {
    // Three genuinely distinct transport generators: balanced, left defect,
    // and right defect.  This is the first executable "more than two" case.
    const std::vector<NormWeightedGrammarChannel> channels{
        {0, 0, 0, 1, {1, 1}},
        {0, 1, 1, 2, {1, 2}},
        {1, 0, 2, 1, {3, 4}},
        {1, 1, 0, 1, {1, 1}},
    };
    const std::vector<PositiveGeneratorWord> boundary{
        {0}, {1, 2}, {0, 1, 2}, {2, 1, 0}
    };
    const auto c = derive_finite_positive_grammar_majorant(2, 3, channels, boundary);
    assert(c.proved);
    assert(c.generator_count == 3);
    assert(c.count_generators.size() == 3);
    assert(c.norm_generators.size() == 3);
    assert(c.universal_word_majorant_derived);
    assert(c.positive_polynomial_majorant_derived);
    assert(c.all_channels_contractive);
    assert(c.ordinary_generatorwise_projection_derived);
    assert(c.ordinary_word_and_polynomial_projection_derived);
    assert(!c.qr_specialization_available);

    // Expansive third generator remains covered only by the weighted theorem.
    auto expansive = channels;
    expansive[2].certified_operator_norm_bound = {2, 1};
    const auto e = derive_finite_positive_grammar_majorant(2, 3, expansive, boundary);
    assert(e.proved);
    assert(e.expansive_channel_detected);
    assert(!e.ordinary_generatorwise_projection_derived);
    assert(e.norm_weighted_theorem_covers_expansive_case);

    // The generic two-generator construction agrees entrywise with the legacy
    // Q/R specialization, preserving the old proof interface.
    const std::vector<NormWeightedGrammarChannel> two{
        {0, 0, 0, 1, {1, 1}}, {0, 0, 1, 1, {1, 2}}
    };
    const auto g2 = derive_finite_positive_grammar_majorant(1, 2, two, {{0,1}});
    const auto qr = specialize_finite_grammar_to_qr(1, two, {{0,1}});
    assert(g2.proved && qr.proved && g2.qr_specialization_available);
    assert(g2.count_generators[0][0][0].numerator == qr.q_count[0][0].numerator);
    assert(g2.norm_generators[1][0][0].numerator == qr.r_norm[0][0].numerator);

    const auto malformed = derive_finite_positive_grammar_majorant(
        1, 2, {{0, 0, 2, 1, {1, 1}}});
    assert(!malformed.proved);

    std::cout << "finite positive grammar majorant PASS\n";
}
