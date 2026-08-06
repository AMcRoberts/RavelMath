#include <cassert>
#include <iostream>
#include "ravel/proof/cyclic_splice_compactness.hpp"

int main() {
    using namespace ravel::proof;
    OneLapControllerRelation relation;
    relation.state_count = 5;
    relation.successors = {{1}, {2}, {}, {4}, {3}};
    const auto proof = find_cyclic_one_lap_orbit(relation);
    std::cout << render_cyclic_one_lap_relation_report(proof);
    assert(proof.valid);
    assert(proof.closed_orbit.size() == 3);

    OneLapControllerRelation dag;
    dag.state_count = 3;
    dag.successors = {{1}, {2}, {}};
    const auto rejected = find_cyclic_one_lap_orbit(dag);
    assert(!rejected.valid);
    std::cout << "cyclic splice compactness PASS\n";
}
