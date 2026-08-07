// Findings 1-16 (the Class-II contact-boundary thread) get a first
// reflection-pipeline connection, same shape as the property-F
// retrofit: class_ii_interior_shell (include/ravel/class_ii_boundary_family.hpp)
// now records a LemmaApplication citing shellNode_propagates
// (lean/class_ii_affine_shells.lean, hand-derived and kernel-checked
// well before the mechanical reflection pipeline existed for this
// project's coincidence/Pisot thread) -- making the EXECUTING code
// cite an already-verified fact, not deriving new math.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_shell_batch");
    std::set<ravel::SNode<3>> shell;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        shell = ravel::class_ii_interior_shell(7);
    }
    std::cout << "shell size at q=7: " << shell.size() << "\n";
    assert(shell.size() == 20);

    auto lemmas = trace.find<mathlib::reflection::LemmaApplication>();
    bool found = false;
    for (auto& [id, l] : lemmas) { (void)id; if (l->theorem_name == "shellNode_propagates") found = true; }
    std::cout << "trace has shellNode_propagates citation: " << found << "\n";
    assert(found);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("shellNode_propagates") != std::string::npos);
    assert(lean.find("shellNode_injective_at_round") != std::string::npos);

    std::ofstream out("/tmp/class_ii_shell_citation_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_shell_citation_test: PASS\n";
    return 0;
}
