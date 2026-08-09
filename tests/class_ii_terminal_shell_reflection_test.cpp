// A new Class-II reflection connection (Finding 9's territory):
// class_ii_terminal_shell(a)'s erase/insert edit matches
// lean/class_ii_terminal_shells.lean's terminalCrossColour1/2 and
// interiorExtreme00/11 exactly (verified by direct comparison of the
// C++ literal (i,x,j) tuples against the Lean definitions before
// wiring). Records the concrete `a`; the renderer instantiates
// terminalCrossColours_not_eq_interior_extremes AT that a via decide.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_terminal_shell_batch");
    std::set<ravel::SNode<3>> shell7, shell12;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        shell7 = ravel::class_ii_terminal_shell(7);
        shell12 = ravel::class_ii_terminal_shell(12);
    }
    std::cout << "terminal shell size at a=7: " << shell7.size() << ", a=12: " << shell12.size() << "\n";
    assert(shell7.size() == 20);
    assert(shell12.size() == 20);

    auto nodes = trace.find<mathlib::reflection::ClassIITerminalShellCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIITerminalShellCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_terminal_shell_instance_0") != std::string::npos);
    assert(lean.find("class_ii_terminal_shell_instance_1") != std::string::npos);
    assert(lean.find("terminalCrossColours_not_eq_interior_extremesG") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_terminal_shell_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_terminal_shell_reflection_test: PASS\n";
    return 0;
}
