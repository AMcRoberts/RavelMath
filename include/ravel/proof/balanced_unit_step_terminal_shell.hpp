#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "ravel/proof/triangular_wave_terminal_shell.hpp"
#include "ravel/proof/covering_translation_tube.hpp"

namespace ravel::proof {

struct BalancedUnitStepTerminalShellCertificate {
    std::size_t dimension = 0;
    std::int64_t radius = 0;
    std::size_t period = 0;
    std::size_t repetitions = 0;
    std::vector<std::int64_t> primitive_word;
    std::vector<ShellState> cycle;
    std::vector<std::int64_t> digits;
    bool period_divides_dimension_minus_one = false;
    bool primitive_zero_sum = false;
    bool increments_admissible = false;
    bool transitions_replay = false;
    bool exact_internal_permutation = false;
    bool spectral_radius_one = false;
    bool valid = false;
    std::string obstruction;
};

inline std::size_t primitive_period(const std::vector<std::int64_t>& word) {
    if (word.empty()) return 0;
    for (std::size_t p = 1; p <= word.size(); ++p) {
        if (word.size() % p != 0) continue;
        bool ok = true;
        for (std::size_t i = p; i < word.size(); ++i)
            ok &= word[i] == word[i % p];
        if (ok) return p;
    }
    return word.size();
}

/** General terminal shell grammar.
 *
 * A zero-sum periodic scalar word w with admissible adjacent increments and
 * period P | (n-1) produces an exact n-bonacci carry cycle of n-windows.
 * The result is a permutation SCC internally, hence has spectral radius one.
 */
inline BalancedUnitStepTerminalShellCertificate
 derive_balanced_unit_step_terminal_shell(
    std::size_t n, std::vector<std::int64_t> word) {
    BalancedUnitStepTerminalShellCertificate c;
    c.dimension = n;
    if (n < 2 || word.empty()) {
        c.obstruction = "terminal shell grammar requires n>=2 and a nonempty word";
        return c;
    }
    const auto p = primitive_period(word);
    c.primitive_word.assign(word.begin(), word.begin() + static_cast<std::ptrdiff_t>(p));
    c.period = p;
    c.period_divides_dimension_minus_one = p > 0 && (n - 1) % p == 0;
    c.repetitions = c.period_divides_dimension_minus_one ? (n - 1) / p : 0;
    c.primitive_zero_sum = std::accumulate(
        c.primitive_word.begin(), c.primitive_word.end(), std::int64_t{0}) == 0;
    c.increments_admissible = true;
    c.radius = 0;
    for (std::size_t k = 0; k < p; ++k) {
        const auto next = c.primitive_word[(k + 1) % p];
        c.increments_admissible &= std::llabs(next - c.primitive_word[k]) <= 1;
        c.radius = std::max<std::int64_t>(c.radius, static_cast<std::int64_t>(std::llabs(c.primitive_word[k])));
    }
    if (!c.period_divides_dimension_minus_one || !c.primitive_zero_sum ||
        !c.increments_admissible || c.radius == 0) {
        c.obstruction = "word is not a nontrivial balanced admissible period dividing n-1";
        return c;
    }

    c.cycle.reserve(p);
    c.digits.reserve(p);
    for (std::size_t k = 0; k < p; ++k) {
        ShellState x(n, 0);
        for (std::size_t j = 0; j < n; ++j)
            x[j] = c.primitive_word[(k + j) % p];
        c.cycle.push_back(std::move(x));
        c.digits.push_back(c.primitive_word[(k + 1) % p] - c.primitive_word[k]);
    }

    c.transitions_replay = true;
    c.exact_internal_permutation = true;
    std::set<ShellState> orbit(c.cycle.begin(), c.cycle.end());
    for (std::size_t k = 0; k < p; ++k) {
        const auto& x = c.cycle[k];
        const auto& expected = c.cycle[(k + 1) % p];
        c.transitions_replay &= shell_radius(x) == c.radius;
        c.transitions_replay &= nbonacci_step(x, c.digits[k]) == expected;
        std::size_t internal = 0;
        for (const auto& y : bounded_carry_successors(x, c.radius))
            if (orbit.contains(y)) ++internal;
        c.exact_internal_permutation &= internal == 1;
    }
    c.spectral_radius_one = c.transitions_replay && c.exact_internal_permutation;
    c.valid = c.spectral_radius_one;
    if (!c.valid) c.obstruction = "balanced terminal cycle failed exact replay";
    return c;
}

} // namespace ravel::proof
