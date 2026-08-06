#include <iostream>
#include <vector>

#include "ravel/proof/first_return_joint_product.hpp"
#include "ravel/proof/residual_signature_transfer.hpp"

using namespace ravel::proof;

namespace {
std::vector<std::filesystem::path> files(std::size_t n) {
    std::vector<std::filesystem::path> out{
        "out/cover_tube_defect_splice/n" + std::to_string(n) + "_M2.json"};
    if (n == 3) out.emplace_back("out/cover_tube_defect_splice/n3_M3.json");
    return out;
}
}

int main() {
    for (const auto n : {3ULL, 4ULL}) {
        const auto product = build_first_return_joint_product_from_files(n, files(n));
        const auto proof = derive_residual_signature_transfer(product);
        std::cout << render_residual_signature_transfer_report(proof);
        if (!proof.valid) return 1;
    }
    std::cout << "residual signature transfer PASS\n";
    return 0;
}
