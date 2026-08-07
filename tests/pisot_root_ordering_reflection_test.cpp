// A new reflection connection for Finding 29: the a-bonacci family's
// dominant root at n=3 is strictly less than at n=4, which is
// strictly less than a+1 -- previously checked only via floating-
// point midpoints (app/probe_pisot_accumulation_structure.cpp).
// Threads the EXACT rational brackets pisot_classify_3x3/_4x4 already
// certify (Sturm-chain isolation, no floating point); the renderer
// instantiates a general ordering lemma with those concrete numbers.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/pisot_root_ordering_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("pisot_root_ordering_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        for (long long a = 1; a <= 5; ++a) {
            ravel::proof::stage_pisot_root_ordering(a);
        }
    }

    auto nodes = trace.find<mathlib::reflection::PisotRootOrderingCertificate>();
    std::cout << "trace recorded " << nodes.size() << " PisotRootOrderingCertificate nodes\n";
    assert(nodes.size() == 5);
    for (long long a = 1; a <= 5; ++a) {
        assert(nodes[static_cast<std::size_t>(a - 1)].second->a == a);
    }

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("pisot_root_ordering_instance_0") != std::string::npos);
    assert(lean.find("pisot_root_ordering_instance_4") != std::string::npos);
    assert(lean.find("pisot_root_strictly_between") != std::string::npos);

    std::ofstream out("/tmp/pisot_root_ordering_generated.lean");
    out << lean;
    out.close();

    std::cout << "pisot_root_ordering_reflection_test: PASS\n";
    return 0;
}
