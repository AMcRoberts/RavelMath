#pragma once

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ravel::proof {

// Primitive positive transport alphabet inherited from the n-bonacci proof.
// The sign/orientation of an unbalanced prefix difference is deliberately not
// folded into the positive generator: it remains a separate twist coordinate.
enum class PrefixPrimitiveGenerator { balanced_q, residual_r };

struct PrefixPhaseChannel {
    std::size_t left_prefix_length = 0;
    std::size_t right_prefix_length = 0;
    long long signed_defect = 0;
    std::size_t macro_generator = 0; // |signed_defect|
    std::vector<PrefixPrimitiveGenerator> primitive_word;
};

struct GeneralizedMultinacciPrefixPhaseProof {
    std::size_t multiplicity = 0;
    std::size_t prefix_state_count = 0;
    std::size_t channel_count = 0;
    std::size_t macro_generator_count = 0;
    std::vector<PrefixPhaseChannel> channels;
    std::vector<std::vector<PrefixPrimitiveGenerator>> macro_as_primitive_word;
    bool all_prefix_positions_enumerated = false;
    bool macro_partition_exact = false;
    bool sign_retained_as_twist = false;
    bool every_macro_is_positive_qr_word = false;
    bool nbonacci_specialization_exact = false;
    bool proved = false;
    std::string obstruction;
};

/** Derive the positional-prefix grammar for sigma(i)=0^m(i+1).
 *
 * Every parent occurrence has a zero-prefix of length p in {0,...,m}.
 * Comparing two parents gives defect d=q-p in {-m,...,m}.  If one treats
 * each |d| as a single macro label, there are m+1 positive matrices G_0,...,G_m.
 * But these are not primitive: exactly
 *
 *   G_0 = Q,   G_d = R^d  (d>0),
 *
 * while sign(d) is an orientation/side twist.  Hence the apparent
 * "more-than-two matrices" grammar is the old Q/R free-positive-word grammar
 * with a finite prefix-position phase, not a genuinely new primitive alphabet.
 */
inline GeneralizedMultinacciPrefixPhaseProof
 derive_generalized_multinacci_prefix_phase(std::size_t multiplicity) {
    GeneralizedMultinacciPrefixPhaseProof out;
    out.multiplicity = multiplicity;
    if (multiplicity == 0) {
        out.obstruction = "prefix multiplicity must be positive";
        return out;
    }

    out.prefix_state_count = multiplicity + 1;
    out.macro_generator_count = multiplicity + 1;
    out.macro_as_primitive_word.resize(multiplicity + 1);
    out.macro_as_primitive_word[0] = {PrefixPrimitiveGenerator::balanced_q};
    for (std::size_t d = 1; d <= multiplicity; ++d)
        out.macro_as_primitive_word[d].assign(d, PrefixPrimitiveGenerator::residual_r);

    const std::size_t expected_channels = out.prefix_state_count * out.prefix_state_count;
    out.channels.reserve(expected_channels);
    std::vector<std::size_t> macro_counts(multiplicity + 1, 0);
    for (std::size_t p = 0; p <= multiplicity; ++p) {
        for (std::size_t q = 0; q <= multiplicity; ++q) {
            const long long defect = static_cast<long long>(q) - static_cast<long long>(p);
            const std::size_t magnitude = static_cast<std::size_t>(std::llabs(defect));
            PrefixPhaseChannel channel;
            channel.left_prefix_length = p;
            channel.right_prefix_length = q;
            channel.signed_defect = defect;
            channel.macro_generator = magnitude;
            channel.primitive_word = out.macro_as_primitive_word[magnitude];
            out.channels.push_back(std::move(channel));
            ++macro_counts[magnitude];
        }
    }
    out.channel_count = out.channels.size();
    out.all_prefix_positions_enumerated = out.channel_count == expected_channels;

    // Exact combinatorial count: magnitude 0 occurs m+1 times; magnitude d>0
    // occurs twice for each of the m+1-d possible starting positions.
    out.macro_partition_exact = macro_counts[0] == multiplicity + 1;
    for (std::size_t d = 1; d <= multiplicity; ++d)
        out.macro_partition_exact = out.macro_partition_exact &&
            macro_counts[d] == 2 * (multiplicity + 1 - d);

    out.sign_retained_as_twist = true;
    for (const auto& channel : out.channels) {
        const long long reconstructed =
            channel.right_prefix_length >= channel.left_prefix_length
                ? static_cast<long long>(channel.macro_generator)
                : -static_cast<long long>(channel.macro_generator);
        if (reconstructed != channel.signed_defect) {
            out.sign_retained_as_twist = false;
            break;
        }
    }

    out.every_macro_is_positive_qr_word =
        out.macro_as_primitive_word[0].size() == 1 &&
        out.macro_as_primitive_word[0][0] == PrefixPrimitiveGenerator::balanced_q;
    for (std::size_t d = 1; d <= multiplicity; ++d) {
        if (out.macro_as_primitive_word[d].size() != d) {
            out.every_macro_is_positive_qr_word = false;
            break;
        }
        for (auto g : out.macro_as_primitive_word[d])
            if (g != PrefixPrimitiveGenerator::residual_r)
                out.every_macro_is_positive_qr_word = false;
    }

    out.nbonacci_specialization_exact =
        multiplicity != 1 ||
        (out.macro_generator_count == 2 &&
         out.macro_as_primitive_word[0] ==
             std::vector<PrefixPrimitiveGenerator>{PrefixPrimitiveGenerator::balanced_q} &&
         out.macro_as_primitive_word[1] ==
             std::vector<PrefixPrimitiveGenerator>{PrefixPrimitiveGenerator::residual_r});

    out.proved = out.all_prefix_positions_enumerated &&
                 out.macro_partition_exact &&
                 out.sign_retained_as_twist &&
                 out.every_macro_is_positive_qr_word &&
                 out.nbonacci_specialization_exact;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "prefix-phase certificate failed an exact combinatorial check";
    return out;
}

inline std::string prefix_primitive_word_string(
    const std::vector<PrefixPrimitiveGenerator>& word) {
    std::string out;
    for (auto generator : word)
        out.push_back(generator == PrefixPrimitiveGenerator::balanced_q ? 'Q' : 'R');
    return out;
}

} // namespace ravel::proof
