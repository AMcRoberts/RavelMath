// Bounded finite-chain evaluator for Fibonacci selection and Rule 30.
#pragma once

#include "ravel/fibonacci_selection.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace ravel {

enum class FibonacciOutcomeMode { Polarizer, Rule30 };

struct FibonacciFiniteLimits {
    std::size_t max_sites = 1u << 24;
    std::size_t max_ca_steps = 1024;
};

struct FibonacciFiniteResult {
    std::array<double, 4> correlators{};
    std::array<double, 4> kappa{};
    std::array<std::size_t, 4> accepted_count{};
    double chsh = 0.0;
    std::size_t sites = 0;
};

inline std::vector<std::uint8_t> fibonacci_word(std::size_t n) {
    constexpr double alpha = 0.618033988749894848204586834365638118;
    std::vector<std::uint8_t> word(n);
    for (std::size_t k = 0; k < n; ++k)
        word[k] = std::fmod(static_cast<double>(k) * alpha, 1.0) < alpha;
    return word;
}

inline void evolve_rule30_periodic(
        std::vector<std::uint8_t>& state, std::size_t steps) {
    if (state.empty()) return;
    std::vector<std::uint8_t> next(state.size());
    const std::size_t n = state.size();
    for (std::size_t step = 0; step < steps; ++step) {
        for (std::size_t k = 0; k < n; ++k) {
            const auto left = state[k == 0 ? n - 1 : k - 1];
            const auto right = state[k + 1 == n ? 0 : k + 1];
            next[k] = left ^ (state[k] | right);
        }
        state.swap(next);
    }
}

inline FibonacciFiniteResult fibonacci_selection_finite(
        std::size_t sites, std::size_t separation,
        std::size_t selection_stride, std::size_t setting_stride,
        const std::array<double, 4>* targets,
        FibonacciOutcomeMode mode = FibonacciOutcomeMode::Polarizer,
        std::size_t ca_steps = 16,
        const std::array<double, 4>* ca_baseline = nullptr,
        const FibonacciFiniteLimits& limits = {}) {
    if (sites == 0 || selection_stride == 0 || setting_stride == 0)
        throw std::invalid_argument(
            "fibonacci_selection_finite: sites and strides must be positive");
    if (sites > limits.max_sites)
        throw std::runtime_error("fibonacci_selection_finite: site cap exceeded");
    if (ca_steps > limits.max_ca_steps)
        throw std::runtime_error("fibonacci_selection_finite: CA step cap exceeded");
    if (mode == FibonacciOutcomeMode::Rule30 && targets && !ca_baseline)
        throw std::invalid_argument(
            "fibonacci_selection_finite: Rule30 retargeting needs a baseline");

    constexpr double alpha = 0.618033988749894848204586834365638118;
    constexpr double pi = 3.141592653589793238462643383279502884;
    const std::array<double, 2> x = {0.0, pi / 2.0};
    const std::array<double, 2> y = {pi / 4.0, -pi / 4.0};
    const std::array<std::pair<std::size_t, std::size_t>, 4> context = {
        std::pair<std::size_t, std::size_t>{0, 0}, {0, 1}, {1, 0}, {1, 1}};

    std::vector<std::uint8_t> ca;
    if (mode == FibonacciOutcomeMode::Rule30) {
        ca = fibonacci_word(sites);
        evolve_rule30_periodic(ca, ca_steps);
    }

    FibonacciFiniteResult result;
    result.sites = sites;
    std::array<double, 4> numerator{};
    const std::array<double, 4> polarizer_baseline = {0.5, 0.5, 0.5, -0.5};
    for (std::size_t q = 0; q < 4; ++q) {
        if (!targets) continue;
        const double target = (*targets)[q];
        if (!std::isfinite(target) || target < -1.0 || target > 1.0)
            throw std::invalid_argument("fibonacci_selection_finite: invalid target");
        const double baseline = mode == FibonacciOutcomeMode::Polarizer
            ? polarizer_baseline[q] : (*ca_baseline)[q];
        const double denominator = 1.0 - target * baseline;
        if (std::abs(denominator) < 1e-15)
            throw std::invalid_argument(
                "fibonacci_selection_finite: singular retargeting");
        result.kappa[q] = (target - baseline) / denominator;
    }

    const std::size_t ca_separation = separation % sites;
    for (std::size_t k = 0; k < sites; ++k) {
        const double theta = std::fmod(static_cast<double>(k) * alpha, 1.0);
        const double setting_phase =
            std::fmod(static_cast<double>(setting_stride) * theta, 1.0);
        const std::size_t q = std::min<std::size_t>(
            3, static_cast<std::size_t>(4.0 * setting_phase));
        const auto [a, b] = context[q];
        double product;
        if (mode == FibonacciOutcomeMode::Polarizer) {
            const double theta_b = std::fmod(
                static_cast<double>(k + separation) * alpha, 1.0);
            product = fibonacci_window_sign(theta, x[a])
                    * fibonacci_window_sign(theta_b, y[b]);
        } else {
            const auto xor_sign = [&](std::size_t pos, std::size_t offset) {
                return ca[pos] == ca[(pos + offset) % sites] ? 1.0 : -1.0;
            };
            product = xor_sign(k, a == 0 ? 1 : 3)
                    * xor_sign((k + ca_separation) % sites, b == 0 ? 2 : 5);
        }
        bool accepted = true;
        if (targets) {
            const double phase =
                std::fmod(static_cast<double>(selection_stride) * theta, 1.0);
            const double kap = result.kappa[q];
            accepted = phase < (1.0 + kap * product) / (1.0 + std::abs(kap));
        }
        if (accepted) {
            numerator[q] += product;
            ++result.accepted_count[q];
        }
    }
    for (std::size_t q = 0; q < 4; ++q) {
        if (result.accepted_count[q] == 0)
            throw std::runtime_error(
                "fibonacci_selection_finite: empty accepted context");
        result.correlators[q] =
            numerator[q] / static_cast<double>(result.accepted_count[q]);
    }
    result.chsh = result.correlators[0] + result.correlators[1]
                + result.correlators[2] - result.correlators[3];
    return result;
}

}  // namespace ravel
