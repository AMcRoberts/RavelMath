// Widens Finding 42's Barge-Diamond reflection pipeline to more real
// substitutions from this project's own history -- pure reuse of the
// already-tested certify_barge_diamond/render_barge_diamond_instances
// pipeline, no new math, no new Lean. sigma_{1,1} (Finding 36) and the
// supergolden canonical substitution (QRS reading list) are both
// genuine, previously-established Pisot substitutions of degree 3,
// within certify_barge_diamond's stated rational-root-theorem scope.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/barge_diamond_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("barge_diamond_wide_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // sigma_{1,1} (Finding 36): 0->012, 1->02, 2->0.
        auto cert1 = certify_barge_diamond<3>(
            {std::vector<long long>{0, 1, 2}, std::vector<long long>{0, 2}, std::vector<long long>{0}});
        assert(cert1.beta_irrational);

        // supergolden canonical substitution: 0->01, 1->2, 2->0.
        auto cert2 = certify_barge_diamond<3>(
            {std::vector<long long>{0, 1}, std::vector<long long>{2}, std::vector<long long>{0}});
        assert(cert2.beta_irrational);

        std::cout << "sigma_{1,1}: " << cert1.conclusion << "\n";
        std::cout << "supergolden: " << cert2.conclusion << "\n";
    }

    auto nodes = trace.find<mathlib::reflection::IntegerEigenvectorNoWitness>();
    std::cout << "trace recorded " << nodes.size() << " IntegerEigenvectorNoWitness nodes\n";
    assert(nodes.size() == 2);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("barge_diamond_instance_0") != std::string::npos);
    assert(lean.find("barge_diamond_instance_1") != std::string::npos);

    std::ofstream out("/tmp/barge_diamond_wide_batch_generated.lean");
    out << lean;
    out.close();

    std::cout << "barge_diamond_wide_batch_test: PASS\n";
    return 0;
}
