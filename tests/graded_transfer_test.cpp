#include <iostream>
#include "ravel/proof/graded_transfer.hpp"

int main() {
    // Competitor has one continuation per state; core has two.
    const std::vector<std::vector<long long>> competitor{{0,1},{1,0}};
    const std::vector<std::vector<long long>> core{{1,1},{1,1}};
    const auto proof = ravel::proof::derive_graded_transfer_proof(competitor, core, 16);
    if (!proof.proved || proof.renewal.block_length == 0 || !proof.paired_intertwiner) {
        std::cerr << "graded transfer FAIL: " << proof.obstruction << "\n";
        return 1;
    }
    std::cout << "graded transfer PASS block=" << proof.renewal.block_length
              << " cone=" << proof.cone_pair.first_separating_horizon << "\n";
    return 0;
}
