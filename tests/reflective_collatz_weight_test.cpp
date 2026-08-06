#include <cstdio>
#include <vector>

#include "ravel/proof/reflective_collatz_weight.hpp"

int main() {
    // Fibonacci incidence has Perron root phi.  The one-cycle matrix has root
    // one, so the derived exact brackets must eventually separate.
    const std::vector<std::vector<long long>> small{{1}};
    const std::vector<std::vector<long long>> fib{{1, 1}, {1, 0}};
    const auto proof = ravel::proof::derive_reflective_perron_dominance(
        small, fib, 24);
    if (!proof.replayed || !proof.strict ||
        !proof.competitor.positive || !proof.core.positive) {
        std::fprintf(stderr, "reflective Collatz weight FAIL\n");
        return 1;
    }
    std::printf("reflective Collatz weight PASS iterations=%zu\n",
                proof.core.iterations);
    return 0;
}
