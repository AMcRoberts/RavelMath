#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "ravel/substitution_neighborhood.hpp"

namespace ravel {

// Closed-form number of distinct one-adjacent-swap neighbors for the
// Class-II word family
//   0 -> 0^a 1^b 2,  1 -> 0^a 2,  2 -> 0
// in the nondegenerate chamber a>=1, b>=1.
constexpr std::size_t class_ii_adjacent_swap_count_closed_form(
        std::size_t a, std::size_t b) {
    if (a == 0 || b == 0)
        throw std::invalid_argument("Class-II closed form requires a,b >= 1");
    return 3;
}

// Closed-form number for the n-bonacci family
//   i -> 0(i+1), i<n-1;  n-1 -> 0.
constexpr std::size_t nbonacci_adjacent_swap_count_closed_form(std::size_t n) {
    if (n < 2)
        throw std::invalid_argument("n-bonacci closed form requires n >= 2");
    return n - 1;
}

inline FiniteSubstitution class_ii_sigma_ab(std::size_t a, std::size_t b) {
    if (a == 0 || b == 0)
        throw std::invalid_argument("Class-II substitution requires a,b >= 1");
    FiniteSubstitution s(3);
    s[0].insert(s[0].end(), a, std::int8_t{0});
    s[0].insert(s[0].end(), b, std::int8_t{1});
    s[0].push_back(2);
    s[1].insert(s[1].end(), a, std::int8_t{0});
    s[1].push_back(2);
    s[2].push_back(0);
    return s;
}

inline FiniteSubstitution nbonacci_substitution(std::size_t n) {
    if (n < 2 || n > 127)
        throw std::invalid_argument("encoded n-bonacci requires 2 <= n <= 127");
    FiniteSubstitution s(n);
    for (std::size_t i = 0; i + 1 < n; ++i)
        s[i] = {0, static_cast<std::int8_t>(i + 1)};
    s[n - 1] = {0};
    return s;
}

inline bool certify_class_ii_adjacent_swap_count(std::size_t a, std::size_t b) {
    const auto sigma = class_ii_sigma_ab(a, b);
    const auto neighbors = adjacent_swap_neighbors(sigma);
    if (neighbors.size() != class_ii_adjacent_swap_count_closed_form(a, b))
        return false;
    for (const auto& neighbor : neighbors)
        if (!same_incidence_matrix(sigma, neighbor.substitution)) return false;
    return true;
}

inline bool certify_nbonacci_adjacent_swap_count(std::size_t n) {
    const auto sigma = nbonacci_substitution(n);
    const auto neighbors = adjacent_swap_neighbors(sigma);
    if (neighbors.size() != nbonacci_adjacent_swap_count_closed_form(n))
        return false;
    for (const auto& neighbor : neighbors)
        if (!same_incidence_matrix(sigma, neighbor.substitution)) return false;
    return true;
}

}  // namespace ravel
