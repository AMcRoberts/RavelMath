// Finding 38, wired through the reflection pipeline: exact dual of
// the Finding 17 test. Uses two of Finding 37's real substitutions
// (Barge's class requires constant last letter as half its
// hypothesis, so these are genuine, not constructed for this test).

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/constant_last_letter_forces_depth1_coincidence.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("constant_last_letter_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Finding 37's ex1 and ex3 (both end every image in letter 0).
        auto cert1 = check_constant_last_letter_forces_depth1<4>({
            std::vector<long long>{2,3,1,0}, std::vector<long long>{1,0,3,0},
            std::vector<long long>{3,3,0,0}, std::vector<long long>{0,3,0}});
        assert(cert1.has_constant_last_letter);

        auto cert3 = check_constant_last_letter_forces_depth1<4>({
            std::vector<long long>{3,1,0,0}, std::vector<long long>{1,2,0},
            std::vector<long long>{0,2,0}, std::vector<long long>{2,0,2,0}});
        assert(cert3.has_constant_last_letter);

        // Control: images with no shared last letter -- must not record.
        auto cert_no = check_constant_last_letter_forces_depth1<3>({
            std::vector<long long>{0, 1}, std::vector<long long>{1, 2}, std::vector<long long>{2, 0}});
        assert(!cert_no.has_constant_last_letter);
    }

    auto nodes = trace.find<mathlib::reflection::ConstantLastLetterCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ConstantLastLetterCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("constant_last_letter_forces_suffix_coincidence") != std::string::npos);
    // 2 nodes x C(4,2)=6 pairs each = 12 instances.
    assert(lean.find("constant_last_letter_instance_11") != std::string::npos);

    std::ofstream out("lean/generated/constant_last_letter_batch.lean");
    out << lean;
    out.close();

    std::cout << "constant_last_letter_reflection_test: PASS\n";
    return 0;
}
