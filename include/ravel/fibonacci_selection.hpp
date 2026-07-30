// Exact infinite-chain measure for the Fibonacci selection model.
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace ravel {

struct FibonacciSelectionLimits {
    std::size_t max_partition_cells = 2000000;
};

struct FibonacciSelectionResult {
    std::array<double, 4> correlators{};
    std::array<double, 4> accepted_measure{};
    double chsh = 0.0;
    std::size_t partition_cells = 0;
};

inline std::array<double, 4> quantum_chsh_targets() {
    const double q = std::sqrt(0.5);
    return {q, q, q, -q};
}

inline double fibonacci_accept_measure(
        double u, double v, double threshold, std::size_t stride) {
    auto primitive = [threshold, stride](double t) {
        double x = static_cast<double>(stride) * t;
        double whole = std::floor(x);
        return whole * threshold / static_cast<double>(stride)
             + std::min((x - whole) / static_cast<double>(stride),
                        threshold / static_cast<double>(stride));
    };
    return primitive(v) - primitive(u);
}

inline double fibonacci_window_sign(double theta, double center) {
    constexpr double pi = 3.141592653589793238462643383279502884;
    double angle = std::fmod(2.0 * pi * theta - center, 2.0 * pi);
    if (angle < 0.0) angle += 2.0 * pi;
    return (angle < pi / 2.0 || angle > 3.0 * pi / 2.0) ? 1.0 : -1.0;
}

inline FibonacciSelectionResult fibonacci_selection_exact(
        std::size_t selection_stride,
        std::size_t setting_stride,
        double shift,
        const std::array<double, 4>& targets = quantum_chsh_targets(),
        const FibonacciSelectionLimits& limits = {}) {
    if (selection_stride == 0 || setting_stride == 0)
        throw std::invalid_argument("fibonacci_selection_exact: strides must be positive");
    if (!std::isfinite(shift))
        throw std::invalid_argument("fibonacci_selection_exact: shift must be finite");
    constexpr double pi = 3.141592653589793238462643383279502884;
    const std::array<double, 2> x = {0.0, pi / 2.0};
    const std::array<double, 2> y = {pi / 4.0, -pi / 4.0};
    const std::array<std::pair<std::size_t, std::size_t>, 4> context = {
        std::pair<std::size_t, std::size_t>{0, 0}, {0, 1}, {1, 0}, {1, 1}};

    std::vector<double> breaks;
    const std::size_t base_cells = 4 * setting_stride;
    if (base_cells + 9 > limits.max_partition_cells)
        throw std::runtime_error(
            "fibonacci_selection_exact: partition cell cap exceeded");
    breaks.reserve(base_cells + 9);
    for (std::size_t j = 0; j <= base_cells; ++j)
        breaks.push_back(static_cast<double>(j) / base_cells);
    for (double center : x) {
        for (double offset : {0.25, 0.75}) {
            double t = std::fmod(center / (2.0 * pi) + offset, 1.0);
            if (t < 0.0) t += 1.0;
            breaks.push_back(t);
        }
    }
    for (double center : y) {
        for (double offset : {0.25, 0.75}) {
            double t = std::fmod(
                center / (2.0 * pi) + offset - shift, 1.0);
            if (t < 0.0) t += 1.0;
            breaks.push_back(t);
        }
    }
    std::sort(breaks.begin(), breaks.end());
    breaks.erase(std::unique(breaks.begin(), breaks.end(),
                             [](double a, double b) {
                                 return std::abs(a - b) < 1e-15;
                             }),
                 breaks.end());
    if (breaks.empty() || breaks.front() > 1e-15)
        breaks.insert(breaks.begin(), 0.0);
    if (breaks.back() < 1.0 - 1e-15) breaks.push_back(1.0);

    std::array<double, 4> numerator{};
    std::array<double, 4> denominator{};
    for (std::size_t i = 0; i + 1 < breaks.size(); ++i) {
        double u = breaks[i], v = breaks[i + 1];
        if (v - u < 1e-15) continue;
        double mid = 0.5 * (u + v);
        double setting_phase = std::fmod(
            static_cast<double>(setting_stride) * mid, 1.0);
        std::size_t q = std::min<std::size_t>(
            3, static_cast<std::size_t>(4.0 * setting_phase));
        auto [a, b] = context[q];
        double product =
            fibonacci_window_sign(mid, x[a])
          * fibonacci_window_sign(
                std::fmod(mid + shift + 1.0, 1.0), y[b]);
        double difference = x[a] - y[b];
        double wrapped = std::abs(std::remainder(difference, 2.0 * pi));
        double baseline = 1.0 - 2.0 * wrapped / pi;
        double kappa =
            (targets[q] - baseline) / (1.0 - targets[q] * baseline);
        double threshold =
            (1.0 + kappa * product) / (1.0 + std::abs(kappa));
        double measure = fibonacci_accept_measure(
            u, v, threshold, selection_stride);
        numerator[q] += product * measure;
        denominator[q] += measure;
    }

    FibonacciSelectionResult result;
    result.partition_cells = breaks.size() - 1;
    result.accepted_measure = denominator;
    for (std::size_t q = 0; q < 4; ++q) {
        if (!(denominator[q] > 0.0))
            throw std::runtime_error(
                "fibonacci_selection_exact: empty accepted context");
        result.correlators[q] = numerator[q] / denominator[q];
    }
    result.chsh = result.correlators[0] + result.correlators[1]
                + result.correlators[2] - result.correlators[3];
    return result;
}

}  // namespace ravel
