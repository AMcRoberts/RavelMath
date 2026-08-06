#include <cassert>
#include <iostream>

#include "ravel/proof/continuation_controller_family.hpp"

int main() {
    using namespace ravel::proof;

    ControllerPlant plant;
    plant.state_count = 2;
    plant.successors[{0, -1}] = {0};
    plant.successors[{1, -1}] = {0};
    plant.successors[{0, 0}] = {1};
    plant.successors[{1, 0}] = {1};
    plant.successors[{0, 1}] = {0, 1};
    plant.successors[{1, 1}] = {1};

    const ResidualController accept_zero = {true, false};
    const ResidualController accept_one = {false, true};

    const auto family = close_controller_family(
        plant, {accept_zero, accept_one}, {-1, 0, 1});

    assert(family.closed);
    assert(family.members.size() >= 2);
    assert(family.terminal_generators.size() == 2);

    for (std::size_t id = 0; id < family.members.size(); ++id)
        for (const auto input : {-1LL, 0LL, 1LL})
            assert(family.predecessor_table.contains({id, input}));

    std::cout << "continuation controller family algebra PASS\n";
}
