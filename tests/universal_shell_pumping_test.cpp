#include <cassert>
#include <iostream>

#include "ravel/proof/universal_shell_pumping.hpp"

int main() {
    using namespace ravel::proof;

    const auto cert = replay_shell_pumping_certificate(
        7,
        {
            {"M2", "M3", 2, 3, true, true, false},
            {"M3", "M4", 3, 4, true, true, false},
        });

    assert(cert.every_step_recurrent);
    assert(cert.every_step_strict);
    assert(cert.replay_checked);
    assert(outer_recurrence_contradicts_bound(2, 7, 8));
    assert(!outer_recurrence_contradicts_bound(1, 7, 8));

    std::cout << "universal shell pumping composition PASS\n";
}
