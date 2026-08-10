#include <cassert>
#include <iostream>
#include <vector>

#include "adelic/adelic_cocycle_extension.hpp"

int main() {
    const auto unit = adelic::derive_adelic_cocycle_extension(-1, {});
    assert(unit.proved);
    assert(unit.unit_specialization);
    assert(unit.local_fiber_count == 0);
    assert(unit.sofic_projection && unit.transport_projection);

    const std::vector<adelic::AdelicLocalFiberDescriptor> local{{2, 1, 2},
                                                                  {3, 2, 1}};
    const auto nonunit = adelic::derive_adelic_cocycle_extension(6, local);
    assert(nonunit.proved);
    assert(nonunit.nonunit_local_fibers);
    assert(nonunit.local_fibers_are_prime_ideal_indexed);
    assert(nonunit.local_fiber_count == 2);
    assert(nonunit.projections_commute);
    std::cout << "adelic cocycle extension PASS\n";
}
