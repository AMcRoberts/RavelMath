#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ravel/proof/twisted_dimension_extension.hpp"

int main(int argc, char** argv) {
    std::size_t max_dimension = 10;
    std::string lean_path = "lean/generated/twisted_dimension_extension.lean";
    if (argc > 1) max_dimension = static_cast<std::size_t>(std::stoull(argv[1]));
    if (argc > 2) lean_path = argv[2];
    if (max_dimension < 3) throw std::invalid_argument("max dimension must be >= 3");

    bool ok = true;
    for (std::size_t n = 3; n <= max_dimension; ++n) {
        const auto proof = ravel::proof::derive_twisted_predicted_core_extension(n);
        std::cout << ravel::proof::render_twisted_dimension_extension_report(proof);
        ok = ok && proof.proved;
    }
    std::ofstream lean(lean_path);
    if (!lean) throw std::runtime_error("cannot open Lean output");
    lean << ravel::proof::render_twisted_dimension_extension_lean();
    std::cout << "lean_output=" << lean_path << "\n";
    return ok ? 0 : 1;
}
