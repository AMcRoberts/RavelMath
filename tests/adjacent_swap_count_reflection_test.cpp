// Wires family_of_families_adjacent_swap_counts.lean's claim (Class-II has
// exactly 3 adjacent unequal-letter swap sites; n-bonacci has exactly n-1)
// to the reflection pipeline for real: family_closed_forms.hpp's
// certify_class_ii_adjacent_swap_count/certify_nbonacci_adjacent_swap_count
// already independently construct the substitution and verify the count --
// this stages the CONCRETE images those checks pass against, so Lean
// recomputes the same count via `decide`, not merely restates the number.
// The previous lean/family_of_families_adjacent_swap_counts.lean checked a
// Finset.univ-cardinality placeholder unrelated to any actual substitution
// image -- archived, superseded by this.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/adjacent_swap_count_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("adjacent_swap_count_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::proof::stage_class_ii_adjacent_swap_count(
            2, 3, "Class-II sigma_{2,3}: 0->0^2 1^3 2, 1->0^2 2, 2->0");
        ravel::proof::stage_class_ii_adjacent_swap_count(
            1, 1, "Class-II sigma_{1,1}: 0->0 1 2, 1->0 2, 2->0");
        ravel::proof::stage_nbonacci_adjacent_swap_count(3, "tribonacci (n=3)");
        ravel::proof::stage_nbonacci_adjacent_swap_count(5, "5-bonacci (n=5)");

        // Negative control: n=1 is out of the closed form's domain and
        // certify_nbonacci_adjacent_swap_count throws for it internally via
        // nbonacci_adjacent_swap_count_closed_form -- must not be called
        // uncaught here, so this is deliberately omitted rather than staged.
    }

    auto nodes = trace.find<mathlib::reflection::AdjacentSwapCountCertificate>();
    std::cout << "trace recorded " << nodes.size() << " AdjacentSwapCountCertificate nodes\n";
    assert(nodes.size() == 4);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("adjacentUnequalCount") != std::string::npos);
    assert(lean.find("adjacent_swap_count_instance_0") != std::string::npos);
    assert(lean.find("adjacent_swap_count_instance_3") != std::string::npos);
    assert(lean.find("= 3 := by decide") != std::string::npos);
    assert(lean.find("= 2 := by decide") != std::string::npos);
    assert(lean.find("= 4 := by decide") != std::string::npos);

    std::ofstream out("lean/generated/adjacent_swap_count_batch.lean");
    out << lean;
    out.close();

    std::cout << "adjacent_swap_count_reflection_test: PASS\n";
    return 0;
}
