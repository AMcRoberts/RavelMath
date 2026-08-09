// NOTE (2026-08-08): this test's Lean-emission path is retired. It fed
// lean/generated/cyclic_splice_compactness.lean, built for the shell-rank/
// carry-bound recurrent-radius route that superseded itself once the
// universal n-bonacci boundary-dominance theorem closed via the canonical
// Q/R parent-prefix split instead (see docs/NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md).
// The archived output lives at lean/archive/cyclic_splice_compactness.lean.
// The C++ combinatorics below (find_cyclic_one_lap_orbit) remain real and
// are still exercised elsewhere (stepped_face_role_junction.hpp and its
// test), so this test is kept as a plain regression check with the Lean
// staging/emission removed rather than deleted outright.
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
