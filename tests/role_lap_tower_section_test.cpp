#include <cassert>
#include <iostream>
#include "ravel/proof/role_lap_tower_section.hpp"
int main() {
    using namespace ravel::proof;
    OneLapControllerRelation r;
    r.state_count = 4;
    r.successors = {{1}, {2}, {0}, {}};
    const auto cert = certify_role_lap_tower_section(r, {0,1,2,0,1});
    assert(cert.valid);
    assert(cert.closed_cycle == std::vector<std::size_t>({0,1,2,0}));
    std::cout << "role lap tower section PASS cycle="
              << cert.closed_cycle.size()-1 << "\n";
}
