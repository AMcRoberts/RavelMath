#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "ravel/proof/phase_rank_transport.hpp"

int main() {
    using namespace ravel::proof;

    const auto cert = close_phase_rank_transport(
        "synthetic.multi-shell.transport",
        {"A", "B", "C"},
        {
            {"A", "B", 1},
            {"B", "C", 2},
            {"A", "C", 2},
        },
        {"n=3,M=2", "n=3,M=3"});

    assert(cert.feasible);
    assert(cert.replay_checked);
    assert(cert.offsets.at("B") >= cert.offsets.at("A") + 1);
    assert(cert.offsets.at("C") >= cert.offsets.at("B") + 2);

    const auto impossible = close_phase_rank_transport(
        "synthetic.positive.cycle",
        {"A", "B"},
        {{"A", "B", 1}, {"B", "A", 1}},
        {"bad"});
    assert(!impossible.feasible);
    assert(!impossible.replay_checked);

    std::cout << "phase-rank transport machinery PASS\n";
}
