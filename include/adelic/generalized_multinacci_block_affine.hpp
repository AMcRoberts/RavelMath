// Exact coefficient-lattice endpoint maps for generalized-multinacci return
// blocks.  This is the algebraic object on which the eventual rank proof
// should operate; it does not claim that the endpoint maps alone prove
// Property (F).
#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "adelic/generalized_multinacci_block_transport.hpp"

namespace adelic {

using IntegerMatrix = std::vector<std::vector<long long>>;
using IntegerVector = std::vector<long long>;

struct GeneralizedMultinacciBlockAffineMap {
    std::size_t height = 0;
    std::size_t initial_carry = 0;
    IntegerMatrix linear;
    IntegerVector offset;
};

inline IntegerMatrix generalized_multinacci_beta_inverse_matrix(
    std::size_t dimension, std::size_t multiplicity) {
    if (dimension < 2 || multiplicity < 1)
        throw std::invalid_argument("beta inverse matrix requires d>=2 and m>=1");
    IntegerMatrix a(dimension, IntegerVector(dimension, 0));
    // beta^(-1) = beta^(d-1) - m(1 + beta + ... + beta^(d-2)).
    for (std::size_t row = 0; row + 1 < dimension; ++row) a[row][0] = -static_cast<long long>(multiplicity);
    a[dimension - 1][0] = 1;
    for (std::size_t column = 1; column < dimension; ++column)
        a[column - 1][column] = 1;
    return a;
}

inline IntegerVector multiply(const IntegerMatrix& a, const IntegerVector& x) {
    IntegerVector out(a.size(), 0);
    for (std::size_t i = 0; i < a.size(); ++i)
        for (std::size_t j = 0; j < x.size(); ++j)
            out[i] += a[i][j] * x[j];
    return out;
}

inline IntegerMatrix multiply(const IntegerMatrix& a, const IntegerMatrix& b) {
    IntegerMatrix out(a.size(), IntegerVector(b.front().size(), 0));
    for (std::size_t i = 0; i < a.size(); ++i)
        for (std::size_t k = 0; k < b.size(); ++k)
            for (std::size_t j = 0; j < b[k].size(); ++j)
                out[i][j] += a[i][k] * b[k][j];
    return out;
}

inline GeneralizedMultinacciBlockAffineMap
derive_generalized_multinacci_block_affine_map(
    std::size_t dimension, std::size_t multiplicity,
    const GeneralizedMultinacciBlockChannel& channel) {
    const auto base = generalized_multinacci_beta_inverse_matrix(dimension, multiplicity);
    IntegerMatrix linear(dimension, IntegerVector(dimension, 0));
    for (std::size_t i = 0; i < dimension; ++i) linear[i][i] = 1;
    IntegerVector offset(dimension, 0);
    for (const auto carry : channel.carry_word) {
        linear = multiply(base, linear);
        offset = multiply(base, offset);
        offset[0] += carry;
    }
    return GeneralizedMultinacciBlockAffineMap{
        channel.excursion_height, channel.initial_carry,
        std::move(linear), std::move(offset)};
}

inline std::vector<GeneralizedMultinacciBlockAffineMap>
derive_generalized_multinacci_block_affine_maps(std::size_t dimension,
                                                std::size_t multiplicity) {
    const auto schema = derive_generalized_multinacci_block_transport(
        dimension, multiplicity);
    if (!schema.proved) throw std::invalid_argument(schema.obstruction);
    std::vector<GeneralizedMultinacciBlockAffineMap> maps;
    maps.reserve(schema.channels.size());
    for (const auto& channel : schema.channels)
        maps.push_back(derive_generalized_multinacci_block_affine_map(
            dimension, multiplicity, channel));
    return maps;
}

}  // namespace adelic
