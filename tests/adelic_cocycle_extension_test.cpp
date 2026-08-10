#include <cassert>
#include <iostream>
#include <vector>

#include "adelic/adelic_cocycle_extension.hpp"
#include "ravel/proof/monotone_coefficient_cone.hpp"

int main() {
    const auto cone_unit =
        ravel::proof::derive_monotone_coefficient_cone_certificate({3, 3, 1});
    assert(cone_unit.condition_f_applies);
    assert(cone_unit.unimodular);
    assert(!cone_unit.nonunit);

    const auto cone_nonunit =
        ravel::proof::derive_monotone_coefficient_cone_certificate({4, 3, 2});
    assert(cone_nonunit.condition_f_applies);
    assert(!cone_nonunit.unimodular);
    assert(cone_nonunit.nonunit);
    assert(cone_nonunit.determinant == 2);

    const auto cone_bad =
        ravel::proof::derive_monotone_coefficient_cone_certificate({2, 4, 1});
    assert(!cone_bad.condition_f_applies);

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
    assert(nonunit.rational_prime_support == std::vector<long long>({2, 3}));
    assert(nonunit.projections_commute);
    std::cout << "adelic cocycle extension PASS\n";
}
