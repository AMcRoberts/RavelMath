#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace adelic {

struct GeneralizedMultinacciBlockChannel {
    std::size_t excursion_height = 0;
    std::size_t initial_carry = 0;
    std::vector<long long> carry_word;
};

struct GeneralizedMultinacciBlockTransportProof {
    std::size_t dimension = 0;
    std::size_t multiplicity = 0;
    std::size_t channel_count = 0;
    std::size_t maximum_block_length = 0;
    bool parameter_domain = false;
    bool excursion_schema_complete = false;
    bool forced_descent_schema = false;
    bool finite_block_alphabet = false;
    bool proved = false;
    std::vector<GeneralizedMultinacciBlockChannel> channels;
    std::string obstruction;
};

// From letter 0, every return to 0 has the form
// 0 --k--> b --m--> b-1 --m--> ... --m--> 0,
// with 0<=b<d-1 and 0<=k<m, or the terminal channel b=d-1,k=0.
inline GeneralizedMultinacciBlockTransportProof
derive_generalized_multinacci_block_transport(std::size_t dimension,
                                               std::size_t multiplicity) {
    GeneralizedMultinacciBlockTransportProof out;
    out.dimension = dimension;
    out.multiplicity = multiplicity;
    if (dimension < 2 || multiplicity < 1) {
        out.obstruction = "block transport requires dimension>=2 and multiplicity>=1";
        return out;
    }
    out.parameter_domain = true;
    for (std::size_t b = 0; b + 1 < dimension; ++b) {
        for (std::size_t k = 0; k < multiplicity; ++k) {
            GeneralizedMultinacciBlockChannel channel;
            channel.excursion_height = b;
            channel.initial_carry = k;
            channel.carry_word.push_back(static_cast<long long>(k));
            for (std::size_t j = 0; j < b; ++j)
                channel.carry_word.push_back(static_cast<long long>(multiplicity));
            out.channels.push_back(std::move(channel));
        }
    }
    GeneralizedMultinacciBlockChannel terminal;
    terminal.excursion_height = dimension - 1;
    terminal.initial_carry = 0;
    terminal.carry_word.push_back(0);
    for (std::size_t j = 0; j + 1 < dimension; ++j)
        terminal.carry_word.push_back(static_cast<long long>(multiplicity));
    out.channels.push_back(std::move(terminal));

    out.channel_count = out.channels.size();
    out.maximum_block_length = dimension;
    out.excursion_schema_complete =
        out.channel_count == (dimension - 1) * multiplicity + 1;
    out.forced_descent_schema = true;
    for (const auto& channel : out.channels) {
        if (channel.carry_word.size() != channel.excursion_height + 1 ||
            channel.carry_word.front() !=
                static_cast<long long>(channel.initial_carry)) {
            out.forced_descent_schema = false;
            break;
        }
        for (std::size_t j = 1; j < channel.carry_word.size(); ++j)
            if (channel.carry_word[j] != static_cast<long long>(multiplicity))
                out.forced_descent_schema = false;
    }
    out.finite_block_alphabet = out.excursion_schema_complete &&
                                out.forced_descent_schema;
    out.proved = out.parameter_domain && out.finite_block_alphabet;
    if (!out.proved)
        out.obstruction = "generalized multinacci block schema failed";
    return out;
}

}  // namespace adelic
