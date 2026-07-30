#include "ravel/fibonacci_selection.hpp"

#include <cmath>
#include <cstdio>
#include <stdexcept>

using namespace ravel;

int main() {
    int failed = 0;
    auto check = [&](bool condition, const char* label) {
        std::printf("[%s] %s\n", condition ? "ok" : "FAIL", label);
        if (!condition) ++failed;
    };
    // F_16=2584 is even: the historical exact theorem predicts zero
    // stride error at shift zero for the quantum targets.
    auto result = fibonacci_selection_exact(46368, 2584, 0.0);
    auto target = quantum_chsh_targets();
    for (std::size_t i = 0; i < 4; ++i)
        check(std::abs(result.correlators[i] - target[i]) < 1e-11,
              "even setting stride reproduces its target correlator");
    check(std::abs(result.chsh - 2.0 * std::sqrt(2.0)) < 1e-11,
          "even setting stride reproduces Tsirelson CHSH");
    check(result.partition_cells <= 4 * 2584 + 8,
          "partition memory is linear in setting stride");

    // Historical theorem_final_results.json fixtures.  These deliberately
    // include the even F_13 case that is not exact, preventing a regression
    // to the overstrong "all even setting strides are exact" claim.
    auto f10 = fibonacci_selection_exact(2584, 144, 0.0);
    check(std::abs(std::abs(f10.chsh - 2.0 * std::sqrt(2.0))
                   - 2.9550260016719676e-09) < 1e-12,
          "F_10 fixture matches the historical exact evaluator");
    auto f13 = fibonacci_selection_exact(10946, 610, 0.0);
    check(std::abs(std::abs(f13.chsh - 2.0 * std::sqrt(2.0))
                   - 4.726774131174061e-04) < 1e-12,
          "F_13 counterexample to blanket even-stride exactness is preserved");

    bool threw = false;
    try {
        FibonacciSelectionLimits limits;
        limits.max_partition_cells = 100;
        (void)fibonacci_selection_exact(46368, 2584, 0.0, target, limits);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    check(threw, "partition cap is enforced");
    return failed ? 1 : 0;
}
