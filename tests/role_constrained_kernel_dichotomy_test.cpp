#include <cassert>
#include <iostream>

#include "ravel/proof/role_constrained_kernel_dichotomy.hpp"

using namespace ravel::proof;

int main() {
    OneLapControllerRelation cyclic;
    cyclic.state_count = 3;
    cyclic.successors = {{1}, {2}, {0}};
    const auto c = derive_role_constrained_kernel_dichotomy(cyclic);
    assert(c.valid && c.has_cyclic_kernel && c.kernel_size == 3);

    OneLapControllerRelation acyclic;
    acyclic.state_count = 4;
    acyclic.successors = {{1}, {2}, {3}, {}};
    const auto a = derive_role_constrained_kernel_dichotomy(acyclic);
    assert(a.valid && !a.has_cyclic_kernel);
    assert(a.elimination_rank[0] > a.elimination_rank[1]);
    assert(a.elimination_rank[1] > a.elimination_rank[2]);
    assert(a.elimination_rank[2] > a.elimination_rank[3]);

    std::cout << "role constrained kernel dichotomy PASS\n";
}
