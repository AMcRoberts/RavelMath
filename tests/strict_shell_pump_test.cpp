#include <cassert>
#include <iostream>

#include "ravel/proof/strict_shell_pump.hpp"

int main() {
    using namespace ravel::proof;

    // A concrete one-dimensional affine instance exercises every proof field.
    // x -> x+d, source digit 0, translation defect 0, and aligned translation
    // +1 produce a closed lifted cycle at radius 3 from radius 2.
    const auto source = replay_carry_cycle(
        "strict-shell.synthetic.source", {{2}, {2}}, {0});
    assert(source.valid);

    CyclicControllerPlant plant;
    plant.states = {{1}};
    plant.successors[{0, 0}] = {0};
    const auto run = synthesize_cyclic_run(
        plant, {0}, "strict-shell.synthetic.controller");
    assert(run.valid);

    const auto cert = certify_strict_shell_pump(
        "strict-shell.synthetic", source, plant, run);
    assert(cert.valid);
    assert(cert.controller_cycle_replay);
    assert(cert.adjusted_digits_admissible);
    assert(cert.affine_transport_replay);
    assert(cert.lifted_cycle.valid);
    assert(cert.face_aligned);
    assert(cert.source_radius == 2);
    assert(cert.lifted_radius == 3);

    auto bad_plant = plant;
    bad_plant.states = {{-1}};
    const auto bad_run = synthesize_cyclic_run(bad_plant, {0});
    const auto unsupported = certify_strict_shell_pump(
        "strict-shell.synthetic.inward", source, bad_plant, bad_run);
    assert(!unsupported.valid);
    assert(!unsupported.unsupported_reason.empty());

    std::cout << "strict shell pump certificate PASS\n";
}
