#include <iostream>
#include "ravel/proof/phase_handoff_renewal.hpp"

int main() {
    const std::vector<std::vector<long long>> A{{0,1},{1,0}};
    const std::vector<std::vector<long long>> B{{1,1},{1,1}};
    const std::vector<std::size_t> phase{0,1};
    const auto linear = ravel::proof::derive_phase_handoff_renewal(
        A, phase, B, phase, 2, 8);
    const auto bellman = ravel::proof::derive_phase_handoff_bellman_renewal(
        A, phase, B, phase, 2, 8);
    if (!linear.proved || !bellman.proved) {
        std::cerr << "phase handoff renewal FAIL\n";
        return 1;
    }
    std::cout << "phase handoff renewal PASS linear=" << linear.block_length
              << " bellman=" << bellman.block_length << "\n";
    return 0;
}
