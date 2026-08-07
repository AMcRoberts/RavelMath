// Finding 39/41's "same-chain" special case, wired through the
// reflection pipeline: derive_zero_run_coincidence_bound records a
// ZeroRunSameChainCertificate node whenever the longest zero-run has
// length >= 2; the renderer mechanically emits a corollary of
// same_chain_forces_coincidence on that run's own local chain model.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/zero_run_forces_bounded_coincidence.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("zero_run_same_chain_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // digits (1,0,0,0,1): zero-run of length 3 at index 1.
        auto cert1 = derive_zero_run_coincidence_bound({1, 0, 0, 0, 1});
        assert(cert1.longest_zero_run == 3);

        // digits (2,0,3): zero-run of length 1 -- must NOT record
        // (offsets 0 and 0 would coincide, degenerate).
        auto cert2 = derive_zero_run_coincidence_bound({2, 0, 3});
        assert(cert2.longest_zero_run == 1);

        // digits (1,2,3): no zero digit -- must NOT record.
        auto cert3 = derive_zero_run_coincidence_bound({1, 2, 3});
        assert(cert3.longest_zero_run == 0);
    }

    auto nodes = trace.find<mathlib::reflection::ZeroRunSameChainCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ZeroRunSameChainCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes[0].second->run_length == 3);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("same_chain_forces_coincidence") != std::string::npos);
    assert(lean.find("zero_run_same_chain_instance_0") != std::string::npos);

    std::ofstream out("/tmp/zero_run_same_chain_generated.lean");
    out << lean;
    out.close();

    std::cout << "zero_run_same_chain_reflection_test: PASS\n";
    return 0;
}
