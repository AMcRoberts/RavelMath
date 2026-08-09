// Wires lean/coupled_automaton_characterization.lean (previously flat:
// its only prior consumer, coupled_winning_predicate.hpp's
// evaluate_predicate_tree, merely evaluates a tree and never checked the
// actual win-condition obligations synthesized_winning_predicate_sound
// requires) to the reflection pipeline.
// certify_synthesized_winning_predicate independently re-evaluates the
// predicate tree at every concrete state and exhaustively replays
// hinit/hstep/haccept over the full finite state/input space.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/coupled_winning_predicate_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    using namespace ravel::proof;

    // Reuses the exact tree from tests/coupled_winning_predicate_test.cpp:
    // feature[0] <= 0 ? leaf(true) : leaf(false).
    std::vector<PredicateTreeNode> tree = {
        {0, 0, 1, 2, false, false},
        {-1, 0, -1, -1, true, true},
        {-1, 0, -1, -1, true, false}};

    mathlib::reflection::Trace trace("coupled_winning_predicate_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Positive: states {0:-1 (Win), 1:1 (not Win), 2:-2 (Win)}, a
        // 2-cycle 0<->2 among the winning states, Init={0}, Accept={0,2}.
        auto cert = certify_synthesized_winning_predicate(
            "coupled.reflect", tree, {-1, 1, -2}, {0}, {0, 2},
            {{0, 0, 2}, {2, 0, 0}}, {0});
        assert(cert.valid);
        stage_synthesized_winning_predicate(cert);

        // Negative control: no successor transition for the winning state 2.
        auto bad = certify_synthesized_winning_predicate(
            "coupled.reflect.broken", tree, {-1, 1, -2}, {0}, {0, 2},
            {{0, 0, 2}}, {0});
        assert(!bad.valid);
        stage_synthesized_winning_predicate(bad);
    }

    auto nodes = trace.find<mathlib::reflection::WinningPredicateReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " WinningPredicateReflectionCertificate nodes\n";
    assert(nodes.size() == 1);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem synthesized_winning_predicate_sound") != std::string::npos);
    assert(lean.find("winning_predicate_instance_0") != std::string::npos);
    assert(lean.find("winning_predicate_instance_1") == std::string::npos);

    std::ofstream out("lean/generated/coupled_winning_predicate_batch.lean");
    out << lean;
    out.close();

    std::cout << "coupled_winning_predicate_reflection_test: PASS\n";
    return 0;
}
