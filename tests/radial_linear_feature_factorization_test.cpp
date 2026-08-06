#include <cassert>
#include <cstdint>
#include <iostream>

#include "ravel/proof/radial_linear_feature_factorization.hpp"

using namespace ravel::proof;

int main() {
    std::size_t checks = 0;
    for (std::size_t n = 2; n <= 8; ++n) {
        ShellState x(n, 0);
        for (std::int64_t bound = 1; bound <= 5; ++bound) {
            enumerate_box_states_rec(n, bound, 0, x, [&](const ShellState& s) {
                // Sample deterministically to keep the regression compact.
                std::int64_t hash = 0;
                for (const auto v : s) hash = hash * 7 + v;
                if (positive_mod(hash, 97) != 0) return;
                const auto cert = derive_radial_linear_feature_factorization(s);
                assert(cert.normalized_cell_determined);
                assert(cert.obstruction.empty());
                ++checks;
            });
        }
    }
    assert(checks > 1000);
    std::cout << "radial linear feature factorization PASS checks=" << checks << "\n";
}
