#include <iostream>
#include <vector>

#include "ravel/proof/face_relative_sign_grammar.hpp"

int main() {
    std::size_t checks = 0;
    for (std::size_t n = 3; n <= 12; ++n) {
        std::size_t words = 1;
        for (std::size_t k = 0; k < n; ++k) words *= 3;
        const std::size_t cap = std::min<std::size_t>(words, 20000);
        for (std::size_t code = 0; code < cap; ++code) {
            auto work = code;
            std::vector<long long> x(n, 0);
            for (std::size_t k = 0; k < n; ++k) {
                x[k] = static_cast<long long>(work % 3) - 1;
                work /= 3;
            }
            for (std::size_t face = 0; face < n; ++face) {
                const auto proof =
                    ravel::proof::derive_face_relative_sign_grammar(
                        x, face, (face + 1) % n, false);
                const auto symmetric =
                    ravel::proof::derive_face_relative_sign_grammar(
                        x, face, (face + 1) % n, true);
                if (!proof.replayed || !symmetric.replayed) return 1;
                checks += 2;
            }
        }
    }
    std::cout << "face relative sign grammar PASS checks=" << checks << "\n";
    return 0;
}