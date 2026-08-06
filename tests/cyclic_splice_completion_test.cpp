#include <cassert>
#include <iostream>
#include "ravel/proof/cyclic_splice_completion.hpp"

int main() {
    using namespace ravel::proof;
    // No one-lap fixed point: 0 -> 1 -> 2 -> 1.  The derived two-lap orbit
    // 1 -> 2 -> 1 is the required repeated-cycle splice.
    OneLapControllerRelation relation;
    relation.state_count = 3;
    relation.successors = {{1}, {2}, {1}};
    const auto proof = derive_periodic_controller_orbit(relation, 0);
    std::cout << render_periodic_controller_orbit_report(proof);
    assert(proof.valid);
    assert(proof.transient_laps == 1);
    assert(proof.period_laps == 2);

    OneLapControllerRelation nonserial;
    nonserial.state_count = 2;
    nonserial.successors = {{1}, {}};
    const auto rejected = derive_periodic_controller_orbit(nonserial, 0);
    assert(!rejected.valid);
    std::cout << "cyclic splice completion PASS\n";
}
