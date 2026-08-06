#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

#include "ravel/proof/stepped_face_residual_seriality.hpp"

using namespace ravel::proof;

static std::vector<std::filesystem::path> files(std::size_t n) {
    std::vector<std::filesystem::path> out{
        "out/cover_tube_defect_splice/n" + std::to_string(n) + "_M2.json"};
    if (n == 3) out.emplace_back("out/cover_tube_defect_splice/n3_M3.json");
    return out;
}

int main() {
    std::size_t checked = 0;
    for (const auto n : {3UL, 4UL}) {
        std::vector<FirstReturnWitness> witnesses;
        for (const auto& file : files(n)) {
            auto part = load_first_return_witnesses(file);
            witnesses.insert(witnesses.end(), part.begin(), part.end());
        }
        const auto product = build_first_return_joint_product(n, witnesses);
        assert(product.replayed);
        for (const auto& witness : witnesses) {
            const auto cert = derive_stepped_face_residual_seriality(product, witness);
            assert(cert.valid);
            assert(cert.every_step_serial);
            assert(cert.whole_segment_serial);
            ++checked;
        }
    }
    std::cout << "stepped face residual seriality PASS witnesses=" << checked << "\n";
}
