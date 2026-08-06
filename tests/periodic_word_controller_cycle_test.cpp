#include <cassert>
#include <iostream>

#include "ravel/proof/periodic_word_controller_cycle.hpp"

int main() {
    using namespace ravel::proof;
    const auto fixed_point_failure_but_cycle =
        prove_periodic_word_controller_cycle(3, {0, 1, 0, 1});
    assert(fixed_point_failure_but_cycle.cycle.valid);

    const auto n3 = audit_periodic_word_controller_cycles(3, 8);
    std::cout << render_periodic_word_cycle_audit(n3);
    assert(n3.all_passed);

    const auto n4 = audit_periodic_word_controller_cycles(4, 6);
    std::cout << render_periodic_word_cycle_audit(n4);
    assert(n4.all_passed);

    std::cout << "periodic word controller cycle PASS\n";
}
