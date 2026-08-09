// Wires lean/class_ii_neighbor_dominance.lean (previously flat: zero C++
// consumer) to the reflection pipeline. certify_regular_shell_charpoly
// independently computes each neighbor's concrete regular-shell compressed
// matrix and its exact characteristic polynomial (Faddeev-LeVerrier,
// self-checked via Cayley-Hamilton), and checks it against
// docs/FAMILY_OF_FAMILIES.md's displayed closed form -- the first
// computational confirmation that these formulas actually hold (the naive
// guess, that "lambda" refers to the FIXED dominant core's own Perron
// root, was checked and found WRONG; the correct object is this growing
// regular-shell family, per that doc's "indexed shell spectra" section).
// On success the renderer instantiates neighbor{0,1,2}_shell_below_* at
// that CONCRETE (a, t).

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/regular_shell_charpoly_certificate.hpp"

int main() {
    mathlib::reflection::Trace trace("regular_shell_charpoly_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::proof::stage_regular_shell_charpoly(
            0, 8, 3, "neighbor 0 regular shell, a=8, t=3");
        ravel::proof::stage_regular_shell_charpoly(
            1, 8, 4, "neighbor 1 regular shell, a=8, t=4");
        ravel::proof::stage_regular_shell_charpoly(
            2, 8, 5, "neighbor 2 regular shell, a=8, k=5");

        // Negative control: t out of the certified range must record nothing.
        ravel::proof::stage_regular_shell_charpoly(
            0, 8, 99, "out-of-range t (negative control)");
    }

    auto nodes = trace.find<mathlib::reflection::RegularShellCharpolyCertificate>();
    std::cout << "trace recorded " << nodes.size() << " RegularShellCharpolyCertificate nodes\n";
    assert(nodes.size() == 3);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("neighbor0_shell_below_pred") != std::string::npos);
    assert(lean.find("neighbor1_shell_below_pred") != std::string::npos);
    assert(lean.find("neighbor2_shell_below_self") != std::string::npos);
    assert(lean.find("regular_shell_charpoly_instance_0") != std::string::npos);
    assert(lean.find("regular_shell_charpoly_instance_1") != std::string::npos);
    assert(lean.find("regular_shell_charpoly_instance_2") != std::string::npos);
    assert(lean.find("regular_shell_charpoly_instance_3") == std::string::npos);

    std::ofstream out("lean/generated/regular_shell_charpoly_batch.lean");
    out << lean;
    out.close();

    std::cout << "regular_shell_charpoly_reflection_test: PASS\n";
    return 0;
}
