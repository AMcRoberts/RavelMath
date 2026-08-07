// Finding 17, wired through the reflection pipeline end-to-end: the
// C++ certificate (constant_factor_forces_depth1_coincidence.hpp)
// records a ConstantFirstLetterCertificate node whenever the premise
// holds; the renderer mechanically emits one Lean corollary of
// constant_first_letter_forces_prefix_coincidence per pair of images,
// using each substitution's own concrete image data.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/constant_factor_forces_depth1_coincidence.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

std::array<std::vector<long long>, 3> sigma_ab(int a, int b) {
    std::array<std::vector<long long>, 3> s;
    for (int i = 0; i < a; ++i) s[0].push_back(0);
    for (int i = 0; i < b; ++i) s[0].push_back(1);
    s[0].push_back(2);
    for (int i = 0; i < a; ++i) s[1].push_back(0);
    s[1].push_back(2);
    s[2] = {0};
    return s;
}

int main() {
    mathlib::reflection::Trace trace("constant_first_letter_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // a=0: no constant factor -- must NOT record a node.
        auto cert0 = check_constant_factor_forces_depth1<3>(sigma_ab(0, 1));
        assert(!cert0.has_constant_factor);

        // a=1, a=2: constant first letter 0 -- must record.
        auto cert1 = check_constant_factor_forces_depth1<3>(sigma_ab(1, 1));
        assert(cert1.has_constant_factor);
        auto cert2 = check_constant_factor_forces_depth1<3>(sigma_ab(2, 1));
        assert(cert2.has_constant_factor);
    }

    auto nodes = trace.find<mathlib::reflection::ConstantFirstLetterCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ConstantFirstLetterCertificate nodes\n";
    assert(nodes.size() == 2);  // a=0 correctly recorded nothing

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("constant_first_letter_forces_prefix_coincidence") != std::string::npos);
    // 2 nodes x C(3,2)=3 pairs each = 6 instances.
    assert(lean.find("constant_first_letter_instance_5") != std::string::npos);

    std::ofstream out("/tmp/constant_first_letter_generated.lean");
    out << lean;
    out.close();

    std::cout << "constant_first_letter_reflection_test: PASS\n";
    return 0;
}
