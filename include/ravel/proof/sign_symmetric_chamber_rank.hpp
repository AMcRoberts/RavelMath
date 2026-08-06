#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "ravel/proof/generated_n4_block_height_chamber_rank.hpp"

namespace ravel::proof {

inline std::string negate_sign_chamber(std::string chamber) {
    for (auto& c : chamber) {
        if (c == '+') c = '-';
        else if (c == '-') c = '+';
        else if (c != '0') throw std::invalid_argument("invalid sign chamber");
    }
    return chamber;
}

// Features are ordered as
// level, forcing support, forcing zero count, forcing gcd,
// moment variation, moment energy, adjacent energy,
// signed sum, maximum, minimum, intercept.
// Under x |-> -x the first seven and intercept are fixed,
// signed sum changes sign, and maximum/minimum become -minimum/-maximum.
inline ChamberRankCoefficients negate_pullback(
    const ChamberRankCoefficients& source) {
    ChamberRankCoefficients out = source;
    out.c[7] = -source.c[7];
    out.c[8] = -source.c[9];
    out.c[9] = -source.c[8];
    return out;
}

inline ChamberRankCoefficients sign_symmetric_n4_coefficients(
    const std::string& chamber) {
    const auto& table = n4_block_height_chamber_rank();
    const auto it = table.find(chamber);
    const auto jt = table.find(negate_sign_chamber(chamber));
    if (it == table.end() || jt == table.end())
        throw std::out_of_range("missing chamber coefficient");
    const auto pulled = negate_pullback(jt->second);
    ChamberRankCoefficients out;
    for (std::size_t i = 0; i < out.c.size(); ++i)
        out.c[i] = it->second.c[i] + pulled.c[i];
    return out;
}

inline bool sign_symmetric_coefficients_match(
    const std::string& chamber) {
    const auto a = sign_symmetric_n4_coefficients(chamber);
    const auto b = negate_pullback(
        sign_symmetric_n4_coefficients(negate_sign_chamber(chamber)));
    return a.c == b.c;
}

} // namespace ravel::proof
