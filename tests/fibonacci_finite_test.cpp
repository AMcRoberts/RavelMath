#include "ravel/fibonacci_finite.hpp"

#include <cmath>
#include <cstdio>

using namespace ravel;

int main() {
    int failed = 0;
    auto check = [&](bool ok, const char* label) {
        std::printf("[%s] %s\n", ok ? "ok" : "FAIL", label);
        failed += !ok;
    };
    auto unbiased = fibonacci_selection_finite(
        1u << 18, 46368, 2584, 1597, nullptr);
    check(std::isfinite(unbiased.chsh), "streamed polarizer result is finite");
    for (auto count : unbiased.accepted_count)
        check(count > 60000 && count < 70000,
              "unbiased settings occupy approximately one quarter");

    auto targets = quantum_chsh_targets();
    auto biased = fibonacci_selection_finite(
        1u << 18, 46368, 2584, 1597, &targets);
    check(std::abs(biased.chsh - 2.0 * std::sqrt(2.0)) < 0.02,
          "finite polarizer selection approaches quantum CHSH");
    auto long_separation = fibonacci_selection_finite(
        1u << 16, (1u << 16) + 46368, 2584, 1597, nullptr);
    auto reduced_separation = fibonacci_selection_finite(
        1u << 16, 46368, 2584, 1597, nullptr);
    check(std::abs(long_separation.chsh - reduced_separation.chsh) > 1e-3,
          "polarizer separation is not incorrectly reduced modulo N");

    auto ca0 = fibonacci_selection_finite(
        1u << 16, 46368, 2584, 1597, nullptr,
        FibonacciOutcomeMode::Rule30, 16);
    auto ca = fibonacci_selection_finite(
        1u << 16, 46368, 2584, 1597, &targets,
        FibonacciOutcomeMode::Rule30, 16, &ca0.correlators);
    check(std::isfinite(ca.chsh), "Rule30 retargeted result is finite");

    bool capped = false;
    try {
        FibonacciFiniteLimits limits;
        limits.max_sites = 100;
        (void)fibonacci_selection_finite(
            101, 1, 1, 1, nullptr, FibonacciOutcomeMode::Polarizer,
            0, nullptr, limits);
    } catch (const std::runtime_error&) {
        capped = true;
    }
    check(capped, "site cap is enforced before allocation");
    return failed ? 1 : 0;
}
