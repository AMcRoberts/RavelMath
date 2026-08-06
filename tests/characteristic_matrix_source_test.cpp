#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ravel/proof/characteristic_matrix_source.hpp"

using ravel::proof::characteristic_matrix_for_n;
using ravel::proof::cofactor_minor;
using ravel::proof::nbonacci_charmpoly;
using ravel::proof::q_matrix_layout;
using ravel::proof::r_matrix_layout;

int main() {
    int failures = 0;
    for (std::size_t n = 2; n <= 8; ++n) {
        const auto src = characteristic_matrix_for_n(n);
        const auto recomputed = mathlib::charpoly_faddeev_leverrier(src.entries);
        if (!(recomputed == src.characteristic)) {
            std::printf("[FAIL] n=%zu charpoly mismatch\n", n);
            ++failures;
            continue;
        }
        if (recomputed != nbonacci_charmpoly(n)) {
            std::printf("[FAIL] n=%zu generator mismatch\n", n);
            ++failures;
            continue;
        }
        const auto ql = q_matrix_layout(n);
        const auto rl = r_matrix_layout(n);
        if (ql.rows != n - 1 || ql.cols != n - 1 || rl.rows != n - 1 || rl.cols != n - 1) {
            std::printf("[FAIL] n=%zu q/r layout\n", n);
            ++failures;
        } else {
            std::printf("[OK] n=%zu Cayley-Hamilton companion + q/r layout (%zux%zu)\n",
                        n, ql.rows, ql.cols);
        }
    }
    if (failures != 0) {
        std::printf("characteristic_matrix_source_test: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("characteristic_matrix_source_test: PASS (7/7)\n");
    return 0;
}
