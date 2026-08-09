#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/third_smallest_pisot_qrs_closure.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("third_smallest_pisot_parry_factorization_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        auto c = derive_third_smallest_pisot_qrs_closure();
        assert(c.proved);
        stage_third_smallest_pisot_parry_factorization(
            c, "d_beta(1)=1001001 simple-Parry factorization for the third-smallest Pisot number");

        // Negative control: corrupt the parry_polynomial so the recomputed
        // product no longer matches -- must NOT record.
        auto bad = c;
        bad.parry_polynomial = {1, 1, 1};
        stage_third_smallest_pisot_parry_factorization(bad, "corrupted, must not record");
    }

    auto nodes = trace.find<mathlib::reflection::ThirdSmallestPisotParryFactorizationCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " ThirdSmallestPisotParryFactorizationCertificate nodes\n";
    assert(nodes.size() == 1);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("third_smallest_pisot_parry_factorization_instance_0") != std::string::npos);
    assert(lean.find("third_smallest_pisot_parry_factorization_instance_1") == std::string::npos);
    assert(lean.find("ring") != std::string::npos);

    std::ofstream out("lean/generated/third_smallest_pisot_parry_factorization_batch.lean");
    out << lean;
    out.close();

    std::cout << "third_smallest_pisot_parry_factorization_reflection_test: PASS\n";
    return 0;
}
