// Probe (not a proof/test): computes the EXACT integer adjacency matrix
// of each Class-II neighbor's REGULAR SHELL component at a concrete
// (a, t), and its exact characteristic polynomial, to confirm it matches
// docs/FAMILY_OF_FAMILIES.md's displayed closed forms:
//   neighbor 0: x^6 * (x^4 - ((t+2)^2-2) x^2 + 1)   for 1<=t<=a-3
//   neighbor 1: x^3 * (x^2 - t(t+2))                for 1<=t<=a-2
//   neighbor 2: x^2 * (x^2 - k^2)                   for 2<=k<=a-1
// -- i.e. that lean/class_ii_neighbor_dominance.lean's neighbor{0,1,2}_
// shell_below_* theorems' lambda/t are this shell's Perron root and
// parameter, not the fixed dominant core's.

#include <cstdio>

#include "math/charpoly.hpp"
#include "ravel/class_ii_neighbor_family.hpp"

using namespace ravel;

int main() {
    // neighbor 0 at a=8, t=1..5 (1<=t<=a-3=5)
    for (long long t = 1; t <= 5; ++t) {
        const auto states = class_ii_neighbor_regular_shell_states(0, 8, t);
        const auto mat = class_ii_neighbor_compressed_matrix_for_states(0, 8, states);
        const auto poly = mathlib::charpoly_faddeev_leverrier(mat);
        std::printf("neighbor 0, a=8, t=%lld: %zu states, charpoly = ", t, states.size());
        for (std::size_t i = 0; i < poly.coeffs_.size(); ++i)
            std::printf("%s ", mathlib::str(poly.coeffs_[i]).c_str());
        std::printf("\n");
    }
    // neighbor 1 at a=8, t=1..6 (1<=t<=a-2=6)
    for (long long t = 1; t <= 6; ++t) {
        const auto states = class_ii_neighbor_regular_shell_states(1, 8, t);
        const auto mat = class_ii_neighbor_compressed_matrix_for_states(1, 8, states);
        const auto poly = mathlib::charpoly_faddeev_leverrier(mat);
        std::printf("neighbor 1, a=8, t=%lld: %zu states, charpoly = ", t, states.size());
        for (std::size_t i = 0; i < poly.coeffs_.size(); ++i)
            std::printf("%s ", mathlib::str(poly.coeffs_[i]).c_str());
        std::printf("\n");
    }
    // neighbor 2 at a=8, k=2..7 (2<=k<=a-1=7)
    for (long long k = 2; k <= 7; ++k) {
        const auto states = class_ii_neighbor_regular_shell_states(2, 8, k);
        const auto mat = class_ii_neighbor_compressed_matrix_for_states(2, 8, states);
        const auto poly = mathlib::charpoly_faddeev_leverrier(mat);
        std::printf("neighbor 2, a=8, k=%lld: %zu states, charpoly = ", k, states.size());
        for (std::size_t i = 0; i < poly.coeffs_.size(); ++i)
            std::printf("%s ", mathlib::str(poly.coeffs_[i]).c_str());
        std::printf("\n");
    }
}
