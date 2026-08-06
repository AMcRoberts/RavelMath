#include <cassert>
#include <iostream>

#include "ravel/proof/strict_shell_pump_existence.hpp"

int main() {
    using namespace ravel::proof;

    const auto outer = replay_carry_cycle(
        "family.outer", {{2}, {2}}, {0});
    const auto inner = replay_carry_cycle(
        "family.inner", {{1}, {1}}, {0});

    CyclicControllerPlant plant;
    plant.states = {{1}};
    plant.successors[{0, 0}] = {0};

    const auto incomplete = certify_strict_shell_pump_existence(
        "family.incomplete", {outer}, plant, false);
    assert(!incomplete.valid);
    assert(!incomplete.unsupported_reason.empty());

    const auto complete = certify_strict_shell_pump_existence(
        "family.complete", {inner, outer}, plant, true);
    assert(complete.valid);
    assert(complete.realized_cycle_count == 2);
    assert(complete.pumped_cycle_count == 1);
    assert(complete.witnesses.size() == 1);
    assert(complete.witnesses.front().valid);

    std::cout << "strict shell pump existence family PASS\n";
}
