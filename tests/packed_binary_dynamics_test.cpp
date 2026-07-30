#include "ravel/fibonacci_finite.hpp"
#include "ravel/packed_binary_dynamics.hpp"

#include <cmath>
#include <cstdio>

using namespace ravel;

int main() {
    int failed = 0;
    auto check = [&](bool ok, const char* label) {
        std::printf("[%s] %s\n", ok ? "ok" : "FAIL", label);
        failed += !ok;
    };

    constexpr std::size_t n = 1u << 16;
    auto packed = packed_fibonacci_word(n);
    auto bytes = fibonacci_word(n);
    for (std::size_t k : {0u, 1u, 2u, 63u, 64u, 65535u})
        check(packed.get(k) == static_cast<bool>(bytes[k]),
              "packed Fibonacci word matches byte representation");

    packed_rule30(packed, 16);
    evolve_rule30_periodic(bytes, 16);
    for (std::size_t k : {0u, 1u, 31u, 63u, 64u, 1000u, 65535u})
        check(packed.get(k) == static_cast<bool>(bytes[k]),
              "packed Rule30 matches byte implementation");

    const std::vector<std::vector<std::size_t>> funcs = {
        {0, 1}, {0, 3}, {0, 2}, {0, 5}, {0, 1, 7}};
    const std::size_t separation = 46368;
    auto matrix = packed_parity_correlation_matrix(
        packed, funcs, separation);
    for (std::size_t i = 0; i < funcs.size(); ++i) {
        for (std::size_t j = 0; j < funcs.size(); ++j) {
            double sum = 0;
            for (std::size_t k = 0; k < n; ++k) {
                bool a = false, b = false;
                for (auto o : funcs[i]) a ^= bytes[(k + o) % n];
                for (auto o : funcs[j])
                    b ^= bytes[(k + separation + o) % n];
                sum += a == b ? 1.0 : -1.0;
            }
            check(std::abs(matrix(i, j) - sum / n) < 1e-15,
                  "bit-parallel correlation matches scalar definition");
        }
    }

    bool capped = false;
    try {
        PackedDynamicsLimits limits;
        limits.max_functionals = 2;
        (void)packed_parity_correlation_matrix(
            packed, funcs, separation, limits);
    } catch (const std::runtime_error&) {
        capped = true;
    }
    check(capped, "functional cap is enforced");

    const std::array<std::vector<std::size_t>, 4> chosen = {
        funcs[0], funcs[1], funcs[2], funcs[3]};
    const std::array<double, 4> baseline = {
        matrix(0, 2), matrix(0, 3), matrix(1, 2), matrix(1, 3)};
    const double q = 1.0 / std::sqrt(2.0);
    auto retarget = packed_parity_retarget(
        packed, chosen, separation, 2584, 1597,
        {q, q, q, -q}, baseline);
    check(std::abs(retarget.signed_chsh - 2.0 * std::sqrt(2.0)) < 0.03,
          "arbitrary packed functionals retarget near quantum CHSH");

    const auto local = packed_local_window_retarget(
        n, 8, separation, 2584, 1597, {q, q, q, -q});
    check(std::abs(local.baseline_chsh - 2.0) < 1e-3,
          "radius-8 snapped windows lie on the classical CHSH facet");
    check(std::abs(local.retargeted.signed_chsh - 2.0 * std::sqrt(2.0))
              < 0.04,
          "bounded snapped-window model retargets near quantum CHSH");
    return failed ? 1 : 0;
}
