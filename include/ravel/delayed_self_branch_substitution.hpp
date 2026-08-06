#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace ravel {

inline std::vector<std::vector<std::int8_t>> delayed_self_branch_rule(std::size_t D) {
    if (D < 2 || D > 127) throw std::invalid_argument("delayed_self_branch_rule requires 2 <= D <= 127");
    std::vector<std::vector<std::int8_t>> r(D);
    for (std::size_t i = 0; i + 1 < D; ++i) r[i] = {static_cast<std::int8_t>(i + 1)};
    r[D - 1] = {0, static_cast<std::int8_t>(D - 1)};
    return r;
}

inline std::vector<long long> delayed_self_branch_characteristic_coefficients(std::size_t D) {
    if (D < 2) throw std::invalid_argument("D >= 2 required");
    std::vector<long long> c(D + 1, 0);
    c[0] = -1;
    c[D - 1] = -1;
    c[D] = 1;
    return c; // x^D - x^(D-1) - 1
}

} // namespace ravel
