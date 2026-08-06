#include <iostream>
#include <vector>
#include "ravel/proof/signed_interval_bellman.hpp"

int main() {
    std::size_t checks = 0;
    for (std::size_t n = 3; n <= 12; ++n) {
        std::size_t cap = 1;
        for (std::size_t k = 0; k < n; ++k) cap *= 3;
        cap = std::min<std::size_t>(cap, 5000);
        for (std::size_t code = 0; code < cap; ++code) {
            std::size_t work = code;
            std::vector<long long> x(n);
            for (std::size_t k = 0; k < n; ++k) {
                x[k] = static_cast<long long>(work % 3) - 1;
                work /= 3;
            }
            for (std::size_t face = 0; face < n; ++face) {
                const auto g = ravel::proof::derive_face_relative_sign_grammar(
                    x, face, (face + 1) % n, false);
                const auto role = ravel::proof::forget_absolute_face(g);
                const auto phase_role = ravel::proof::derive_phase_indexed_interval_role(g);
                if (phase_role.dimension != n || phase_role.active_phase != face) return 4;
                if (ravel::proof::interval_dimension(role) != n) return 1;
                for (int s = -1; s <= 1; ++s) {
                    const auto moved = ravel::proof::shift_append_interval_role(
                        role, s, role.target_offset);
                    auto word = ravel::proof::run_decode(role.runs);
                    word.erase(word.begin()); word.push_back(s);
                    if (ravel::proof::run_decode(moved.runs) != word) return 2;
                    ++checks;
                }
            }
        }
    }
    const auto law = ravel::proof::derive_affine_renewal_candidate({4,5},{5,8});
    if (!law.exact_on_observations || law.slope != 3 || law.intercept != -7) return 3;
    std::cout << "signed interval Bellman PASS checks=" << checks
              << " candidate=L(n)=" << law.slope << "n" << law.intercept << "\n";
    return 0;
}
