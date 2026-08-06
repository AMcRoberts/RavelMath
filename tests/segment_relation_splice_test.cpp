#include <cassert>
#include <iostream>

#include "ravel/proof/segment_relation_splice.hpp"

using namespace ravel::proof;

static SegmentControllerRelation relation(
    std::initializer_list<std::vector<std::size_t>> rows) {
    SegmentControllerRelation r;
    r.state_count = rows.size();
    r.successors.assign(rows.begin(), rows.end());
    r.replay_checked = true;
    return r;
}

int main() {
    const auto a = relation({{1},{2},{0},{}});
    const auto b = relation({{2},{0},{1},{}});
    const auto cert = derive_recurrent_lap_splice({a,b}, {0,1,2}, 0);
    assert(cert.valid);
    assert(cert.every_segment_serial_on_fiber);
    assert(cert.one_lap_serial_on_fiber);
    assert(cert.periodic_orbit.period_laps > 0);

    auto broken = b;
    broken.successors[1].clear();
    const auto bad = derive_recurrent_lap_splice({a,broken}, {0,1,2}, 0);
    assert(!bad.valid);
    assert(!bad.every_segment_serial_on_fiber);

    auto exits = b;
    exits.successors[1] = {3};
    const auto bad_exit = derive_recurrent_lap_splice({a,exits}, {0,1,2}, 0);
    assert(!bad_exit.valid);
    assert(!bad_exit.fiber_preserved);

    std::cout << "segment relation splice PASS period="
              << cert.periodic_orbit.period_laps << "\n";
}
