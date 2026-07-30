// Packed periodic binary dynamics and parity-functional correlations.
#pragma once

#include <bit>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace ravel {

class PackedPeriodicBits {
public:
    explicit PackedPeriodicBits(std::size_t size)
        : size_(size), words_((size + 63) / 64, 0) {
        if (size == 0 || size % 64 != 0)
            throw std::invalid_argument(
                "PackedPeriodicBits: size must be a positive multiple of 64");
    }

    std::size_t size() const { return size_; }
    std::size_t word_count() const { return words_.size(); }
    const std::vector<std::uint64_t>& words() const { return words_; }
    std::vector<std::uint64_t>& words() { return words_; }

    bool get(std::size_t index) const {
        index %= size_;
        return (words_[index / 64] >> (index % 64)) & 1u;
    }

    void set(std::size_t index, bool value) {
        index %= size_;
        const auto mask = std::uint64_t{1} << (index % 64);
        if (value) words_[index / 64] |= mask;
        else words_[index / 64] &= ~mask;
    }

    // Return bits [start,start+64) in the low-to-high bit order.
    std::uint64_t extract64(std::size_t start) const {
        start %= size_;
        const std::size_t wi = start / 64;
        const unsigned shift = static_cast<unsigned>(start % 64);
        if (shift == 0) return words_[wi];
        const std::size_t next = (wi + 1) % words_.size();
        return (words_[wi] >> shift) | (words_[next] << (64 - shift));
    }

private:
    std::size_t size_;
    std::vector<std::uint64_t> words_;
};

struct PackedDynamicsLimits {
    std::size_t max_sites = 1u << 26;
    std::size_t max_steps = 4096;
    std::size_t max_functionals = 256;
    std::size_t max_matrix_entries = 65536;
};

inline PackedPeriodicBits packed_fibonacci_word(std::size_t sites) {
    constexpr double alpha = 0.618033988749894848204586834365638118;
    PackedPeriodicBits result(sites);
    for (std::size_t k = 0; k < sites; ++k) {
        const double phase =
            static_cast<double>(k) * alpha
          - static_cast<std::uint64_t>(static_cast<double>(k) * alpha);
        result.set(k, phase < alpha);
    }
    return result;
}

inline void packed_rule30(
        PackedPeriodicBits& state, std::size_t steps,
        const PackedDynamicsLimits& limits = {}) {
    if (state.size() > limits.max_sites)
        throw std::runtime_error("packed_rule30: site cap exceeded");
    if (steps > limits.max_steps)
        throw std::runtime_error("packed_rule30: step cap exceeded");
    PackedPeriodicBits next(state.size());
    for (std::size_t step = 0; step < steps; ++step) {
        for (std::size_t w = 0; w < state.word_count(); ++w) {
            const std::size_t base = w * 64;
            const auto left = state.extract64(
                base == 0 ? state.size() - 1 : base - 1);
            const auto center = state.words()[w];
            const auto right = state.extract64(base + 1);
            next.words()[w] = left ^ (center | right);
        }
        state.words().swap(next.words());
    }
}

inline PackedPeriodicBits packed_parity_functional(
        const PackedPeriodicBits& state,
        const std::vector<std::size_t>& offsets) {
    if (offsets.empty())
        throw std::invalid_argument(
            "packed_parity_functional: offsets must be nonempty");
    PackedPeriodicBits result(state.size());
    for (std::size_t w = 0; w < state.word_count(); ++w) {
        std::uint64_t value = 0;
        for (std::size_t offset : offsets)
            value ^= state.extract64(w * 64 + offset % state.size());
        result.words()[w] = value;
    }
    return result;
}

struct PackedCorrelationMatrix {
    std::size_t function_count = 0;
    std::size_t sites = 0;
    std::vector<double> values; // row-major; A_i(k) B_j(k+separation)

    double operator()(std::size_t i, std::size_t j) const {
        return values.at(i * function_count + j);
    }
};

struct PackedRetargetResult {
    std::array<double, 4> correlators{};
    std::array<double, 4> kappa{};
    std::array<std::size_t, 4> accepted_count{};
    double signed_chsh = 0.0;
};

struct PackedLocalWindowResult {
    std::array<double, 4> baseline{};
    PackedRetargetResult retargeted;
    std::array<std::array<double, 2>, 4> snapped_arcs{};
    double baseline_chsh = 0.0;
};

inline PackedLocalWindowResult packed_local_window_retarget(
        std::size_t sites, std::size_t radius, std::size_t separation,
        std::size_t selection_stride, std::size_t setting_stride,
        const std::array<double, 4>& targets,
        const PackedDynamicsLimits& limits = {}) {
    if (sites == 0 || sites > limits.max_sites)
        throw std::runtime_error(
            "packed_local_window_retarget: site cap exceeded");
    if (radius == 0 || radius > limits.max_sites)
        throw std::runtime_error(
            "packed_local_window_retarget: radius cap exceeded");
    if (selection_stride == 0 || setting_stride == 0)
        throw std::invalid_argument(
            "packed_local_window_retarget: strides must be positive");

    constexpr double alpha = 0.618033988749894848204586834365638118;
    const double pi = std::acos(-1.0);
    std::vector<double> boundaries;
    boundaries.reserve(radius + 2);
    for (long long j = -1;
         j <= static_cast<long long>(radius); ++j) {
        double value = std::fmod(-static_cast<double>(j) * alpha, 1.0);
        if (value < 0) value += 1.0;
        boundaries.push_back(value);
    }
    std::sort(boundaries.begin(), boundaries.end());
    const auto circular_distance = [](double x, double y) {
        const double direct = std::abs(x - y);
        return std::min(direct, 1.0 - direct);
    };
    const auto snap = [&](double x) {
        const auto next = std::lower_bound(
            boundaries.begin(), boundaries.end(), x);
        const double right =
            next == boundaries.end() ? boundaries.front() : *next;
        const double left =
            next == boundaries.begin() ? boundaries.back() : *(next - 1);
        return circular_distance(x, left) <= circular_distance(x, right)
            ? left : right;
    };
    const std::array<double, 4> centers = {0.0, pi / 2, pi / 4, -pi / 4};
    PackedLocalWindowResult result;
    for (std::size_t index = 0; index < 4; ++index) {
        double lo = std::fmod(centers[index] / (2 * pi) - 0.25, 1.0);
        if (lo < 0) lo += 1.0;
        double hi = std::fmod(lo + 0.5, 1.0);
        result.snapped_arcs[index] = {snap(lo), snap(hi)};
    }
    const auto member = [](const std::array<double, 2>& arc, double theta) {
        double width = std::fmod(arc[1] - arc[0], 1.0);
        if (width < 0) width += 1.0;
        double offset = std::fmod(theta - arc[0], 1.0);
        if (offset < 0) offset += 1.0;
        return offset < width;
    };
    std::array<double, 4> baseline_sum{};
    for (std::size_t k = 0; k < sites; ++k) {
        const double theta = std::fmod(static_cast<double>(k) * alpha, 1.0);
        for (std::size_t q = 0; q < 4; ++q) {
            const std::size_t a = q / 2;
            const std::size_t b = q % 2;
            const double product =
                member(result.snapped_arcs[a], theta)
                    == member(result.snapped_arcs[2 + b], theta)
                ? 1.0 : -1.0;
            baseline_sum[q] += product;
        }
    }
    for (std::size_t q = 0; q < 4; ++q) {
        result.baseline[q] = baseline_sum[q] / static_cast<double>(sites);
        result.baseline_chsh += (q == 3 ? -1 : 1) * result.baseline[q];
        const double denominator =
            1.0 - targets[q] * result.baseline[q];
        if (!std::isfinite(targets[q]) || targets[q] < -1
                || targets[q] > 1 || std::abs(denominator) < 1e-15)
            throw std::invalid_argument(
                "packed_local_window_retarget: invalid target");
        result.retargeted.kappa[q] =
            (targets[q] - result.baseline[q]) / denominator;
    }
    std::array<double, 4> numerator{};
    separation %= sites;
    for (std::size_t k = 0; k < sites; ++k) {
        const double theta = std::fmod(static_cast<double>(k) * alpha, 1.0);
        const double theta_b = std::fmod(
            static_cast<double>(k + separation) * alpha, 1.0);
        const std::size_t q = std::min<std::size_t>(
            3, static_cast<std::size_t>(
                4.0 * std::fmod(setting_stride * theta, 1.0)));
        const std::size_t a = q / 2;
        const std::size_t b = q % 2;
        const double product =
            member(result.snapped_arcs[a], theta)
                == member(result.snapped_arcs[2 + b], theta_b)
            ? 1.0 : -1.0;
        const double phase =
            std::fmod(selection_stride * theta, 1.0);
        const double kappa = result.retargeted.kappa[q];
        if (phase < (1.0 + kappa * product)
                / (1.0 + std::abs(kappa))) {
            numerator[q] += product;
            ++result.retargeted.accepted_count[q];
        }
    }
    for (std::size_t q = 0; q < 4; ++q) {
        if (result.retargeted.accepted_count[q] == 0)
            throw std::runtime_error(
                "packed_local_window_retarget: empty accepted context");
        result.retargeted.correlators[q] =
            numerator[q] / static_cast<double>(
                result.retargeted.accepted_count[q]);
        result.retargeted.signed_chsh +=
            (q == 3 ? -1 : 1) * result.retargeted.correlators[q];
    }
    return result;
}

inline PackedCorrelationMatrix packed_parity_correlation_matrix(
        const PackedPeriodicBits& state,
        const std::vector<std::vector<std::size_t>>& function_offsets,
        std::size_t separation,
        const PackedDynamicsLimits& limits = {}) {
    const std::size_t nf = function_offsets.size();
    if (state.size() > limits.max_sites)
        throw std::runtime_error(
            "packed_parity_correlation_matrix: site cap exceeded");
    if (nf == 0 || nf > limits.max_functionals ||
            nf > limits.max_matrix_entries / nf)
        throw std::runtime_error(
            "packed_parity_correlation_matrix: functional/matrix cap exceeded");

    std::vector<PackedPeriodicBits> functions;
    functions.reserve(nf);
    for (const auto& offsets : function_offsets)
        functions.push_back(packed_parity_functional(state, offsets));

    PackedCorrelationMatrix result;
    result.function_count = nf;
    result.sites = state.size();
    result.values.resize(nf * nf);
    separation %= state.size();
    for (std::size_t i = 0; i < nf; ++i) {
        for (std::size_t j = 0; j < nf; ++j) {
            std::size_t differences = 0;
            for (std::size_t w = 0; w < state.word_count(); ++w) {
                const auto shifted =
                    functions[j].extract64(w * 64 + separation);
                differences += std::popcount(
                    functions[i].words()[w] ^ shifted);
            }
            result.values[i * nf + j] =
                1.0 - 2.0 * static_cast<double>(differences)
                    / static_cast<double>(state.size());
        }
    }
    return result;
}

inline PackedRetargetResult packed_parity_retarget(
        const PackedPeriodicBits& state,
        const std::array<std::vector<std::size_t>, 4>& function_offsets,
        std::size_t separation, std::size_t selection_stride,
        std::size_t setting_stride,
        const std::array<double, 4>& targets,
        const std::array<double, 4>& baseline,
        const std::array<int, 4>& chsh_signs = {1, 1, 1, -1},
        const PackedDynamicsLimits& limits = {}) {
    if (state.size() > limits.max_sites)
        throw std::runtime_error("packed_parity_retarget: site cap exceeded");
    if (selection_stride == 0 || setting_stride == 0)
        throw std::invalid_argument(
            "packed_parity_retarget: strides must be positive");
    std::array<PackedPeriodicBits, 4> functions = {
        packed_parity_functional(state, function_offsets[0]),
        packed_parity_functional(state, function_offsets[1]),
        packed_parity_functional(state, function_offsets[2]),
        packed_parity_functional(state, function_offsets[3])};

    PackedRetargetResult result;
    for (std::size_t q = 0; q < 4; ++q) {
        if (!std::isfinite(targets[q]) || targets[q] < -1 ||
                targets[q] > 1 || !std::isfinite(baseline[q]))
            throw std::invalid_argument(
                "packed_parity_retarget: invalid target/baseline");
        const double denominator = 1.0 - targets[q] * baseline[q];
        if (std::abs(denominator) < 1e-15)
            throw std::invalid_argument(
                "packed_parity_retarget: singular retargeting");
        result.kappa[q] = (targets[q] - baseline[q]) / denominator;
        if (chsh_signs[q] != -1 && chsh_signs[q] != 1)
            throw std::invalid_argument(
                "packed_parity_retarget: signs must be +/-1");
    }
    constexpr double alpha = 0.618033988749894848204586834365638118;
    std::array<double, 4> numerator{};
    separation %= state.size();
    for (std::size_t k = 0; k < state.size(); ++k) {
        const double theta = std::fmod(static_cast<double>(k) * alpha, 1.0);
        const std::size_t q = std::min<std::size_t>(
            3, static_cast<std::size_t>(
                4.0 * std::fmod(setting_stride * theta, 1.0)));
        const std::size_t a = q / 2;
        const std::size_t b = q % 2;
        const bool abit = functions[a].get(k);
        const bool bbit = functions[2 + b].get(k + separation);
        const double product = abit == bbit ? 1.0 : -1.0;
        const double phase =
            std::fmod(static_cast<double>(selection_stride) * theta, 1.0);
        const double kap = result.kappa[q];
        if (phase < (1.0 + kap * product) / (1.0 + std::abs(kap))) {
            numerator[q] += product;
            ++result.accepted_count[q];
        }
    }
    for (std::size_t q = 0; q < 4; ++q) {
        if (result.accepted_count[q] == 0)
            throw std::runtime_error(
                "packed_parity_retarget: empty accepted context");
        result.correlators[q] =
            numerator[q] / static_cast<double>(result.accepted_count[q]);
        result.signed_chsh += chsh_signs[q] * result.correlators[q];
    }
    return result;
}

} // namespace ravel
