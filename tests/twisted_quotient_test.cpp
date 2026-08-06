#include <iostream>
#include <utility>
#include <vector>
#include "ravel/proof/twisted_quotient.hpp"

int main() {
    using ravel::proof::derive_z2_twisted_quotient;
    // Removable twist: two concrete sheets over a 3-cycle, transport 1 on two
    // edges and 0 on one, giving even total holonomy.
    std::vector<std::size_t> role{0,1,2,0,1,2};
    std::vector<int> fiber{0,1,0,1,0,1};
    std::vector<std::pair<std::size_t,std::size_t>> edges{
        {0,1},{1,2},{2,0},{3,4},{4,5},{5,3}};
    const auto removable = derive_z2_twisted_quotient(edges, role, fiber, 3);
    if (!removable.transport_well_defined || !removable.coboundary || removable.genuinely_twisted)
        return 1;

    // Genuine odd holonomy: one trip around the base triangle swaps sheets.
    role = {0,1,2,0,1,2}; fiber = {0,0,0,1,1,1};
    edges = {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0}};
    const auto genuine = derive_z2_twisted_quotient(edges, role, fiber, 3);
    if (!genuine.transport_well_defined || !genuine.genuinely_twisted)
        return 2;

    // Ambiguous transport: same base edge appears with both xor values.
    role = {0,1,0,1}; fiber = {0,0,0,1}; edges = {{0,1},{2,3}};
    const auto ambiguous = derive_z2_twisted_quotient(edges, role, fiber, 2);
    if (ambiguous.transport_well_defined || ambiguous.ambiguous_base_edges != 1)
        return 3;

    std::cout << "twisted quotient PASS\n";
    return 0;
}
