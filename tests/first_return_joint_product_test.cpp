#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

#include "ravel/proof/first_return_joint_product.hpp"

int main() {
    using namespace ravel::proof;
    const std::vector<std::filesystem::path> files = {
        "out/cover_tube_defect_splice/n3_M2.json",
        "out/cover_tube_defect_splice/n3_M3.json"
    };
    const auto product = build_first_return_joint_product_from_files(3, files);
    if (!product.replayed) { std::cerr << "failure=" << product.failure << "\n"; return 2; }
    assert(product.witness_count > 0);
    assert(!product.reachable.empty());
    std::cout << "first return joint product PASS witnesses="
              << product.witness_count << " reachable="
              << product.reachable.size() << "\n";
}
