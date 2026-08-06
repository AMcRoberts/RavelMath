#include <cassert>
#include <iostream>
#include "ravel/proof/maximum_shell_exclusion.hpp"
using namespace ravel::proof;
int main() {
    OneLapControllerRelation acyclic;
    acyclic.state_count = 3;
    acyclic.successors = {{1},{2},{}};
    const auto tower = derive_role_lap_residual_tower(acyclic);
    assert(tower.valid && tower.reached_empty);
    const auto rank = derive_maximum_shell_return_rank({{1},{2},{}});
    assert(rank.valid);
    const auto cert = certify_maximum_shell_exclusion(tower, {}, rank);
    assert(cert.valid);
    assert(cert.branch == MaximumShellExclusionBranch::strict_first_return_rank);

    const auto cyclic_rank = derive_maximum_shell_return_rank({{1},{0}});
    assert(!cyclic_rank.valid);
    const auto bad = certify_maximum_shell_exclusion(tower, {}, cyclic_rank);
    assert(!bad.valid);
    std::cout << "maximum shell exclusion PASS\n";
}
