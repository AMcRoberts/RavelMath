#include <cassert>
#include <iostream>

#include "ravel/proof/defect_corrected_shell_rank.hpp"

using namespace ravel::proof;

int main() {
    const auto n3 = derive_defect_corrected_shell_rank(3, 2, 7);
    assert(n3.rank.feasible && n3.rank.replay_checked);
    std::cout << "defect-corrected shell rank n=3 bounds=2..7 phases="
              << n3.rank.phases.size() << " constraints="
              << n3.rank.constraints.size() << " raw_edges=" << n3.raw_edges
              << " PASS\n";

    const auto n4 = derive_defect_corrected_shell_rank(4, 2, 5);
    assert(n4.rank.feasible && n4.rank.replay_checked);
    std::cout << "defect-corrected shell rank n=4 bounds=2..5 phases="
              << n4.rank.phases.size() << " constraints="
              << n4.rank.constraints.size() << " raw_edges=" << n4.raw_edges
              << " PASS\n";
    return 0;
}
