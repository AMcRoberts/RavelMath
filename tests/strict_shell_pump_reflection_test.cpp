// Wires lean/universal_shell_pumping_proof.lean (previously flat: zero C++
// consumer) to the reflection pipeline. certify_strict_shell_pump
// independently replays a concrete closed n-bonacci carry cycle, a cyclic
// continuation-controller run over it, and the affine-transported lifted
// cycle -- checking recurrence replay, admissible digits, closure, and
// strict outward face-aligned radius growth. On success the renderer
// instantiates StrictShellPump at that CONCRETE (source_radius,
// lifted_radius) pair.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
#include "ravel/proof/strict_shell_pump.hpp"
#include "ravel/proof/strict_shell_pump_reflection.hpp"

int main() {
    using namespace ravel::proof;

    mathlib::reflection::Trace trace("strict_shell_pump_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Positive: the same synthetic one-dimensional instance already
        // exercised (as a plain C++ unit test, with no Lean persistence)
        // by tests/strict_shell_pump_test.cpp -- x -> x+d closes a lifted
        // cycle at radius 3 from radius 2.
        const auto source = replay_carry_cycle(
            "strict-shell.reflect.source", {{2}, {2}}, {0});
        assert(source.valid);
        CyclicControllerPlant plant;
        plant.states = {{1}};
        plant.successors[{0, 0}] = {0};
        const auto run = synthesize_cyclic_run(
            plant, {0}, "strict-shell.reflect.controller");
        assert(run.valid);
        const auto cert = certify_strict_shell_pump(
            "strict-shell.reflect", source, plant, run);
        assert(cert.valid);
        stage_strict_shell_pump_instance(cert);

        // Negative control: an inward-only plant must not stage anything.
        auto bad_plant = plant;
        bad_plant.states = {{-1}};
        const auto bad_run = synthesize_cyclic_run(bad_plant, {0});
        const auto bad_cert = certify_strict_shell_pump(
            "strict-shell.reflect.inward", source, bad_plant, bad_run);
        assert(!bad_cert.valid);
        stage_strict_shell_pump_instance(bad_cert);
    }

    auto nodes = trace.find<mathlib::reflection::StrictShellPumpInstanceCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " StrictShellPumpInstanceCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes.front().second->source_radius == 2);
    assert(nodes.front().second->lifted_radius == 3);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem iterate_strict_shell_lift") != std::string::npos);
    assert(lean.find("def StrictShellPump") != std::string::npos);
    assert(lean.find("strict_shell_pump_instance_0") != std::string::npos);
    assert(lean.find("strict_shell_pump_instance_1") == std::string::npos);

    std::ofstream out("lean/generated/strict_shell_pump_batch.lean");
    out << lean;
    out.close();

    std::cout << "strict_shell_pump_reflection_test: PASS\n";
    return 0;
}
