// Cross-dimension laws for the generalized-multinacci transport suspension.
//
// The sofic primitive grammar is dimension-free.  The affine transport has a
// second, fibered cross-dimension structure: d is a forced-descent height and
// m is a carry-color multiplicity.  These certificates compare the finite
// return-block schemas without claiming Property-(F) closure.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "adelic/generalized_multinacci_block_transport.hpp"
#include "adelic/generalized_multinacci_block_affine.hpp"

namespace adelic {

struct GeneralizedMultinacciDimensionLiftProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t old_channel_count = 0;
    std::size_t new_channel_count = 0;
    std::size_t added_channels = 0;
    bool old_channels_embed = false;
    bool old_terminal_becomes_zero_height_new_channel = false;
    bool new_terminal_is_one_level_extension = false;
    bool new_height_channels_share_linear_part = false;
    bool new_height_offsets_are_carry_translates = false;
    bool new_terminal_has_one_extra_linear_step = false;
    bool proved = false;
    std::string obstruction;
};

struct GeneralizedMultinacciMultiplicityLiftProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t old_channel_count = 0;
    std::size_t new_channel_count = 0;
    std::size_t added_channels = 0;
    bool old_channels_embed_after_tail_relabel = false;
    bool added_color_at_each_nonterminal_height = false;
    bool terminal_tail_relabelled = false;
    bool proved = false;
    std::string obstruction;
};

inline std::vector<long long> relabel_forced_tail(
    const std::vector<long long>& word, std::size_t old_m,
    std::size_t new_m) {
    std::vector<long long> out = word;
    for (std::size_t i = 1; i < out.size(); ++i)
        if (out[i] == static_cast<long long>(old_m))
            out[i] = static_cast<long long>(new_m);
    return out;
}

inline GeneralizedMultinacciDimensionLiftProof
derive_generalized_multinacci_dimension_lift(std::size_t dimension,
                                              std::size_t multiplicity) {
    GeneralizedMultinacciDimensionLiftProof out;
    out.dimension = dimension;
    out.multiplicity = multiplicity;
    if (dimension < 2 || multiplicity < 1) {
        out.obstruction = "dimension lift requires d>=2 and m>=1";
        return out;
    }
    const auto old_schema = derive_generalized_multinacci_block_transport(
        dimension, multiplicity);
    const auto new_schema = derive_generalized_multinacci_block_transport(
        dimension + 1, multiplicity);
    if (!old_schema.proved || !new_schema.proved) {
        out.obstruction = "dimension lift source schema failed";
        return out;
    }
    out.old_channel_count = old_schema.channels.size();
    out.new_channel_count = new_schema.channels.size();
    out.added_channels = out.new_channel_count - out.old_channel_count;
    out.old_channels_embed = true;
    for (std::size_t i = 0; i + 1 < old_schema.channels.size(); ++i) {
        if (old_schema.channels[i].carry_word !=
            new_schema.channels[i].carry_word) {
            out.old_channels_embed = false;
            break;
        }
    }
    const auto& old_terminal = old_schema.channels.back();
    const auto& new_zero_height =
        new_schema.channels[(dimension - 1) * multiplicity];
    out.old_terminal_becomes_zero_height_new_channel =
        old_terminal.carry_word == new_zero_height.carry_word;
    const auto& new_terminal = new_schema.channels.back();
    std::vector<long long> expected_terminal{0};
    for (std::size_t i = 0; i < dimension; ++i)
        expected_terminal.push_back(static_cast<long long>(multiplicity));
    out.new_terminal_is_one_level_extension =
        new_terminal.carry_word == expected_terminal;
    const auto new_maps = derive_generalized_multinacci_block_affine_maps(
        dimension + 1, multiplicity);
    const std::size_t top_base = (dimension - 1) * multiplicity;
    out.new_height_channels_share_linear_part = true;
    for (std::size_t k = 1; k < multiplicity; ++k)
        out.new_height_channels_share_linear_part =
            out.new_height_channels_share_linear_part &&
            new_maps[top_base + k].linear == new_maps[top_base].linear;
    const auto beta_inverse = generalized_multinacci_beta_inverse_matrix(
        dimension + 1, multiplicity);
    IntegerMatrix power(dimension + 1, IntegerVector(dimension + 1, 0));
    for (std::size_t i = 0; i <= dimension; ++i) power[i][i] = 1;
    for (std::size_t i = 0; i + 1 < dimension; ++i)
        power = multiply(beta_inverse, power);
    // The first coordinate is the only injected carry coordinate; construct
    // A^(d-1)e_0 rather than A^(d-1)(1,...,1).
    IntegerVector e0(dimension + 1, 0);
    e0[0] = 1;
    const auto carry_direction = multiply(power, e0);
    out.new_height_offsets_are_carry_translates = true;
    for (std::size_t k = 1; k < multiplicity; ++k) {
        for (std::size_t i = 0; i <= dimension; ++i)
            if (new_maps[top_base + k].offset[i] - new_maps[top_base].offset[i] !=
                static_cast<long long>(k) * carry_direction[i])
                out.new_height_offsets_are_carry_translates = false;
    }
    const auto extra_linear = multiply(beta_inverse, new_maps[top_base].linear);
    out.new_terminal_has_one_extra_linear_step =
        extra_linear == new_maps.back().linear;
    out.proved = out.old_channel_count == (dimension - 1) * multiplicity + 1 &&
                 out.new_channel_count == dimension * multiplicity + 1 &&
                 out.added_channels == multiplicity && out.old_channels_embed &&
                 out.old_terminal_becomes_zero_height_new_channel &&
                 out.new_terminal_is_one_level_extension &&
                 out.new_height_channels_share_linear_part &&
                 out.new_height_offsets_are_carry_translates &&
                 out.new_terminal_has_one_extra_linear_step;
    if (!out.proved) out.obstruction = "dimension lift law failed";
    return out;
}

inline GeneralizedMultinacciMultiplicityLiftProof
derive_generalized_multinacci_multiplicity_lift(std::size_t dimension,
                                                std::size_t multiplicity) {
    GeneralizedMultinacciMultiplicityLiftProof out;
    out.dimension = dimension;
    out.multiplicity = multiplicity;
    if (dimension < 2 || multiplicity < 1) {
        out.obstruction = "multiplicity lift requires d>=2 and m>=1";
        return out;
    }
    const auto old_schema = derive_generalized_multinacci_block_transport(
        dimension, multiplicity);
    const auto new_schema = derive_generalized_multinacci_block_transport(
        dimension, multiplicity + 1);
    if (!old_schema.proved || !new_schema.proved) {
        out.obstruction = "multiplicity lift source schema failed";
        return out;
    }
    out.old_channel_count = old_schema.channels.size();
    out.new_channel_count = new_schema.channels.size();
    out.added_channels = out.new_channel_count - out.old_channel_count;
    out.old_channels_embed_after_tail_relabel = true;
    for (std::size_t b = 0; b + 1 < dimension &&
                         out.old_channels_embed_after_tail_relabel; ++b) {
        for (std::size_t k = 0; k < multiplicity; ++k) {
            const std::size_t old_index = b * multiplicity + k;
            const std::size_t new_index = b * (multiplicity + 1) + k;
            if (relabel_forced_tail(old_schema.channels[old_index].carry_word,
                                    multiplicity, multiplicity + 1) !=
                new_schema.channels[new_index].carry_word)
                out.old_channels_embed_after_tail_relabel = false;
        }
    }
    const auto& old_terminal = old_schema.channels.back();
    const auto& new_terminal = new_schema.channels.back();
    out.terminal_tail_relabelled =
        relabel_forced_tail(old_terminal.carry_word, multiplicity,
                            multiplicity + 1) == new_terminal.carry_word;
    out.added_color_at_each_nonterminal_height = true;
    for (std::size_t b = 0; b + 1 < dimension; ++b) {
        const std::size_t new_index = b * (multiplicity + 1) + multiplicity;
        const auto& added = new_schema.channels[new_index];
        std::vector<long long> expected{static_cast<long long>(multiplicity)};
        for (std::size_t i = 0; i < b; ++i)
            expected.push_back(static_cast<long long>(multiplicity + 1));
        if (added.carry_word != expected)
            out.added_color_at_each_nonterminal_height = false;
    }
    out.proved = out.old_channel_count == (dimension - 1) * multiplicity + 1 &&
                 out.new_channel_count == (dimension - 1) * (multiplicity + 1) + 1 &&
                 out.added_channels == dimension - 1 &&
                 out.old_channels_embed_after_tail_relabel &&
                 out.added_color_at_each_nonterminal_height &&
                 out.terminal_tail_relabelled;
    if (!out.proved) out.obstruction = "multiplicity lift law failed";
    return out;
}

}  // namespace adelic
