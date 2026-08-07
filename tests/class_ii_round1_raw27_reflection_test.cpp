// A seventh Class-II reflection connection: the 27-state raw pre-Red
// target for Round 1's neighbor tau_a (previously duplicated inline in
// app/class_ii_neighbor2_round1_red_forward_check.cpp, now shared via
// ravel::class_ii_round1_raw27_targets()) is threaded as concrete data
// and checked EQUAL (content and order, via decide) to
// lean/class_ii_round1_red_pruning.lean's round1Raw27 -- not a name
// citation.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/class_ii_round1_red_pruning_data.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_round1_raw27_batch");
    std::vector<std::array<long long, 5>> targets;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        targets = ravel::class_ii_round1_raw27_targets();
    }
    std::cout << "round1_raw27 target count: " << targets.size() << "\n";
    assert(targets.size() == 27);

    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIFixedTableCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes[0].second->table == "round1_raw27");
    assert(nodes[0].second->nodes.size() == 27);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("round1_raw27_instance_0") != std::string::npos);
    assert(lean.find("round1Raw27G") != std::string::npos);

    std::ofstream out("/tmp/class_ii_round1_raw27_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_round1_raw27_reflection_test: PASS\n";
    return 0;
}
