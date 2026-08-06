#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include "ravel/proof/parametric_predicate_updates.hpp"

using namespace ravel::proof;

int main() {
    std::size_t checked = 0;
    for (std::size_t n = 1; n <= 8; ++n) {
        std::size_t words = 1;
        for (std::size_t i = 0; i < n; ++i) words *= 3;
        for (std::size_t code = 0; code < words; ++code) {
            auto q = code;
            SymbolicControllerState x(n);
            for (std::size_t i = 0; i < n; ++i) {
                x[n - 1 - i] = static_cast<std::int64_t>(q % 3) - 1;
                q /= 3;
            }
            for (const auto tail : {-1LL, 0LL, 1LL}) {
                const auto proof = derive_local_word_update_proof(x, tail);
                if (!proof.valid) {
                    std::cerr << "update proof failed n=" << n << " code=" << code
                              << " tail=" << tail << " reason=" << proof.failure << '\n';
                    return 1;
                }
                ++checked;
            }
        }
        for (std::size_t remaining = 1; remaining <= 4 * (n + 1); ++remaining) {
            if (!derive_phase_update_proof(n, remaining).valid) {
                std::cerr << "phase update failed n=" << n
                          << " remaining=" << remaining << '\n';
                return 1;
            }
        }
    }
    std::cout << render_parametric_update_explanation();
    std::cout << "parametric predicate updates PASS checked=" << checked << '\n';
    return 0;
}
