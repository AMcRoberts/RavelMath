#include <iostream>
#include "ravel/proof/twisted_dimension_extension.hpp"

int main() {
    for (std::size_t n = 3; n <= 10; ++n) {
        const auto proof = ravel::proof::derive_twisted_predicted_core_extension(n);
        std::cout << ravel::proof::render_twisted_dimension_extension_report(proof);
        if (!proof.proved) return 1;
    }
    std::cout << "twisted dimension extension PASS\n";
    return 0;
}
