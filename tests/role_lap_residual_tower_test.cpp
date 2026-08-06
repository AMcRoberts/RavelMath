#include <cassert>
#include <iostream>
#include "ravel/proof/role_lap_residual_tower.hpp"

int main() {
    using namespace ravel::proof;
    OneLapControllerRelation cyclic;
    cyclic.state_count = 4;
    cyclic.successors = {{1}, {2}, {0}, {}};
    const auto positive = derive_role_lap_residual_tower(cyclic);
    assert(positive.valid);
    assert(positive.nonempty_periodic);
    assert(!positive.reached_empty);

    OneLapControllerRelation acyclic;
    acyclic.state_count = 4;
    acyclic.successors = {{1}, {2}, {3}, {}};
    const auto negative = derive_role_lap_residual_tower(acyclic);
    assert(negative.valid);
    assert(negative.reached_empty);
    assert(!negative.nonempty_periodic);
    assert(negative.extinction_height <= acyclic.state_count);

    std::cout << "role lap residual tower PASS extinction="
              << negative.extinction_height << "\n";
}
