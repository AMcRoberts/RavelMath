#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/defect_corrected_shell_rank.hpp"

namespace ravel::proof {

struct TriangularWaveTerminalShellCertificate {
    std::size_t dimension = 0;
    std::int64_t radius = 0;
    std::size_t period = 0;
    std::size_t repetitions = 0;
    std::vector<std::int64_t> wave;
    std::vector<ShellState> cycle;
    std::vector<std::int64_t> digits;
    bool parameter_relation = false;
    bool wave_zero_sum = false;
    bool wave_unit_slope = false;
    bool states_on_shell = false;
    bool transitions_replay = false;
    bool closes = false;
    bool cycle_permutation = false;
    bool cycle_spectral_radius_one = false;
    bool refutes_uniform_shell_exclusion = false;
    std::string obstruction;
};

inline std::vector<std::int64_t> triangular_shell_wave(std::int64_t radius) {
    std::vector<std::int64_t> wave;
    if (radius < 1) return wave;
    wave.reserve(static_cast<std::size_t>(4 * radius));
    for (std::int64_t v = radius; v >= -radius; --v) wave.push_back(v);
    for (std::int64_t v = -radius + 1; v <= radius - 1; ++v) wave.push_back(v);
    return wave;
}

/** Closed-form recurrent shell family.
 *
 * Let w be the zero-sum triangular wave of period P=4M and unit adjacent
 * increments.  Whenever n=qP+1, the n-window state
 *
 *   x_k=(w_k,w_{k+1},...,w_{k+n-1})
 *
 * evolves to x_{k+1} with digit d_k=w_{k+1}-w_k in {-1,0,1}.  Indeed the
 * last n-1 coordinates contain q complete wave periods and therefore sum to
 * zero, so the carry tail is w_k and the digit changes it to w_{k+1}.
 * The resulting P states form a permutation cycle at shell M.
 */
inline TriangularWaveTerminalShellCertificate
 derive_triangular_wave_terminal_shell(std::size_t n, std::int64_t radius) {
    TriangularWaveTerminalShellCertificate c;
    c.dimension = n;
    c.radius = radius;
    if (radius < 1 || n < 2) {
        c.obstruction = "triangular shell family requires radius>=1 and n>=2";
        return c;
    }
    c.wave = triangular_shell_wave(radius);
    c.period = c.wave.size();
    if (c.period == 0 || (n - 1) % c.period != 0) {
        c.obstruction = "dimension does not satisfy n = q*(4M)+1";
        return c;
    }
    c.repetitions = (n - 1) / c.period;
    c.parameter_relation = c.repetitions > 0;
    c.wave_zero_sum = std::accumulate(c.wave.begin(), c.wave.end(),
                                      std::int64_t{0}) == 0;
    c.wave_unit_slope = true;
    for (std::size_t k = 0; k < c.period; ++k) {
        const auto next = c.wave[(k + 1) % c.period];
        c.wave_unit_slope &= std::llabs(next - c.wave[k]) == 1;
    }
    if (!(c.parameter_relation && c.wave_zero_sum && c.wave_unit_slope)) {
        c.obstruction = "triangular wave grammar failed";
        return c;
    }

    c.cycle.reserve(c.period);
    c.digits.reserve(c.period);
    for (std::size_t k = 0; k < c.period; ++k) {
        ShellState x(n, 0);
        for (std::size_t j = 0; j < n; ++j)
            x[j] = c.wave[(k + j) % c.period];
        c.cycle.push_back(std::move(x));
        c.digits.push_back(c.wave[(k + 1) % c.period] - c.wave[k]);
    }

    c.states_on_shell = true;
    c.transitions_replay = true;
    c.cycle_permutation = true;
    for (std::size_t k = 0; k < c.period; ++k) {
        const auto& x = c.cycle[k];
        const auto& y = c.cycle[(k + 1) % c.period];
        c.states_on_shell &= shell_radius(x) == radius;
        const auto successors = bounded_carry_successors(x, radius);
        std::size_t internal = 0;
        bool found = false;
        for (const auto& z : successors) {
            if (z == y) found = true;
            if (std::find(c.cycle.begin(), c.cycle.end(), z) != c.cycle.end())
                ++internal;
        }
        c.transitions_replay &= found;
        c.cycle_permutation &= internal == 1;
    }
    // The edge replay includes the final state back to the first state.
    c.closes = c.transitions_replay;
    c.cycle_spectral_radius_one = c.cycle_permutation && c.closes;
    c.refutes_uniform_shell_exclusion = c.states_on_shell &&
        c.transitions_replay && c.closes && c.cycle_permutation;
    if (!c.refutes_uniform_shell_exclusion)
        c.obstruction = "closed triangular shell cycle did not replay";
    return c;
}


inline std::vector<std::int64_t>
 triangular_wave_terminal_radii(std::size_t n) {
    std::vector<std::int64_t> radii;
    if (n < 2) return radii;
    for (std::int64_t radius = 1;
         static_cast<std::size_t>(4 * radius) <= n - 1; ++radius)
        if ((n - 1) % static_cast<std::size_t>(4 * radius) == 0)
            radii.push_back(radius);
    return radii;
}

} // namespace ravel::proof
