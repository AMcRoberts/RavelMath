// check_sigma12_coincidence.cpp
//
// Runs the project's existing check_strong_coincidence (built for the
// adelic/non-unit tiling work, thread B) against sigma1 and sigma2,
// thread A's own unimodular Pisot counterexamples (lambda(G_B) !=
// BP-rho_nc, proven exactly). Neither substitution is covered by any
// known theorem (checked against the literature -- see conversation);
// strong coincidence is the standard, decidable, NECESSARY condition
// for the Pisot Substitution Conjecture, so this is the natural first
// diagnostic on whether they are honestly-open or something sharper.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/check_sigma12_coincidence.cpp math/out/libmath.a -o check_sigma12_coincidence

#include <array>
#include <cstdio>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"

int main() {
    // sigma1: 0 -> 0001, 1 -> 002, 2 -> 0
    std::array<std::vector<long long>, 3> sigma1 = {
        std::vector<long long>{0, 0, 0, 1},
        std::vector<long long>{0, 0, 2},
        std::vector<long long>{0},
    };
    // sigma2: 0 -> 001, 1 -> 0002, 2 -> 0
    std::array<std::vector<long long>, 3> sigma2 = {
        std::vector<long long>{0, 0, 1},
        std::vector<long long>{0, 0, 0, 2},
        std::vector<long long>{0},
    };

    for (auto& [name, images] : {std::pair{"sigma1", sigma1}, std::pair{"sigma2", sigma2}}) {
        auto res = adelic::check_strong_coincidence<3>(images, /*max_depth=*/40, /*max_word_len=*/20'000'000);
        std::printf("%s: holds=%s inconclusive=%s depth_reached=%lld unresolved_pairs=%lld\n",
                    name, res.holds ? "true" : "false", res.inconclusive ? "true" : "false",
                    res.depth_reached, res.unresolved_pairs);
    }
    return 0;
}
