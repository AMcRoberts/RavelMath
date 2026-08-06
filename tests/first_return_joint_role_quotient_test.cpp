#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

#include "ravel/proof/first_return_joint_role_quotient.hpp"

int main() {
    using namespace ravel::proof;
    const std::vector<std::filesystem::path> files = {
        "out/cover_tube_defect_splice/n3_M2.json",
        "out/cover_tube_defect_splice/n3_M3.json"
    };
    const auto product = build_first_return_joint_product_from_files(3, files);
    const auto proof = synthesize_reachable_joint_role_quotient(product);
    assert(proof.role_normalized);
    assert(proof.predecessor_congruent);
    assert(proof.exact_on_reachable_product);
    std::cout << "first return joint role quotient PASS classes="
              << proof.refined_classes << " rounds=" << proof.refinement_rounds << "\n";
}
