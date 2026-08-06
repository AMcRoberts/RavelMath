#include <cstdio>
#include <vector>

#include "ravel/proof/paired_matrix_dominance.hpp"

int main() {
    // rho(A)=1 and rho(B)=2.  The operation must derive the implicit positive
    // rank-one intertwiner rather than receive one as input.
    const std::vector<std::vector<long long>> competitor{{0,1},{1,0}};
    const std::vector<std::vector<long long>> core{{1,1},{1,1}};
    const auto proof = ravel::proof::derive_rank_one_paired_dominance(
        competitor, core, 8);
    if (!proof.replayed || proof.strict_entries != 4) {
        std::fprintf(stderr, "paired matrix dominance FAIL\n");
        return 1;
    }
    std::printf("paired matrix dominance PASS entries=%zu\n",
                proof.strict_entries);
    return 0;
}
