#include <filesystem>
#include <iostream>
#include <vector>

#include "ravel/proof/symbolic_residual_formula.hpp"

int main() {
    using namespace ravel::proof;
    const std::vector<std::filesystem::path> files3 = {
        "out/cover_tube_defect_splice/n3_M2.json",
        "out/cover_tube_defect_splice/n3_M3.json"
    };
    const std::vector<std::filesystem::path> files4 = {
        "out/cover_tube_defect_splice/n4_M2.json"
    };
    for (const auto& [n, files] : std::vector<std::pair<std::size_t, std::vector<std::filesystem::path>>>{{3, files3}, {4, files4}}) {
        const auto product = build_first_return_joint_product_from_files(n, files);
        if (!product.replayed) {
            std::cerr << "joint product failure n=" << n << ": " << product.failure << "\n";
            return 1;
        }
        const auto proof = derive_symbolic_residual_formula_proof(product);
        std::cout << render_symbolic_residual_formula_report(proof);
        if (!proof.valid) return 2;
    }
    std::cout << "symbolic residual formula PASS\n";
    return 0;
}
