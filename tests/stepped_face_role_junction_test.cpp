#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

#include "ravel/proof/stepped_face_role_junction.hpp"

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
            const auto cert = derive_stepped_face_role_relation(product, witness);
            assert(cert.valid);
            assert(cert.every_source_role_continues);
            assert(cert.every_target_role_has_predecessor);
            ++checked;
        }
    }

    SteppedFaceRoleRelationCertificate a;
    a.source_roles = {{0, 1}, {1, -1}};
    a.target_roles = {{0, 1}, {1, -1}};
    a.relation = {{false, true}, {true, false}};
    a.every_source_role_continues = true;
    a.every_target_role_has_predecessor = true;
    a.transition_replay = true;
    a.valid = true;
    const auto lap = derive_stepped_face_role_lap({a});
    assert(lap.valid);
    assert(lap.role_cycle.valid);
    assert(!lap.selected_roles.empty());

    auto bad = a;
    bad.target_roles = {{2, 1}};
    bad.relation = {{true}, {true}};
    const auto malformed = derive_stepped_face_role_lap({bad});
    assert(!malformed.valid);

    std::cout << "stepped face role junction PASS witnesses=" << checked << "\n";
}
