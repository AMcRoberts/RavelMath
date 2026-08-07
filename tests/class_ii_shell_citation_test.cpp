// Findings 1-16 (the Class-II contact-boundary thread), original
// per-instance pattern (not a static citation): class_ii_interior_shell
// (include/ravel/class_ii_boundary_family.hpp) records the CONCRETE
// round q it built; the renderer instantiates shellNode_propagates
// and shellNode_injective_at_round AT that q, discharged by `decide`
// (concrete integer arithmetic), not cited abstractly. Two different
// rounds (7 and 10) demonstrate genuine per-instance rendering, not
// one fixed block of text.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_shell_batch");
    std::set<ravel::SNode<3>> shell7, shell10;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        shell7 = ravel::class_ii_interior_shell(7);
        shell10 = ravel::class_ii_interior_shell(10);
    }
    std::cout << "shell size at q=7: " << shell7.size() << ", q=10: " << shell10.size() << "\n";
    assert(shell7.size() == 20);
    assert(shell10.size() == 20);

    auto nodes = trace.find<mathlib::reflection::ClassIIShellRoundCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIShellRoundCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_shell_round_instance_0") != std::string::npos);
    assert(lean.find("class_ii_shell_round_instance_2") != std::string::npos);
    assert(lean.find("class_ii_shell_round_injective_instance_1") != std::string::npos);
    assert(lean.find("class_ii_shell_round_injective_instance_3") != std::string::npos);

    std::ofstream out("/tmp/class_ii_shell_citation_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_shell_citation_test: PASS\n";
    return 0;
}
