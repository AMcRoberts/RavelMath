// Finding 32, wired through the reflection pipeline: sigma_{0,2}'s
// charpoly x^3-x-2 is exactly the depressed cubic
// depressed_cubic_complex_pair_modulus.lean's headline instance
// covers. certify_sigma_0_2_not_pisot records a citation; the
// renderer emits the already-verified lemma text.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/sigma_0_2_not_pisot_certificate.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("sigma_0_2_batch");
    Sigma02NotPisotCertificate cert;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        cert = certify_sigma_0_2_not_pisot(-1, -2);
    }
    std::cout << cert.note << "\n";
    assert(cert.applies);

    // Control: a different polynomial must NOT record anything.
    {
        mathlib::reflection::Trace trace2("control");
        mathlib::reflection::ScopedTrace scope2(&trace2);
        auto cert2 = certify_sigma_0_2_not_pisot(0, -1);
        assert(!cert2.applies);
        auto n2 = trace2.find<mathlib::reflection::LemmaApplication>();
        assert(n2.empty());
    }

    auto lemmas = trace.find<mathlib::reflection::LemmaApplication>();
    bool found = false;
    for (auto& [id, l] : lemmas) { (void)id; if (l->theorem_name == "sigma_0_2_charpoly_not_pisot") found = true; }
    assert(found);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("sigma_0_2_charpoly_not_pisot") != std::string::npos);
    assert(lean.find("depressed_cubic_factors") != std::string::npos);

    std::ofstream out("/tmp/sigma_0_2_not_pisot_generated.lean");
    out << lean;
    out.close();

    std::cout << "sigma_0_2_not_pisot_reflection_test: PASS\n";
    return 0;
}
