#include <cassert>
#include <iostream>

#include "ravel/proof/nonunit_property_f_theorem.hpp"

int main() {
    const auto cubic = ravel::proof::derive_nonunit_property_f_theorem({4, 3, 2});
    assert(cubic.degree == 3);
    assert(cubic.primitive_companion);
    assert(cubic.dominant_pisot_root);
    assert(cubic.nonunit);
    assert(cubic.frougny_solomyak_hypothesis);
    assert(cubic.property_f_holds);

    const auto unit = ravel::proof::derive_nonunit_property_f_theorem({3, 3, 1});
    assert(unit.frougny_solomyak_hypothesis);
    assert(!unit.nonunit);
    assert(!unit.property_f_holds);

    const auto bad = ravel::proof::derive_nonunit_property_f_theorem({2, 4, 1});
    assert(!bad.monotone_coefficients);
    assert(!bad.property_f_holds);
    std::cout << "non-unit Property-F number-system theorem: PASS\n";
}
