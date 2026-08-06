#include <cassert>
#include <iostream>

#include "ravel/proof/cyclic_continuation_controller.hpp"

int main() {
    using namespace ravel::proof;

    CyclicControllerPlant plant;
    plant.states = {{-1}, {0}, {1}};
    plant.successors[{0, 0}] = {1};
    plant.successors[{1, 0}] = {0};
    plant.successors[{2, 0}] = {2};

    const auto cert =
        synthesize_cyclic_run(plant, {0, 0}, "synthetic.two_step");
    assert(cert.valid);
    assert(cert.closes);
    assert(cert.controller_states.front() ==
           cert.controller_states.back());

    const auto impossible =
        synthesize_cyclic_run(plant, {1}, "synthetic.no_transition");
    assert(!impossible.valid);

    std::cout << "cyclic continuation controller PASS\n";
}
